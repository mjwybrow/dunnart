/*
 * $Revision: 1.9 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2007-11-12 15:59:17 +0100 (Mo, 12 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of Fast Multipole Multilevel Method (FM^3).
 * 
 * \author Stefan Hachul
 * 
 * \par License:
 * This file is part of the Open Graph Drawing Framework (OGDF).
 * Copyright (C) 2005-2007
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


#ifdef _MSC_VER
#pragma once
#endif
 
#ifndef FMMLAYOUT_H
#define FMMLAYOUT_H

#include <ogdf/basic/Graph.h>
#include <ogdf/module/LayoutModule.h>
#include <ogdf/basic/geometry.h>
#include <ogdf/internal/energybased/FruchtermanReingold.h>
#include <ogdf/internal/energybased/NMM.h>

  
namespace ogdf {

	class Rectangle;

/**
 * \brief The fast multipole multilevel layout algorithm.
 *
 * The class FMMLayout implements a force-directed graph drawing
 * method suited also for very large graphs. It is based on a
 * combination of an efficient multilevel scheme and a strategy for
 * approximating the repusive forcesin the system by rapidly
 * evaluating potential fields.
 *
 * The implementation is based on the following publication:
 *
 * Stefan Hachul, Michael Jünger: <i>Drawing Large Graphs with a
 * Potential-Field-Based Multilevel Algorithm</i>. 12th International
 * Symposium on %Graph Drawing 1998, New York (GD '04), LNCS 3383,
 * pp. 285-295, 2004.
 *
 * <H3>Optional parameters</H3>
 * The following options are the most important. You can set
 * useHighLevelOptions to true and just need to asjust a few parameters.
 * However, you can also adjust all parameters that the implementation
 * uses (see below), but this requires good knowledge of the algorithm.
 *
 * <table>
 *   <tr>
 *     <th><i>Option</i><th><i>Type</i><th><i>Default</i><th><i>Description</i>
 *   </tr><tr>
 *     <td><i>useHighLevelOptions</i><td>bool<td>false
 *     <td>Wether high-level options are used or not.
 *   </tr><tr>
 *     <td><i>pageFormat</i><td> #PageFormatType <td> #pfSquare
 *     <td>The desired apsect ratio of the layout.
 *   </tr><tr>
 *     <td><i>unitEdgeLength</i><td>double<td>100.0
 *     <td>The unit edge length.
 *   </tr><tr>
 *     <td><i>newInitialPlacement</i><td>bool<td>false
 *     <td>Specifies if initial placement of nodes is varied.
 *   </tr><tr>
 *     <td><i>qualityVersusSpeed</i><td> #QualityVsSpeed <td> #qvsBeautifulAndFast
 *     <td>Indicates if the algorithm is tuned either for best quality or best speed.
 *   </tr>
 * </table>
 *
 * If you want to do more detailed fine-tuning, you can adjust all parameters
 * used by the algorithm. Please refer to the paper cited above for better
 * understanding of the algorithm.
 *
 * <table>
 *   <tr>
 *     <th colspan="4" align="center"><b>General</b>
 *   </tr><tr>
 *     <td><i>randSeed</i><td>int<td>100
 *     <td>The seed of the random number generator.
 *   </tr><tr>
 *     <td><i>edgeLengthMeasurement</i><td> #EdgeLengthMeasurement <td> #elmBoundingCircle
 *     <td>Indicates how the length of an edge is measured.
 *   </tr><tr>
 *     <td><i>allowedPositions</i><td> #AllowedPositions <td> #apInteger
 *     <td>Defines which positions for a node are allowed.
 *   </tr><tr>
 *     <td><i>maxIntPosExponent</i><td>int<td>40
 *     <td>Defines the exponent used if allowedPositions == apExponent.
 *   </tr><tr>
 *     <th colspan="4" align="center"><b>Divide et impera step</b>
 *   </tr><tr>
 *     <td><i>pageRatio</i><td>double<td>1.0
 *     <td>The desired page ratio.
 *   </tr><tr>
 *     <td><i>stepsForRotatingComponents</i><td>int<td>10
 *     <td>The number of rotations per connected component.
 *   </tr><tr>
 *     <td><i>tipOverCCs</i><td> #TipOver <td> #toNoGrowingRow
 *     <td>Specifies when it is allowed to tip over drawings.
 *   </tr><tr>
 *     <td><i>minDistCC</i><td>double<td>100
 *     <td>The  minimal distance between connected components.
 *   </tr><tr>
 *     <td><i>presortCCs</i><td> #PreSort <td> #psDecreasingHeight
 *     <td>Defines if the connected components are sorted before
 *     the packing algorithm is applied.
 *   </tr><tr>
 *     <th colspan="4" align="center"><b>Multilevel step</b>
 *   </tr><tr>
 *     <td><i>minGraphSize</i><td>int<td>50
 *     <td>Determines the number of nodes of a graph in the for which
 *     no more collapsing of galaxies is performed
 *   </tr><tr>
 *     <td><i>galaxyChoice</i><td> #GalaxyChoice <td> #gcNonUniformProbLowerMass
 *     <td>Defines how sun nodes of galaxies are selected.
 *   </tr><tr>
 *     <td><i>randomTries</i><td>int<td>20
 *     <td>Defines the number of tries to get a random node with
 *     minimal star mass.
 *   </tr><tr>
 *     <td><i>maxIterChange</i><td> #MaxIterChange <td> #micLinearlyDecreasing
 *     <td>Defines how MaxIterations is changed in subsequent multilevels.
 *   </tr><tr>
 *     <td><i>maxIterFactor</i><td>int<td>10
 *     <td>Defines the factor used for decrasing MaxIterations.
 *   </tr><tr>
 *     <td><i>initialPlacementMult</i><td> #InitialPlacementMult <td> #ipmAdvanced
 *     <td>Defines how the initial placement is generated.
 *   </tr><tr>
 *     <th colspan="4" align="center"><b>Force calculation step</b>
 *   </tr><tr>
 *     <td><i>forceModel</i><td> #ForceModel <td> #fmNew
 *     <td>The used force model.
 *   </tr><tr>
 *     <td><i>springStrength</i><td>double<td>1.0
 *     <td>The strength of the springs.
 *   </tr><tr>
 *     <td><i>repForcesStrength</i><td>double<td>1.0
 *     <td>The strength of the repulsive forces.
 *   </tr><tr>
 *     <td><i>repulsiveForcesCalculation</i><td> #RepulsiveForcesMethod <td> #rfcNMM
 *     <td>Defines how to calculate repulsive forces.
 *   </tr><tr>
 *     <td><i>stopCriterion</i><td> #StopCriterion <td> #scFixedIterationsOrThreshold
 *     <td>The stop criterion.
 *   </tr><tr>
 *     <td><i>threshold</i><td>double<td>0.01
 *     <td>The threshold for the stop criterion.
 *   </tr><tr>
 *     <td><i>fixedIterations</i><td>int<td>30
 *     <td>The fixed number of iterations for the stop criterion.
 *   </tr><tr>
 *     <td><i>forceScalingFactor</i><td>double<td>0.05
 *     <td>The scaling factor for the forces.
 *   </tr><tr>
 *     <td><i>coolTemperature</i><td>bool<td>false
 *     <td>Use coolValue for scaling forces.
 *   </tr><tr>
 *     <td><i>coolValue</i><td>double<td>0.99
 *     <td>The value by which forces are decreased.
 *   </tr><tr>
 *     <td><i>initialPlacementForces</i><td> #InitialPlacementForces <td> #ipfRandomRandIterNr
 *     <td>Defines how the initial placement is done.
 *   </tr><tr>
 *     <th colspan="4" align="center"><b>Force calculation step</b>
 *   </tr><tr>
 *     <td><i>resizeDrawing</i><td>bool<td>true
 *     <td>Specifies if the resulting drawing is resized.
 *   </tr><tr>
 *     <td><i>resizingScalar</i><td>double<td>1
 *     <td>Defines a parameter to scale the drawing if resizeDrawing is true.
 *   </tr><tr>
 *     <td><i>fineTuningIterations</i><td>int<td>20
 *     <td>The number of iterations for fine tuning.
 *   </tr><tr>
 *     <td><i>fineTuneScalar</i><td>double<td>0.2
 *     <td>Defines a parameter for scaling the forces in the fine-tuning iterations.
 *   </tr><tr>
 *     <td><i>adjustPostRepStrengthDynamically</i><td>bool<td>true
 *     <td>If set to true, the strength of the repulsive force field is calculated.
 *   </tr><tr>
 *     <td><i>postSpringStrength</i><td>double<td>2.0
 *     <td>The strength of the springs in the postprocessing step.
 *   </tr><tr>
 *     <td><i>postStrengthOfRepForces</i><td>double<td>0.01
 *     <td>The strength of the repulsive forces in the postprocessing step.
 *   </tr><tr>
 *     <th colspan="4" align="center"><b>Repulsive force approximation methods</b>
 *   </tr><tr>
 *     <td><i>frGridQuotient</i><td>int<td>2
 *     <td>The grid quotient.
 *   </tr><tr>
 *     <td><i>nmTreeConstruction</i><td> #ReducedTreeConstruction <td> #rtcSubtreeBySubtree
 *     <td>Defines how the reduced bucket quadtree is constructed.
 *   </tr><tr>
 *     <td><i>nmSmallCell</i><td> #SmallestCellFinding <td> #scfIteratively
 *     <td>Defines how the smallest quadratic cell that surrounds
 *     the particles of a node in the reduced bucket quadtree is calculated.
 *   </tr><tr>
 *     <td><i>nmParticlesInLeaves</i><td>int<td>25
 *     <td>The maximal number of particles that are contained in
 *     a leaf of the reduced bucket quadtree.
 *   </tr><tr>
 *     <td><i>nmPrecision</i><td>int<td>4
 *     <td>The precision \a p for the <i>p</i>-term multipole expansions.
 *   </tr>
 * </table>
 *
 * <H3>Running time</H3>
 * The running time of the algorithm is
 * O(<i>n</i> log <i>n</i> + <i>m</i>) for graphs with \a n nodes
 * and \a m edges. The required space is linear in the input size.
 */
class FMMLayout : public LayoutModule
{
public:
    static const Graph::NodeType FIXED;
    static const Graph::NodeType UNFIXED;
	//! Possible page formats.
	enum PageFormatType {
		pfPortrait,  //!< A4 portrait page.
		pfLandscape, //!< A4 landscape page.
		pfSquare     //!< Square format.
	};

	//! Trade-off between run-time and quality.
	enum QualityVsSpeed { 
		qvsGorgeousAndEfficient,  //!< Best quality.
		qvsBeautifulAndFast,      //!< Medium quality and speed.
		qvsNiceAndIncredibleSpeed //!< Best speed.
	};

	//! Specifies how the length of an edge is measured.
	enum EdgeLengthMeasurement { 
		elmMidpoint,      //!< Measure from center point of edge end points.
		elmBoundingCircle //!< Measure from border of circle s surrounding edge end points.
	};

	//! Specifies which positions for a node are allowed.
	enum AllowedPositions { 
		apAll, 
		apInteger, 
		apExponent 
	};

	//! Specifies in which case it is allowed to tip over drawings of connected components. 
	enum TipOver { 
		toNone, 
		toNoGrowingRow, 
		toAlways 
	};

	//! Specifies how connected components are sorted before the packing algorithm is applied.
	enum PreSort { 
		psNone, //!< Do not presort.
		psDecreasingHeight, //!< Presort by decreasing height of components.
		psDecreasingWidth   //!< Presort by decreasing width of components.
	};

	//! Specifies how sun nodes of galaxies are selected.
	enum GalaxyChoice { 
		gcUniformProb, 
		gcNonUniformProbLowerMass, 
		gcNonUniformProbHigherMass 
	};

	//! Specifies how MaxIterations is changed in subsequent multilevels. 
	enum MaxIterChange { 
		micConstant, 
		micLinearlyDecreasing, 
		micRapidlyDecreasing 
	};

	//! Specifies how the initial placement is generated.
	enum InitialPlacementMult { 
		ipmSimple, 
		ipmAdvanced 
	};

	//! Specifies the force model.
	enum ForceModel { 
		fmFruchtermanReingold, //!< The force-model by Fruchterman, Reingold.
		fmEades,               //!< The force-model by Eades.
		fmNew                  //!< The new force-model.
	};

	//! Specifies how to calculate repulsive forces.
	enum RepulsiveForcesMethod { 
		rfcExact,             //!< Exact calculation.
		rfcGridApproximation, //!< Grid approximation.
		rfcNMM                //!< Calculation as for new multipole method.
	};

	//! Specifies the stop criterion.
	enum StopCriterion { 
		scFixedIterations,           //!< Stop if fixedIterations() is reached.
		scThreshold,                 //!< Stop if threshold() is reached.
		scFixedIterationsOrThreshold //!< Stop if fixedIterations() or threshold() is reached.
	};

	//! Specifies how the initial placement is done.
	enum InitialPlacementForces { 
		ipfUniformGrid,     //!< Uniform placement on a grid.
		ipfRandomTime,      //!< Random placement (based on current time).
		ipfRandomRandIterNr //!< Random placement (based on randIterNr()).
	};

	//! Specifies how the reduced bucket quadtree is constructed.
	enum ReducedTreeConstruction { 
		rtcPathByPath,      //!< Path-by-path construction.
		rtcSubtreeBySubtree //!< Subtree-by-subtree construction.
	};

	//! Specifies how to calculate the smallest quadratic cell surrounding particles of a node in the reduced bucket quadtree.
	enum SmallestCellFinding { 
		scfIteratively, //!< Iteratively (in constant time).
		scfAluru        //!< According to formula by Aluru et al. (in constant time).
	};
  

    //! Creates an instance of the layout algorithm.
    FMMLayout();
 
    // destructor
	~FMMLayout() { }
 
 
	/**
	 *  @name The algorithm call
	 *  @{
	 */

	//! Calls the algorithm for graph \a GA and returns the layout information in \a AG.
    void call(GraphAttributes &GA, char* ps_file);
    void call(GraphAttributes &GA);
 
    //! Extended algorithm call: Allows to pass desired lengths of the edges. 
    /**
	 * @param edgeLength is an edge array of the graph associated with \a GA
	 *        of positive edge length.
	 */
    void call(GraphAttributes &AG,   //graph and layout
		const EdgeArray<double> &edgeLength); //factor for desired edge lengths 

	/** @}
	 *  @name Further information.
	 *  @{
	 */

    //! Returns the runtime (=CPU-time) of the layout algorithm in seconds. 
    double getCpuTime();


	/** @}
	 *  @name High-level options
	 *  Allow to specify the most relevant parameters.
	 *  @{
	 */
  
    //! Returns the current setting of option useHighLevelOptions.
	/**
	 * If set to true, the high-level options are used to set all low-level options.
	 * Usually, it is sufficient just to set high-level options; if you want to
	 * be more specific, set this parameter to false and set the low level options.
	 */
    bool useHighLevelOptions() const { return m_useHighLevelOptions; }

	//! Sets the option useHighLevelOptions to \a uho.
    void useHighLevelOptions(bool uho) { m_useHighLevelOptions = uho; } 

    //! Returns the current setting of option pageFormat.
    /** 
	 * This option defines the desired aspect ratio of the drawing area.
	 *  - \a pfPortrait: A4 page in portrait orientation
     *  - \a pfLandscape: A4 page in landscape orientation
     *  - \a pfSquare: square page format
	 */
    PageFormatType pageFormat() const { return m_pageFormat; }

	//! Sets the option pageRatio to \a t.
    void pageFormat(PageFormatType t) { m_pageFormat = t; }
 
    //! Returns the current setting of option unitEdgeLength.
    double unitEdgeLength() const { return m_unitEdgeLength; }

	//! Sets the option unitEdgeLength to \a x.
    void unitEdgeLength(double x) {m_unitEdgeLength = (( x > 0.0) ? x : 1);}

	//! Returns the current setting of option newInitialPlacement.
    /**
	 * This option defines if the initial placement of the nodes at the
	 * coarsest multilevel is varied for each distinct call of FMMLayout
	 * or keeps always the same. 
	 */
    bool newInitialPlacement() const { return m_newInitialPlacement; }

	//! Sets the option newInitialPlacement to \a nip.
    void newInitialPlacement(bool nip) { m_newInitialPlacement = nip; }
  
    //! Returns the current setting of option qualityVersusSpeed.
	/**
	 * Indicates if the algorithm is tuned either for best quality or best speed. 
     *  - \a qvsGorgeousAndEfficient: gorgeous quality and efficient speed
     *  - \a qvsBeautifulAndFast: beautiful quality and fast speed
     *  - \a qvsNiceAndIncredibleSpeed: nice quality and incredible speed
	 */
    QualityVsSpeed qualityVersusSpeed() const { return m_qualityVersusSpeed; }

	//! Sets the option qualityVersusSpeed to \a qvs.
    void qualityVersusSpeed(QualityVsSpeed qvs) {m_qualityVersusSpeed = qvs; }
        

	/** @}
	 *  @name General low-level options
	 * The low-level options in this and the following sections are meant for
	 * experts or interested people only.
	 *  @{
	 */

    //! Sets the seed of the random number generator. 
    void randSeed(int p) { m_randSeed = ((0<=p) ? p : 1);}

	//! Returns the seed of the random number generator.
    int randSeed() const {return m_randSeed;}
         
    //! Returns the current setting of option edgeLengthMeasurement.
	/**
	 * This option indicates how the length of an edge is measured.
	 * Possible values:
     *   - \a elmMidpoint: from center to center
     *   - \a elmBoundingCircle: the distance between the two tight circles bounding the 
     *     graphics of two adjacent nodes
	 */
    EdgeLengthMeasurement edgeLengthMeasurement() const {
		return m_edgeLengthMeasurement;
	}

	//! Sets the option edgeLengthMeasurement to \a elm.
    void edgeLengthMeasurement(EdgeLengthMeasurement elm) { m_edgeLengthMeasurement = 
							       elm; }
 
    //! Returns the current setting of option allowedPositions.
	/**
	 * This option defines which positions for a node are allowed.
	 * Possibly values:
     *   - \a apAll: every position is allowed
     *   - \a apInteger: only integer positions in the range depending on the number of 
     *     nodes
     *   - \a apExponent: only integer positions in the range of -2^MaxIntPosExponent to
     *     2^MaxIntPosExponent
	 */
    AllowedPositions allowedPositions() const { return m_allowedPositions; }

	//! Sets the option allowedPositions to \a ap.
    void allowedPositions(AllowedPositions ap) { m_allowedPositions = ap; }
 
    //! Returns the current setting of option maxIntPosExponent.
	/**
	 * This option defines the exponent used if allowedPositions() == \a apExponent.
	 */
    int maxIntPosExponent() const { return m_maxIntPosExponent; }

	//! Sets the option maxIntPosExponent to \a e.
    void maxIntPosExponent(int e) {
		m_maxIntPosExponent = (((e >= 31)&&(e<=51))? e : 31);
	}

 
	/** @}
	 *  @name Options for the force calculation step 
	 *  @{
	 */

    //! Returns the used force model.
	/**
	 * Possibly values:
     *   - \a fmFruchtermanReingold: model of Fruchterman and Reingold
     *   - \a fmEades: model of Eades
     *   - \a fmNew: new model
	 */
    ForceModel forceModel() const { return m_forceModel; }

	//! Sets the used force model to \a fm.
    void forceModel(ForceModel fm) { m_forceModel = fm; }
 
    //! Returns the strength of the springs.
    double springStrength() const { return m_springStrength; }

	//! Sets the strength of the springs to \a x.
    void springStrength(double x) { m_springStrength  = ((x > 0)? x : 1);}
 
    //! Returns the strength of the repulsive forces.
    double repForcesStrength() const { return m_repForcesStrength; }

	//! Sets the strength of the repulsive forces to \a x.
    void repForcesStrength(double x) { m_repForcesStrength =((x > 0)? x : 1);}
          
    //! Returns the current setting of option repulsiveForcesCalculation.
	/**
	 * This option defines how to calculate repulsive forces.
	 * Possible values:
     *   - \a rfcExact: exact calculation (slow)
     *   - \a rfcGridApproximation: grid approxiamtion (inaccurate)
     *   - \a rfcNMM: like in NMM (= New Multipole Method; fast and accurate)
	 */
    RepulsiveForcesMethod repulsiveForcesCalculation() const {
        return m_repulsiveForcesCalculation;
    }

	//! Sets the option repulsiveForcesCalculation to \a rfc.
    void repulsiveForcesCalculation(RepulsiveForcesMethod rfc) {
        m_repulsiveForcesCalculation = rfc;
    }
 
    //! Returns the stop criterion.
	/**
	 * Possible values:
     *   - \a rscFixedIterations: stop if fixedIterations() is reached
     *   - \a rscThreshold: stop if threshold() is reached
     *   - \a rscFixedIterationsOrThreshold: stop if fixedIterations() or threshold() 
     *     is reached
	 */
    StopCriterion stopCriterion() const { return m_stopCriterion; }

    //! Sets the stop criterion to \a rsc.
	void stopCriterion(StopCriterion rsc) { m_stopCriterion = rsc; }
 
    //! Returns the threshold for the stop criterion.
	/**
	 * (If the average absolute value of all forces in 
     * an iteration is less then threshold() then stop.)
	 */
    double threshold() const { return m_threshold; }

	//! Sets the threshold for the stop criterion to \a x.
    void threshold(double x) {m_threshold = ((x > 0) ? x : 0.1);}
       
    //! Returns the fixed number of iterations for the stop criterion.
    int fixedIterations() const { return m_fixedIterations; }

	//! Sets the fixed number of iterations for the stop criterion to \a n.
    void fixedIterations(int n) { m_fixedIterations = ((n >= 1) ? n : 1);}
         
    //! Returns the scaling factor for the forces.
    double forceScalingFactor() const { return m_forceScalingFactor; }

	//! Sets the scaling factor for the forces to \ f.
    void forceScalingFactor(double f) { m_forceScalingFactor = ((f > 0) ? f : 1);}
        
	//! Returns the current setting of option coolTemperature.
    /**
	 * If set to true, forces are scaled by coolValue()^(actual iteration) * 
     * forceScalingFactor(); otherwise forces are scaled by forceScalingFactor().
	 */
    bool coolTemperature() const { return m_coolTemperature; }

	//! Sets the option coolTemperature to \a b.
    void coolTemperature(bool b) { m_coolTemperature = b; }
 
    //! Returns the current setting of option coolValue.
	/**
	 * This option defines the value by which forces are decreased
	 * if coolTemperature is true.
	 */
    double coolValue() const { return m_coolValue; }

	//! Sets the option coolValue to \a x.
    void coolValue(double x) { m_coolValue = (((x >0 )&&(x<=1) )? x : 0.99);}
 
 
    
	/** @}
	 *  @name Options for the postprocessing step 
	 *  @{
	 */

    //! Returns the number of iterations for fine tuning.
    int fineTuningIterations() const { return m_fineTuningIterations; }

    //! Sets the number of iterations for fine tuning to \a n.
    void fineTuningIterations(int n) { m_fineTuningIterations =((n >= 0) ? n : 0);}
 
    //! Returns the curent setting of option fineTuneScalar.
	/**
	 * This option defines a parameter for scaling the forces in the
	 * fine-tuning iterations.
	 */
    double fineTuneScalar() const { return m_fineTuneScalar; }

	//! Sets the option fineTuneScalar to \a s
    void fineTuneScalar(double s) { m_fineTuneScalar = ((s >= 0) ? s : 1);}

	//! Returns the current setting of option adjustPostRepStrengthDynamically.
    /**
	 * If set to true, the strength of the repulsive force field is calculated 
     * dynamically by a formula depending on the number of nodes; otherwise the 
     * strength are scaled by PostSpringStrength and PostStrengthOfRepForces.
	 */
    bool adjustPostRepStrengthDynamically() const {
		return m_adjustPostRepStrengthDynamically; 
	}

	//! Sets the option adjustPostRepStrengthDynamically to \a b.
    void adjustPostRepStrengthDynamically(bool b) {
		m_adjustPostRepStrengthDynamically = b;
	}

    //! Returns the strength of the springs in the postprocessing step.
    double postSpringStrength() const { return m_postSpringStrength; }

    //! Sets the strength of the springs in the postprocessing step to \a x.
    void postSpringStrength(double x) { m_postSpringStrength  = ((x > 0)? x : 1);}
 
    //! Returns the strength of the repulsive forces in the postprocessing step.
    double postStrengthOfRepForces() const { return m_postStrengthOfRepForces; }

    //! Sets the strength of the repulsive forces in the postprocessing step to \a x.
    void postStrengthOfRepForces(double x) {
		m_postStrengthOfRepForces = ((x > 0)? x : 1);
	}

    
	/** @}
	 *  @name Options for repulsive force approximation methods 
	 *  @{
	 */

	//! Returns the current setting of option frGridQuotient.
    /**
	 * The number k of rows and columns of the grid is sqrt(|V|) / frGridQuotient().
     * (Note that in [Fruchterman,Reingold] frGridQuotient is 2.) 
	 */
    int  frGridQuotient() const {return m_frGridQuotient;}

	//! Sets the option frGridQuotient to \a p.
    void frGridQuotient(int p) { m_frGridQuotient = ((0<=p) ? p : 2);}

    //! Returns the current setting of option nmTreeConstruction.
	/**
	 * This option defines how the reduced bucket quadtree is constructed.
	 * Possible values:
     *   - \a rtcPathByPath: path by path construction
     *   - \a rtcSubtreeBySubtree: subtree by subtree construction
	 */
    ReducedTreeConstruction nmTreeConstruction() const { return m_NMTreeConstruction; }

	//! Sets the option nmTreeConstruction to \a rtc.
    void nmTreeConstruction(ReducedTreeConstruction rtc) { m_NMTreeConstruction = rtc; }
 
	//! Returns the current setting of option nmSmallCell.
    /**
	 * This option defines how the smallest quadratic cell that surrounds
	 * the particles of a node in the reduced bucket quadtree is calculated.
	 * Possible values:
     *   - \a scfIteratively: iteratively (in constant time)
     *   - \a scfAluru: by the formula by Aluru et al. (in constant time)
	 */
    SmallestCellFinding nmSmallCell() const { return m_NMSmallCell; }

	//! Sets the option nmSmallCell to \a scf.
    void nmSmallCell(SmallestCellFinding scf) { m_NMSmallCell = scf; }
  
    //! Returns the current setting of option nmParticlesInLeaves.
	/**
	 * Defines the maximal number of particles that are contained in
	 * a leaf of the reduced bucket quadtree.
	 */
    int nmParticlesInLeaves() const { return m_NMParticlesInLeaves; }

	//! Sets the option nmParticlesInLeaves to \a n.
    void nmParticlesInLeaves(int n) { m_NMParticlesInLeaves = ((n>= 1)? n : 1);}
       
    //! Returns the precision \a p for the <i>p</i>-term multipole expansions.
    int nmPrecision() const { return m_NMPrecision; }

	//! Sets the precision for the multipole expansions to \ p.
    void nmPrecision(int p) { m_NMPrecision  = ((p >= 1 ) ? p : 1);}

private:
      
	//high level options
	bool                  m_useHighLevelOptions; //!< The option for using high-level options.
	PageFormatType        m_pageFormat; //!< The option for the page format.
	double                m_unitEdgeLength; //!< The unit edge length.
	bool                  m_newInitialPlacement; //!< The option for new initial placement.
	QualityVsSpeed        m_qualityVersusSpeed; //!< The option for quality-vs-speed trade-off.

	//low level options
	//general options
	int                   m_randSeed; //!< The random seed.
	EdgeLengthMeasurement m_edgeLengthMeasurement; //!< The option for edge length measurement.
	AllowedPositions      m_allowedPositions; //!< The option for allowed positions.
	int                   m_maxIntPosExponent; //!< The option for the used	exponent.

	//options for force calculation step
	ForceModel            m_forceModel; //!< The used force model.
	double                m_springStrength; //!< The strengths of springs.
	double                m_repForcesStrength; //!< The strength of repulsive forces.
	RepulsiveForcesMethod m_repulsiveForcesCalculation; //!< Option for how to calculate repulsive forces.
	StopCriterion         m_stopCriterion; //!< The stop criterion.
	double                m_threshold; //!< The threshold for the stop criterion.
	int                   m_fixedIterations; //!< The fixed number of iterations for the stop criterion.
	double                m_forceScalingFactor; //!< The scaling factor for the forces.
	bool                  m_coolTemperature; //!< The option for how to scale forces.
	double                m_coolValue; //!< The value by which forces are decreased.

	//options for postprocessing step
	int                   m_fineTuningIterations; //!< The number of iterations for fine tuning.
	double                m_fineTuneScalar; //!< Parameter for scaling forces during fine tuning.
	bool                  m_adjustPostRepStrengthDynamically; //!< The option adjustPostRepStrengthDynamically.
	double                m_postSpringStrength; //!< The strength of springs during postprocessing.
	double                m_postStrengthOfRepForces; //!< The strength of repulsive forces during postprocessing.

	//options for repulsive force approximation methods
	int                   m_frGridQuotient; //!< The grid quotient.
	ReducedTreeConstruction m_NMTreeConstruction; //!< The option for how to construct reduced bucket quadtree.
	SmallestCellFinding   m_NMSmallCell; //!< The option for how to calculate smallest quadtratic cells.
	int                   m_NMParticlesInLeaves; //!< The maximal number of particles in a leaf.
	int                   m_NMPrecision; //!< The precision for multipole expansions.

	//other variables
	double max_integer_position; //!< The maximum value for an integer position.
	double cool_factor; //!< Needed for scaling the forces if coolTemperature is true.
	double average_ideal_edgelength; //!< Measured from center to center.
	double boxlength; //!< Holds the length of the quadratic comput. box.
	int number_of_components; //!< The number of components of the graph.
	DPoint down_left_corner; //!< Holds down left corner of the comput. box.
	NodeArray<double> radius; //!< Holds the radius of the surrounding circle for each node.
	double time_total; //!< The runtime (=CPU-time) of the algorithm in seconds.

	FruchtermanReingold FR; //!< Class for repulsive force calculation (Fruchterman, Reingold).
	NMM NM; //!< Class for repulsive force calculation.


	//------------------- most important functions ----------------------------

	//! Calls the divide (decomposition into connected components) and impera (drawing and packing of the componenets) step.
    void call_DIVIDE_ET_IMPERA_step(Graph& G,NodeArray<NodeAttributes>& A,
                              EdgeArray<EdgeAttributes>& E);
	//! Calls the force calculation step for \a G, \a A, \a E.
	/**
	 */
	void call_FORCE_CALCULATION_step (Graph& G,NodeArray<NodeAttributes>& A,
				EdgeArray<EdgeAttributes>& E);

	//! Calls the postprocessing step. 
	void call_POSTPROCESSING_step(Graph& G, NodeArray<NodeAttributes>& A,EdgeArray
				<EdgeAttributes>& E,NodeArray<DPoint>& F,NodeArray
				<DPoint>& F_attr, NodeArray<DPoint>& F_rep,
				NodeArray<DPoint>& last_node_movement);

	void call_step_for_subGraph(Graph& G,NodeArray<NodeAttributes>& 
						     A,EdgeArray<EdgeAttributes>& E);

	//---------------- functions for pre/pos-processing -----------------------------

	//! All parameter options are set to the default values.
	void initialize_all_options();

	//! Updates several low level parameter options due to the settings of the high level parameter options.
	void update_low_level_options_due_to_high_level_options_settings();

	//! Imports for each node \a v of \a G its width, height and position(given from \a GA) in \a A.
	void import_NodeAttributes(
		const Graph& G, 
		GraphAttributes& GA, 
		NodeArray<NodeAttributes>& A);

	//! Imports for each edge e of G its desired length given via edgeLength. 
	void import_EdgeAttributes (
		const Graph& G, 
		const EdgeArray<double>& edgeLength, 
		EdgeArray <EdgeAttributes>& E);

	//! Sets the individual ideal edge length for each edge \a e.
	void init_ind_ideal_edgelength(
		const Graph& G,
		NodeArray<NodeAttributes>&A, 
		EdgeArray <EdgeAttributes>& E);

	//! The radii of the surrounding circles of the bounding boxes are computed.
	void set_radii(const Graph& G,NodeArray<NodeAttributes>& A);

	//! Exports for each node \a v in \a G_reduced the position of the original_node in \a G.
	void export_NodeAttributes(
		Graph& G_reduced, 
		NodeArray<NodeAttributes>& A_reduced,
		GraphAttributes& GA);

	//! Creates a simple and loopfree copy of \a G and stores the corresponding node / edge attributes.
	/**
	 * The corresponding node / edge attributes are stored in \a A_reduced and
	 * \a E_reduced; the links to the copy_node and original node are stored in \a A,
	 * \a A_reduced, too.
	 */
	void make_simple_loopfree(
		const Graph& G,
		NodeArray<NodeAttributes>& A, 
		EdgeArray<EdgeAttributes>E, 
		Graph& G_reduced,
		NodeArray<NodeAttributes>& A_reduced,
		EdgeArray<EdgeAttributes>& E_reduced);

	//! Deletes parallel edges of \a G_reduced.
	/**
	 * Saves for each set of parallel edges one representative edge in \a S and
	 * saves in \a new_edgelength the new edge length of this edge in \a G_reduced.
	 */
	void delete_parallel_edges(
		const Graph& G,
		EdgeArray<EdgeAttributes>& E,
		Graph& G_reduced,
		List<edge>& S,
		EdgeArray<double>& new_edgelength);

	//! Sets for each edge \a e of \a G_reduced in \a S its edgelength to \a new_edgelength[\a e].
	/**
	 * Also stores this information in \a E_reduced.
	 */
	void update_edgelength(
		List<edge>& S,
		EdgeArray <double>& new_edgelength,
		EdgeArray<EdgeAttributes>& E_reduced);

	//! Returns the value for the strength of the repulsive forces.
	/**
	 * Used in the postprocessing step; depending on \an = G.numberOfNodes().
	 */
	double get_post_rep_force_strength(int n);


	//------------------ functions for divide et impera step -----------------------

	//! Constructs the list of connected components of G.
	/**
	 * Also constructs the corresponding lists with the node / edge attributes
	 * (containing a pointer to the original node in \a G for each node in a subgraph).
	 */
	void create_maximum_connected_subGraphs(
		Graph& G,
		NodeArray<NodeAttributes>&A, 
		EdgeArray<EdgeAttributes>&E,
		Graph G_sub[],
		NodeArray<NodeAttributes> A_sub[],
		EdgeArray<EdgeAttributes> E_sub[],
		NodeArray<int>& component);

	//! The bounding rectangles of all connected componenents of \a G are calculated and stored in \a R.
	void  calculate_bounding_rectangles_of_components(
		List<Rectangle>& R,
		Graph  G_sub[],
		NodeArray<NodeAttributes> A_sub[]);

	//! The bounding rectangle of the componenet_index-th. component of G is returned.
	Rectangle calculate_bounding_rectangle(
		Graph& G,
		NodeArray<NodeAttributes>& A,
		int component_index);

	//! Frees dynamically allocated memory for the connected component subgraphs.
	void delete_all_subGraphs(
		Graph G_sub[],
		NodeArray<NodeAttributes> A_sub[],
		EdgeArray<EdgeAttributes> E_sub[]);


	//------------------  functions for multilevel step    --------------------------

	/**
	 * Returns the maximum number of iterations for the force calc. step depending
	 * on act_level, max_level, FixedIterations, MaxIterChange, MaxIterFactor,
	 * and the number of nodes of the Graph in the actual mutilevel.
	 */
	int get_max_mult_iter(int act_level, int max_level, int node_nr);


	//------------------  functions for force calculation ---------------------------

	//! The forces are calculated here.
	void calculate_forces(
		Graph& G, 
		NodeArray<NodeAttributes>& A,
		EdgeArray<EdgeAttributes>& E,NodeArray<DPoint>& F,
		NodeArray<DPoint>& F_attr, 
		NodeArray<DPoint>& F_rep,
		NodeArray<DPoint>& last_node_movement,
		int iter,
		int fine_tuning_step);

	//! Sets all entries of \a F to (0,0).
	void  init_F (Graph& G, NodeArray<DPoint>& F);


	//! Make initializations for the data structures that are used in the choosen class for rep. force calculation.
	void make_initialisations_for_rep_calc_classes(
		Graph& G,
		NodeArray<NodeAttributes> &A, 
		NodeArray<DPoint>& F_rep);

	//! Calculates repulsive forces for each node. 
	void calculate_repulsive_forces(
		Graph &G,
		NodeArray<NodeAttributes>& A,
		NodeArray<DPoint>& F_rep);

	//! Deallocates dynamically allocated memory of the choosen rep. calculation class.
	void deallocate_memory_for_rep_calc_classes();

	//! Calculates attractive forces for each node.
	void calculate_attractive_forces(
		Graph& G,
		NodeArray<NodeAttributes> & A,
		EdgeArray<EdgeAttributes>& E, 
		NodeArray<DPoint>& F_attr);

	//! Returns the attractive force scalar.
	double f_attr_scalar (double d,double ind_ideal_edge_length);

	//! Add attractive and repulsive forces for each node.
	void add_attr_rep_forces(
		Graph& G,
		NodeArray<NodeAttributes> & A,
		NodeArray<DPoint>& F_attr,
		NodeArray<DPoint>& F_rep,
		NodeArray<DPoint>& F,
		int iter,
		int fine_tuning_step);

	//! Move the nodes.
	void move_nodes(Graph& G,NodeArray<NodeAttributes>& A,NodeArray<DPoint>& F);

	//! Computes a new tight computational square-box.
	/**
	 * (Guaranteeing, that all midpoints are inside the square.)
	 */
	void update_boxlength_and_cornercoordinate(Graph& G,NodeArray<NodeAttributes>& A);

	//! Describes the max. radius of a move in one time step, depending on the number of iterations.
	double max_radius(int iter);

	//! The average_ideal_edgelength for all edges is computed.
	void set_average_ideal_edgelength(Graph& G,EdgeArray<EdgeAttributes>& E);

	/**
	 * Calculates the average force on each node in the actual iteration, which is 
	 * needed if StopCriterion is scThreshold() or scFixedIterationsOrThreshold().
	 */
	double get_average_forcevector_length (Graph& G, NodeArray<DPoint>& F);

	/**
	 * Depending on the direction of \a last_node_movement[\a v], the length of the next 
	 * displacement of node \a v is restricted.
	 */
	void prevent_oscillations(
		Graph& G, 
		NodeArray<DPoint>& F,
		NodeArray<DPoint>&
		last_node_movement,
		int iter);

	//! Calculates the angle between \a PQ and \a PS in [0,2pi).
	double angle(DPoint& P, DPoint& Q, DPoint& R);

	//! \a last_node_movement is initialized to \a F (used after first iteration).
	void init_last_node_movement(
		Graph& G, 
		NodeArray<DPoint>& F,
		NodeArray<DPoint>& last_node_movement);  

	/**
	 * If resizeDrawing is true, the drawing is adapted to the ideal average 
	 * edge length by shrinking respectively expanding the drawing area.
	 */
	void adapt_drawing_to_ideal_average_edgelength(
		Graph& G,
		NodeArray<NodeAttributes>& A,
		EdgeArray<EdgeAttributes>& E);
					  
	/**
	 * The force is restricted to have values within the comp. box (needed for 
	 * exception handling, if the force is too large for further calculations).
	 */
	void restrict_force_to_comp_box(DPoint& force);


	//------------------- functions for analytic information -------------------------

	//! Sets time_total to zero.
	void init_time() { time_total = 0; }
    void create_postscript_drawing(GraphAttributes& AG, char* ps_file);

};


} //end namespace ogdf 
#endif
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

