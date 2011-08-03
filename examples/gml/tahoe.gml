graph
[
node
[
id 24
label "{Tahoe::SolidMaterialT\n|# fThermal\l# fDensity\l# fConstraint\l# fCTE\l|+ Constraint()\l+ IncrementalHeat()\l+ StrainEnergyDensity()\l+ AcousticalTensor()\l+ WaveSpeeds()\l+ Strain()\l+ HasThermalStrain()\l+ ThermalStrainSchedule()\l+ SetThermalSchedule()\l+ ThermalElongation()\l+ Density()\l+ IsLocalized()\l+ IsLocalized()\l+ IsLocalized()\l+ SupportsThermalStrain()\l+ c_ijkl()\l+ ce_ijkl()\l+ s_ij()\l+ Pressure()\l+ C_IJKL()\l+ S_IJ()\l+ HasLocalization()\l+ HasIncrementalHeat()\l+ NeedDisp()\l+ NeedLastDisp()\l+ NeedVel()\l+ HasChangingDensity()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l+ InternalDOF()\l+ InternalStressVars()\l+ InternalStrainVars()\l* ConstraintT\l* int2ConstraintT()\l* SolidMaterialT()\l* ~SolidMaterialT()\l* c_ijkl()\l* ce_ijkl()\l* s_ij()\l* Pressure()\l* C_IJKL()\l* S_IJ()\l* HasLocalization()\l* HasIncrementalHeat()\l* NeedDisp()\l* NeedLastDisp()\l* NeedVel()\l* HasChangingDensity()\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l* InternalDOF()\l* InternalStressVars()\l* InternalStrainVars()\l}"
template "UML:CLASS"
graphics
[
x 819.0
y 940.0
w 20.0
h 20.0
]
]
node
[
id 25
label "{Tahoe::ContinuumMaterialT\n|# fMaterialSupport\l# fNumDOF\l# fNumSD\l# fNumIP\l|+ ContinuumMaterialT()\l+ ~ContinuumMaterialT()\l+ SetMaterialSupport()\l+ TangentType()\l+ RelaxCode()\l+ MaterialSupport()\l+ ContinuumElement()\l+ NumElementNodes()\l+ NumDOF()\l+ NumSD()\l+ NumIP()\l+ CurrIP()\l+ NumElements()\l+ CurrElementNumber()\l+ ElementCard()\l+ CurrentElement()\l+ InitStep()\l+ CloseStep()\l+ HasHistory()\l+ NeedsPointInitialization()\l+ PointInitialize()\l+ UpdateHistory()\l+ ResetHistory()\l* HasHistory()\l* NeedsPointInitialization()\l* PointInitialize()\l* UpdateHistory()\l* ResetHistory()\l* NumOutputVariables()\l* OutputLabels()\l* ComputeOutput()\l* CompatibleOutput()\l}"
template "UML:CLASS"
graphics
[
x 672.0
y 193.0
w 20.0
h 20.0
]
]
node
[
id 26
label "{Tahoe::MaterialSupportT\n|- fCurrIP\l- fGroupCommunicator\l- fElementCards\l- fContinuumElement\l- fGroup\l- fNumDOF\l- fNumIP\l- fInitCoords\l- fDisp\l|+ MaterialSupportT()\l+ ~MaterialSupportT()\l+ GroupCommunicator()\l+ NumDOF()\l+ NumIP()\l+ CurrIP()\l+ SetCurrIP()\l+ ContinuumElement()\l+ GroupIterationNumber()\l+ NumElements()\l+ CurrElementNumber()\l+ ElementCard()\l+ CurrentElement()\l+ TopElement()\l+ NextElement()\l+ LocalArray()\l+ Interpolate()\l+ Interpolate()\l+ SetContinuumElement()\l+ SetLocalArray()\l+ SetElementCards()\l+ SetGroup()\l* fNumDOF\l* fNumIP\l* fInitCoords\l* fDisp\l* NumDOF()\l* NumIP()\l* CurrIP()\l* SetCurrIP()\l* ContinuumElement()\l* GroupIterationNumber()\l* NumElements()\l* CurrElementNumber()\l* ElementCard()\l* CurrentElement()\l* TopElement()\l* NextElement()\l* LocalArray()\l* Interpolate()\l* Interpolate()\l* SetContinuumElement()\l* SetLocalArray()\l* SetElementCards()\l* SetGroup()\l}"
template "UML:CLASS:START"
graphics
[
x 139.0
y 479.0
w 20.0
h 20.0
]
]
node
[
id 27
label "{Tahoe::FSMatSupportT\n|- fFiniteStrain\l- fF_List\l- fF_last_List\l|+ FSMatSupportT()\l+ Interpolate_current()\l+ DeformationGradient()\l+ DeformationGradient()\l+ DeformationGradient_last()\l+ DeformationGradient_last()\l+ SetDeformationGradient()\l+ SetDeformationGradient_last()\l+ ComputeGradient()\l+ ComputeGradient()\l+ ComputeGradient_reference()\l+ ComputeGradient_reference()\l+ FiniteStrain()\l+ SetContinuumElement()\l* fF_List\l* fF_last_List\l* DeformationGradient()\l* DeformationGradient()\l* DeformationGradient_last()\l* DeformationGradient_last()\l* SetDeformationGradient()\l* SetDeformationGradient_last()\l* ComputeGradient()\l* ComputeGradient()\l* ComputeGradient_reference()\l* ComputeGradient_reference()\l* FiniteStrain()\l* SetContinuumElement()\l}"
template "UML:CLASS"
graphics
[
x 630.0
y 338.0
w 20.0
h 20.0
]
]
node
[
id 20
label "{Tahoe::ShapeFunctionT\n|# fCoords\l- fDet\l- fDNaX\l- fDDNaX\l- pNaU\l- pDNaU\l- pDDNaU\l- fGrad_x_temp\l- fv1\l- fv2\l- fStore\l- fCurrElementNumber\l- fDet_store\l- fDNaX_store\l|+ ShapeFunctionT()\l+ ShapeFunctionT()\l+ ShapeFunctionT()\l+ DomainCoordType()\l+ Coordinates()\l+ SetDerivatives()\l+ SetDerivatives_DN_DDN()\l+ IPDets()\l+ IPCoords()\l+ IPCoords()\l+ InterpolateU()\l+ InterpolateU()\l+ InitializeDomain()\l+ CurlU()\l+ CurlU()\l+ Print()\l+ Derivatives_X()\l+ Derivatives_X()\l+ Derivatives_U()\l+ Derivatives_U()\l+ IPShapeX()\l+ IPShapeX()\l+ IPShapeU()\l+ IPShapeU()\l+ GradU()\l+ GradU()\l+ GradU()\l+ TransformDerivatives()\l+ TransformDerivatives()\l+ GradNa()\l+ GradNa()\l+ Grad_GradNa()\l+ Grad_GradNa()\l+ InitStore()\l+ Store()\l+ CloseStore()\l# SetGrad_x()\l# SetUShapeFunctions()\l# DNaX()\l# DDNaX()\l- Construct()\l- Construct_DN_DDN()\l- IPShape()\l* fStore\l* fCurrElementNumber\l* fDet_store\l* fDNaX_store\l* Derivatives_X()\l* Derivatives_X()\l* Derivatives_U()\l* Derivatives_U()\l* IPShapeX()\l* IPShapeX()\l* IPShapeU()\l* IPShapeU()\l* GradU()\l* GradU()\l* GradU()\l* TransformDerivatives()\l* TransformDerivatives()\l* GradNa()\l* GradNa()\l* Grad_GradNa()\l* Grad_GradNa()\l* InitStore()\l* Store()\l* CloseStore()\l}"
template "UML:CLASS"
graphics
[
x 608.0
y 956.0
w 20.0
h 20.0
]
]
node
[
id 21
label "{Tahoe::DomainIntegrationT\n|# fNumIP\l# fCurrIP\l# fDomain\l# fSurfShapes\l# fDelete\l- fDeleteDomain\l- frefCurrIP\l|+ DomainIntegrationT()\l+ DomainIntegrationT()\l+ ~DomainIntegrationT()\l+ Initialize()\l+ IPWeights()\l+ IPShape()\l+ IPDShape()\l+ IPWeight()\l+ Extrapolate()\l+ ExtrapolateAll()\l+ Extrapolation()\l+ Print()\l+ NumFacets()\l+ NumNodesOnFacets()\l+ NodesOnFacet()\l+ NodesOnEdges()\l+ FacetGeometry()\l+ NeighborNodeMap()\l+ FacetShapeFunction()\l+ ParentDomain()\l+ ParentCoords()\l+ EvaluateShapeFunctions()\l+ NumSD()\l+ NumIP()\l+ GeometryCode()\l+ TopIP()\l+ NextIP()\l+ SetIP()\l+ CurrIP()\l# Na()\l- SetSurfaceShapes()\l* NumSD()\l* NumIP()\l* GeometryCode()\l* TopIP()\l* NextIP()\l* SetIP()\l* CurrIP()\l}"
template "UML:CLASS"
graphics
[
x 586.0
y 851.0
w 20.0
h 20.0
]
]
node
[
id 22
label "{Tahoe::ScheduleT\n|- fFunction\l- fCurrentTime\l- fCurrentValue\l|+ ~ScheduleT()\l+ SetTime()\l+ ScheduleT()\l+ ScheduleT()\l+ Value()\l+ Value()\l+ Time()\l+ Rate()\l+ Rate()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l* fCurrentTime\l* fCurrentValue\l* ScheduleT()\l* ScheduleT()\l* Value()\l* Value()\l* Time()\l* Rate()\l* Rate()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 70.0
y 343.0
w 20.0
h 20.0
]
]
node
[
id 23
label "{Tahoe::MaterialListT\n|# fHasHistory\l|+ MaterialListT()\l+ MaterialListT()\l+ ~MaterialListT()\l+ InitStep()\l+ CloseStep()\l+ HasHistoryMaterials()\l}"
template "UML:CLASS"
graphics
[
x 745.0
y 883.0
w 20.0
h 20.0
]
]
node
[
id 28
label "{Tahoe::SolidMatSupportT\n|- fSolidElement\l- fLastDisp\l- fVel\l- fAcc\l- fTemperatures\l- fLastTemperatures\l|+ SolidMatSupportT()\l+ SolidElement()\l+ LocalArray()\l+ Temperatures()\l+ LastTemperatures()\l+ SetContinuumElement()\l+ SetLocalArray()\l+ SetTemperatures()\l+ SetLastTemperatures()\l* fLastDisp\l* fVel\l* fAcc\l* fTemperatures\l* fLastTemperatures\l* SolidElement()\l* LocalArray()\l* Temperatures()\l* LastTemperatures()\l* SetContinuumElement()\l* SetLocalArray()\l* SetTemperatures()\l* SetLastTemperatures()\l}"
template "UML:CLASS"
graphics
[
x 118.0
y 927.0
w 20.0
h 20.0
]
]
node
[
id 29
label "{Tahoe::MeshFreeShapeFunctionT\n|# fMFSupport\l# fCurrElement\l# fNeighbors\l# fNaU\l# fDNaU\l# fXConnects\l# fExactNodes\l# fElemHasExactNode\l# fElemFlags\l# fR\l# fDR\l# fNa_tmp\l# fDNa_tmp\l# felSpace\l# fndSpace\l# fNeighExactFlags\l|+ MeshFreeShapeFunctionT()\l+ ~MeshFreeShapeFunctionT()\l+ Initialize()\l+ SetSupportSize()\l+ SetNeighborData()\l+ SetExactNodes()\l+ SetSkipNodes()\l+ SetSkipElements()\l+ SetNodalParameters()\l+ GetNodalParameters()\l+ NodalParameters()\l+ SetDerivatives()\l+ SetDerivativesAt()\l+ UseDerivatives()\l+ SetCuttingFacets()\l+ ResetFacets()\l+ ResetNodes()\l+ ResetCells()\l+ NumberOfNeighbors()\l+ Neighbors()\l+ ElementNeighborsCounts()\l+ ElementNeighbors()\l+ NodeNeighbors()\l+ SelectedNodalField()\l+ NodalField()\l+ NodalField()\l+ Print()\l+ PrintAt()\l+ WriteParameters()\l+ WriteStatistics()\l+ BlendElementData()\l+ BlendNodalData()\l+ MeshFreeSupport()\l- InitBlend()\l}"
template "UML:CLASS"
graphics
[
x 142.0
y 401.0
w 20.0
h 20.0
]
]
node
[
id 1
label "{Tahoe::MeshFreeFSSolidAxiT\n|# fMFShapes\l# fMFFractureSupport\l# fAutoBorder\l# fStressStiff_wrap\l# fB_wrap\l# fGradNa_wrap\l# fDNa_x_wrap\l# fConnectsAll\l# fMeshfreeParameters\l|+ MeshFreeFSSolidAxiT()\l+ ~MeshFreeFSSolidAxiT()\l+ Equations()\l+ ConnectsU()\l+ WriteOutput()\l+ RelaxSystem()\l+ InterpolantDOFs()\l+ NodalDOFs()\l+ WeightNodalCost()\l+ InitStep()\l+ CloseStep()\l+ ResetStep()\l+ MeshFreeSupport()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# SetShape()\l# NextElement()\l# ComputeOutput()\l- WriteField()\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 923.0
y 321.0
w 20.0
h 20.0
]
]
node
[
id 3
label "{Tahoe::FiniteStrainAxiT\n|# fMat2D\l# fLocCurrCoords\l# fRadius_X\l# fRadius_x\l|+ FiniteStrainAxiT()\l+ CollectMaterialInfo()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# Axisymmetric()\l# SetLocalArrays()\l# NewMaterialSupport()\l# SetGlobalShape()\l* fRadius_X\l* fRadius_x\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 708.0
y 289.0
w 20.0
h 20.0
]
]
node
[
id 2
label "{Tahoe::TotalLagrangianAxiT\n|# fIPShape\l# fOutputInit\l# fOutputCell\l# fStressMat\l# fStressStiff\l# fGradNa\l# fTemp2\l# fTempMat1\l# fTempMat2\l# fDNa_x\l|+ TotalLagrangianAxiT()\l+ TakeParameterList()\l# FormStiffness()\l# FormKd()\l* fStressMat\l* fStressStiff\l* fGradNa\l* fTemp2\l* fTempMat1\l* fTempMat2\l* fDNa_x\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 306.0
y 294.0
w 20.0
h 20.0
]
]
node
[
id 5
label "{Tahoe::SolidElementT\n|# fMassType\l# fNormal\l# fv_ss\l# fDensity\l# fCurrMaterial\l# fMaterialNeeds\l# fIncrementalHeat\l# qUseSimo\l# qNoExtrap\l# fEigenvalueInc\l# fLocLastDisp\l# fLocVel\l# fLocAcc\l# fLocTemp\l# fLocTemp_last\l# fElementHeat\l# fD\l# fB\l# fStress\l# fStoreInternalForce\l# fForce\l# NumNodalOutputCodes\l# NumElementOutputCodes\l|+ SolidElementT()\l+ ~SolidElementT()\l+ CloseStep()\l+ TangentType()\l+ AddNodalForce()\l+ AddLinearMomentum()\l+ InternalEnergy()\l+ SendOutput()\l+ SetStoreInternalForce()\l+ InternalForce()\l+ LastDisplacements()\l+ Velocities()\l+ Accelerations()\l+ Temperatures()\l+ LastTemperatures()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# MaxEigenvalue()\l# SetLocalArrays()\l# SetShape()\l# SetGlobalShape()\l# NewMaterialSupport()\l# Set_B()\l# Set_B_axi()\l# Set_B_bar()\l# Set_B_bar_axi()\l# NextElement()\l# FormStiffness()\l# FormKd()\l# StructuralMaterialList()\l# ComputeOutput()\l# LHSDriver()\l# ElementLHSDriver()\l# RHSDriver()\l# ElementRHSDriver()\l# SetNodalOutputCodes()\l# SetElementOutputCodes()\l# GenerateOutputLabels()\l* fLocLastDisp\l* fLocVel\l* fLocAcc\l* fLocTemp\l* fLocTemp_last\l* fElementHeat\l* fD\l* fB\l* fStress\l* fStoreInternalForce\l* fForce\l* LastDisplacements()\l* Velocities()\l* Accelerations()\l* Temperatures()\l* LastTemperatures()\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l* LHSDriver()\l* ElementLHSDriver()\l* RHSDriver()\l* ElementRHSDriver()\l* SetNodalOutputCodes()\l* SetElementOutputCodes()\l* GenerateOutputLabels()\l}"
template "UML:CLASS"
graphics
[
x 226.0
y 279.0
w 20.0
h 20.0
]
]
node
[
id 4
label "{Tahoe::FiniteStrainT\n|# fCurrShapes\l# fFSMatSupport\l# fF_List\l# fF_all\l# fF_last_List\l# fF_last_all\l- fNeedsOffset\l|+ FiniteStrainT()\l+ ~FiniteStrainT()\l+ CollectMaterialInfo()\l+ CurrShapeFunction()\l+ DeformationGradient()\l+ DeformationGradient()\l+ DeformationGradient_last()\l+ DeformationGradient_last()\l+ ComputeGradient()\l+ ComputeGradient()\l+ IP_Interpolate_current()\l+ ComputeGradient_reference()\l+ ComputeGradient_reference()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l# NewMaterialSupport()\l# NewMaterialList()\l# SetGlobalShape()\l# Needs_F()\l# Needs_F_last()\l# CurrElementInfo()\l* fF_List\l* fF_all\l* fF_last_List\l* fF_last_all\l* DeformationGradient()\l* DeformationGradient()\l* DeformationGradient_last()\l* DeformationGradient_last()\l* ComputeGradient()\l* ComputeGradient()\l* IP_Interpolate_current()\l* ComputeGradient_reference()\l* ComputeGradient_reference()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 673.0
y 501.0
w 20.0
h 20.0
]
]
node
[
id 7
label "{Tahoe::ElementBaseT\n|# fIntegrator\l# fElementCards\l# fBlockData\l# fOutputID\l# fConnectivities\l# fEqnos\l# fLHS\l# fRHS\l- fSupport\l- fField\l|+ ElementBaseT()\l+ ~ElementBaseT()\l+ FormLHS()\l+ FormRHS()\l+ AddNodalForce()\l+ InitStep()\l+ CloseStep()\l+ ResetStep()\l+ RelaxSystem()\l+ InternalEnergy()\l+ Equations()\l+ InitialCondition()\l+ InterpolantDOFs()\l+ NodalDOFs()\l+ WeightNodalCost()\l+ NodesUsed()\l+ LumpedMass()\l+ InternalForce()\l+ ElementGroupNumber()\l+ NumElements()\l+ NumElementNodes()\l+ TangentType()\l+ Axisymmetric()\l+ ElementBlockID()\l+ ElementSupport()\l+ Field()\l+ RunState()\l+ IterationNumber()\l+ InGroup()\l+ Schedule()\l+ NumSD()\l+ ElementBlockIDs()\l+ BlockData()\l+ NumDOF()\l+ SetStatus()\l+ GetStatus()\l+ RegisterOutput()\l+ WriteOutput()\l+ SendOutput()\l+ ResolveOutputVariable()\l+ ConnectsX()\l+ ConnectsU()\l+ GeometryCode()\l+ WriteRestart()\l+ ReadRestart()\l+ ElementCard()\l+ ElementCard()\l+ CurrElementNumber()\l+ CurrentElement()\l+ CurrentElement()\l+ DefineParameters()\l+ TakeParameterList()\l# BlockToGroupElementNumbers()\l# Group()\l# CurrElementInfo()\l# SetElementCards()\l# CollectBlockInfo()\l# DefineElements()\l# SetLocalX()\l# SetLocalU()\l# LHSDriver()\l# RHSDriver()\l# AssembleRHS()\l# AssembleLHS()\l# Top()\l# NextElement()\l- DefaultNumElemNodes()\l* fConnectivities\l* fEqnos\l* fLHS\l* fRHS\l* fSupport\l* fField\l* ElementGroupNumber()\l* NumElements()\l* NumElementNodes()\l* TangentType()\l* Axisymmetric()\l* ElementBlockID()\l* ElementSupport()\l* Field()\l* RunState()\l* IterationNumber()\l* InGroup()\l* Schedule()\l* NumSD()\l* ElementBlockIDs()\l* BlockData()\l* NumDOF()\l* SetStatus()\l* GetStatus()\l* RegisterOutput()\l* WriteOutput()\l* SendOutput()\l* ResolveOutputVariable()\l* ConnectsX()\l* ConnectsU()\l* GeometryCode()\l* WriteRestart()\l* ReadRestart()\l* ElementCard()\l* ElementCard()\l* CurrElementNumber()\l* CurrentElement()\l* CurrentElement()\l* DefineParameters()\l* TakeParameterList()\l* CollectBlockInfo()\l* DefineElements()\l* SetLocalX()\l* SetLocalU()\l* LHSDriver()\l* RHSDriver()\l* AssembleRHS()\l* AssembleLHS()\l* Top()\l* NextElement()\l}"
template "UML:CLASS"
graphics
[
x 332.0
y 612.0
w 20.0
h 20.0
]
]
node
[
id 6
label "{Tahoe::ContinuumElementT\n|# fGroupCommunicator\l# fMaterialList\l# fNodalOutputCodes\l# fElementOutputCodes\l# fBodySchedule\l# fBody\l# fTractionList\l# fTractionBCSet\l# fShapes\l# fStoreShape\l# fLocInitCoords\l# fLocDisp\l# fNEEvec\l# fDOFvec\l- fNumIP\l- fGeometryCode\l|+ ContinuumElementT()\l+ ~ContinuumElementT()\l+ NumIP()\l+ ShapeFunction()\l+ CurrIP()\l+ GroupCommunicator()\l+ IP_Coords()\l+ IP_Interpolate()\l+ IP_Interpolate()\l+ IP_ComputeGradient()\l+ IP_ComputeGradient()\l+ IP_ExtrapolateAll()\l+ InitialCoordinates()\l+ Displacements()\l+ Equations()\l+ TangentType()\l+ ReadRestart()\l+ WriteRestart()\l+ FacetGeometry()\l+ GeometryCode()\l+ SetStatus()\l+ InitialCondition()\l+ MaterialsList()\l+ InitStep()\l+ CloseStep()\l+ ResetStep()\l+ RelaxSystem()\l+ RegisterOutput()\l+ WriteOutput()\l+ ResolveOutputVariable()\l+ DefineParameters()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l+ int2MassTypeT()\l# SetLocalArrays()\l# SetShape()\l# RHSDriver()\l# ApplyTractionBC()\l# SetGlobalShape()\l# FormMass()\l# AddBodyForce()\l# FormMa()\l# TakeNaturalBC()\l# CurrElementInfo()\l# CollectMaterialInfo()\l# NewMaterialList()\l# NewMaterialSupport()\l# CheckMaterialOutput()\l# SetNodalOutputCodes()\l# SetElementOutputCodes()\l# ComputeOutput()\l# GenerateOutputLabels()\l- SetTractionBC()\l- DefaultNumElemNodes()\l* fLocInitCoords\l* fLocDisp\l* fNEEvec\l* fDOFvec\l* InitStep()\l* CloseStep()\l* ResetStep()\l* RelaxSystem()\l* RegisterOutput()\l* WriteOutput()\l* ResolveOutputVariable()\l* DefineParameters()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l* CollectMaterialInfo()\l* NewMaterialList()\l* NewMaterialSupport()\l* CheckMaterialOutput()\l* SetNodalOutputCodes()\l* SetElementOutputCodes()\l* ComputeOutput()\l* GenerateOutputLabels()\l}"
template "UML:CLASS"
graphics
[
x 2.0
y 765.0
w 20.0
h 20.0
]
]
node
[
id 9
label "{Tahoe::BasicSupportT\n|- fFEManager\l- fNodeManager\l- fTimeManager\l- fModelManager\l- fCommManager\l- fCommunicator\l- fRunState\l- fNumSD\l|+ BasicSupportT()\l+ SetFEManager()\l+ SetNodeManager()\l+ SetNumSD()\l+ Version()\l+ PrintInput()\l+ Logging()\l+ NumNodes()\l+ NumSD()\l+ InitialCoordinates()\l+ CurrentCoordinates()\l+ RegisterCoordinates()\l+ RunState()\l+ Schedule()\l+ IterationNumber()\l+ IterationNumber()\l+ CurrentGroup()\l+ Time()\l+ StepNumber()\l+ NumberOfSteps()\l+ TimeStep()\l+ Field()\l+ Size()\l+ Rank()\l+ Communicator()\l+ ProcessorMap()\l+ ExternalNodes()\l+ BorderNodes()\l+ NodeMap()\l+ ElementMap()\l+ PartitionNodes()\l+ AssembleLHS()\l+ AssembleLHS()\l+ AssembleLHS()\l+ AssembleRHS()\l+ RHS()\l+ LHS()\l+ InputFile()\l+ Output()\l+ RegisterOutput()\l+ WriteOutput()\l+ WriteOutput()\l+ WriteOutput()\l+ OutputSet()\l+ FEManager()\l+ NodeManager()\l+ XDOF_Manager()\l+ TimeManager()\l+ ModelManager()\l+ CommManager()\l+ NumElementGroups()\l+ ElementGroup()\l* fFEManager\l* fNodeManager\l* fTimeManager\l* fModelManager\l* fCommManager\l* fCommunicator\l* fRunState\l* fNumSD\l* SetFEManager()\l* SetNodeManager()\l* SetNumSD()\l* Version()\l* PrintInput()\l* Logging()\l* NumNodes()\l* NumSD()\l* InitialCoordinates()\l* CurrentCoordinates()\l* RegisterCoordinates()\l* RunState()\l* Schedule()\l* IterationNumber()\l* IterationNumber()\l* CurrentGroup()\l* Time()\l* StepNumber()\l* NumberOfSteps()\l* TimeStep()\l* Field()\l* Size()\l* Rank()\l* Communicator()\l* ProcessorMap()\l* ExternalNodes()\l* BorderNodes()\l* NodeMap()\l* ElementMap()\l* PartitionNodes()\l* AssembleLHS()\l* AssembleLHS()\l* AssembleLHS()\l* AssembleRHS()\l* RHS()\l* LHS()\l* InputFile()\l* Output()\l* RegisterOutput()\l* WriteOutput()\l* WriteOutput()\l* WriteOutput()\l* OutputSet()\l* FEManager()\l* NodeManager()\l* XDOF_Manager()\l* TimeManager()\l* ModelManager()\l* CommManager()\l* NumElementGroups()\l* ElementGroup()\l}"
template "UML:CLASS"
graphics
[
x 466.0
y 348.0
w 20.0
h 20.0
]
]
node
[
id 8
label "{Tahoe::ElementSupportT\n||+ ElementSupportT()\l+ ~ElementSupportT()\l+ ElementGroupNumber()\l+ AssembleLHS()\l+ AssembleRHS()\l+ AssembleLHS()\l+ AssembleLHS()\l+ ResetAverage()\l+ AssembleAverage()\l+ OutputAverage()\l+ OutputUsedAverage()\l+ WriteOutput()\l+ WriteOutput()\l+ WriteOutput()\l+ WriteOutput()\l+ Argv()\l+ CommandLineOption()\l+ CommandLineOption()\l* AssembleLHS()\l* AssembleRHS()\l* AssembleLHS()\l* AssembleLHS()\l* ResetAverage()\l* AssembleAverage()\l* OutputAverage()\l* OutputUsedAverage()\l* WriteOutput()\l* WriteOutput()\l* WriteOutput()\l* WriteOutput()\l* Argv()\l* CommandLineOption()\l* CommandLineOption()\l}"
template "UML:CLASS"
graphics
[
x 570.0
y 33.0
w 20.0
h 20.0
]
]
node
[
id 11
label "{Tahoe::XDOF_ManagerT\n|# fDOFElements\l# fXDOF_Eqnos\l# fXDOFs\l# fStartTag\l# fNumTags\l# fNumTagSets\l# fTagSetLength\l|+ XDOF_ManagerT()\l+ ~XDOF_ManagerT()\l+ XDOF_Register()\l+ XDOF_Eqnos()\l+ XDOF()\l+ XDOF_SetLocalEqnos()\l+ XDOF_SetLocalEqnos()\l+ XDOF_SetLocalEqnos()\l# NumEquations()\l# SetStartTag()\l# ResetState()\l# ResetTags()\l# NumTagSets()\l# Reset()\l# Update()\l# ConfigureElementGroup()\l# SetEquations()\l# CheckEquationNumbers()\l# EquationNumbers()\l# TagSetIndex()\l# ResolveTagSet()\l* fStartTag\l* fNumTags\l* fNumTagSets\l* fTagSetLength\l* XDOF_SetLocalEqnos()\l* XDOF_SetLocalEqnos()\l* XDOF_SetLocalEqnos()\l}"
template "UML:CLASS"
graphics
[
x 913.0
y 233.0
w 20.0
h 20.0
]
]
node
[
id 10
label "{Tahoe::NodeManagerT\n|# fFEManager\l# fCommManager\l# fFieldSupport\l# fFields\l# fMessageID\l# fHistoryNodeSetIDs\l# fHistoryOutputID\l# fCoordUpdateIndex\l# fFieldStart\l# fFieldEnd\l- fXDOFRelaxCodes\l- fInitCoords\l- fCoordUpdate\l- fCurrentCoords\l- fCurrentCoords_man\l- fNeedCurrentCoords\l|+ NodeManagerT()\l+ ~NodeManagerT()\l+ RelaxSystem()\l+ SetTimeStep()\l+ InitialCondition()\l+ InitStep()\l+ CloseStep()\l+ Update()\l+ UpdateCurrentCoordinates()\l+ CopyNodeToNode()\l+ ResetStep()\l+ GetUnknowns()\l+ WeightNodalCost()\l+ Schedule()\l+ Rank()\l+ Size()\l+ FEManager()\l+ NumEquations()\l+ NumDOF()\l+ NumNodes()\l+ NumSD()\l+ NumFields()\l+ NumFields()\l+ Field()\l+ Field()\l+ TangentType()\l+ CollectFields()\l+ ConnectsU()\l+ ImplicitExplicit()\l+ RegisterOutput()\l+ WriteOutput()\l+ SetEquationNumbers()\l+ RenumberEquations()\l+ SetEquationNumberScope()\l+ EquationNumber()\l+ WriteEquationNumbers()\l+ Equations()\l+ FormLHS()\l+ FormRHS()\l+ EndRHS()\l+ EndLHS()\l+ PackSize()\l+ Pack()\l+ Unpack()\l+ ReadRestart()\l+ WriteRestart()\l+ InitialCoordinates()\l+ CurrentCoordinates()\l+ Field()\l+ RegisterCoordinates()\l+ CommManager()\l+ CoordinateUpdate()\l+ ResizeNodes()\l+ XDOF_Register()\l+ XDOF_SetLocalEqnos()\l+ XDOF_SetLocalEqnos()\l+ XDOF_SetLocalEqnos()\l+ NewKBC_Controller()\l+ NewFBC_Controller()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# WriteData()\l# SetCoordinates()\l# EquationNumbers()\l# CheckEquationNumbers()\l- WriteNodalHistory()\l- NodeManagerT()\l- operator=()\l* fFieldSupport\l* fFields\l* fMessageID\l* fHistoryNodeSetIDs\l* fHistoryOutputID\l* fCoordUpdateIndex\l* fFieldStart\l* fFieldEnd\l* fInitCoords\l* fCoordUpdate\l* fCurrentCoords\l* fCurrentCoords_man\l* fNeedCurrentCoords\l* Rank()\l* Size()\l* FEManager()\l* NumEquations()\l* NumDOF()\l* NumNodes()\l* NumSD()\l* NumFields()\l* NumFields()\l* Field()\l* Field()\l* TangentType()\l* CollectFields()\l* ConnectsU()\l* ImplicitExplicit()\l* RegisterOutput()\l* WriteOutput()\l* SetEquationNumbers()\l* RenumberEquations()\l* SetEquationNumberScope()\l* EquationNumber()\l* WriteEquationNumbers()\l* Equations()\l* FormLHS()\l* FormRHS()\l* EndRHS()\l* EndLHS()\l* PackSize()\l* Pack()\l* Unpack()\l* ReadRestart()\l* WriteRestart()\l* InitialCoordinates()\l* CurrentCoordinates()\l* Field()\l* RegisterCoordinates()\l* CommManager()\l* CoordinateUpdate()\l* ResizeNodes()\l* XDOF_Register()\l* XDOF_SetLocalEqnos()\l* XDOF_SetLocalEqnos()\l* XDOF_SetLocalEqnos()\l* NewKBC_Controller()\l* NewFBC_Controller()\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l* SetCoordinates()\l* EquationNumbers()\l* CheckEquationNumbers()\l* NodeManagerT()\l* operator=()\l}"
template "UML:CLASS"
graphics
[
x 529.0
y 981.0
w 20.0
h 20.0
]
]
node
[
id 13
label "{Tahoe::FieldT\n|- fFieldSupport\l- fGroup\l- fIntegrator\l- fnIntegrator\l- fField_last\l- fIC\l- fKBC\l- fKBC_Controllers\l- fFBC\l- fFBCValues\l- fFBCEqnos\l- fFBC_Controllers\l- fUpdate\l- fEquationStart\l- fNumEquations\l- fID\l- fSourceOutput\l- fSourceID\l- fSourceBlocks\l- fTrackTotalEnergy\l- fTotalEnergyOutputInc\l- fTotalEnergyOutputID\l- fWork\l- fPointConnect\l- fActiveMass\l- fActiveForce\l- fActiveVel\l|+ FieldT()\l+ ~FieldT()\l+ Connectivities()\l+ SystemType()\l+ InitialConditions()\l+ KinematicBC()\l+ ForceBC()\l+ KBC_Controllers()\l+ FBC_Controllers()\l+ RelaxSystem()\l+ CloseStep()\l+ ResetStep()\l+ RegisterOutput()\l+ WriteOutput()\l+ WriteParameters()\l+ AddKBCController()\l+ AddFBCController()\l+ GetfFBCValues()\l+ GetfFBCEqnos()\l+ Initialize()\l+ RegisterLocal()\l+ SetGroup()\l+ Dimension()\l+ Clear()\l+ FieldSupport()\l+ operator()()\l+ operator()()\l+ Group()\l+ Integrator()\l+ Integrator()\l+ nIntegrator()\l+ nIntegrator()\l+ SetTimeStep()\l+ InitialCondition()\l+ InitStep()\l+ FormLHS()\l+ FormRHS()\l+ EndRHS()\l+ EndLHS()\l+ Update()\l+ AssembleUpdate()\l+ ApplyUpdate()\l+ CopyNodeToNode()\l+ InitEquations()\l+ FinalizeEquations()\l+ NumEquations()\l+ EquationStart()\l+ EquationSets()\l+ SetLocalEqnos()\l+ SetLocalEqnos()\l+ SetLocalEqnos()\l+ SetLocalEqnos()\l+ SetLocalEqnos()\l+ SetLocalEqnos()\l+ WriteRestart()\l+ ReadRestart()\l+ RegisterSource()\l+ Source()\l+ DefineParameters()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l- Apply_IC()\l- SetBCCode()\l- SetFBCEquations()\l- SourceIndex()\l* fIC\l* fKBC\l* fKBC_Controllers\l* fFBC\l* fFBCValues\l* fFBCEqnos\l* fFBC_Controllers\l* fUpdate\l* fEquationStart\l* fNumEquations\l* fID\l* fSourceOutput\l* fSourceID\l* fSourceBlocks\l* fTrackTotalEnergy\l* fTotalEnergyOutputInc\l* fTotalEnergyOutputID\l* fWork\l* fPointConnect\l* fActiveMass\l* fActiveForce\l* fActiveVel\l* Initialize()\l* RegisterLocal()\l* SetGroup()\l* Dimension()\l* Clear()\l* FieldSupport()\l* operator()()\l* operator()()\l* Group()\l* Integrator()\l* Integrator()\l* nIntegrator()\l* nIntegrator()\l* SetTimeStep()\l* InitialCondition()\l* InitStep()\l* FormLHS()\l* FormRHS()\l* EndRHS()\l* EndLHS()\l* Update()\l* AssembleUpdate()\l* ApplyUpdate()\l* CopyNodeToNode()\l* InitEquations()\l* FinalizeEquations()\l* NumEquations()\l* EquationStart()\l* EquationSets()\l* SetLocalEqnos()\l* SetLocalEqnos()\l* SetLocalEqnos()\l* SetLocalEqnos()\l* SetLocalEqnos()\l* SetLocalEqnos()\l* WriteRestart()\l* ReadRestart()\l* RegisterSource()\l* Source()\l* DefineParameters()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 161.0
y 531.0
w 20.0
h 20.0
]
]
node
[
id 12
label "{Tahoe::FieldSupportT\n||+ FieldSupportT()\l+ NewKBC_Controller()\l+ NewFBC_Controller()\l* NewKBC_Controller()\l* NewFBC_Controller()\l}"
template "UML:CLASS"
graphics
[
x 494.0
y 559.0
w 20.0
h 20.0
]
]
node
[
id 15
label "{Tahoe::IntegratorT\n|# fdt\l|+ IntegratorT()\l+ ~IntegratorT()\l+ SetTimeStep()\l+ FormNodalForce()\l+ ImplicitExplicit()\l+ Order()\l+ OrderOfUnknown()\l+ eIntegrator()\l+ nIntegrator()\l+ New()\l# ComputeParameters()\l* ImplicitExplicit()\l* Order()\l* OrderOfUnknown()\l* eIntegrator()\l* nIntegrator()\l}"
template "UML:CLASS"
graphics
[
x 338.0
y 349.0
w 20.0
h 20.0
]
]
node
[
id 14
label "{Tahoe::BasicFieldT\n|# fFieldName\l# fField\l# fLabels\l# fEqnos\l- fdArray2DGroup\l- fiArray2DGroup\l|+ BasicFieldT()\l+ ~BasicFieldT()\l+ Initialize()\l+ SetLabels()\l+ Dimension()\l+ Clear()\l+ FieldName()\l+ Labels()\l+ operator[]()\l+ operator[]()\l+ NumNodes()\l+ NumDOF()\l+ Order()\l+ EquationNumber()\l+ Equations()\l+ Equations()\l+ WriteEquationNumbers()\l# RegisterArray2D()\l# RegisterArray2D()\l* fdArray2DGroup\l* fiArray2DGroup\l* Initialize()\l* SetLabels()\l* Dimension()\l* Clear()\l* FieldName()\l* Labels()\l* operator[]()\l* operator[]()\l* NumNodes()\l* NumDOF()\l* Order()\l* EquationNumber()\l* Equations()\l* Equations()\l* WriteEquationNumbers()\l* RegisterArray2D()\l* RegisterArray2D()\l}"
template "UML:CLASS"
graphics
[
x 953.0
y 638.0
w 20.0
h 20.0
]
]
node
[
id 17
label "{Tahoe::GlobalMatrixT\n|# fOut\l# fComm\l# fPrintTag\l# fCheckCode\l# fTotNumEQ\l# fLocNumEQ\l# fStartEQ\l# sOutputCount\l|+ GlobalMatrixT()\l+ GlobalMatrixT()\l+ ~GlobalMatrixT()\l+ SolvePreservesData()\l+ Initialize()\l+ Info()\l+ Clear()\l+ Solve()\l+ AbsRowSum()\l+ operator=()\l+ Clone()\l+ Multx()\l+ MultTx()\l+ MultmBn()\l+ CopyDiagonal()\l+ SetPrintTag()\l+ Communicator()\l+ AddEquationSet()\l+ AddEquationSet()\l+ Assemble()\l+ Assemble()\l+ Assemble()\l+ OverWrite()\l+ Disassemble()\l+ DisassembleDiagonal()\l+ EquationNumberScope()\l+ RenumberEquations()\l+ CheckCode()\l+ NumEquations()\l+ NumTotEquations()\l+ StartEquation()\l+ MatrixType()\l+ PrintAllPivots()\l+ PrintZeroPivots()\l+ PrintLHS()\l# Factorize()\l# BackSubstitute()\l# PrintRHS()\l# PrintSolution()\l# Min()\l# Max()\l# Max()\l# Min()\l# Dot()\l* fCheckCode\l* fTotNumEQ\l* fLocNumEQ\l* fStartEQ\l* AddEquationSet()\l* AddEquationSet()\l* Assemble()\l* Assemble()\l* Assemble()\l* OverWrite()\l* Disassemble()\l* DisassembleDiagonal()\l* EquationNumberScope()\l* RenumberEquations()\l* CheckCode()\l* NumEquations()\l* NumTotEquations()\l* StartEquation()\l* MatrixType()\l* PrintAllPivots()\l* PrintZeroPivots()\l* PrintLHS()\l* PrintRHS()\l* PrintSolution()\l* Min()\l* Max()\l* Max()\l* Min()\l* Dot()\l}"
template "UML:CLASS"
graphics
[
x 498.0
y 510.0
w 20.0
h 20.0
]
]
node
[
id 16
label "{Tahoe::nIntegratorT\n||+ nIntegratorT()\l+ ~nIntegratorT()\l+ nIntegrator()\l+ Dimension()\l+ ExternalNodeCondition()\l+ Predictor()\l+ Corrector()\l+ Corrector()\l+ MappedCorrector()\l+ MappedCorrectorField()\l+ ConsistentKBC()\l# nComputeParameters()\l}"
template "UML:CLASS"
graphics
[
x 241.0
y 184.0
w 20.0
h 20.0
]
]
node
[
id 19
label "{Tahoe::eIntegratorT\n||+ eIntegratorT()\l+ ~eIntegratorT()\l+ eIntegrator()\l+ FormM()\l+ FormC()\l+ FormK()\l+ FormMa()\l+ FormCv()\l+ FormKd()\l# eComputeParameters()\l* FormM()\l* FormC()\l* FormK()\l* FormMa()\l* FormCv()\l* FormKd()\l}"
template "UML:CLASS"
graphics
[
x 186.0
y 117.0
w 20.0
h 20.0
]
]
node
[
id 18
label "{Tahoe::CommManagerT\n|- fComm\l- fModelManager\l- fSize\l- fRank\l- fSkin\l- fBounds\l- fPartition\l- fNodeManager\l- fFirstConfigure\l- fIsPeriodic\l- fPeriodicBoundaries\l- fPeriodicLength\l- fNumRealNodes\l- fPBCNodes\l- fPBCNodes_ghost\l- fPBCNodes_face\l- fProcessor\l- fNodeMap\l- fPartitionNodes\l- fPartitionNodes_inv\l- fExternalNodes\l- fBorderNodes\l- fNumValues\l- fCommunications\l- fGhostCommunications\l- fd_send_buffer\l- fd_recv_buffer\l- fd_send_buffer_man\l- fd_recv_buffer_man\l- fi_send_buffer\l- fi_recv_buffer\l- fi_send_buffer_man\l- fi_recv_buffer_man\l- fAdjacentCommID\l- fSwap\l- fSendNodes\l- fRecvNodes\l|+ CommManagerT()\l+ ~CommManagerT()\l+ Size()\l+ Rank()\l+ Skin()\l+ SetSkin()\l+ SetPartition()\l+ SetNodeManager()\l+ Communicator()\l+ Configure()\l+ SetPeriodicBoundaries()\l+ ClearPeriodicBoundaries()\l+ PeriodicBoundaries()\l+ EnforcePeriodicBoundaries()\l+ NumRealNodes()\l+ ProcessorMap()\l+ NodeMap()\l+ PartitionNodes()\l+ PartitionNodesChanging()\l+ PartitionNodes_inv()\l+ ExternalNodes()\l+ BorderNodes()\l+ NodesWithGhosts()\l+ GhostNodes()\l+ Init_AllGather()\l+ Init_AllGather()\l+ Init_AllGather()\l+ Clear_AllGather()\l+ AllGather()\l+ AllGather()\l- Partition()\l- NodeManager()\l- CollectPartitionNodes()\l- FirstConfigure()\l- GetProcessorBounds()\l- InitConfigure()\l- Distribute()\l- SetExchange()\l- CloseConfigure()\l- CommManagerT()\l- operator=()\l* fIsPeriodic\l* fPeriodicBoundaries\l* fPeriodicLength\l* fNumRealNodes\l* fPBCNodes\l* fPBCNodes_ghost\l* fPBCNodes_face\l* fProcessor\l* fNodeMap\l* fPartitionNodes\l* fPartitionNodes_inv\l* fExternalNodes\l* fBorderNodes\l* fNumValues\l* fCommunications\l* fGhostCommunications\l* fd_send_buffer\l* fd_recv_buffer\l* fd_send_buffer_man\l* fd_recv_buffer_man\l* fi_send_buffer\l* fi_recv_buffer\l* fi_send_buffer_man\l* fi_recv_buffer_man\l* fAdjacentCommID\l* fSwap\l* fSendNodes\l* fRecvNodes\l* SetPeriodicBoundaries()\l* ClearPeriodicBoundaries()\l* PeriodicBoundaries()\l* EnforcePeriodicBoundaries()\l* NumRealNodes()\l* ProcessorMap()\l* NodeMap()\l* PartitionNodes()\l* PartitionNodesChanging()\l* PartitionNodes_inv()\l* ExternalNodes()\l* BorderNodes()\l* NodesWithGhosts()\l* GhostNodes()\l* Init_AllGather()\l* Init_AllGather()\l* Init_AllGather()\l* Clear_AllGather()\l* AllGather()\l* AllGather()\l* InitConfigure()\l* Distribute()\l* SetExchange()\l* CloseConfigure()\l* CommManagerT()\l* operator=()\l}"
template "UML:CLASS"
graphics
[
x 291.0
y 320.0
w 20.0
h 20.0
]
]
node
[
id 31
label "{Tahoe::MeshFreeT\n||}"
template "UML:CLASS"
graphics
[
x 121.0
y 502.0
w 20.0
h 20.0
]
]
node
[
id 30
label "{Tahoe::MeshFreeSupportT\n|# fDextra\l# fStoreShape\l# fScaledSupport\l# fMeshfreeType\l# fNumFacetNodes\l# fCutCoords\l# fCoords\l# fDomain\l# fSD\l# fIP\l# fEFG\l# fRKPM\l# fGrid\l# fSkipNode\l# fnNeighborCount\l# fnNeighborData\l# fSkipElement\l# feNeighborCount\l# feNeighborData\l# fneighbors\l# fvolume\l# fvolume_man\l# fnodal_param\l# fnodal_param_ip\l# fnodal_param_man\l# fcoords\l# fcoords_man\l# fx_ip_table\l# felShapespace\l# fndShapespace\l# fConnects\l# fNonGridNodes\l# fVolume\l# fNodalParameters\l# fNodesUsed\l# fnPhiData\l# fnDPhiData\l# fePhiData\l# feDPhiData\l# fResetNodes\l# fResetElems\l# fReformNode\l# fReformElem\l|+ MeshFreeSupportT()\l+ MeshFreeSupportT()\l+ ~MeshFreeSupportT()\l+ InitSupportParameters()\l+ InitNeighborData()\l+ SetSkipNodes()\l+ SkipNodes()\l+ SetSkipElements()\l+ SkipElements()\l+ SynchronizeSupportParameters()\l+ SetSupportParameters()\l+ GetSupportParameters()\l+ NodalParameters()\l+ NodalVolumes()\l+ SetCuttingFacets()\l+ ResetFacets()\l+ ResetNodes()\l+ ResetCells()\l+ BuildNeighborhood()\l+ ElementNeighborsCounts()\l+ ElementNeighbors()\l+ NodeNeighbors()\l+ NodesUsed()\l+ NodalCoordinates()\l+ WriteParameters()\l+ WriteStatistics()\l+ WriteNodalNeighbors()\l+ WriteNodalShapes()\l+ LoadNodalData()\l+ LoadElementData()\l+ SetFieldAt()\l+ SetFieldUsing()\l+ NeighborsAt()\l+ FieldAt()\l+ DFieldAt()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l+ New_MLSSolverT()\l# SetSearchGrid()\l# SetNodeNeighborData()\l# SetNodeNeighborData_2()\l# SetElementNeighborData()\l# SetElementNeighborData_2()\l# SetNodalShapeFunctions()\l# SetElementShapeFunctions()\l# InitNodalShapeData()\l# InitElementShapeData()\l# ProcessBoundaries()\l# Visible()\l- Covers()\l- ComputeElementData()\l- ComputeNodalData()\l- SetSupport_Spherical_Search()\l- SetSupport_Spherical_Connectivities()\l- SetSupport_Cartesian_Connectivities()\l- SetNodesUsed()\l- SwapData()\l* WriteParameters()\l* WriteStatistics()\l* WriteNodalNeighbors()\l* WriteNodalShapes()\l* LoadNodalData()\l* LoadElementData()\l* SetFieldAt()\l* SetFieldUsing()\l* NeighborsAt()\l* FieldAt()\l* DFieldAt()\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 545.0
y 528.0
w 20.0
h 20.0
]
]
node
[
id 36
label "{Tahoe::MeshFreeNodalShapeFunctionT\n|# fMFSupport\l# fSD\l# fNeighbors\l# fNaU\l# fDNaU\l# fNonGridNodes\l|+ MeshFreeNodalShapeFunctionT()\l+ ~MeshFreeNodalShapeFunctionT()\l+ SetSupportSize()\l+ SetNeighborData()\l+ SetSkipNodes()\l+ SetNodalParameters()\l+ GetNodalParameters()\l+ NodalParameters()\l+ NodalVolumes()\l+ SetFieldAt()\l+ SetFieldUsing()\l+ FieldAt()\l+ SetDerivatives()\l+ SetDerivativesAt()\l+ DFieldAt()\l+ UseDerivatives()\l+ SetCuttingFacets()\l+ ResetFacets()\l+ ResetNodes()\l+ ResetCells()\l+ NumberOfNeighbors()\l+ Neighbors()\l+ NodeNeighbors()\l+ SelectedNodalField()\l+ NodalField()\l+ NodalField()\l+ Print()\l+ PrintAt()\l+ WriteParameters()\l+ WriteStatistics()\l+ MeshFreeSupport()\l}"
template "UML:CLASS"
graphics
[
x 382.0
y 230.0
w 20.0
h 20.0
]
]
node
[
id 35
label "{Tahoe::MeshFreeElementSupportT\n|# fMFShapes\l# fNodalShapes\l# fLocGroup\l# fNumElemenNodes\l# fElemNodesEX\l# fElemEqnosEX\l# fUNodeLists\l# fNEEArray\l# fNEEMatrix\l# fFENodes\l# fEFGNodes\l# fAllFENodes\l# fOffGridNodes\l# fFieldSet\l# fNodalU\l# fGlobalToNodesUsedMap\l# fMapShift\l- fOffGridID\l- fInterpolantID\l- fMeshlessID\l|+ MeshFreeElementSupportT()\l+ ~MeshFreeElementSupportT()\l+ MeshFreeSupport()\l+ SetShape()\l+ MarkActiveCells()\l+ InitSupport()\l+ TraceNode()\l+ WeightNodes()\l+ InterpolantNodes()\l+ OffGridNodes()\l+ ElementNodes()\l+ ElementEquations()\l+ NumElementNodes()\l+ SetElementNodes()\l+ Register()\l+ Register()\l+ Register()\l+ DefineSubs()\l+ TakeParameterList()\l+ SetNodalField()\l+ GetNodalField()\l+ FreeNodalField()\l- CollectNodesData()\l- SetAllFENodes()\l* fOffGridID\l* fInterpolantID\l* fMeshlessID\l* InterpolantNodes()\l* OffGridNodes()\l* ElementNodes()\l* ElementEquations()\l* NumElementNodes()\l* SetElementNodes()\l* Register()\l* Register()\l* Register()\l* DefineSubs()\l* TakeParameterList()\l* SetNodalField()\l* GetNodalField()\l* FreeNodalField()\l}"
template "UML:CLASS"
graphics
[
x 183.0
y 746.0
w 20.0
h 20.0
]
]
node
[
id 34
label "{Tahoe::MeshFreeFractureSupportT\n|- fNumFacetNodes\l- fFacets\l- fFacetman\l- fResetFacets\l- fInitTractions\l- fInitTractionMan\l- fs_i\l- fda\l- fda_s\l- fcone\l- fn_s\l- fFrontList\l- fSamplingSurfaces\l- fCriterion\l- fs_u\l- fhoop\l- ftmp_nsd\l|+ MeshFreeFractureSupportT()\l+ ~MeshFreeFractureSupportT()\l+ NumFacetNodes()\l+ Facets()\l+ ResetFacets()\l+ InitTractions()\l+ WriteOutput()\l+ InitStep()\l+ CloseStep()\l+ ResetStep()\l+ HasActiveCracks()\l+ FractureCriterion()\l+ CheckGrowth()\l+ InitSupport()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# int2FractureCriterionT()\l- InitCuttingFacetsAndFronts()\l- InitSamplingSurfaces()\l- InitializeFronts()\l- CheckFronts()\l- CheckSurfaces()\l- InitFacetDatabase()\l- ComputeCriterion()\l- SetStreamPrefs()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 536.0
y 27.0
w 20.0
h 20.0
]
]
node
[
id 33
label "{Tahoe::WindowT\n||+ WindowT()\l+ ~WindowT()\l+ SearchType()\l+ NumberOfSupportParameters()\l+ SynchronizeSupportParameters()\l+ Name()\l+ WriteParameters()\l+ Window()\l+ Covers()\l+ SphericalSupportSize()\l+ RectangularSupportSize()\l+ Window()\l+ Covers()\l+ SphericalSupportSize()\l+ RectangularSupportSize()\l* Window()\l* Covers()\l* SphericalSupportSize()\l* RectangularSupportSize()\l* Window()\l* Covers()\l* SphericalSupportSize()\l* RectangularSupportSize()\l}"
template "UML:CLASS"
graphics
[
x 714.0
y 757.0
w 20.0
h 20.0
]
]
node
[
id 32
label "{Tahoe::MLSSolverT\n|# fNumSD\l# fComplete\l# fNumThirdDer\l# fOrder\l# fNumNeighbors\l# fBasis\l# fLocCoords\l# fw\l# fDw\l# fDDw\l# fDDDw\l# fb\l# fDb\l# fDDb\l# fDDDb\l# fMinv\l# fDM\l# fDDM\l# fDDDM\l# fC\l# fDC\l# fDDC\l# fDDDC\l# fphi\l# fDphi\l# fDDphi\l# fDDDphi\l# fArrayGroup\l# fArray2DGroup2\l# fArray2DGroup3\l# fArray2DGroup4\l# fLocCoords_man\l# fWindowType\l# fWindow\l# fOrigin\l- fNSDsym\l- fMtemp\l- fbtemp1\l- fbtemp2\l- fbtemp3\l- fbtemp4\l- fbtemp5\l- fbtemp6\l- fbtemp7\l- fbtemp8\l- fbtemp9\l|+ MLSSolverT()\l+ ~MLSSolverT()\l+ WriteParameters()\l+ Initialize()\l+ SetField()\l+ phi()\l+ Dphi()\l+ DDphi()\l+ DDDphi()\l+ SearchType()\l+ Covers()\l+ BasisDimension()\l+ NumberOfSupportParameters()\l+ SynchronizeSupportParameters()\l+ w()\l+ Dw()\l+ DDw()\l+ DDDw()\l+ b()\l+ Db()\l+ DDb()\l+ DDDb()\l+ C()\l+ DC()\l+ DDC()\l+ DDDC()\l+ SphericalSupportSize()\l+ SphericalSupportSize()\l+ RectangularSupportSize()\l+ RectangularSupportSize()\l- Window()\l- Dimension()\l- SetMomentMatrix()\l- ComputeM()\l- ComputeDM()\l- ComputeDDM()\l- ComputeDDDM()\l- SetCorrectionCoefficient()\l- SetCorrection()\l- SetShapeFunctions()\l- SymmetricInverse3x3()\l- SymmetricInverse4x4()\l* fWindowType\l* fWindow\l* fOrigin\l* SphericalSupportSize()\l* SphericalSupportSize()\l* RectangularSupportSize()\l* RectangularSupportSize()\l}"
template "UML:CLASS"
graphics
[
x 149.0
y 384.0
w 20.0
h 20.0
]
]
edge
[
source 2
target 1
generalization 0
]
edge
[
source 3
target 2
generalization 0
]
edge
[
source 4
target 3
generalization 0
]
edge
[
source 5
target 4
generalization 0
]
edge
[
source 6
target 5
generalization 0
]
edge
[
source 7
target 6
generalization 0
]
edge
[
source 8
target 7
generalization 1
]
edge
[
source 9
target 8
generalization 0
]
edge
[
source 10
target 9
generalization 1
]
edge
[
source 11
target 10
generalization 0
]
edge
[
source 12
target 10
generalization 1
]
edge
[
source 9
target 12
generalization 0
]
edge
[
source 13
target 10
generalization 1
]
edge
[
source 14
target 13
generalization 0
]
edge
[
source 15
target 13
generalization 1
]
edge
[
source 12
target 13
generalization 1
]
edge
[
source 16
target 13
generalization 1
]
edge
[
source 15
target 16
generalization 0
]
edge
[
source 17
target 13
generalization 1
]
edge
[
source 18
target 10
generalization 1
]
edge
[
source 10
target 18
generalization 1
]
edge
[
source 18
target 9
generalization 1
]
edge
[
source 13
target 7
generalization 1
]
edge
[
source 19
target 7
generalization 1
]
edge
[
source 15
target 19
generalization 0
]
edge
[
source 20
target 6
generalization 1
]
edge
[
source 21
target 20
generalization 0
]
edge
[
source 22
target 6
generalization 1
]
edge
[
source 23
target 6
generalization 1
]
edge
[
source 24
target 5
generalization 1
]
edge
[
source 25
target 24
generalization 0
]
edge
[
source 26
target 25
generalization 1
]
edge
[
source 9
target 26
generalization 0
]
edge
[
source 6
target 26
generalization 1
]
edge
[
source 20
target 4
generalization 1
]
edge
[
source 27
target 4
generalization 1
]
edge
[
source 28
target 27
generalization 0
]
edge
[
source 26
target 28
generalization 0
]
edge
[
source 5
target 28
generalization 1
]
edge
[
source 4
target 27
generalization 1
]
edge
[
source 29
target 1
generalization 1
]
edge
[
source 20
target 29
generalization 0
]
edge
[
source 30
target 29
generalization 1
]
edge
[
source 31
target 30
generalization 0
]
edge
[
source 32
target 30
generalization 1
]
edge
[
source 33
target 32
generalization 1
]
edge
[
source 34
target 1
generalization 1
]
edge
[
source 35
target 34
generalization 0
]
edge
[
source 36
target 35
generalization 1
]
edge
[
source 30
target 36
generalization 1
]
edge
[
source 29
target 35
generalization 1
]
]
