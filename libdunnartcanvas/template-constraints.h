/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2009-2010  Monash University
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


#ifndef _TEMPLATE_CONSTRAINTS_H
#define _TEMPLATE_CONSTRAINTS_H

#include "libcola/compound_constraints.h"
#include <list>
#include "libdunnartcanvas/templates.h"

namespace dunnart {


typedef std::list<cola::AlignmentConstraint*> ACList;

//============================================================================
// Linear Template

class LinearTemplateConstraint : public cola::CompoundConstraint 
{
 public:
    LinearTemplateConstraint(const vpsc::Dim dim, CompoundConstraint *cc,
                std::list<unsigned> idList, double sep) 
            : CompoundConstraint(dim),
              alignment(dynamic_cast<cola::AlignmentConstraint *> (cc)),
              position(alignment->position()),
              isFixed(false),
              variable(NULL),
              idList(idList),
              sep(sep)
        {
//            printf("---- %g\n", sep);
        }
        void generateVariables(const vpsc::Dim dim, vpsc::Variables& vars);
        void generateSeparationConstraints(const vpsc::Dim dim, 
                vpsc::Variables& vars, vpsc::Constraints& cs,
                std::vector<vpsc::Rectangle*>& bbs);
        void updatePosition(const vpsc::Dim dim) 
        {
            if (dim == _primaryDim)
            {
                position = variable->finalPosition;
            }
        }
        void fixPos(double pos) {
            position=pos;
            isFixed=true;
        }
        void unfixPos() {
            isFixed=false;
        }
        std::string toString(void) const
        {
            return "LinearTemplateConstraint()";
        }

        cola::SubConstraintAlternatives 
        getCurrSubConstraintAlternatives(vpsc::Variables vs[])
        {
            COLA_UNUSED(vs);

            fprintf(stderr,
                    "getCurrSubConstraintAlternatives() not implemented.\n");
            return cola::SubConstraintAlternatives();
        }

        cola::AlignmentConstraint *alignment;
        // the pointer is used by dunnart to keep a ref to its local
        // representation of the linear template
        void *indicator;
        
        // The position of the template
        double position;
        bool isFixed;
        vpsc::Variable* variable;

        std::list<unsigned> idList;
        double sep;
};


class RectangleConstraint : public cola::CompoundConstraint 
{
    public:
        RectangleConstraint(double xLow, double xHigh, double yLow, 
                double yHigh, std::vector<unsigned> idList) 
            : CompoundConstraint(vpsc::XDIM),
              xLow(xLow),
              xHigh(xHigh),
              yLow(yLow),
              yHigh(yHigh),
              xVariableLow(NULL),
              xVariableHigh(NULL),
              yVariableLow(NULL),
              yVariableHigh(NULL),
              idList(idList)
        {
            stateListX = std::vector<double>(idList.size(), 0); 
            stateListY = std::vector<double>(idList.size(), 0); 
        }
        void generateVariables(const vpsc::Dim dim, vpsc::Variables& vars);
        void generateSeparationConstraints(const vpsc::Dim dim, 
                vpsc::Variables& vars, vpsc::Constraints& cs,
                std::vector<vpsc::Rectangle*>& bbs);

        cola::SubConstraintAlternatives 
        getCurrSubConstraintAlternatives(vpsc::Variables vs[])
        {
            COLA_UNUSED(vs);

            fprintf(stderr,
                    "getCurrSubConstraintAlternatives() not implemented.\n");
            return cola::SubConstraintAlternatives();
        }
        std::string toString(void) const
        {
            return "RectangleConstraint()";
        }

        // the pointer is used by dunnart to keep a ref to its local
        // representation of the linear template
        void *indicator;
        
        // The position of the template
        double xLow, xHigh, yLow, yHigh;
        vpsc::Variable *xVariableLow, *xVariableHigh;
        vpsc::Variable *yVariableLow, *yVariableHigh;

        std::vector<unsigned> idList;
        std::vector<double> stateListX;
        std::vector<double> stateListY;
};


//============================================================================
// Branched Template

class BranchedTemplateConstraint : public cola::CompoundConstraint 
{
    public:
        BranchedTemplateConstraint(const vpsc::Dim dim, ACList &aclist, 
                double pos, double guide_sep, 
                std::vector< std::list<unsigned> > idLists,
                double separation, int cgi, int lbi)
            : CompoundConstraint(dim),
              alignment(aclist),
              guide_sep(guide_sep),
              position(pos),
              isFixed(false),
              variable(NULL),
              idLists(idLists),
              separation(separation),
              centreguide_index(cgi),
              longest_branch_index(lbi)
        {
        }

        void generateVariables(const vpsc::Dim dim, vpsc::Variables& vars);
        void generateSeparationConstraints(const vpsc::Dim dim, 
                vpsc::Variables& vars, vpsc::Constraints& cs,
                std::vector<vpsc::Rectangle*>& bbs);

        cola::SubConstraintAlternatives 
        getCurrSubConstraintAlternatives(vpsc::Variables vs[])
        {
            COLA_UNUSED(vs);

            fprintf(stderr,
                    "getCurrSubConstraintAlternatives() not implemented.\n");
            return cola::SubConstraintAlternatives();
        }

        void updatePosition(const vpsc::Dim dim) 
        {
            if (dim == _primaryDim)
            {
                position = variable->finalPosition;
            }
        }
        void fixPos(double pos) {
            position=pos;
            isFixed=true;
        }
        void unfixPos() {
            isFixed=false;
        }
        std::string toString(void) const
        {
            return "BranchedTemplateConstraint()";
        }

        ACList alignment; // list of Alignment Constraints
        double guide_sep; // separation value for the guides
        // the pointer is used by dunnart to keep a ref to its local
        // representation of the branched template
        void *indicator;
        
        // The position of the template
        double position;
        bool isFixed;
        vpsc::Variable* variable;

        // idList of nodes for each guide
        std::vector< std::list<unsigned> > idLists; 
        
        // node separation values for each idList
        //std::vector<double> separations; 
        double separation;
        
        int centreguide_index;
        int longest_branch_index;
        BranchedTemplate* templatPtr;
};


}
#endif
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

