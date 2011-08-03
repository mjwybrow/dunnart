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

#include "libdunnartcanvas/undo.h"
#include "libdunnartcanvas/canvas.h"
#include "libdunnartcanvas/canvasitem.h"

namespace dunnart {

UndoMacro::UndoMacro(Canvas *canvas)
    : m_canvas(canvas)
{
}

UndoMacro::~UndoMacro()
{
    foreach (QUndoCommand *cmd, m_undo_commands)
    {
        delete cmd;
    }
    m_undo_commands.clear();
}

void UndoMacro::addCommand(QUndoCommand *command)
{
    for (int i = 0; i < m_undo_commands.size(); ++i)
    {
        if (m_undo_commands.at(i)->mergeWith(command))
        {
            delete command;
            return;
        }
    }
    m_undo_commands.append(command);
}

void UndoMacro::undo()
{
    //qDebug("- Undo:");
    for (int i = m_undo_commands.size() - 1; i >= 0; --i)
    {
        //qDebug("-- %s", qPrintable(m_undo_commands.at(i)->text()));
        m_undo_commands.at(i)->undo();
    }
    m_canvas->processUndoResponseTasks();
}

void UndoMacro::redo()
{
    //qDebug("- Redo:");
    for (int i = 0; i < m_undo_commands.size(); ++i)
    {
        //qDebug("++ %s", qPrintable(m_undo_commands.at(i)->text()));
        m_undo_commands.at(i)->redo();
    }

    if (!m_undo_commands.empty())
    {
        m_canvas->processUndoResponseTasks();
    }
}


CmdCanvasSceneAddItem::CmdCanvasSceneAddItem(Canvas *canvas, CanvasItem *item)
    : QUndoCommand("add item to canvas"),
      m_canvas(canvas),
      m_item(item)
{
    redo();
}

CmdCanvasSceneAddItem::~CmdCanvasSceneAddItem()
{
    if (!m_item_memory_owned_by_canvas)
    {
        // We can free the item with the undo event
        delete m_item;
        m_item = NULL;
    }
}

void CmdCanvasSceneAddItem::undo()
{
    m_canvas->removeItem(m_item);
    m_item_memory_owned_by_canvas = false;
}

void CmdCanvasSceneAddItem::redo()
{
    m_canvas->addItem(m_item);
    m_item_memory_owned_by_canvas = true;
}


CmdCanvasSceneRemoveItem::CmdCanvasSceneRemoveItem(Canvas *canvas, CanvasItem *item)
    : QUndoCommand("remove item from canvas"),
      m_canvas(canvas),
      m_item(item)
{
    redo();
}

CmdCanvasSceneRemoveItem::~CmdCanvasSceneRemoveItem()
{
    if (!m_item_memory_owned_by_canvas)
    {
        // We can free the item with the undo event
        delete m_item;
        m_item = NULL;
    }
}

void CmdCanvasSceneRemoveItem::undo()
{
    m_canvas->addItem(m_item);
    m_item_memory_owned_by_canvas = true;
}

void CmdCanvasSceneRemoveItem::redo()
{
    m_canvas->removeItem(m_item);
    m_item_memory_owned_by_canvas = false;
}

}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

