/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2010-2011  Monash University
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 *
 * 
 * Author(s): Sarah Boyd  <Sarah.Boyd@monash.edu>
*/


// This class represents an SBGN Process Diagram (Level 1) Entity Pool Node.
// By default, this class is drawn as "unspecified entity".


#ifndef PDEPN_H
#define PDEPN_H

#include "libdunnartcanvas/shape.h"

#include <QGraphicsItem>
#include <QFont>
#include <QObject> // need this class for the QOBJECT macro, which is needed for type checking
#include <QPainter> // this is actually included in the coreclasses.cpp file, but needed here because of order of inclusion

using namespace dunnart;

//enum Cloneable { IS_CLONEABLE, NOT_CLONEABLE };
static const int GLYPH_PADDING = 5; // this is used as padding when drawing glyphs, to prevent auxiliary units and multimeric states from crossing outside the glyph bounding box

#if 0
// Draws a label with boundaries of its own (!= boundaries of the glyph it qualifies)
class Label: public QGraphicsItem
{
public:
    Label(QString t);

//    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    QRectF boundingRect() const;
    QPainterPath shape() const;

protected:
    QString text;
    QFont font;
    int textFlags;
};
#endif

#if 0
// Defines a glyph to which a label has been attached
class LabelledGlyph : public ShapeObj
{
//    Q_OBJECT
public:
    LabelledGlyph(QString l);
    LabelledGlyph(Label * l, float w, float h);
    LabelledGlyph(QString t, float w, float h);

protected:
    Label * label;
    void init();
};
#endif

class PDEPN: public ShapeObj
{
    Q_OBJECT

public:
    // By default an EPN does not have a label, and is not cloneable
    PDEPN() : ShapeObj("sbgn.NucleicAcidEPN"), cloned(false), cloneLabel(QString("")), multimer(false) {}
//    PDEPN(float w, float h) : LabelledGlyph("", w, h), cloned(false), cloneLabel(new Label("")), multimer(false) {}
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    virtual QPainterPath clone_marker() const = 0; // creates the clone marker for this glyph: MUST be instantiated in the inheriting class
    bool isCloned();
    bool isMultimeric() { return false; } // an EPN is not multimeric

protected:
    PDEPN(QString l, bool cb, QString cl, bool m);
//    PDEPN(Label *l, bool cb, Label *cl, bool m);
//    PDEPN(Label *l, float w, float h, bool cb, Label *cl, bool m);
//    PDEPN(QString t, float w, float h, bool cb, QString cl, bool m);
    bool cloned;
    QString cloneLabel;
    bool multimer;

  // QT
  #if 0
protected:
    virtual QAction *buildAndExecContextMenu(
            QGraphicsSceneMouseEvent *event, QMenu& menu);
public:
  PDEPN();
  PDEPN(const double x, const double y, const int w=60, const int h=40);
  PDEPN(xmlNode *node, xmlNs *ns);
  virtual ~PDEPN();
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
  SDL_Rect labelBoundingRect(void) const;
  int get_type(void)  {  return SHAPE_TYPE_PDEPN;  }
  void set_is_cloned(bool b) { has_clone_marker = b; }

  // even though not every EPN can have a clone marker, this should be
  // the same for every class that uses it:
  const Uint32 get_clone_marker_col() { return clone_marker_col; }


  /*** 
       The following functions need to be implemented depending on
       each EPN type, because they are not common to all EPNs
  ***/

  // Functions to switch the current clone marker on/off. Most EPNs
  // can carry this marker, so implement in this class, rather than as virtual:
  void switchClone(void);
  static void switchCloning(GuiObj **c);

  // Switch multimeric state.  Most EPNs (including this class) can't
  // be multimeric, so implement as virtual.
  virtual void switchMultimer(void) {} 

  /* "unspecified entity" can carry a clone marker */
  virtual bool is_cloned(void) { return has_clone_marker; }
  
  /* "unspecified entity" cannot be cloned, so this function by
     default has to return false */
  virtual const bool is_multimeric() { return false; } 

  bool has_clone_marker; // tracks whether node has a clone marker
  bool is_a_multimer; // tracks whether node is multimeric

private:
  Uint32 clone_marker_col;  // colour to be used for clone marker drawing

  // this function sets all the default values for the member variables
  void set_class_members() { 
    clone_marker_col = gfxCol(125, 125, 125); 
    has_clone_marker = false; // "unspecified entity" not cloned by default
    is_a_multimer = false; // "unspecified entity" cannot be multimeric
  }
#endif
};

#endif //PDEPN_H
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

