/** \file
 * \brief Implementation of KandinskyLayout.
 *
 * \author Moritz Schallaböck
 *
 * \par License:
 * This file is part of the Open Graph Drawing Framework (OGDF).
 * Copyright (C) 2005-2009
 *
 * \par
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 or 3 as published by the Free Software Foundation
 * and appearing in the files LICENSE_GPL_v2.txt and
 * LICENSE_GPL_v3.txt included in the packaging of this file.
 *
 * \par
 * In addition, as a special exception, you have permission to link
 * this software with the libraries of the COIN-OR Osi project
 * (http://www.coin-or.org/projects/Osi.xml), all libraries required
 * by Osi, and all LP-solver libraries directly supported by the
 * COIN-OR Osi project, and distribute executables, as long as
 * you follow the requirements of the GNU General Public License
 * in regard to all of the software in the executable aside from these
 * third-party libraries.
 *
 * \par
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * \par
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 * \see  http://www.gnu.org/copyleft/gpl.html
 ***************************************************************/

#ifdef COIN_OSI_SYM
#include <ogdf/orthogonal/KandinskyLayout.h>

#include <ogdf/basic/basic.h>
#include <ogdf/basic/List.h>
#include <ogdf/basic/Graph_d.h>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/basic/GraphCopy.h>
#include <ogdf/basic/CombinatorialEmbedding.h>

#include <ogdf/planarity/PlanRep.h>

#include <ogdf/orthogonal/OrthoRep.h>
#include <ogdf/orthogonal/FlowCompaction.h>
#include <ogdf/orthogonal/EdgeRouter.h>

#include <ogdf/external/coin.h>

#include <OsiSymSolverParameters.hpp>
#include <OsiSymSolverInterface.hpp>


namespace ogdf
  {


    // Einfache Debug-Methode die den komplettten Graphen ausgibt
    void dumpGraph(const Graph& G)
  {
#ifdef OGDF_DEBUG
    cout << "graph dump: " << (&G) << " num nodes:" << G.numberOfNodes() << ", num edges=" << G.numberOfEdges() << ", genus=" << G.genus() << " (->" << G.consistencyCheck() << ")" << endl;

    node v;
    adjEntry ae;

    forall_nodes(v, G)
    {
      forall_adj(ae, v)
      {
        cout << "node " << ae->theNode()->index() << "\tae " <<  ae->index() << " via " << ae->theEdge()->index() << " to node:" << ae->twinNode()->index() << endl;
      }
    }
    cout << endl;
#endif
  }

  KandinskyLayout::KandinskyLayout()
  {
    opt1 = true;
    alwaysCenterStraight = true;
    sep = 20;
  }

  void KandinskyLayout::call(PlanRepUML &PG, adjEntry adjExternal, Layout &drawing)
  {
    if (PG.numberOfEdges() == 0 || PG.numberOfNodes() == 0)
      {
        return;
      }

    node n;
    edge e;
    adjEntry ae;

    // 2: PlanRep -> Embedding
    CombinatorialEmbedding E(PG);
    E.setExternalFace(E.rightFace(adjExternal));

    OGDF_ASSERT(E.consistencyCheck());

    //  3: Embedding -> IP
    //OsiSymSolverInterface * osi = dynamic_cast<OsiSymSolverInterface *> (getIP(E));
    OsiSolverInterface * osi = getIP(E);
#ifdef OGDF_DEBUG
#endif

#ifndef OGDF_DEBUG
    //osi->setSymParam(OsiSymVerbosity, -2); // -2: keine Ausgaben (ausser bei Fehlern?)
#endif

#ifdef COIN_OSI_SYM
    OsiSymSolverInterface * osisym = dynamic_cast<OsiSymSolverInterface *>(osi);
    osisym->setSymParam(OsiSymVerbosity, -11); // 0, 1, 2, ...?: viele Ausgaben
#endif

    osi->branchAndBound();

    if (!osi->isProvenOptimal())
      return;

    const double * sol = osi->getColSolution();

#ifdef OGDF_DEBUG
    // Loesung ausgeben
    for (int j = 0; j < osi->getNumCols(); j++)
      {
        printf("x%i: %.0f ", j, sol[j]);
      }
#endif

    //  4: Graph, Embedding, IP ->  Graph', Embedding', Angles, Bends
    String error;

    Graph intermediate;
    CombinatorialEmbedding intermediateEmb(intermediate);

    AdjEntryArray<int> angles;
    AdjEntryArray<BendString> bends;
    NodeArray<node> nodeMapping(PG, 0); // von PG nach intermediate
    NodeArray<node> nodeMappingBack(intermediate, 0); // von intermediate nach PG
    EdgeArray<edge> edgeMapping(PG, 0);
    EdgeArray<edge> edgeMappingBack(intermediate, 0);
    AdjEntryArray<double> aeOffsets(PG, -1);
    AdjEntryArray<int> aeNumNeighbours(PG, 0);

    createExtension(E, sol, osi->getNumCols() / 8,
                    intermediate, intermediateEmb, angles, bends,
                    nodeMapping, nodeMappingBack, edgeMapping, edgeMappingBack, aeOffsets, aeNumNeighbours);

    // postprocessing um ergebnisgraphen zu vereinfachen
    // pfade von knoten mit grad 2 mit knotenwinkel 2 entfernen
    forall_nodes(n, intermediate)
    {
      // nur nodes entfernen die nicht im ursprungsgraphen sind
      bool remove = nodeMappingBack[n] == 0; //((node *)0);
      if (n->degree() == 2 && remove)
        {
          adjEntry ae1 = n->firstAdj();
          edge e1 = ae1->theEdge();
          adjEntry ae2 = ae1->succ();
          edge e2 = ae2->theEdge();
          if (angles[ae1] == 2 && angles[ae2] == 2)
            {
              BendString bs, bs1, bs2;
#ifdef OGDF_DEBUG
              cout << "node " << n->index() << "   ae1: " << angles[ae1] << " " << angles[ae1] << ", ae2: " << angles[ae2] << " " << angles[ae2];
#endif

              bs1 = bends[ae1];
              if (e1->source() == n)
                intermediate.reverseEdge(e1);

              bs2 = bends[ae2];
              if (e2->source() != n)
                intermediate.reverseEdge(e2);

              // jetzt gilt: --e1-->  (v)  --e2-->

              bs = reverseBendString(bs1);
              bs += bs2;

#ifdef OGDF_DEBUG
              if (bs.size() > 0)
                cout << " >" << bs << "< ";
              cout << "\te1 " << e1 << " e2 " << e2 << endl;
#endif

              bends[e1->adjSource()] = bs;
              bends[e2->adjTarget()] = reverseBendString(bs);

              // wenn edgerefs[X] == e2, setze edgerefs[X] = e1
              if (edgeMappingBack[e2])
                {
                  edgeMapping[edgeMappingBack[e2]] = e1;
                }

              intermediate.unsplit(e1, e2); // entfernt e2 und v
              n = intermediate.firstNode();
            }
        }
    }

    GraphAttributes intermediateAttr(intermediate);
    forall_nodes(n, intermediate) {
        node back;
        if ((back=nodeMappingBack[n]) != 0) {
            intermediateAttr.width(n) = PG.widthOrig(back);
            intermediateAttr.height(n) = PG.heightOrig(back);
        } else {
            intermediateAttr.width(n) = 5;
            intermediateAttr.height(n) = 5;

        }
    }
    intermediateAttr.width().fill(20); //== TODO ursprüngliche Größe setzen; 5x5 fuer pseudo-Knoten?
    intermediateAttr.height().fill(20);

    // PlanRep kann erst hier initialisiert werden, nachdem H aufgebaut ist
    PlanRep extension(intermediateAttr);
    extension.initCC(0);

    if (!extension.consistencyCheck())
      return;

    // OrthoRep für Extension aus Angles und Bends erzeugen
    CombinatorialEmbedding extensionEmb(extension);
    OrthoRep extensionOR(extensionEmb);
    adjEntry adjExternal2;

    forall_nodes(n, extension)
    {
      forall_adj(ae, n)
      {
        edge orig = extension.original(ae->theEdge());
        adjEntry origae = (ae->theEdge()->adjSource() == ae) ? orig->adjSource() : orig->adjTarget();
        extensionOR.angle(ae) = angles[origae];
        extensionOR.bend(ae) = bends[origae];
        if (intermediateEmb.leftFace(origae) == intermediateEmb.externalFace())
          adjExternal2 = ae;
      }
    }
    extensionEmb.computeFaces();
    extensionEmb.setExternalFace(extensionEmb.leftFace(adjExternal2));

#ifdef OGDF_DEBUG
    cout << "Extension OR: " << extensionOR << endl;
    OGDF_ASSERT(extensionOR.check(error));
#endif


    //  5: Graph'..., OrthoRep' -> Kompaktierung/Drawing
    Layout extensionDrawing(extension);
#ifdef OGDF_DEBUG
    double t = 0;
    t = usedTime(t);
#endif

    draw(extension, extensionOR, extensionEmb, adjExternal2, extensionDrawing);


#ifdef OGDF_DEBUG
    cout << " used time = " << usedTime(t) << endl;
    extension.writeGML("layout1.gml");
#endif

    forall_nodes(n, PG)
    {
      // references: node_G -> node_V
      node mappedNode = extension.copy(nodeMapping[n]);
      drawing.x(n) = extensionDrawing.x(mappedNode);
      drawing.y(n) = extensionDrawing.y(mappedNode);
    }

    forall_edges(e, PG)
    {
      double offsetS = -aeOffsets[e->adjSource()] / (aeNumNeighbours[e->adjSource()] * 2) * 0.9;
      double offsetT = -aeOffsets[e->adjTarget()] / (aeNumNeighbours[e->adjTarget()] * 2) * 0.9;

      edge mappedEdge = edgeMapping[e];
      node msS = mappedEdge->source();
      node msT = mappedEdge->target();

#ifdef OGDF_DEBUG
      cout << "original edge " << e << " mapped to " << edgeMapping[e] << "  " << endl;
      cout << "offsets: source " << offsetS << ", target " << offsetT << endl;
      cout << "\texpanded edge: " << mappedEdge << endl;

      cout << "\texp node: " << msS->index() << " mapped to " << extension.copy(msS)->index() << endl;
      cout << "\texp node: " << msT->index() << " mapped to " << extension.copy(msT)->index() << endl;
#endif

      DPoint p1 = DPoint(extensionDrawing.x(extension.copy(msS)), extensionDrawing.y(extension.copy(msS)));
      DPoint p2 = DPoint(extensionDrawing.x(extension.copy(msT)), extensionDrawing.y(extension.copy(msT)));

      DPolyline line;
      extensionDrawing.computePolyline(extension, mappedEdge, line);
      if (e->source()->index() < e->target()->index())
        {
          line.pushFront(p1);
          line.pushBack(p2);
        }
      else
        {
          line.reverse();
          line.pushFront(p2);
          line.pushBack(p1);
        }
      if (line.front().m_x != drawing.x(e->source()) || line.front().m_y != drawing.y(e->source()))
        line.pushFront(DPoint(drawing.x(e->source()), drawing.y(e->source())));
      if (line.back().m_x != drawing.x(e->target()) || line.back().m_y != drawing.y(e->target()))
        line.pushBack(DPoint(drawing.x(e->target()), drawing.y(e->target())));

      DPoint lp1 = *line.get(0);
      DPoint lp2 = *line.get(1);
      DPoint lpV = lp2 - lp1;

      int linedirS;
      if (lpV.m_y > 0)
        linedirS = 2;
      if (lpV.m_x > 0)
        linedirS = 1;
      if (lpV.m_y < 0)
        linedirS = 0;
      if (lpV.m_x < 0)
        linedirS = 3;

#ifdef OGDF_DEBUG
      cout << " edge direction (S): " << lp2 << " - " << lp1 << " = " << lpV;
      cout << "    -> " << linedirS << endl;
#endif

      /////////
      //double innerGridS = PG.widthOrig(e->source()) / (aeNumNeighbours[e->adjSource()] * 2);
      //double innerGridT = PG.widthOrig(e->target()) / (aeNumNeighbours[e->adjTarget()] * 2);

      DPoint pOffset(0, 0);
      if (linedirS == 0) // N
        pOffset.m_x = offsetS * PG.widthOrig(e->source());
      else if (linedirS == 1) // E
        pOffset.m_y = offsetS * PG.heightOrig(e->source());
      else if (linedirS == 2) // S
        pOffset.m_x = -offsetS * PG.widthOrig(e->source());
      else if (linedirS == 3) // W
        pOffset.m_y = -offsetS * PG.heightOrig(e->source());


      DPoint lp4 = *line.get(line.size() - 2);
      DPoint lp5 = *line.get(line.size() - 1);
      DPoint lpV2 = lp4 - lp5;

      int linedirT = -1;
      if (lpV2.m_y > 0)
        linedirT = 2;
      if (lpV2.m_x > 0)
        linedirT = 1;
      if (lpV2.m_y < 0)
        linedirT = 0;
      if (lpV2.m_x < 0)
        linedirT = 3;

#ifdef OGDF_DEBUG
      cout << " edge direction (T): " << lp4 << " - " << lp5 << " = " << lpV2;
      cout << "    -> " << linedirT << endl;
#endif

      DPoint pOffsetT(0, 0);
      if (linedirT == 0)
        pOffsetT.m_x = offsetT * PG.widthOrig(e->target());
      else if (linedirT == 1)
        pOffsetT.m_y = offsetT * PG.heightOrig(e->target());
      else if (linedirT == 2)
        pOffsetT.m_x = -offsetT * PG.widthOrig(e->target());
      else if (linedirT == 3)
        pOffsetT.m_y = -offsetT * PG.heightOrig(e->target());

      // TODO Refactoring: die zwei durchlaeufe zusammenfassen
      ListIterator<DPoint> iter = line.begin();
      DPoint bpV;
      do   // walk forward along the line until hitting a bend, moving points by pOffset
        {
          DPoint * bp1 = &(*iter++);
          DPoint * bp2 = iter.valid() ? &(*iter) : bp1;

          bpV = *bp2 - *bp1 + pOffset;
          (*bp1).m_x += pOffset.m_x;
          (*bp1).m_y += pOffset.m_y;
        }
      while (iter.valid() && bpV.m_x != 0 && bpV.m_y != 0);

      iter = line.rbegin();
      do   // walk backward along the line until hitting a bend, moving points by pOffsetT
        {
          DPoint * bp1 = &(*iter--);
          DPoint * bp2 = iter.valid() ? &(*iter) : bp1;

          bpV = *bp2 - *bp1 + pOffsetT;
          (*bp1).m_x += pOffsetT.m_x;
          (*bp1).m_y += pOffsetT.m_y;
        }
      while (iter.valid() && bpV.m_x != 0 && bpV.m_y != 0);

      drawing.bends(e) = line;
#ifdef OGDF_DEBUG
      cout << endl << " resulting line of " << line.size() << " segments " << endl << "   " << line << endl << endl;
#endif
    }

#ifdef OGDF_DEBUG
    forall_edges(e, PG)
    {
      cout << "line " << e << ": " << drawing.bends(e) << endl;
    }
    cout << endl << "no of bends: " << osi->getObjValue() << endl;
#endif

    return;
  }


// createExtension wandelt die Einbettung emb des Graphen G um in: die Extension H mit Einbettung embH und Repraesentation OR
  void KandinskyLayout::createExtension(CombinatorialEmbedding &emb, const double * sol, const int ncols,
                                        Graph &H, CombinatorialEmbedding &embH, AdjEntryArray<int> &angles, AdjEntryArray<BendString> &bends,
                                        NodeArray<node> &nodeRefs, NodeArray<node> &nodeRefsBack, EdgeArray<edge> &edgeRefs, EdgeArray<edge> &edgeRefsBack,
                                            AdjEntryArray<double> &aeOffsets, AdjEntryArray<int> &aeNumNeighbours)
  {

    AdjEntryArray<node> benddata(emb.getGraph(), 0);
    angles.init(H, 0);
    bends.init(H, BendString());

    node n;

    adjEntry leftExtAe = 0;
    adjEntry origae;

    forall_nodes(n, emb.getGraph())
    {
      node m = H.newNode();

      nodeRefs[n] = m;
      nodeRefsBack[m] = n;

      // korrekten start-adjEntry finden
      adjEntry startae;

      origae = n->firstAdj();
      int angle = getAngleFor(origae, sol, ncols);
      while (angle == 0)   // irgendwann muss ein angle > 0 kommen (summe der winkel = 4)
        {
          origae = origae->cyclicSucc();
          angle = getAngleFor(origae, sol, ncols);
        }
      origae = startae = origae->cyclicSucc();

      adjEntry ae;
      int i = 0;

#ifdef OGDF_DEBUG
      cout << "mapping node " << n->index() << " --> " << m->index() << "\tdegree " << n->degree() << " starting at " << startae->index() << endl;


      forall_adj(ae, n)
      {
        printf("%d. ae %02d \t edge %02d (%02d->%02d)\t angle %d, bend %d, edgebends %s\n", ++i, ae->index(), ae->theEdge()->index(), ae->theNode()->index(), ae->twinNode()->index(), getAngleFor(ae, sol, ncols), getBendFor(ae, sol, ncols), getBendString(ae, sol, ncols).toString());
      }
#endif

      adjEntry historicAdj, insertAdj;
      do
        {
          List<adjEntry> left, right, straight;

          // kanten dieser seite sammeln
          int numNeighbours = 0;
          int angle; 
          do
            {
              int bend = getBendFor(origae, sol, ncols);

              if (bend == -1)
                left.pushBack(origae);
              else if (bend == 1)
                right.pushBack(origae);
              else if (bend == 0)
                {
                  if (!straight.empty())
                    throw Exception();
                  straight.pushBack(origae);
                }

              angle = getAngleFor(origae, sol, ncols);
              origae = origae->cyclicSucc();
              ++numNeighbours;
            }
          while (angle == 0);

          // jetzt gilt:  ...(angle>0) - aes in left - straight ae - aes in right - (angle>0) und weitere aes...
          // neuen graph zusammensetzen
          //    abwechselnd left und right, zuletzt straight behandeln

#ifdef OGDF_DEBUG
          cout << "origae " << origae->index() << endl;
          cout << "now handling aes: (" << left.size() << "," << (straight.size()) << "," << right.size() << ")" << endl;
#endif

          int i = 0;
          node prednode = m;

          double initialOffset = ((left.size() - right.size()) / 2.0);
          while (!left.empty() || !right.empty() || !straight.empty())
            {

              double offset = alwaysCenterStraight ? 0 : initialOffset;
              adjEntry cur;

              if (right.empty() && left.empty())
                {
                  cur = straight.popFrontRet();
                  offset += 0;
                }
              else if (left.empty())
                {
                  offset += right.size() - (straight.empty() ? 1 : 0); // sonderfall: abbiegung rechts, aber kein gradeaus, dann soll rechts zentriert gezeichnet werden
                  cur = right.popBackRet();
                }
              else if (right.empty())
                {
                  offset += left.size() * -1 + (straight.empty() ? 1 : 0);;
                  cur = left.popFrontRet();
                }
              else
                {
                  offset += (i % 2 == 0 && !right.empty()) ?
                            left.size() * -1 + (straight.empty() ? 1 : 0)
                          : right.size() - (straight.empty() ? 1 : 0);
                  cur = (i % 2 == 0 && !right.empty()) ?
                        left.popFrontRet() : right.popBackRet();
                  i++;
                }

              // offset ist jetzt: ...-1 wenn links, 0 wenn center, +1... wenn rechts
              aeOffsets[cur] = offset;
              aeNumNeighbours[cur] = numNeighbours;

              // letzter durchlauf?
              bool finished = left.empty() && right.empty() && straight.empty();
              int benddir = getBendFor(cur, sol, ncols);

#ifdef OGDF_DEBUG
              cout << "  -- handling (AE" << cur->index() << ": " << cur->theNode()->index() << "->" << cur->twinNode()->index() << " AE" << cur->twin()->index() << ")  benddir: " << benddir << endl;
              cout << "  !! offset " << offset << endl;
              cout << "  -- remaining bends (" << left.size() << "," << (straight.size()) << "," << right.size() << ")" << endl;
#endif

              bool leftexternal = (emb.leftFace(cur) == emb.externalFace());

              // cur behandeln
              // wurde cur bereits (bei behandlung von nachbarn der kante) eingefuegt?
              node curnode = benddata[cur];

              if (curnode == 0)
                {
                  // nein. also neu einfuegen!
                  // bend-knoten curnode fuer ae erzeugen
                  // und in benddata einfuegen
                  //                    if (benddir == 0) // neu: wenn benddir=0 keine bendnode erstellen
                  //                        curnode = cur;
                  //                    else
                  curnode = H.newNode();
                  benddata[cur] = curnode;

#ifdef OGDF_DEBUG
                  cout << "\t bend node " << curnode->index() << " for ae " << cur->index() << endl;
#endif

                  // Quelle
                  // bei left: adjEntry bei >curnode< -- twinnode (in vorrunde kante f)
                  // bei right: adjEntry bei m -- >curnode< (in vorrunde kante e)
                  // bei erster Runde: wenn degree(m) == 0: m, sonst: adjEntry an m den man sich gemerkt haben muss (historicAdj)

                  // Ziel bei neuer curnode: curnode
                  // bei bestehender curnode: auch...?? egal da deg == 1

                  // bend-knoten twinnode fuer ae-twin erzeugen
                  // und in benddata einfuegen

                  node twinnode = H.newNode();
                  benddata[cur->twin()] = twinnode;

#ifdef OGDF_DEBUG
                  cout << "\t twin node " << twinnode->index() << " for ae " << cur->twin()->index() << endl;
#endif

                  // kante e von "m" nach curnode // achtung!!
                  edge e;
                  if (prednode == m && m->degree() == 0)
                    {
                      e = H.newEdge(prednode, curnode);
                      historicAdj = e->adjSource(); // fuer (viel) spaeter merken
                    }
                  else if (prednode == m && m->degree() > 0)
                    {
                      e = H.newEdge(historicAdj, curnode);
                      historicAdj = e->adjSource(); // fuer (viel) spaeter merken
                    }
                  else
                    {
#ifdef OGDF_DEBUG
                      if (insertAdj == 0 || insertAdj->theNode() != prednode)
                        cout << "insertAdj node is incorrect: " << insertAdj << ", prednode: " << prednode->index() << endl;
#endif
                      e = H.newEdge(insertAdj, curnode);
                    }

                  // winkel von e bei "m"
                  adjEntry angle1 = e->adjSource(); // ist m source? wenn ja, dann angle1 inlinen
                  // falls "m" == m (also curnode erster eingefuegter bend-knoten) ist winkel der beim kantensammeln gemerkte angle!
                  // sonst haengt der winkel davon ab, ob vorher ein links oder rechtsbend eingefuegt wurde --> dann merken
                  angles[angle1] = angle;

#ifdef OGDF_DEBUG
                  cout << "\t angle at node " << angle1->theNode()->index() << " ae " << angle1->index() << " set to " << angles[angle1] << endl;
#endif

                  // winkel von e bei curnode: leftbend => 1, rightbend => 2 oder 3 falls kein straighft/nachfolger, straight == ??
                  if (benddir == -1)
                    angles[e->adjTarget()] = 1;
                  else if (benddir == 1)
                    angles[e->adjTarget()] = finished ? 3 : 2;
                  else
                    angles[e->adjTarget()] = 2; // ??

#ifdef OGDF_DEBUG
                  cout << "\t angle at node " << e->adjTarget()->theNode()->index() << " ae " << e->adjTarget()->index() << " set to " << angles[e->adjTarget()] << endl;
#endif

                  // kante f von curnode nach twinnode
                  OGDF_ASSERT(curnode->degree() == 1);
                  OGDF_ASSERT(twinnode->degree() == 0);
                  edge f = H.newEdge(curnode, twinnode);

                  edgeRefs[cur->theEdge()] = f;
                  edgeRefsBack[f] = cur->theEdge();
#ifdef OGDF_DEBUG
                  cout << "!!! mapped " << cur->theEdge() << "  to  " << f << endl;
#endif

                  if (leftexternal && !leftExtAe)
                    leftExtAe = f->adjSource();

                  // bends AUF der kante
                  bends[f->adjSource()] = getBendString(cur, sol, ncols);

                  // winkel von f bei curnode: rightbend => 1, leftbend => 1 oder 3 falls kein nachfolger, straight == ??
                  if (benddir == -1)
                    angles[f->adjSource()] = finished ? 3 : 1;
                  else if (benddir == 1)
                    angles[f->adjSource()] = 1;
                  else
                    angles[f->adjSource()] = 2; // ??

#ifdef OGDF_DEBUG
                  cout << "\t angle at node " << f->adjSource()->theNode()->index() << " ae " << f->adjSource()->index() << " set to " << angles[f->adjSource()] << endl;
#endif

                  // insertNode fuer spaeter setzen
                  if (benddir == -1)
                    insertAdj = f->adjSource();
                  else if (benddir == 1)
                    insertAdj = e->adjTarget();
                  else
                    insertAdj = 0;
                  // bei straight spielt insertAdj keine Rolle mehr

                  // winkel von f bei twinnode: wird bei berechnen des twinknoten gesetzt
                }
              else
                {
                  // curnode (und twinnode) existieren schon
                  //   --> winkel an twinnode sind schon gesetzt...
                  // dieser fall ist sehr ähnlich zum obigen, aber doch unterschiedlich genug,
                  // dass das zusammenfassen schwer ist
                  OGDF_ASSERT(curnode->degree() == 1); // curnode: eine kante, naemlich die von twinnode
                  OGDF_ASSERT(benddata[cur->twin()] != 0); // twinnode existiert
                  node twinnode = benddata[cur->twin()];

#ifdef OGDF_DEBUG
                  cout << "\t node already created" << endl;
                  cout << "\t bend node " << curnode->index() << " for ae " << cur->index() << endl;
                  cout << "\t twin node " << twinnode->index() << " for ae " << cur->twin()->index() << endl;
#endif
                  // achtung: kanten so einfuegen, dass bestehende kante zw curnode und twinnode links bzw rechts liegt

                  // kante e von "m" nach curnode einfuegen
                  edge e;
                  if (prednode == m && m->degree() == 0)
                    {
                      e = H.newEdge(prednode, curnode);
                      historicAdj = e->adjSource(); // fuer (viel) spaeter merken
                    }
                  else if (prednode == m && m->degree() > 0)
                    {
                      OGDF_ASSERT(curnode->lastAdj() == curnode->firstAdj());
                      e = H.newEdge(historicAdj, curnode->firstAdj());
                      historicAdj = e->adjSource(); // fuer (viel) spaeter merken
                    }
                  else
                    {
#ifdef OGDF_DEBUG
                      if (insertAdj == 0 || insertAdj->theNode() != prednode)
                        cout << "insertAdj node is incorrect: " << insertAdj << ", prednode: " << prednode->index() << endl;
#endif
                      e = H.newEdge(insertAdj, curnode);
                    }

                  // winkel von e bei "m" setzen
                  angles[e->adjSource()] = angle;
#ifdef OGDF_DEBUG
                  cout << "\t angle at node " << e->adjSource()->theNode()->index() << " ae " << e->adjSource()->index() << " set to " << angles[e->adjSource()] << endl;
#endif
                  // winkel von e bei curnode setzen
                  if (benddir == -1)
                    angles[e->adjTarget()] = 1;
                  else if (benddir == 1)
                    angles[e->adjTarget()] = finished ? 3 : 2;
                  else
                    angles[e->adjTarget()] = 2;

#ifdef OGDF_DEBUG
                  cout << "\t angle at node " << e->adjTarget()->theNode()->index() << " ae " << e->adjTarget()->index() << " set to " << angles[e->adjTarget()] << endl;
#endif

                  // kante f von curnode nach twinnode -- existiert schon -- finden:
                  edge f = e->adjTarget()->cyclicSucc()->theEdge();

                  if (leftexternal && !leftExtAe)
                    leftExtAe = f->adjSource(); //target?

                  // achtung: die kante sollte VON twinnode NACH curnode gehen
                  OGDF_ASSERT(f->source() == twinnode);

                  // bends auf der kante -- bereits gesetzt (rueckrichtung?)
                  bends[f->adjTarget()] = getBendString(cur, sol, ncols);
                  // winkel von f bei curnode setzen
                  // winkel von e bei curnode setzen
                  if (benddir == -1)
                    angles[f->adjTarget()] = finished ? 3 : 1;
                  else if (benddir == 1)
                    angles[f->adjTarget()] = 1;
                  else
                    angles[f->adjTarget()] = 2;

#ifdef OGDF_DEBUG
                  cout << "\t angle at node " << f->adjTarget()->theNode()->index() << " ae " << f->adjTarget()->index() << " set to " << angles[f->adjTarget()] << endl;
#endif

                  // winkel von f bei twinnode -- bereits gesetzt

                  // insertNode fuer spaeter setzen
                  if (benddir == -1)
                    insertAdj = f->adjTarget();
                  else if (benddir == 1)
                    insertAdj = e->adjTarget();
                  else
                    insertAdj = 0;
                  // bei straight spielt insertAdj keine Rolle mehr
                }

#ifdef OGDF_DEBUG
              if (insertAdj && insertAdj->theNode())
                cout << "  iii insertAdj is now " << insertAdj << " (ae " << insertAdj->index() << ", n " << insertAdj->theNode() << ")" << endl;
              else
                cout << "  iii insertAdj null" << endl;
#endif

              // --> jetzt angle fuer naechste iteration merken
              angle = (benddir == -1) ? 2 : 1;
              prednode = curnode;

            } // while !finished

        }
      while (origae != startae);

    } // forall_nodes

    embH.init(H);
    embH.computeFaces();


    if (leftExtAe != 0) //== das sollte eigentlich NIE null sein
      embH.setExternalFace(embH.leftFace(leftExtAe));

#ifdef OGDF_DEBUG
    for (face ff = embH.firstFace(); (ff); ff = ff->succ())
      {
        int sum = 0;
        cout << "face #" << ff->index() << ": ";
        for (adjEntry fae = ff->firstAdj(); (fae); fae = ff->nextFaceEdge(fae))
          {
            //cout << "[" << fae->theNode() << "] " << fae->index() << " (e" << fae->theEdge()->index() << ") " << fae->twin()->index() << " ";
            cout << "[" << fae->theNode() << "] +" << angles[fae] << " " << bends[fae] << "";
            sum += angles[fae];
          }
        cout << " sum: " << sum << endl << endl;
      }

    node v;
    forall_nodes(v, H)
    {
      cout << v->index() << " (deg " << v->degree() << "): ";
      adjEntry faae;

      forall_adj(faae, v)
      {
        cout << "\t -> " << faae->twinNode()->index() << "";
      }

      cout << endl;
    }
#endif
  }

  // gibt einen gegebenen BendString in umgekehrter Reihenfolge zurück
  // returns a given BendString in reverse order
  BendString KandinskyLayout::reverseBendString(const BendString & bs)
  {
    BendString rev;
    for (int i = 0; i < bs.size(); ++i)   //== warum nicht einfacht rueckwaerts durchgehen
      {
        rev += (bs[i] == '1' ? "0" : "1");
      }
    return rev;
  }

  // fügt in einem CoinPackedVector einen index ein bzw verändert ihn falls er schon existiert
  // inserts a new index into a CoinPackedVector or adjusts an existing one
  void KandinskyLayout::adjustPackedVector(CoinPackedVector &vector, const int index, const double adjust)
  {
    if (vector.isExistingIndex(index))
      vector.getElements()[vector.findIndex(index)] += adjust;
    else
      vector.insert(index, adjust);
  }

  OsiSolverInterface * KandinskyLayout::getIP(CombinatorialEmbedding &emb)
  {
#ifdef OGDF_DEBUG
    dumpGraph(emb.getGraph());
#endif

    ///////////////////////////
    // Variablen
    /*
     * Variable für Fluss von face f zu face f' (Kosten 1)
     *
     * f und f' werden durch Kante e getrennt, identifiziere Fluß mit e
     *  (Hin und Rückrichtung)
     * x0     x1    x2    x3    x4    ...
     * x_e0A  x_e0B x_e1A x_e1B x_e2A ...
     * bzw x_e0Hin, x_e0Rück
     * insgesamt 2E
     */
    // Variablen für Fluss von node v in Winkel an v (Kosten 0)
    // identifiziere Winkel zwischen (v,w) und (v,u) mit Kante (v,w)
    // .... x0+2E    x1+2E   x2+2E   x3+2E    ...
    // .... x_e0Hin x_e0Rück x_e1Hin x_e1Rück ...
    // insgesamt 2E
    //
    // Kantenknickvariablen (Kosten 1)
    // .... x0+4E     x1+4E     x2+4E      x3+4E        x4+4E ...
    // .... x_e0HinLi x_e0HinRe x_e0RückLi x_e0RückRe   x_e1HinLi...
    // insgesamt 4E
    ////////////////////////////////

    /////////
    // Bedarf an Face f
    // Eine Zeile pro Face
    // Summe eingehende Kanten = P(f) +/- 4
    // ==
    //     Für alle inzidenten Knoten
    // Summe von Kanten inzidenter Knoten
    //
    //     Für alle inzidenten Graph-Kanten
    // + Summe der Kanten von Face auf anderer Seite der Graph-Kante
    //
    //     Für alle inzidenten Knoten?
    // + Summe der Kantenknicke

    //const int E = emb.getGraph().numberOfEdges();
    const int E = emb.getGraph().maxEdgeIndex() + 1;

    // hässlich aber notwendig um spaeter setInteger machen zu koennen
    //OsiSymSolverInterface * osi = dynamic_cast<OsiSymSolverInterface *> (CoinManager::createCorrectOsiSolverInterface());
    OsiSolverInterface * osi = CoinManager::createCorrectOsiSolverInterface();

    Array<double> lower(8 * E), upper(8 * E);

    CoinPackedMatrix matrix(false, 0, 0);
    matrix.setDimensions(0, 8 * E);

    int index = 0;

    face f = emb.firstFace();
    while (f != 0)
      {
        adjEntry ae = f->firstAdj();

        CoinPackedVector vec;

        int count = 0;
        while (ae != 0)
          {
            count++;
            node curNode = ae->theNode();

#ifdef OGDF_DEBUG
            cout << "[n" << ae->theNode() << "] e" << ae->theEdge()->index() << " ";
#endif

            // Fluß von Nachbarface hinter ae->theEdge()
            int neighborindex = ae->theEdge()->index() * 2 + 0;
            int fac;
            if (curNode->index() < ae->twinNode()->index())
              fac = -1;
            else
              fac = 1;

            adjustPackedVector(vec, neighborindex, fac);
            adjustPackedVector(vec, neighborindex + 1, -fac);

            // Fluß durch den Winkel an ae->theNode()
            int nodeindex = ae->theEdge()->index() * 2 + 2 * E;
            if (curNode->index() > ae->twinNode()->index())
              nodeindex++;

            try
              {
                vec.insert(nodeindex, 1);
              }
            catch (CoinError ce)
              {
#ifdef OGDF_DEBUG
                cout << ce.message() << endl;
#endif
                throw ce;
              }

            // Fluß wegen Kantenknicken
            int bendindex = ae->theEdge()->index() * 4 + 4 * E;
            if (curNode->index() < ae->twinNode()->index())
              fac = 1;
            else
              fac = -1;

            adjustPackedVector(vec, bendindex, fac); // hin links
            adjustPackedVector(vec, bendindex + 1, -fac); // hin rechts
            adjustPackedVector(vec, bendindex + 2, -fac); // rück links
            adjustPackedVector(vec, bendindex + 3, fac); // rück rechts

            ae = f->nextFaceEdge(ae);
          }
#ifdef OGDF_DEBUG
        cout << "- face " << f->index() << ": " << count << " adj edges" << "\n";
#endif

        int right = 2 * f->size() + (4 * (f->index() == emb.externalFace()->index() ? 1 : -1));

        matrix.appendRow(vec);
        lower[index] = right;
        upper[index] = right;
        ++index;

        f = f->succ();
      }

    ////////
    // Produktion an Knoten
    // Eine Zeile pro node
    // Summe aller "Winkel" am Knoten = 4
    node n = emb.getGraph().firstNode();
    do
      {
        CoinPackedVector vec;

        List<adjEntry> adjEntries;
        emb.getGraph().adjEntries(n, adjEntries);

        ListIterator<adjEntry> iter = adjEntries.begin();
        while (iter.valid())
          {
            const adjEntry ae = *iter;

            edge e = ae->theEdge();
            int aindex = e->index()*2;

            if (n->index() > ae->twinNode()->index())
              aindex++;

            vec.insert(aindex + 2 * E, 1);
            iter++;
          }

        matrix.appendRow(vec);
        lower[index] = 4;
        upper[index] = 4;
        ++index;
      }
    while ((n = n->succ()) != 0);
    // Produktion an Knoten
    // Eine Zeile pro inzidenter Kante
    // An jeder Kante mindestens Winkel 1, wenn Knotengrad <= 4, sonst min. Winkel 0
    if (opt1)
      {
        n = emb.getGraph().firstNode();
        do
          {
            List<adjEntry> adjEntries;
            emb.getGraph().adjEntries(n, adjEntries);
            int degree = adjEntries.size();

            ListIterator<adjEntry> iter = adjEntries.begin();
            while (iter.valid())
              {
                CoinPackedVector vec;
                const adjEntry ae = *iter;

                edge e = ae->theEdge();
                int aindex = e->index()*2;

                if (n->index() > ae->twinNode()->index())
                  aindex++;

                vec.insert(aindex + 2 * E, 1);
                matrix.appendRow(vec);
                lower[index] = degree <= 4 ? 1 : 0; // Winkel 1, wenn Knotengrad klein genug
                upper[index] = 4; //degree > 4 ? 1 : 4;
                ++index;
                iter++;
              }
          }
        while ((n = n->succ()) != 0);
      }

    ////////
    // Für alle Knoten v, für alle inzidenten Kanten (v,w)
    // entweder links oder rechts knicken
    // (v,w)Li + (v,w)Re <= 1
    n = emb.getGraph().firstNode();
    while (n != 0)
      {
        adjEntry ae = n->firstAdj();
        while (ae != 0)
          {
            CoinPackedVector vec;

            edge e = ae->theEdge();
            int arrayindex = ae->theEdge()->index() * 4 + (4 * E);

            if (n->index() > ae->twinNode()->index())
              arrayindex += 2;

            vec.insert(arrayindex, 1);
            vec.insert(arrayindex + 1, 1);
            matrix.appendRow(vec);

            lower[index] = -osi->getInfinity();
            upper[index] = 1;
            ++index;

            ae = ae->succ();
          }
        n = n->succ();
      }

    /////////
    // Für alle Knoten v, für alle inzidente Kanten (v,w) mit Nachfolger (v, w') und Face f zwischen (v,w) und (v,w')
    // face-angle + (v,w)Li + (v,w')Re >= 1
    n = emb.getGraph().firstNode();
    while (n != 0)
      {
#ifdef OGDF_DEBUG
        cout << "bend constraints for node " << n << endl;
#endif
        adjEntry ae = n->firstAdj();
        while (ae != 0)
          {
            CoinPackedVector vec;

            adjEntry succ = ae->cyclicSucc();

            edge e = ae->theEdge();

            // face-angle wurde mit "linker" kante des angles identifiziert
            int faceangleindex = 2 * E + 2 * e->index();
            if (n->index() > ae->twinNode()->index())
              faceangleindex++;

            int leftedgeindex = 4 * E + 4 * e->index();
            if (n->index() > ae->twinNode()->index())
              leftedgeindex += 2;

            int rightedgeindex = 4 * E + 4 * succ->theEdge()->index() + 1;
            if (n->index() > succ->twinNode()->index())
              rightedgeindex += 2;

#ifdef OGDF_DEBUG
            cout << "  LIe" << e->index() << " " << e << " + REe" << succ->theEdge()->index() << " " << succ->theEdge() << " >= 1" << endl;
#endif

            vec.insert(faceangleindex, 1);
            vec.insert(leftedgeindex, 1);
            vec.insert(rightedgeindex, 1);
            matrix.appendRow(vec);

            lower[index] = 1;
            upper[index] = osi->getInfinity();
            ++index;

            ae = ae->succ(); // damit das funktioniert, muss ae->succ() zur im uhrzeigersinn folgenden kante gehören!
          }
        n = n->succ();
      }

    Array<double> obj(8 * E); // Zielfunktion zusammenbauen
    for (int i = 0; i < 8 * E; i++)
      {
        if (i < 2 * E || i >= 4 * E)
          {
            obj[i] = 1;
          }
        else obj[i] = 0;
      }

    osi->loadProblem(matrix, 0, 0, obj.begin(), lower.begin(), upper.begin());

    for (int i = 0; i < 8 * E; ++i)
      osi->setInteger(i);

    return osi;
  }

  int KandinskyLayout::getAngleFor(adjEntry ae, const double * sol, const int E)
  {
    edge e = ae->theEdge();
    int i = e->index();

    int result;
    if (ae->theNode()->index() > ae->twinNode()->index())
      {
        result = (int) (sol[2 * E + 2 * i + 1] + 0.5);
      }
    else
      {
        result = (int) (sol[2 * E + 2 * i + 0] + 0.5);
      }

    //cout << ">>>>>> Winkel an Kante " << e->index() << ((ae->theNode()->index() > ae->twinNode()->index()) ? "(Rück) " : "(Hin) ") << e->source()->index() << "->" << e->target()->index() << ": " << result << endl;
    return result;
  }

  int KandinskyLayout::getBendFor(adjEntry ae, const double * sol, const int E)   // -1=links, 0=geradeaus, 1=rechts
  {
    edge e = ae->theEdge();

    int base = 4 * E + 4 * e->index();
    if (ae->theNode()->index() > ae->twinNode()->index())
      base += 2;

    int result;
    if (sol[base] > 0)
      {
        result = -1;
      }
    else if (sol[1 + base] > 0)
      {
        result = 1;
      }
    else
      result = 0;

    //cout << "  >>  Kantenknick an Kante " << e->index() << ((ae->theNode()->index() > ae->twinNode()->index()) ? "(Rück) " : "(Hin) ") << e->source()->index() << "->" << e->target()->index() << ": " << result << endl;
    return result;
  }

  BendString KandinskyLayout::getBendString(adjEntry ae, const double * sol, const int E)
  {
    edge e = ae->theEdge();
    int base = 2 * e->index();

    bool reverse = ae->theNode()->index() > ae->twinNode()->index();
    int fac = reverse ? -1 : 1;

    int bends = sol[base], bendsrev = sol[base + 1];
    int ffbends = fac * bends - fac * bendsrev;

    BendString ret;
    while (ffbends > 0)
      {
        ret += BendString("0");
        ffbends -= 1;
      }
    while (ffbends < 0)
      {
        ret += BendString("1");
        ffbends += 1;
      }

    //cout << "  >>  Knicke auf Kante " << e->index() << ((ae->theNode()->index() > ae->twinNode()->index()) ? "(Rück) " : "(Hin) ") << e->source()->index() << "->" << e->target()->index() << ": " << ret << endl;
    return ret;
  }

  void KandinskyLayout::draw(PlanRep& PG, OrthoRep& OR, CombinatorialEmbedding& E, adjEntry adjExternal, Layout & drawing)
  {
    //drawing object distances
    double m_separation = this->separation(); //1.0; //40?
    double m_cOverhang = 0.2; //0.2;
    //edge costs
    int m_costAssoc = 1; //should be set by profile
    int m_costGen = 1;
    //align hierarchy nodes on same level
    bool m_align = false;
    //scale layout while improving it
    //bool m_useScalingCompaction = false;
    int m_scalingSteps = 4;

    String error;
    OGDF_ASSERT(OR.check(error));

    //    // remove face splitter
    //    edge e, eSucc;
    //    for (e = PG.firstEdge(); e; e = eSucc) {
    //        eSucc = e->succ();
    //        if (PG.faceSplitter(e)) {
    //            OR.angle(e->adjSource()->cyclicPred()) = 2;
    //            OR.angle(e->adjTarget()->cyclicPred()) = 2;
    //            PG.delEdge(e);
    //        }
    //    }

    OGDF_ASSERT(E.consistencyCheck());

    // expand low degree vertices
    PG.expandLowDegreeVertices(OR);

    OGDF_ASSERT(OR.check(error));
    OGDF_ASSERT(PG.representsCombEmbedding());

    // restore embedding
    E.computeFaces();
    E.setExternalFace(E.leftFace(adjExternal));

    OGDF_ASSERT(E.consistencyCheck());
    OGDF_ASSERT(OR.check(error));

    // apply constructive compaction heuristics
    E.computeFaces();
    E.setExternalFace(E.leftFace(adjExternal));

    OGDF_ASSERT(OR.check(error));

    OR.normalize();

    E.computeFaces();
    E.setExternalFace(E.leftFace(adjExternal));

    OGDF_ASSERT(OR.check(error));

    OR.dissect();
    OR.orientate(PG, odNorth);

    // compute cage information and routing channels
    OR.computeCageInfoUML(PG);

    OGDF_ASSERT(OR.check(error));

    //temporary grid layout
    GridLayoutMapped gridDrawing(PG, OR, m_separation, m_cOverhang, 2);
    RoutingChannel<int> rcGrid(PG, gridDrawing.toGrid(m_separation), m_cOverhang);
    rcGrid.computeRoutingChannels(OR, true);

    node v;
    const OrthoRep::VertexInfoUML *pInfoExp;

    forall_nodes(v, PG)
    {
      pInfoExp = OR.cageInfo(v);
      if (pInfoExp) break;
    }

    FlowCompaction fca(0, m_costGen, m_costAssoc);
    fca.constructiveHeuristics(PG, OR, rcGrid, gridDrawing);

    OR.undissect(m_align);

    if (!m_align)
      {
        assert(OR.check(error));
      }

    // call flow compaction on grid
    FlowCompaction fc(0, m_costGen, m_costAssoc);
    fc.align(m_align);
    fc.scalingSteps(m_scalingSteps);
    fc.improvementHeuristics(PG, OR, rcGrid, /*gridDrawing.toGrid(m_separation),*/ gridDrawing);

    //remove alignment edges before edgerouter call because compaction
    //may do an unsplit at the nodes corners, which is impossible if
    //there are alignment edges attached
    if (m_align) OR.undissect(false);

    assert(OR.check(error));

    EdgeRouter router;
    MinimumEdgeDistances<int> minDistGrid(PG, gridDrawing.toGrid(m_separation));

    //router.setOrSep(int(gridDrawing.toGrid(l_orsep))); //scaling test
    router.call(PG, OR, gridDrawing, E, rcGrid, minDistGrid, gridDrawing.width(),
                gridDrawing.height(), m_align);


    assert(OR.check(error));

    // call flow compaction on grid
    double l_orsep = m_separation;
    fc.improvementHeuristics(PG, OR, minDistGrid/*,gridWidth,gridHeight*/, gridDrawing,
                             int(gridDrawing.toGrid(l_orsep)));

    // re-map result
    gridDrawing.remap(drawing);

    // collapse all expanded vertices by introducing a new node in the center
    // of each cage representing the original vertex
    PG.collapseVertices(OR, drawing);

    // finally set the bounding box
    // computeBoundingBox(PG,drawing);

    m_separation = l_orsep;
  }

//! Returns the minimal allowed distance between edges and vertices.
  double KandinskyLayout::separation() const
    {
      return this->sep;
    }

//! Sets the minimal allowed distance between edges and vertices to \a sep.
  void KandinskyLayout::separation(double sep)
  {
    this->sep = sep;
  }
}

#endif
