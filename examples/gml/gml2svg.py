import re
infilename="/home/dwyer/devel/dunnart/examples/gml/interesting_planar_orthogonallayout.gml"
outfilename="/home/dwyer/devel/dunnart/examples/planarorthogonal.svg"
fin=open(infilename)
fout=open(outfilename,'w')

lines=fin.readlines()

i=0
nodes={}
edges=[]

class Node:
    pat=re.compile('\s*node\s*')
    idpat=re.compile('\s*id\s+(\w+)')
    labelpat=re.compile('\s*label\s+\"(.*)"')
    wpat=re.compile('\s*w\s+(\d+\.?\d*)')
    hpat=re.compile('\s*h\s+(\d+\.?\d*)')
    xpat=re.compile('\s*x\s+(\d+\.?\d*)')
    ypat=re.compile('\s*y\s+(\d+\.?\d*)')
    def __init__(self):
        self.label=''
        self.width=7
        self.height=7

class Edge:
    pat=re.compile('\s*edge\s*')
    sourcepat=re.compile('\s*source\s+(\d+)')
    targetpat=re.compile('\s*target\s+(\d+)')    
    pointpat=re.compile('\s*point\s*')
    
def levelchange(l):
    return l.count('[')-l.count(']')

def readnode():
    global i
    level=0
    node=Node()
    node.line=i
    i=i+1
    while i<len(lines):
        l=lines[i]
        level=level+levelchange(l)
        if level==0:
            return node
        m=Node.idpat.match(l)
        if m:
            node.id=int(m.group(1))+1
        m=Node.labelpat.match(l)
        if m:
            node.label=m.group(1)
        m=Node.wpat.match(l)
        if m:
            node.width=float(m.group(1))
        m=Node.hpat.match(l)
        if m:
            node.height=float(m.group(1))
        m=Node.xpat.match(l)
        if m:
            node.x=float(m.group(1))
        m=Node.ypat.match(l)
        if m:
            node.y=float(m.group(1))
        i=i+1

def readpoint(edge):
    global i
    level=0
    xpat=re.compile('\s*x\s+(\d+\.?\d*)')
    ypat=re.compile('\s*y\s+(\d+\.?\d*)')
    u=nodes[edge.source]
    w=nodes[edge.target]
    i=i+1
    x=0
    y=0
    while i<len(lines):
        l=lines[i]
        level=level+levelchange(l)
        if level==0:
            break
        m=Node.xpat.match(l)
        if m:
            x=float(m.group(1))
        m=Node.ypat.match(l)
        if m:
            y=float(m.group(1))
        i=i+1
    if(x==u.x and y==u.y):
        print "skipping start point"
        return
    if(x==w.x and y==w.y):
        print "skipping end point"
        return
    v=Node()
    print "dx=%f,dy=%f"%(x-u.x,y-u.y)
    print "dx=%f,dy=%f"%(x-w.x,y-w.y)
    if abs(u.x-x)<15:
        v.x=u.x
    elif abs(w.x-x)<15:
        v.x=w.x
    else:
        v.x=x
    if abs(u.y-y)<15:
        v.y=u.y
    elif abs(w.y-y)<15:
        v.y=w.y
    else:
        v.y=y
    print "point on edge %d,%d"%(edge.source,edge.target)
    v.id=len(nodes)+100
    nodes[v.id]=v
    edge2=Edge()
    edge2.source=edge.source
    edge2.target=v.id
    edge.source=v.id
    edges.append(edge2)
    edge2.line=i
    print "source at %f,%f; edge %d->%d; point at %f,%f; edge %d->%d; target at %f,%f"%(u.x,u.y,edge2.source,edge2.target,v.x,v.y,edge.source,edge.target,w.x,w.y)

def readedge():
    global i
    level=0
    edge=Edge()
    edge.line=i
    i=i+1
    while i<len(lines):
        l=lines[i]
        level=level+levelchange(l)
        if level==0:
            return edge
        m=Edge.sourcepat.match(l)
        if m:
            edge.source=int(m.group(1))+1
        m=Edge.targetpat.match(l)
        if m:
            edge.target=int(m.group(1))+1
        m=Edge.pointpat.match(l)
        if m:
            readpoint(edge)        
        i=i+1
        
while i<len(lines):
    if Node.pat.match(lines[i]):
        n=readnode()
        nodes[n.id]=n
    if Edge.pat.match(lines[i]):
        edges.append(readedge())
    i=i+1

fout.write('<?xml version="1.0" encoding="UTF-8"?>'
+'<!--Created with Dunnart (http://www.csse.monash.edu.au/~mwybrow/dunnart/)-->'
+'<svg xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape" xmlns:dunnart="http://www.csse.monash.edu.au/~mwybrow/dunnart.dtd" xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns="http://www.w3.org/2000/svg" height="646px" viewBox="190.000000 18.000000 484.000000 646.000000">'
  +'<dunnart:options useMultiway="1" instantFeedback="1" newConnType="53" orientation="0" graphMode="1" layoutMethod="0" layoutMode="0" nonOverlapConstraints="1" straightenConnectors="0" avoidBuffer="4" pageBoundaryConstraints="0" defaultIdealConnectorLength="1" penaliseCrossings="0"/>'
  +'<sodipodi:namedview showguides="false" guidetolerance="5.0" guidecolor="#000000ff" guideopacity="0.25" bordercolor="#000000" borderopacity="0.5" width="484px" height="646px"/>'
  +'<style type="text/css">.shape   { stroke:black; stroke-width:1px; fill:#f0f0d2; } .connector { fill:none; stroke:black; stroke-width:1px; stroke-linecap:butt; stroke-linejoin:miter; stroke-opacity:1; } .cluster { fill:#60cdf3; fill-opacity:0.33333333; } .tLabel { text-anchor:middle; text-align:center; } .tsLabel { font-size:11px; font-style:normal; font-weight:normal; fill:black; fill-opacity:1; stroke:none; font-family:DejaVu Sans; } .frLabel { font-size:11px; font-style:normal; font-weight:normal; fill:black; fill-opacity:1; stroke:none; stroke-width:1px; stroke-linecap:butt; stroke-linejoin:miter; stroke-opacity:1; font-family:DejaVu Sans; font-stretch:normal; font-variant:normal; text-anchor:middle; text-align:center; progression-align:center; writing-mode:lr; line-height:125%; } </style>'
  +'<defs>'
    +'<marker inkscape:stockid="ConnArrowEnd" orient="auto" refX="7.5" refY="0.0" id="ConnArrowEnd" style="overflow:visible;">'
      +'<path d="M 0.0,0.0 L 1.0,-5.0 L -12.5,0.0 L 1.0,5.0 L 0.0,0.0 z" style="fill-rule:evenodd; stroke:black; stroke-width:1px; marker-start:none;" transform="scale(0.6) rotate(180)"/>'
    +'</marker>'
  +'</defs>'
)
for v in nodes.values():
    fout.write('<rect dunnart:width="%f" dunnart:height="%f" x="%f" y="%f" id="%d" '
               %(v.width,v.height,v.x,v.y,v.id)
               +'class="shape" dunnart:type="rect" '
               +'dunnart:xPos="%f" dunnart:yPos="%f" '%(v.x,v.y)
               +'dunnart:label="%s" '%(v.label)
               +'/>\n')
for e in edges:
    oc=0
    if nodes[e.source].y==nodes[e.target].y:
        oc=1
    if nodes[e.source].x==nodes[e.target].x:
        oc=2
    fout.write('<path id="%d" class="connector" '%e.line
               +'dunnart:srcID="%d" dunnart:dstID="%d" dunnart:directed="0" '
               %(e.source,e.target)
               +'dunnart:srcFlags="544" dunnart:dstFlags="544" '
               +'dunnart:orthogonalConstraint="%d" '%oc
               +'dunnart:type="connStraight"/>\n'
               )

def createGuides1():
    horizontalGuides={}
    verticalGuides={}
    for e in edges:
        x=nodes[e.source].x
        if nodes[e.target].x==x:
            g=set([])
            if verticalGuides.has_key(x):
                g=verticalGuides[x]
            else:
                verticalGuides[x]=g
            g.add(e.source)
            g.add(e.target)
        y=nodes[e.source].y
        if nodes[e.target].y==y:
            g=set([])
            if horizontalGuides.has_key(y):
                g=horizontalGuides[y]
            else:
                horizontalGuides[y]=g
            g.add(e.source)
            g.add(e.target)

    gid=1000
    for (pos,vs) in horizontalGuides.iteritems():
        print "horizontal guide at %f"%pos
        gid=gid+1
        fout.write('<sodipodi:guide dunnart:type="indGuide" orientation="horizontal" dunnart:position="%f" dunnart:direction="101" id="%d"/>'%(pos,gid))
        for v in vs:
            print "  %d"%v
            fout.write('<dunnart:node dunnart:type="constraint" isMultiway="1" relType="alignment" constraintID="%d" objOneID="%d" alignmentPos="1"/>'%(gid,v))

    for (pos,vs) in verticalGuides.iteritems():
        print "vertical guide at %f"%pos
        gid=gid+1
        fout.write('<sodipodi:guide dunnart:type="indGuide" orientation="vertical" dunnart:position="%f" dunnart:direction="100" id="%d"/>'%(pos,gid))
        for v in vs:
            print "  %d"%v
            fout.write('<dunnart:node dunnart:type="constraint" isMultiway="1" relType="alignment" constraintID="%d" objOneID="%d" alignmentPos="4"/>'%(gid,v))

def createGuides2():
    horizontalGuides=[]
    verticalGuides=[]
    for e in edges:
        x=nodes[e.source].x
        if nodes[e.target].x==x:
            verticalGuides.append((x,(e.source,e.target)))
        y=nodes[e.source].y
        if nodes[e.target].y==y:
            horizontalGuides.append((y,(e.source,e.target)))

    gid=1000
    for (pos,(u,v)) in horizontalGuides:
        gid=gid+1
        fout.write('<sodipodi:guide dunnart:type="indGuide" orientation="horizontal" dunnart:position="%f" dunnart:direction="101" id="%d"/>'%(pos,gid))
        fout.write('<dunnart:node dunnart:type="constraint" isMultiway="1" relType="alignment" constraintID="%d" objOneID="%d" alignmentPos="1"/>'%(gid,u))
        fout.write('<dunnart:node dunnart:type="constraint" isMultiway="1" relType="alignment" constraintID="%d" objOneID="%d" alignmentPos="1"/>'%(gid,v))
    for (pos,(u,v)) in verticalGuides:
        gid=gid+1
        fout.write('<sodipodi:guide dunnart:type="indGuide" orientation="vertical" dunnart:position="%f" dunnart:direction="100" id="%d"/>'%(pos,gid))
        fout.write('<dunnart:node dunnart:type="constraint" isMultiway="1" relType="alignment" constraintID="%d" objOneID="%d" alignmentPos="4"/>'%(gid,u))
        fout.write('<dunnart:node dunnart:type="constraint" isMultiway="1" relType="alignment" constraintID="%d" objOneID="%d" alignmentPos="4"/>'%(gid,v))

fout.write('</svg>\n')
fout.close()
print "generated svg with %d nodes and %d edges"%(len(nodes),len(edges))
