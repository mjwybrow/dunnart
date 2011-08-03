/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2007  Michael Woodward
 * Copyright (C) 2007  Michael Wybrow
 * Copyright (C) 2008, 2010  Monash University
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
 * Author(s): Michael Woodward
 *            Michael Wybrow  <http://michael.wybrow.info/>
*/

#include <string>
#include <algorithm>
#include <vector>

#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/canvasitem.h"

#include "umlclass.h"


//static void center_text(QPixmap*, int, int, int, int, const char*);
static void get_text_dimensions(QString text, int *w, int *h);
static void get_text_width(QString text, int *w);

unsigned int umlFontSize = 10;
QFont *mono = NULL;

static const int minSectHeight = 23;
static const int classLineHeight = 12;

void ClassShape::initWithXMLProperties(Canvas *canvas,
        const QDomElement& node, const QString& ns)
{
    // Call equivalent superclass method.
    RectangleShape::initWithXMLProperties(canvas, node, ns);

    do_init();

    /* load class name, params, and methods from xml */
    QDomNodeList children = node.childNodes();

    for (uint i = 0; i < children.length(); ++i)
    {
        QDomElement element = children.item(i).toElement();
        QString name = element.nodeName();

        if (name == "class_name")
        {
            class_name = element.text();
        }
        else if (name == "class_stereotype")
        {
            class_stereotype = element.text();
        }
        else if (name == "mode")
        {
            mode = UML_Class_Abbrev_Mode(element.text().toInt());
        }
        else if (name == "detailLevel")
        {
            detailLevel = element.text().toInt();
        }
        else if (name == "attr_section_height")
        {
            attr_section_size = element.text().toInt();
        }
        else if (name == "method_section_height")
        {
            method_section_size = element.text().toInt();
        }
        else if (name == "attribute")
        {   
            /* makes assumption properties are in order */        
            attributes.resize(attributes.size()+1);
            if (element.hasAttribute("stereotype"))
            {
                attributes[attributes.size()-1].stereotype =
                        element.attribute("stereotype");
            }
            attributes[attributes.size()-1].is_public =
                    (element.attribute("private") == "no");
            attributes[attributes.size()-1].name = 
                    element.attribute("name");
            attributes[attributes.size()-1].type = 
                    element.attribute("type");
        }
        else if (name == "method")
        {
            /* makes assumption properties are in order */      
            methods.resize(methods.size()+1);
            methods[methods.size()-1].is_public =
                    (element.attribute("private") == "no");
            methods[methods.size()-1].name = 
                    element.attribute("name");
            methods[methods.size()-1].return_type = 
                    element.attribute("return_type");

            QDomNodeList params = element.childNodes();

            for (uint i = 0; i < params.length(); ++i)
            {
                QDomElement param = params.item(i).toElement();

                if (param.nodeName() == "parameter")
                {
                    /* makes assumption properties are in order */      
                    methods[methods.size()-1].params.resize(
                            methods[methods.size()-1].params.size()+1);
                    Parameter *p = &(methods[methods.size()-1].params[
                            methods[methods.size()-1].params.size()-1]);
                    
                    if (param.hasAttribute("mode"))
                    {
                        p->mode = (param.attribute("mode") == "in") ? IN : OUT;
                    }
					else 
                    {
                        p->mode = UNSPECIFIED;
                    }
                    p->name = param.attribute("name");
                    p->type = param.attribute("type");
                }
            }
        }
    }
}


#if 0
//middle click to edit contents, scroll wheel to adjust detail level
void UMLClass::handler(GuiObj **object_addr, int action)
{
    GuiObj *object = *object_addr;
    UMLClass *uml = dynamic_cast<UMLClass *> (object);

    if (action == MOUSE_MCLICKUP)
    {
        if( gmlGraph ) {
            gmlGraph->expandNeighbours(uml);
        } else {
            uml->middle_click(mouse.x, mouse.y);
        }
    }
    else if (action == MOUSE_WHEELUP)
    {
        uml->setMode(NOABBREV);
        uml->change_detail_level(true);
    }
    else if (action == MOUSE_WHEELDOWN)
    {
        uml->setMode(NOABBREV);
        uml->change_detail_level(false);
    }
    else
    {
        shape_handler(object_addr, action);
    }
}


void UMLClass::change_label(void)
{
    do_edit(EDIT_CLASS_NAME);
}


//middle click will open a text box for editing, section edited depends on
//mouse co-ordinates
void UMLClass::middle_click(const int& mouse_x, const int& mouse_y)
{
    if (mode == CLASS_NAME_ONLY)
        do_edit(EDIT_CLASS_NAME);
    else
    {
        if (mouse_y - ypos  - this->get_parent()->get_absypos() < (int) get_class_name_section_height())
            do_edit(EDIT_CLASS_NAME);
        else if (mouse_y - ypos - this->get_parent()->get_absypos() < (int) get_class_name_section_height() +
                                                     (int) get_attr_section_height())
            do_edit(EDIT_ATTRIBUTES);
        else
            do_edit(EDIT_METHODS);
    }
}
#endif

//after editing, parse the contents of the text area and update class contents
//no seperate tokeniser. If the grammars are extended it may be worth using one.
//information that could not be parsed is simply left blank.
void ClassShape::update_contents(UML_Class_Edit_Type edit_type, const std::vector<QString>& lines, const bool store_undo)
{
    Q_UNUSED (store_undo)
    if (edit_type == EDIT_CLASS_NAME)
	{
                char *s = strdup(lines[0].toUtf8().constData());
		if (*s == '<' && *(s+1) == '<')
		{
			s+=2;
			char *stype = s;    //beginning of stereotype
			
			while (*s != '>') s++;
			*s = '\0';
			class_stereotype = stype;
			s += 2;
			while (*s == ' ') s++;
			class_name = s;
		}
		else
			class_name = lines[0];
	}
    else if (edit_type == EDIT_ATTRIBUTES)
    {
        unsigned int i;

        if (lines.size() == 1 && lines[0].length() == 0)
            attributes.resize(0);
        else
            attributes.resize(lines.size());

        for (i=0; i<lines.size(); i++)
        {
            char *s = strdup(lines[i].toUtf8().constData());

            //optionally a stereotype first
            if (*s == '<' && *(s+1) == '<')
            {
                s += 2;
                char *stype = s;    //beginning of stereotype

                while (*s != '>') s++;
                *s = '\0';
                attributes[i].stereotype = stype;
                s += 2;
                while (*s == ' ') s++;

            }
            else
            {
                attributes[i].stereotype = "";
            }

            while (*s == ' ') s++;

            if (*s == '+' || *s == '-')
            {
                attributes[i].is_public = (*s == '+');
                s++;
                while (*s == ' ') s++;
            }
                   

            //at the start of a word
            char *attr_name = s;

            //find end of word
            while (*s != ' ' && *s != '\0' && *s != ':') s++;

            if (*s == '\0')
                attributes[i].name = attr_name;
            else
            {
                int ch = *s;
                *s = '\0'; 
                attributes[i].name = attr_name;
                *s = ch;
                while (*s == ' ') s++;
                if (*s == ':')
                {
                    s++;
                    while (*s == ' ') s++;
                    char *attr_type = s;
                    while (*s != ' ' && *s != '\0') s++;
                    if (*s != '\0') 
                        *s = '\0';
                    attributes[i].type = attr_type;
                }
                else
                    attributes[i].type = "";
            }

        }
    }
    else if (edit_type == EDIT_METHODS)
    {
        unsigned int i;
        methods.resize(lines.size());
        for (i=0; i<lines.size(); i++)
        {
            char *s = strdup(lines[i].toUtf8().constData());

            while (*s == ' ') s++;

            if (*s == '+' || *s == '-')
            {
                methods[i].is_public = (*s == '+');
                s++;
                while (*s == ' ') s++;

            }

            //at the start of a word
            char *method_name = s;

            //find end of word
            while (*s != ' ' && *s != '\0' && *s != '(') s++;

            int ch = *s;
            *s = '\0'; 
            methods[i].name = method_name;
            *s = ch;

            while (*s == ' ') s++;
            if (*s == '(')
            {
                int j = 0;
                methods[i].params.resize(0);
                s++;
                while (*s == ' ') s++;

                while (*s != ')' && *s != '\0')
                {
                    methods[i].params.resize(j+1);

                    //parse param
                    while (*s == ' ') s++;
                    if (*s == ',') s++;
                    while (*s == ' ') s++;       

                    //optionally an 'in' or 'out' first.
                    if (*s == 'i' && *(s+1) == 'n')
                    {
                        methods[i].params[j].mode = IN;
                        s += 2;
                        while (*s == ' ') s++;
                    }
                    else if (*s == 'o' && *(s+1) == 'u' && *(s+2) == 't')
                    {
                        methods[i].params[j].mode = OUT;
                        s += 3;
                        while (*s == ' ') s++;
                    }
                    else
                        methods[i].params[j].mode = UNSPECIFIED;

                    char *param_name = s;

                    while (*s != ' ' && *s != '\0' && *s != ':' && *s != ')') s++;

                    //record param name
                    int ch = *s;
                    *s = '\0'; 
                    methods[i].params[j].name = param_name;
                    *s = ch;
                    while (*s == ' ') s++;
                    
                    //param type
                    if (*s == ':')
                    {
                        s++;
                        while (*s == ' ') s++;
                        char *param_type = s;
                        while (*s != ' ' && *s != '\0' && *s != ')' && *s != ',') s++;
                        
                        //record param type
                        ch = *s;
                        *s = '\0';
                        methods[i].params[j].type = param_type;
                        *s = ch;
                    }
                    else
                        methods[i].params[j].type = "";
                    while (*s == ' ') s++;
                    j++;
                }
                if (*s == ')') s++;
                while (*s == ' ') s++;
            }
            
            //method type
            if (*s == ':')
            {
                s++;
                while (*s == ' ') s++;
                char *method_type = s;
                while (*s != ' ' && *s != '\0') s++;
                if (*s != '\0') 
                    *s = '\0';
                methods[i].return_type = method_type;
            }
            else
                methods[i].return_type = "";
        }
    }
#if 0
    //QT

    //check to see if a resize is needed
    int new_width, new_height;
    determine_best_dimensions(&new_width, &new_height);
    setPosAndSize(QPointF(xpos, ypos), QSizeF(new_width+16, new_height));
#endif
#if 0
    mathematicallyRepositionLabels();
    GraphLayout::getInstance()->setRestartFromDunnart();
    repaint_canvas();
#endif
}


//bring up a text box for editing
void ClassShape::do_edit(UML_Class_Edit_Type edit_type)
{
    Q_UNUSED (edit_type)
#if 0
    int fypos = 0;
    int fxpos = absxpos + (width / 2);
    FieldLines lines;

    int texth = TTF_FontHeight(mono);
    int textw;
    int padding = 7;
    int fheight = texth + padding;
    int fwidth = 360;
    
    int lineCount = 1;
    int extraLines = 3;
    if (edit_type == EDIT_CLASS_NAME)
    {
        lines.push_back(class_name);
        if (mode == CLASS_NAME_ONLY)
        {
            fypos = absypos + (height / 2) - (fheight / 2);
        }
        else
        {
            fypos = absypos + 5;
        }
    }
    if (edit_type == EDIT_ATTRIBUTES)
    {
        lines.resize(attributes.size());
        for (unsigned int i = 0; i < attributes.size(); ++i)
        {
            QString str = attribute_to_string(i);
            lines[i] = str;
            TTF_SizeText(mono, str.c_str(), &textw, NULL);
            fwidth = std::max(fwidth, textw);
        }
        fypos = absypos + 25;
        lineCount = attributes.size() + extraLines;
    }
    if (edit_type == EDIT_METHODS)
    {
        lines.resize(methods.size());
        for (unsigned int i = 0; i < methods.size(); ++i)
        {
            QString str = method_to_string(i);
            lines[i] = str;
            TTF_SizeText(mono, str.c_str(), &textw, NULL);
            fwidth = std::max(fwidth, textw);
        }
        fypos = absypos + 27 + ((height - 23) / 2);
        lineCount = methods.size() + extraLines;
    }
    // Set minimum number of lines if inputing methods or attributes, or
    // use the current number of lines plus a couple of extra:
    if (edit_type != EDIT_CLASS_NAME) lineCount = std::max(13, lineCount);
    fheight = (lineCount * texth) + padding;
    fwidth += padding + 40;


    fxpos -= (fwidth / 2);

    int screenPadding = 8;
    // Make sure the text field doesn't go off the screen:
    fxpos = std::min(fxpos, screen->w - (fwidth + screenPadding));
    fxpos = std::max(fxpos, screenPadding);
    fypos = std::min(fypos, screen->h - (fheight + screenPadding));
    fypos = std::max(fypos, screenPadding);
    

    Field *field = new Field(NULL, fxpos, fypos, fwidth, fheight);
    if (edit_type == EDIT_CLASS_NAME)
    {
        // Limit field to one line.
        field->setLimits(1, 0);
    }
    field->setFont(mono);
    field->setValue(lines);
    SDL_FastFlip(screen);
    bool modified = field->editText();

    if (modified)
    {
        //Save undo information:
        // canvas()->beginUndoMacro(tr("Change Label Property"));
        //add_undo_record(DELTA_LABEL, this);
        //end_undo_scope();
        FieldLines& lines = field->getLines();
        update_contents(edit_type, lines);
    }
    delete field;
#endif
}

int ClassShape::get_longest_text_width(UML_Class_Abbrev_Mode mode)
{
    int i, j, longest, width;

        QString s;
	
    //name of class.
	s = class_stereotype.length() > class_name.length() ? "<<" + class_stereotype + ">>" : class_name;
    get_text_width(s, &longest);

    if (mode == CLASS_NAME_ONLY)
        return longest;
    
    //attributes
    for (i=0; i<(int)attributes.size(); i++)
    {
        if (attributes[i].stereotype.length() > 0)
            s = "<<" + attributes[i].stereotype + ">> ";
        else
            s = "";
        s += attributes[i].name;
        if (mode < NO_TYPES)
            s += " : " + attributes[i].type;
        get_text_width(s, &width);
        longest=std::max(longest, width+9);      //the + or - takes up 9 pixels.
    } 

    //methods
    for (i=0; i<(int)methods.size(); i++)
    {
        s = methods[i].name + "(";
        if (mode < NO_PARAMS)
        {
            for (j = 0; j < (int)methods[i].params.size(); j++)
            {
                s += j == 0 ? "" : ", ";
                if (methods[i].params[j].mode == IN)
                    s += "in ";
                if (methods[i].params[j].mode == OUT)
                    s += "out ";
                s += methods[i].params[j].name;
                if (mode < NO_PARAM_TYPES)
                    s += ":" + methods[i].params[j].type;

            }
        }
        s += ")";        

        if (mode < NO_TYPES)
            s += ":" + methods[i].return_type;
        get_text_width(s, &width);
        longest=std::max(longest, width+9); //the + or - takes up 9 pixels..
    } 
    
    return longest;
}

void ClassShape::determine_best_mode(void)
{
    //width irrelevant if height too small.
    if (height() < (int) get_class_name_section_height() + (HANDLE_PADDING * 2))
    {
        mode = CLASS_NAME_ONLY;
        detailLevel = 0;
        return;
    }

    UML_Class_Abbrev_Mode i;
    for (i = NOABBREV; i < CLASS_NAME_ONLY; i = (UML_Class_Abbrev_Mode) (i+1))
    {
        if (get_longest_text_width(i) <= width() - 15)
        {
            mode = i;
            return;
        }
    }
    mode = CLASS_NAME_ONLY;
}


void ClassShape::determine_small_dimensions(int *w, int *h)
{
    *w = 22;
    *h = 7;
}


void ClassShape::determine_best_dimensions(int *inner_width, int *h)
{
    *inner_width = get_longest_text_width(mode) + 15;
    *h = get_class_name_section_height();
    if (mode != CLASS_NAME_ONLY)
    {
        attr_section_size = std::max(classLineHeight, 
                (int)attributes.size()*classLineHeight) + 2;
        method_section_size = std::max(classLineHeight, 
                (int)methods.size()*classLineHeight) + 2;
        *h += attr_section_size + method_section_size;
    }
}


#if 0
void UMLClass::draw(QPixmap *surface, const int x, const int y,
        const int type, const int w, const int h)
{
	if (!surface)
	    return;

	Rect::draw(surface, x, y, type, w, h);
    const QColor black = Qt::black;
    SDL_Color SDL_black = {0x00, 0x00, 0x00, 0};

    int classSectHeight = get_class_name_section_height();

    if (detailLevel > 0)
    {
	    Draw_HLine(surface, x+3, y+classSectHeight, x+w-6, black);
	    Draw_HLine(surface, x+3, y+classSectHeight+get_attr_section_height(), 
                x+w-6, black);

        //print class name in top section
		if (class_stereotype.length() > 0)
		{
                        QString s = "<<" + class_stereotype + ">>";
			center_text(surface, x+6, x+w-7, y+5, y+19, 
						s.c_str());
			center_text(surface, x+6, x+w-7, y+19, y+33, 
						class_name.c_str());
		}
		else
			center_text(surface, x+6, x+w-7, y+5, y+21, 
						class_name.c_str());

        QString s;
        unsigned int lines = get_attr_section_height() / classLineHeight;
        for (unsigned int i = 0; (i < lines) && (i < attributes.size()); ++i)
        {
            int lineYPos = y+classSectHeight+1+i*classLineHeight;
            if ((i == lines - 1) && (i < attributes.size() - 1))
            {
                // Last line, but not the last attribute, so print "...":
                SDL_WriteText(surface, x+15, 
                        lineYPos, "...", &SDL_black, mono);
                break;
            }
            if (attributes[i].stereotype.length() > 0)
                s = "<<" + attributes[i].stereotype + ">> ";
            else
                s = "";
            s += attributes[i].name;
            if (mode < NO_TYPES)
                s += ":" + attributes[i].type;
            if (attributes[i].stereotype.length() > 0)
            {
                SDL_WriteText(surface, x+6, lineYPos, s.c_str(), 
                        &SDL_black, mono);
            }
            else
            {
                SDL_WriteText(surface, x+6, lineYPos, 
                              attributes[i].is_public ? "+" : "-",
                              &SDL_black, mono);
                SDL_WriteText(surface, x+15, lineYPos, 
                              s.c_str(), &SDL_black, mono);
            }
        }
        
        lines = get_method_section_height() / classLineHeight;
        for (unsigned int i = 0; (i < lines) && (i < methods.size()); ++i)
        {
            int lineYPos = y+classSectHeight+1+i*classLineHeight + 
                    get_attr_section_height();
            if ((i == lines - 1) && (i < methods.size() - 1))
            {
                // Last line, but not the last method, so print "...":
                SDL_WriteText(surface, x+15, lineYPos, 
                        "...", &SDL_black, mono);
                break;
            }
            s = methods[i].name + "(";
            unsigned int j;
            if (mode < NO_PARAMS)
            {
                for (j = 0; j < methods[i].params.size(); j++)
                {
                    s += j == 0 ? "" : ", ";
                    if (methods[i].params[j].mode == IN)
                        s += "in ";
                    if (methods[i].params[j].mode == OUT)
                        s += "out ";
                    s += methods[i].params[j].name;
                    if (mode < NO_PARAM_TYPES)
                        s += ":" + methods[i].params[j].type;
                };
            }

            s += ")";
            if (mode < NO_TYPES && methods[i].return_type.length() > 0)
                s += ":" + methods[i].return_type;

            SDL_WriteText(surface, x+6, lineYPos, 
                    methods[i].is_public ? "+" : "-", &SDL_black, mono);
            SDL_WriteText(surface, x+15, lineYPos, s.c_str(), 
                    &SDL_black, mono);
        }
    }
    else
    {
        //print class name in center
		if (class_stereotype.length() > 0)
		{
                        QString s = "<<" + class_stereotype + ">>";
			center_text(surface, x+6, x+w-7, y+5, y+h-23, 
						s.c_str());
			center_text(surface, x+6, x+w-7, y+19, y+h-9, 
						class_name.c_str());
		}
		else
			center_text(surface, x+6, x+w-7, y+5, y+h-9, 
						class_name.c_str());
    }

}
#endif


QString ClassShape::attribute_to_string(int i, UML_Class_Abbrev_Mode mode)
{
    QString s;
    if (attributes[i].stereotype.length() > 0)
        s = "<<" + attributes[i].stereotype + ">> ";
    else
        s = attributes[i].is_public ? "+" : "-";
    s += attributes[i].name;
    if (mode < NO_TYPES)
        s += ":" + attributes[i].type;
    return s;
}

QString ClassShape::method_to_string(int i, UML_Class_Abbrev_Mode mode)
{
    QString s;
    s = (methods[i].is_public ? "+" : "-") + methods[i].name + "(";
    unsigned int j;
    if (mode < NO_PARAMS)
    {
        for (j = 0; j < methods[i].params.size(); j++)
        {
            s += j == 0 ? "" : ", ";
            if (methods[i].params[j].mode == IN)
                s += "in ";
            if (methods[i].params[j].mode == OUT)
                s += "out ";
            s += methods[i].params[j].name;
            if (mode < NO_PARAM_TYPES)
                s += ":" + methods[i].params[j].type;
        };
    }

    s += ")";
    if (mode < NO_TYPES && methods[i].return_type.length() > 0)
        s += ":" + methods[i].return_type;
    return s;
}

QString ClassShape::class_name_to_string(bool one_line)
{
	return class_stereotype.length() > 0 ? "<<" + class_stereotype + ">>" + (one_line ? "" : "\n") + class_name : class_name;
}

template <typename T>
QDomElement newTextChild(QDomElement& node, const QString& ns, 
        const QString& name, T arg, QDomDocument& doc)
{
    Q_UNUSED (ns)

    QByteArray bytes;
    QDataStream o(&bytes, QIODevice::WriteOnly);
    o << arg;
    
    QDomElement new_node = doc.createElement(name);
    node.appendChild(new_node);
    
    QDomText text = doc.createTextNode(bytes);
    new_node.appendChild(text);

    return new_node;
}


QDomElement ClassShape::to_QDomElement(const unsigned int subset,
        QDomDocument& doc)
{
    QDomElement groupnode = doc.createElement("g");
    QDomElement node = doc.createElement("rect");
    groupnode.appendChild(node);

    if (subset & XMLSS_IOTHER)
    {
        newNsProp(node, x_dunnartNs, x_type, x_shUMLClass);
    }

    addXmlProps(subset, node, doc);
    
    if (subset & XMLSS_IOTHER)
    {
        /* UML Class specific xml (represented as children */
        //class name    
        newTextChild(node, x_dunnartNs, "class_name", class_name, doc);
        newTextChild(node, x_dunnartNs, "class_stereotype", class_stereotype,
                doc);

        //class mode
        newTextChild(node, x_dunnartNs, "mode", (int) mode, doc); 

        //section heights
        newTextChild(node, x_dunnartNs, 
                "attr_section_height", attr_section_size, doc);
        newTextChild(node, x_dunnartNs, 
                "method_section_height", method_section_size, doc);
        
        //attributes
        for (unsigned int i=0; i<attributes.size(); i++)
        {
            QDomElement attr_node = doc.createElement("dunnart:attribute");
            if (attributes[i].stereotype.length() > 0)
            {
                newProp(attr_node, "stereotype", attributes[i].stereotype);
            }
            newProp(attr_node, "private", 
                    attributes[i].is_public ? "no" : "yes");
            newProp(attr_node, "name", attributes[i].name);
            newProp(attr_node, "type", attributes[i].type);
            node.appendChild(attr_node);
        }
        for (unsigned int i=0; i<methods.size(); i++)
        {
            QDomElement method_node = doc.createElement("dunnart:method");
            newProp(method_node, "private", 
                    methods[i].is_public? "no" : "yes");
            newProp(method_node, "name", methods[i].name);
            newProp(method_node, "return_type", methods[i].return_type);

            for (unsigned int j=0; j<methods[i].params.size(); j++)
            {
                QDomElement param_node = doc.createElement("dunnart:parameter");
                if (methods[i].params[j].mode != UNSPECIFIED)
                {
                    newProp(param_node, "mode", 
                            methods[i].params[j].mode == IN ? "in" : "out");
                }
                newProp(param_node, "name", methods[i].params[j].name);
                newProp(param_node, "type", methods[i].params[j].type);
                method_node.appendChild(param_node);
            }
            node.appendChild(method_node);
        }
    }

#if 0
    // XML
    if (subset & XMLSS_ISVG)
    {
        double x, y, w, h;
        getPosAndSize(x, y, w, h);

        sprintf(value, "%.10g", w);
        newProp(node, "width", value);
        sprintf(value, "%.10g", h);
        newProp(node, "height", value);
        sprintf(value, "%.10g", x);
        newProp(node, "x", value);
        sprintf(value, "%.10g", y);
        newProp(node, "y", value);

        /* add svg versions of class contents to group node here */
        if (detailLevel > 0)
        {
            nodePtr line = newNode(NULL, "line");

            sprintf(value, "%.10g", x);
            newProp(line, "x1", value);
            sprintf(value, "%.10g", y+h-attr_section_size-method_section_size);
            newProp(line, "y1", value);
            sprintf(value, "%.10g", x+w);
            newProp(line, "x2", value);
            sprintf(value, "%.10g", y+h-attr_section_size-method_section_size);
            newProp(line, "y2", value);
            newProp(line, "style", "stroke:rgb(0,0,0;stroke-width:1px"));

            groupnode.appendChild(line);

            line = newNode(NULL, "line");

            sprintf(value, "%.10g", x);
            newProp(line, "x1", value);
            sprintf(value, "%.10g", y+h-method_section_size);
            newProp(line, "y1", value);
            sprintf(value, "%.10g", x+w);
            newProp(line, "x2", value);
            sprintf(value, "%.10g", y+h-method_section_size);
            newProp(line, "y2", value);
            newProp(line, "style", "stroke:rgb(0,0,0;stroke-width:1px"));

            groupnode.appendChild(line);

            int tw;
            int hoffset = 0;
            if (class_stereotype.length() > 0)
            {
                hoffset = 14;
                QString s = "<<" + class_stereotype + ">>";
                
                nodePtr class_stereo = newNode(NULL, "text");
                newProp(class_stereo, "class", "umlText");
                newProp(class_stereo, "style", 
                        "text-anchor: middle; text-align: center;");
                
                sprintf(value, "%.10g", x+(w/2));       //centering of text.
                newProp(class_stereo, "x", value);
                sprintf(value, "%.10g", y+11);
                newProp(class_stereo, "y", value);
                
                class_stereo.setNodeValue(s.c_str());
                groupnode.appendChild(class_stereo);			
            }
            nodePtr class_text = newNode(NULL, "text");
            newProp(class_text, "class", "umlText");
            newProp(class_text, "style", 
                    "text-anchor: middle; text-align: center;");


            get_text_width(class_name.c_str(), &tw);
            sprintf(value, "%.10g", x+ w/2);       //centering of text.
            newProp(class_text, "x", value);
            sprintf(value, "%.10g", y+11+hoffset);
            newProp(class_text, "y", value);

            class_text.setNodeValue(class_name.c_str());
            groupnode.appendChild(class_text);
            //here: end of class text.

            unsigned int lines = get_attr_section_height() / classLineHeight;
            for (unsigned int i = 0; (i < lines) && (i < attributes.size()); ++i)
            {
                double lineYPos = y+get_class_name_section_height()+
                        5+i*classLineHeight;
                if ((i == lines - 1) && (i < attributes.size() - 1))
                {
                    nodePtr dotdotdot = newNode(NULL, "text");
                    newProp(dotdotdot, "class", 
                            "umlText");

                    sprintf(value, "%.10g", x+15);
                    newProp(dotdotdot, "x", value);
                    sprintf(value, "%.10g", lineYPos);
                    newProp(dotdotdot, "y", value);
                    
                    dotdotdot.setNodeValue("...");
                    groupnode.appendChild(dotdotdot);
                    break;
                }
                
                nodePtr attr_text = newNode(NULL, "text");
                newProp(attr_text, "class", "umlText");
                sprintf(value, "%.10g", x+6);
                newProp(attr_text, "x", value);
                sprintf(value, "%.10g", lineYPos);
                newProp(attr_text, "y", value);
                
                attr_text.setNodeValue(attribute_to_string(i, mode.c_str()));
                groupnode.appendChild(attr_text);
            }
            lines = get_method_section_height() / classLineHeight;
            for (unsigned int i = 0; (i < lines) && (i < methods.size()); ++i)
            {
                double lineYPos = y+get_class_name_section_height()+
                        5+i*classLineHeight + get_attr_section_height();
                if ((i == lines - 1) && (i < methods.size() - 1))
                {
                    nodePtr dotdotdot = newNode(NULL, "text");
                    newProp(dotdotdot, "class", 
                            "umlText");

                    sprintf(value, "%.10g", x+15);
                    newProp(dotdotdot, "x", value);
                    sprintf(value, "%.10g", lineYPos);
                    newProp(dotdotdot, "y", value);
                    
                    dotdotdot.setNodeValue("...");
                    groupnode.appendChild(dotdotdot);
                    break;
                }
                nodePtr method_text = newNode(NULL, "text");
                newProp(method_text, "class", "umlText");
                
                sprintf(value, "%.10g", x+6);
                newProp( method_text, "x", value);
                sprintf(value, "%.10g", lineYPos);
                newProp( method_text, "y", value);
                
                 method_text.setNodeValue(
                        method_to_string(i, mode.c_str()));
                groupnode.appendChild( method_text);
            }
        }
        else
        {
            //class name only, in the centre.
            
            int tw, th;
            if (class_stereotype.length() > 0)
            {
                QString s = "<<" + class_stereotype + ">>";
                get_text_dimensions(s.c_str(), &tw, &th);
                
                nodePtr class_stereo = newNode(NULL, "text");
                newProp(class_stereo, "class", "umlText");
                
                sprintf(value, "%.10g", x+8+(w-15-tw)/2);       //centering of text.
                newProp(class_stereo, "x", value);
                sprintf(value, "%.10g", y+10+(h-th)/2-7);
                newProp(class_stereo, "y", value);
                
                class_stereo.setNodeValue(s.c_str());
                groupnode.appendChild(class_stereo);			
            }
            int toffset = class_stereotype.length() > 0 ? 7 : 0;
            get_text_dimensions(class_name.c_str(), &tw, &th);

            nodePtr class_text = newNode(NULL, "text");
            newProp(class_text, "class", "umlText");

            sprintf(value, "%.10g", x+8+(w-15-tw)/2);       //centering of text.
            newProp(class_text, "x", value);
            sprintf(value, "%.10g", y+10+(h-th)/2+toffset);
            newProp(class_text, "y", value);

            class_text.setNodeValue(class_name.c_str());
            groupnode.appendChild(class_text);

        }
    }
#endif

    return groupnode;
}


#if 0
static void center_text(QPixmap* surface, const int left, const int right, 
    const int top, const int bottom, const char* text)
{
    SDL_Color SDL_black = {0x00, 0x00, 0x00, 0};
    int tw, th;
    get_text_dimensions(text, &tw, &th);
    SDL_WriteText(surface, left + (right-(left+tw))/2,
                  top + (bottom-(top+th))/2, text, &SDL_black, mono);
}
#endif


static void get_text_dimensions(QString text, int *w, int *h)
{
    Q_UNUSED (text)
    Q_UNUSED (w)
    Q_UNUSED (h)
    //QT TTF_SizeText(mono, text, w, h);
}

static void get_text_width(QString text, int *w)
{
    int h;
    get_text_dimensions(text, w, &h);
}

void ClassShape::do_init()
{
    //QT set_handler(UMLClass::handler);
    mode_changed_manually = false;

    class_name = "Class";
    
    attributes.resize(0);
    methods.resize(0);
    mode = CLASS_NAME_ONLY;

    if (!mono)
    {
       //QT  mono = FONT_LoadTTF("VeraMono.ttf", umlFontSize);
    }
}

int ClassShape::get_min_width()
{
    int w;
    get_text_width(class_name, &w);
    return w+10;
}


void ClassShape::on_resize(bool setDetailLevel)
{
    if (!setDetailLevel)
    {
        detailLevel = 50;
    }
    mode_changed_manually = true;
    determine_best_mode(); 
    int section_sizes(height() - (HANDLE_PADDING * 2) - 
            get_class_name_section_height());
    attr_section_size = 2 +
            std::max(classLineHeight, (int)attributes.size() * classLineHeight);
    method_section_size = 2 +
            std::max(classLineHeight, (int)methods.size() * classLineHeight);
    double proportion = attr_section_size / 
            (double) (attr_section_size + method_section_size);
    attr_section_size = static_cast<int>(proportion * section_sizes);
    method_section_size = section_sizes - attr_section_size;

    update();
}


unsigned int ClassShape::get_class_name_section_height(void)
{
	return minSectHeight + (class_stereotype.length() > 0 ? 14 : 0);
}

unsigned int ClassShape::get_attr_section_height(void)
{
    return attr_section_size;
}

unsigned int ClassShape::get_method_section_height(void)
{
    return method_section_size;
}


// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

