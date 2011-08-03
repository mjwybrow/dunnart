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

#include "libvpsc/rectangle.h"
#include "libvpsc/variable.h"
#include "libvpsc/constraint.h"
#include "libdunnartcanvas/template-constraints.h"

namespace dunnart {


class CmpVariablePos
{
    public:
        CmpVariablePos(vpsc::Variables& vars)
            : vars(vars)
        {
        }
        bool operator()(const unsigned int& lhs, const unsigned int& rhs)
        {
            return vars[lhs]->desiredPosition < vars[rhs]->desiredPosition;
        }
        
    private:
        const vpsc::Variables& vars;
};

// ----------------------------------------------------------------------
// LinearTemplate code:

void LinearTemplateConstraint::generateVariables(const vpsc::Dim dim, 
        vpsc::Variables& vars) 
{
    if (dim == _primaryDim)
    {
        // Create a variable representing the position of the template.
        variable = new vpsc::Variable(vars.size(),position,0.0001);
        if(isFixed) {
            variable->fixedDesiredPosition=true;
            variable->weight=100000;
        }
        vars.push_back(variable);
    }
    else
    {
        if (idList.size() >= 2)
        {
            //idList.sort(CmpVariablePos(vars));
            //sep = (vars[idList.back()]->desiredPosition - 
            //      vars[idList.front()]->desiredPosition) / (idList.size() - 1);
            //sep = 50;
        }
        // Create a variable representing the spacing distance for nodes
        // on the guideline
        //variable = new vpsc::Variable(vars.size(),length,0.0000001);
        //vars.push_back(variable);
    }
}


void LinearTemplateConstraint::generateSeparationConstraints(
        const vpsc::Dim dim, vpsc::Variables& vars, vpsc::Constraints& cs,
        std::vector<vpsc::Rectangle*>& bbs) 
{
    COLA_UNUSED(bbs);

    if (dim == _primaryDim)
    {
        assert(variable!=NULL);

        // The centre position for the template and for the guideline should 
        // be an equality with zero separation.
        cs.push_back(new vpsc::Constraint(variable,alignment->variable, 0, 
                true));
    }
    else
    {
        idList.sort(CmpVariablePos(vars));
        int lastId = -1;
        for (std::list<unsigned>::iterator curr = idList.begin(); 
                curr != idList.end(); ++curr)
        {
            if (lastId >= 0)
            {
                cs.push_back(new vpsc::Constraint(
                        vars[lastId],vars[*curr], sep, true));
            }
            lastId = (int) *curr;
        }

    }
}

void RectangleConstraint::generateVariables(const vpsc::Dim dim, 
        vpsc::Variables& vars) 
{
    if (dim == vpsc::XDIM)
    {
        // Create a variable representing the position of the template.
        xVariableLow = new vpsc::Variable(vars.size(),xLow,100000000);
        vars.push_back(xVariableLow);
        
        xVariableHigh = new vpsc::Variable(vars.size(),xHigh,100000000);
        vars.push_back(xVariableHigh);
    }
    else
    {
        // Create a variable representing the position of the template.
        yVariableLow = new vpsc::Variable(vars.size(),yLow,100000000);
        vars.push_back(yVariableLow);
        
        yVariableHigh = new vpsc::Variable(vars.size(),yHigh,100000000);
        vars.push_back(yVariableHigh);
    }
}


void RectangleConstraint::generateSeparationConstraints(
        const vpsc::Dim dim, vpsc::Variables& vars, vpsc::Constraints& cs,
        std::vector<vpsc::Rectangle*>& bbs) 
{
    COLA_UNUSED(bbs);

    double aligned = 0.05;
    double close = 20;

    if (dim == vpsc::XDIM)
    {
        for (size_t i = 0; i < idList.size(); ++i)
        {
            size_t index = idList[i];
            double currPos = vars[index]->desiredPosition;
            stateListX[i] = currPos; 
            //printf("X %g %g %g\n", currPos, xLow, xHigh);
            //printf("  %g %g %g\n", stateListY[i], yLow, yHigh);
            
            if ((fabs(stateListY[i] - yLow) < aligned) || 
                (fabs(stateListY[i] - yHigh) < aligned))
            {
                cs.push_back(new vpsc::Constraint(xVariableLow,
                            vars[index], 0, false));
                cs.push_back(new vpsc::Constraint(vars[index], 
                            xVariableHigh, 0, false));
            }
            else
            {
                double low = fabs(currPos - xLow);
                double high = fabs(currPos - xHigh);
                if ((low < high) && (low < close))
                {
                    cs.push_back(new vpsc::Constraint(xVariableLow,
                                vars[index], 0, true));
                }
                else if (high < close)
                {
                    cs.push_back(new vpsc::Constraint(xVariableHigh,
                                vars[index], 0, true));
                }
                else
                {
                    cs.push_back(new vpsc::Constraint(xVariableLow,
                                vars[index], 0, false));
                    cs.push_back(new vpsc::Constraint(vars[index], 
                                xVariableHigh, 0, false));
                }
            }
        }
    }
    else
    {
        for (size_t i = 0; i < idList.size(); ++i)
        {
            size_t index = idList[i];
            double currPos = vars[index]->desiredPosition;
            stateListY[i] = currPos; 
            //printf("Y %g %g %g\n", currPos, yLow, yHigh);
            //printf("  %g %g %g\n", stateListX[i], xLow, xHigh);

            if ((fabs(stateListX[i] - xLow) < aligned) || 
                (fabs(stateListX[i] - xHigh) < aligned))
            {
                cs.push_back(new vpsc::Constraint(yVariableLow,
                            vars[index], 0, false));
                cs.push_back(new vpsc::Constraint(vars[index], 
                            yVariableHigh, 0, false));
            }
            else
            {
                double low = fabs(currPos - yLow);
                double high = fabs(currPos - yHigh);
                if ((low < high) && (low < close))
                {
                    cs.push_back(new vpsc::Constraint(yVariableLow,
                                vars[index], 0, true));
                }
                else if (high < close)
                {
                    cs.push_back(new vpsc::Constraint(yVariableHigh,
                                vars[index], 0, true));
                }
                else
                {
                    cs.push_back(new vpsc::Constraint(yVariableLow,
                                vars[index], 0, false));
                    cs.push_back(new vpsc::Constraint(vars[index], 
                                yVariableHigh, 0, false));
                }
            }
        }
    }
}


// ----------------------------------------------------------------------
// BranchedTemplate code:

void BranchedTemplateConstraint::generateVariables(const vpsc::Dim dim,
        vpsc::Variables& vars) 
{
    if (dim == _primaryDim)
    {
        // Create a variable representing the position of the template.
        variable = new vpsc::Variable(vars.size(),position,0.0001);
        if(isFixed) {
            variable->fixedDesiredPosition=true;
            variable->weight=100000;
        }
        vars.push_back(variable);
    }
    else // no need to create any extra variable for shapes along the guides
    {
    }
}

void BranchedTemplateConstraint::generateSeparationConstraints(
        const vpsc::Dim dim, vpsc::Variables& vars, vpsc::Constraints& cs,
        std::vector<vpsc::Rectangle*>& bbs) 
{
    COLA_UNUSED(bbs);

    if (dim == _primaryDim)
    {
//       assert(variable!=NULL);
//       ACList::iterator curr1 = alignment.begin();
//       if(curr1 != alignment.end()) {
// 	ACList::iterator curr2 = alignment.begin();
// 	curr2++;
// 	while(curr2 != alignment.end()) {
// 	  cs.push_back(new vpsc::Constraint((*curr1)->variable, 
// 					    (*curr2)->variable, sep, true));
// 	  curr1++;
// 	  curr2++;
// 	}
//       }
    }
    else { // separate the nodes along the guide
        double end_sep = 100.0; // distance from centre guide end nodes to branch end nodes
    
         // sort each list of IDs for nodes on each guide by preferred location:
         for (size_t i = 0; i < idLists.size(); ++i) {
             std::list<unsigned>& idList = idLists[i];
             idList.sort(CmpVariablePos(vars));
         }

         // set the constraints on the centre guide so it is fully constrained:


        // check if the centre guide has a first node
        bool centreguide_hasfirst = false;
        bool centreguide_haslast = false;
        // find if there's a first:
        if (!idLists[centreguide_index].empty()) { centreguide_hasfirst = true; 
            // if there's a first, there might be a last
            if (idLists[centreguide_index].front() !=
                idLists[centreguide_index].back()) { centreguide_haslast = true; }
        }
                
        
        

        // for each list of IDs for nodes on each guide:
        for (size_t i = 0; i < idLists.size(); ++i) {
            // get all the nodes along the current guide:
            std::list<unsigned>& idList = idLists[i];

//printf("_________________ branch_sep = %g, %g, %d\n", branch_sep, separation, idList.size());
            
            // if this is the centre guide, make the nodes fully constrained:
            if ((int)i == centreguide_index) {
                // constraint first and last and distribute middle nodes:
                size_t counter = 0;
		if (idLists[i].size() > 2) {
                    // calculate the node separation for the centre guide
                    double branch_sep = separation/((int)idList.size()-3);
                    std::list<unsigned>::iterator curr = idList.begin();
                    curr++; // get the second node
		    counter++;
                    // constraint first node to second (middle) node:
                    cs.push_back(new vpsc::Constraint(
					  		  vars[idLists[centreguide_index].front()],
                              vars[*curr], end_sep, true));

                    // distribute the middle nodes
                    std::list<unsigned>::iterator prev = curr;
                    curr++;
		    counter++;
                    while (counter < (idLists[i].size() - 1)) {
                        cs.push_back(new vpsc::Constraint(
					  		  vars[*prev],
                              vars[*curr], branch_sep, true));
                        prev = curr;
                        curr++;
			counter++;
                    }

                    // now constrain the second-to-last (middle) node from
                    // the end node of the centre guide:
                    cs.push_back(new vpsc::Constraint(
                                     vars[idLists[centreguide_index].front()],
                                     vars[idLists[centreguide_index].back()], 
                                     end_sep * 2 + separation, true));
                } 
                // in this case there are two nodes, just separate them:
                else if (idLists[i].size() == 2) {
                     cs.push_back(new vpsc::Constraint(
					  		  vars[idLists[centreguide_index].front()],
                              vars[idLists[centreguide_index].back()], 
                              (separation+end_sep*2), true));
                }
            }

            // for all other guides, separate the nodes from the first node 
            // of the centre guide, and then distribute all the nodes evenly
            // along the guide:
            else {
                if (idList.size() > 0) {
                    std::list<unsigned>::iterator curr = idList.begin();

                    // constrain the first centreguide node to be to the 
                    // left of all the nodes on the other guides
                    if (idLists[centreguide_index].size() > 0) {
                        cs.push_back(new vpsc::Constraint(
					  		  vars[idLists[centreguide_index].front()],
                              vars[*curr], end_sep, true));
                    }

                    std::list<unsigned>::iterator prev = curr;
                    curr++;
                    // distribute the remaining nodes (if any) evenly:
                    while (curr != idList.end()) {
                        // calculate the node separation for the current guide
                        double branch_sep = separation/((int)idList.size()-1);
                        cs.push_back(new vpsc::Constraint(
                             vars[*prev],vars[*curr], branch_sep, true));
                        prev = curr;
                        curr++;
                    }
                }
            }
        } // end of processing current guide
    } // end of processing along-the-guide constraints
}

}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
