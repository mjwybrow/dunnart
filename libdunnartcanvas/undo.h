/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2003-2007  Michael Wybrow
 * Copyright (C) 2006-2011  Monash University
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
 * Author(s): Michael Wybrow  <http://michael.wybrow.info/>
*/

#ifndef UNDO_H
#define UNDO_H

#include <QList>
#include <QUndoCommand>

#include "libdunnartcanvas/canvas.h"

namespace dunnart {

class Canvas;
class CanvasItem;

// The UndoMacro represents an object we can push to the undo stack
// immediately, but that may has some of it's items modified later by
// the automatically layout.  We can't do this using just children of
// QUndoCommand cause the actions happen only when the parent is pushed
// to the undo stack and the SIGNALS are sent at the wrong time.  Hence,
// we manually handle addition and merging in addCommand.
//
class UndoMacro : public QUndoCommand
{
    public:
        UndoMacro(Canvas *canvas);
        ~UndoMacro();
        virtual void undo();
        virtual void redo();
        void addCommand(QUndoCommand *command);
    private:
        Canvas *m_canvas;
        QList<QUndoCommand *> m_undo_commands;
};


class CmdCanvasSceneAddItem : public QUndoCommand
{
    public:
        CmdCanvasSceneAddItem(Canvas *canvas, CanvasItem *item);
        ~CmdCanvasSceneAddItem();
        virtual void undo();
        virtual void redo();
    private:
        Canvas *m_canvas;
        CanvasItem *m_item;
        bool m_item_memory_owned_by_canvas;
};


class CmdCanvasSceneRemoveItem : public QUndoCommand
{
    public:
        CmdCanvasSceneRemoveItem(Canvas *canvas, CanvasItem *item);
        ~CmdCanvasSceneRemoveItem();
        virtual void undo();
        virtual void redo();
    private:
        Canvas *m_canvas;
        CanvasItem *m_item;
        bool m_item_memory_owned_by_canvas;
};



enum {
    UNDO_SHAPE_POS  = 1,
    UNDO_SHAPE_SIZE,
    UNDO_GUIDELINE_POS
};

#define UNDO_ACTION(OBJECT, TYPE, GETTER, SETTER, ID, STRDESC) \
private: \
class Cmd##OBJECT##SETTER : public QUndoCommand \
{ \
public: \
    Cmd##OBJECT##SETTER(OBJECT *item, const TYPE& old_value, \
            const TYPE& new_value) \
        : QUndoCommand(STRDESC), \
          m_item(item), \
          m_old_value(old_value), \
          m_new_value(new_value) \
    { \
        redo(); \
    } \
    virtual int id (void) const \
    { \
        return ID; \
    } \
    virtual bool mergeWith(const QUndoCommand *command) \
    { \
        if (command->id() != id()) \
        { \
            return false; \
        } \
        const Cmd##OBJECT##SETTER *rhs = \
               static_cast<const Cmd##OBJECT##SETTER *>(command); \
        if (m_item == rhs->m_item) \
        { \
            m_new_value = rhs->m_new_value; \
            return true; \
        } \
        return false; \
    } \
    virtual void undo() \
    { \
        m_item->SETTER(m_old_value); \
    } \
    virtual void redo() \
    { \
        m_item->SETTER(m_new_value); \
    } \
private: \
    OBJECT *m_item; \
    TYPE m_old_value; \
    TYPE m_new_value; \
}; \
public: \
void cmd_##SETTER(const TYPE newPos) \
{ \
    if (GETTER() == newPos) \
    { \
        return; \
    } \
    if (canvas()) \
    { \
        UndoMacro *macro = canvas()->currentUndoMacro(); \
        Cmd##OBJECT##SETTER *cmd = new Cmd##OBJECT##SETTER(this, GETTER(), newPos); \
        macro->addCommand(cmd); \
    } \
    else \
    { \
        this->SETTER(newPos); \
    } \
}

}
#endif
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

