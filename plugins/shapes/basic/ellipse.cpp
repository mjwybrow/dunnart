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

#include <QString>

#include "libdunnartcanvas/canvasitem.h"
using namespace dunnart;

#include "ellipse.h"

//===========================================================================
//  "Beginning or End of Process" shape code:


QPainterPath EllipseShape::buildPainterPath(void)
{
    QPainterPath painter_path;

    double yround = (height() / 2);
    double xround = (width() / 2);

    double round = (xround <= yround) ? xround : yround;

    /* QT
    if (gmlGraph)
    {
        xround = yround = 0;
        round = 10;
    }
    */

    painter_path.addRoundedRect(-width() / 2, -height() / 2, width(), height(),
            round, round);

    return painter_path;
}


QDomElement EllipseShape::to_QDomElement(const unsigned int subset,
        QDomDocument& doc)
{
    QDomElement node = doc.createElement("dunnart:node");

    if (subset & XMLSS_IOTHER)
    {
        newNsProp(node, x_dunnartNs, x_type, x_shEndOProc);
    }

    addXmlProps(subset, node, doc);

    return node;
}

#if 0
void EndOProc::draw(SDL_Surface *surface, const int x, const int y,
        const int type, const int w, const int h)
{
    if (!surface)
    {
        return;
    }

    int cx = (int) (x + (w / 2.0));
    int cy = (int) (y + (h / 2.0));
    int dx = x + HANDLE_PADDING;
    int dy = y + HANDLE_PADDING;
    int dx2 = x + w + 1 - (2 * HANDLE_PADDING);
    int dy2 = y + h + 1 - (2 * HANDLE_PADDING);

    QColor highlight_col = 0;

    QColor outline_col = getLineCol();
    QColor fill_col = getFillCol();

    if (_collapsed)
    {
        // Just draw a rectangle.
        boxColor(surface, dx, dy, dx2, dy2, fill_col);
        rectangleColor(surface, dx, dy, dx2, dy2, outline_col);
        return;
    }

    int strtx = dx;
    int strty = dy;
    int endx = dx2;
    int endy = dy2;
    int yround = ((h - 8) / 2);
    int xround = ((w - 8) / 2);

    int round = (xround <= yround) ? xround : yround;

    if (gmlGraph || sbgnShape)
    {
        // SBGN nodes are rectangles with rounded corners.
        xround = yround = 0;
        round = 10;
    }
    if (sbgnBubbles > 0)
    {
        endy -= 6;
    }

    if (sbgnShape && (h == w) && (type != SHAPE_DRAW_OUTLINE))
    {
        // SBGN chemicals are just circles.
        int rad = (int) ((w / 2.0) - HANDLE_PADDING);

        filledCircleColor(surface, cx, cy, rad, fill_col);
        if (sbgnClone)
        {
            // Clones are marked by a horizontal bar at the bottom of the shape.
            SDL_Rect crect = { cx - rad, cy + rad - 14, cx + rad, cy + rad  };
            SDL_SetClipRect(surface, &crect);
            filledPieColor(surface, cx, cy, rad, 0, 180, outline_col);
            SDL_SetClipRect(surface, NULL);
        }
        aacircleColor(surface, cx, cy, rad, outline_col);
    }
    else if (type != SHAPE_DRAW_OUTLINE)
    {
        if (cascade_glow)
        {
            filledEllipseColor(surface, endx - round, endy - round,
                    round, round, shLightGlow);
            filledEllipseColor(surface, strtx + round, strty + round,
                    round, round, shLightGlow);
            if (xround == yround)
            {
                filledEllipseColor(surface, strtx + round, endy - round,
                        round, round, shLightGlow);
                filledEllipseColor(surface, endx - round, strty + round,
                        round, round, shLightGlow);
            }
        }
        glowSetClipRect(surface);
        filledEllipseColor(surface, endx - round, endy - round,
                round, round, fill_col);
        filledEllipseColor(surface, strtx + round, strty + round,
                round, round, fill_col);
        if (xround == yround)
        {
            filledEllipseColor(surface, strtx + round, endy - round,
                    round, round, fill_col);
            filledEllipseColor(surface, endx - round, strty + round,
                    round, round, fill_col);
        }
        glowClearClipRect(surface);

        SDL_Rect crect = { x, y, w, h };
        if (xround == yround)
        {
            crect.x = endx - round;
            crect.w = round + 1;
            crect.y = endy - round;
            crect.h = round + 1;
            SDL_SetClipRect(surface, &crect);
            aaellipseColor(surface, endx - round, endy - round,
                    round, round, outline_col);

            crect.x = dx;
            SDL_SetClipRect(surface, &crect);
            aaellipseColor(surface, strtx + round, endy - round,
                    round, round, outline_col);

            crect.y = dy;
            SDL_SetClipRect(surface, &crect);
            aaellipseColor(surface, strtx + round, strty + round,
                    round, round, outline_col);

            crect.x = endx - round;
            SDL_SetClipRect(surface, &crect);
            aaellipseColor(surface, endx - round, strty + round,
                    round, round, outline_col);
        }
        else
        {
            if (xround > yround)
            {
                crect.x = endx - round;
                crect.w = round + 1;
            }
            else
            {
                crect.y = endy - round;
                crect.h = round + 1;
            }
            SDL_SetClipRect(surface, &crect);
            aaellipseColor(surface, endx - round, endy - round,
                    round, round, outline_col);
            if (xround > yround)
            {
                crect.x = dx;
                crect.w = round + 1;
                crect.y = y;
                crect.h = h;
            }
            else
            {
                crect.y = dy;
                crect.h = round + 1;
                crect.x = x;
                crect.w = w;
            }
            SDL_SetClipRect(surface, &crect);
            aaellipseColor(surface, strtx + round, strty + round,
                    round, round, outline_col);
        }
        SDL_SetClipRect(surface, NULL);

        rectangleColor(surface, strtx, strty + round, endx, endy - round,
                outline_col);
        rectangleColor(surface, strtx + round, strty, endx - round, endy,
                outline_col);
        if (cascade_glow)
        {
            boxColor(surface, strtx + 1, strty + round, endx - 1,
                    endy - round, shLightGlow);
            boxColor(surface, strtx + round, strty + 1, endx - round,
                    endy - 1, shLightGlow);
        }
        glowSetClipRect(surface);
        boxColor(surface, strtx + 1, strty + round, endx - 1, endy - round,
                fill_col);
        boxColor(surface, strtx + round, strty + 1, endx - round, endy - 1,
                fill_col);
        glowClearClipRect(surface);
    }
    if (sbgnBubbles > 0)
    {
        int rad = 7;
        if (sbgnBubbles & 1)
        {
            int bcx = cx;
            int bcy = dy2 - 6;
            filledCircleColor(surface, bcx, bcy, rad, fill_col);
            aacircleColor(surface, bcx, bcy, rad, outline_col);
            if (sbgnBubbles & 2)
            {
                SDL_WriteText(surface, bcx - 3, bcy - 6, "P",
                        &black, shapeFont);
            }
        }
        if (sbgnBubbles & 4)
        {
            int bcx = cx + 12;
            int bcy = dy2 - 6;
            filledCircleColor(surface, bcx, bcy, rad, fill_col);
            aacircleColor(surface, bcx, bcy, rad, outline_col);
            if (sbgnBubbles & 8)
            {
                SDL_WriteText(surface, bcx - 3, bcy - 6, "P",
                        &black, shapeFont);
            }
        }
        if (sbgnBubbles & 16)
        {
            int bcx = cx - 12;
            int bcy = dy2 - 6;
            filledCircleColor(surface, bcx, bcy, rad, fill_col);
            aacircleColor(surface, bcx, bcy, rad, outline_col);
            if (sbgnBubbles & 32)
            {
                SDL_WriteText(surface, bcx - 3, bcy - 6, "P",
                        &black, shapeFont);
            }
        }
    }

    switch (type)
    {
        case SHAPE_DRAW_NORMAL:
            // Nothing to do.
            break;
        case SHAPE_DRAW_HIGHLIGHTED:
            highlight_col = SDL_MapRGB(screen->format, 0,255,255);
            // No break
        case SHAPE_DRAW_LEAD_HIGHLIGHTED:
            if (highlight_col == 0)
            {
                highlight_col = SDL_MapRGB(screen->format, 0,255,0);
            }
            for(int i = dx; i <= dx2 - 3; i += 6)
            {
                Draw_HLine(surface, i, dy, i + 2, highlight_col);
                Draw_HLine(surface, i, dy2, i + 2, highlight_col);
            }
            for(int i = dy; i <= dy2 - 3; i += 6)
            {
                Draw_VLine(surface, dx, i, i + 2, highlight_col);
                Draw_VLine(surface, dx2, i, i + 2, highlight_col);
            }
            break;
        case SHAPE_DRAW_OUTLINE:
            Draw_Rect(surface, dx, dy, w - 7, h - 7, BLACK);
            for(int i = dx; i <= dx2; i += 2)
            {
                putpixel(surface, i, dy, WHITE);
                putpixel(surface, i, dy2, WHITE);
            }
            for(int i = dy; i <= dy2; i += 2)
            {
                putpixel(surface, dx, i, WHITE);
                putpixel(surface, dx2, i, WHITE);
            }
            break;
        default:
            break;
    }

    // Shift label up to make room for bubbles.
    int labelY = (sbgnBubbles > 0) ? y - 4 : y;
    common_draw(surface, x, labelY);
}
#endif
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
