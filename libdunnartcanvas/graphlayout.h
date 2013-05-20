/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2003-2007  Michael Wybrow
 * Copyright (C) 2006-2008  Monash University
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
 * Author(s): Tim Dwyer
 *            Michael Wybrow  <http://michael.wybrow.info/>
*/

#ifndef GRAPHLAYOUT_H
#define GRAPHLAYOUT_H

#include <QMutex>
#include <QWaitCondition>
#include <QSet>

#include <set>

#include "libcola/cola.h"
#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/canvas.h"

namespace vpsc {
    class Rectangle;
};

namespace dunnart {

class GraphLayout;
class Cluster;
class GraphData;
class LayoutThread;

/**
 * A PosInfo is used primarily to pass position info for shapes and constraint
 * widgets between the GUI and graph layout threads.  At the end of each layout
 * iteration, the PostIteration callback creates a list of PosInfos containing
 * the updated positions of all objects.  The GUI thread later calls process()
 * to handle the updated positions.  Optionally, processHUD may be called by
 * the GUI thread to draw a debug "Head Up Display" for the associated
 * shape/widget.  PosInfos can also be created by the GUI thread to tell the
 * layout thread that certain objects should be fixed at their current position
 * (these are processed by the layout thread by calling
 * fixGraphLayoutPosition().
 */
class PosInfo
{
public:
    PosInfo();
    PosInfo(bool debugHUD, unsigned short HUDTimer);

    // called by GUI thread to whatever needs to be done with the
    // object after each layout iteration
    virtual void process(Canvas *canvas) = 0;

    // called by GUI thread to draw any debug HUD
    bool processHUD(QPixmap *sur) {
        if(debugHUD) {
            HUDTimer--;
            myProcessHUD(sur);
            return HUDTimer>0;
        }
        return false;
    };

    virtual bool isNull(void)
    {
        return true;
    }

    // HUD details specific to implementation
    virtual void myProcessHUD(QPixmap *sur) { Q_UNUSED (sur) };

    // called by graphlayout thread to fix the position of objects
    virtual void fixGraphLayoutPosition(GraphData* g,cola::Locks& locks,cola::Resizes& resizes)
    {
        Q_UNUSED (g)
        Q_UNUSED (locks)
        Q_UNUSED (resizes)
    }

    virtual ~PosInfo() {};

    // allows HUDs to be controlled at a finer level of granularity
    bool debugHUD;
    unsigned int processOrder;
protected:
    GraphLayout *gl;
private:
    // controls the life time of the HUD, i.e. the number of times it is to be redrawn
    unsigned short HUDTimer;
};
typedef std::list<PosInfo *> PosInfos;

struct ShapePosInfo;

/**
 * GraphLayout is a singleton class (there should only ever be one layout instance)
 * which provides the interface between the dunnart GUI
 * and layout threads.
 */
class GraphLayout {
public:
    GraphLayout(Canvas *canvas);
    ~GraphLayout();

    //! controls the style of layout
    enum Mode {
        ORGANIC, //! The standard force-directed layout with no automatically
                 //! generated constraints
        FLOW,    //! Generates a vertical separation constraint for every pair
                 //! of edges so that the edge is required to point downwards
        LAYERED  //! Uses FLOW layout but also generates level alignment
                 //! constraints so that nodes positions are in layers
    };
    Mode mode; //!< controls the style of layout
    //! selects a method to minimise layout stress.
    enum OptimizationMethod {
        MAJORIZATION, //!< the robust stress majorization method
        STEEPESTDESCENT//!< method based on a constrained newton method, it's experimental
    };
    OptimizationMethod optimizationMethod; //!< selects a method to minimise layout stress
    //! max iterations to perform
    unsigned graph_layout_iterations;
    /**
     * GraphLayout may need to be applied in several stages.  Typically:
     * runLevel=
     * 0) perform layout with only structural and placement constraints
     * 1) enforce non-overlap and topology preserving constraints
     */
    unsigned runLevel;
    //! apply topology preserving layout to only the first topologyNodesCount
    //  nodes 
    unsigned topologyNodesCount;
    //! do not tell the optimiser about edges, i.e. just resolve user constraints
    bool ignoreEdges;
    //! (re)starts the layout thread
    void apply(bool ignoreEdges);
    //! interrupt from the GUI thread, basically abondons any in-process layout
    void setInterruptFromDunnart(void);
    //! interrupt from the GUI thread caused by "alt-dragging"
    void setLayoutSuspended(bool value);
    //! indicates "alt-dragging" going on
    bool isFreeShiftFromDunnart(void);
    //! interrupt layout thread to restart layout
    void setRestartFromDunnart(void);
    //! spawns the thread, call only once on setup of GUI
    int initThread();
    //! lock the specified shape's position
    void lockShape(ShapeObj* sh);
    //! unselected objects are pinned (fixed in position)
    void pinUnselectedShapes(QWidget **c);
    //! unclocks all shapes
    void unpinAllShapes(QWidget **c);
    //! stores current shape positions for use in "sticky node" behaviour
    void initialise(void);
    //! called by GUI thread to update object positions in handling
    //  USEREVENT_LAYOUT_UPDATES event
    int processReturnPositions(void);
    //! choose layout mode (default is ORGANIC)
    void setLayoutMode(Mode newMode);
    //! choose optimization method (default is MAJORIZATION)
    void setOptimizationMethod(OptimizationMethod newOM);
    GraphData *getGraphData(void);
    void setOutputDebugFiles(const bool value);
    //! whether the layout thread is currently active.
    bool isRunning(void) const;

private:
    Canvas *m_canvas;

    //! the graph itself and mappings to/from dunnart objects
    GraphData *m_graph;
    bool m_is_running;
    // The following are lists of PosInfo used to communicate between the GUI
    PosInfos retPositions;
    PosInfos fixedPositions;
    bool retPositionsHandled;
    bool outputDebugFiles;
    // The following control IPC between layout and GUI threads
    QMutex m_return_positions_mutex;
    QMutex m_layout_mutex;
    QWaitCondition m_layout_wait_condition;
    QMutex m_layout_signal_mutex;
    QMutex m_changed_list_mutex;
    cola::Locks locks;
    cola::Resizes resizes;
    std::map<ShapeObj*,ShapePosInfo*> fixedShapeLookup;
    QSet<ShapeObj*> pinnedShapes;
    bool positionChangesFromDunnart;
    bool interruptFromDunnart;
    bool freeShiftFromDunnart;
    bool restartFromDunnart;
    bool askedToFinish;
    LayoutThread *m_layout_thread;

    cola::UnsatisfiableConstraintInfos unsatisfiableX, unsatisfiableY;
    void run(const bool shouldReinitialise);
    void showUnsatisfiable(cola::UnsatisfiableConstraintInfo* i);
    void addToFixedList(CanvasItemsList & objList);
    void addPinnedShapesToFixedList(void);
    void addToResizedList(CanvasItemsList & objList);
    void clearReturnPosInfos(void);

    friend struct PreIteration;
    friend class PostIteration;
#ifndef NOGRAPHVIZ
    friend int graphvizLayout(GraphLayout& gl);
#endif
};


}
#endif
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

