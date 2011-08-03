graph
[
node
[
id 1
label "{Tahoe::RampedDampingT\n|# qNodesInRegion\l# fBeta\l|+ RampedDampingT()\l+ ApplyDamping()\l* fBeta\l}"
template "UML:CLASS"
graphics
[
x 911.0
y 363.0
w 20.0
h 20.0
]
]
node
[
id 2
label "{Tahoe::MeshFreeFSSolidAxiT\n|# fMFShapes\l# fMFFractureSupport\l# fAutoBorder\l# fStressStiff_wrap\l# fB_wrap\l# fGradNa_wrap\l# fDNa_x_wrap\l# fConnectsAll\l# fMeshfreeParameters\l|+ MeshFreeFSSolidAxiT()\l+ ~MeshFreeFSSolidAxiT()\l+ Equations()\l+ ConnectsU()\l+ WriteOutput()\l+ RelaxSystem()\l+ InterpolantDOFs()\l+ NodalDOFs()\l+ WeightNodalCost()\l+ InitStep()\l+ CloseStep()\l+ ResetStep()\l+ MeshFreeSupport()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# SetShape()\l# NextElement()\l# ComputeOutput()\l- WriteField()\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 646.0
y 560.0
w 20.0
h 20.0
]
]
node
[
id 3
label "{Tahoe::tevp2D\n|# fStress\l# fModulus\l# fStrainEnergyDensity\l# fInternal\l# fTempKirchoff\l# fTempCauchy\l- fDt\l- fFtot_2D\l- fFtot\l- fDtot\l- fGradV_2D\l- fGradV\l- fLocVel\l- fF_temp\l- fSpin\l- fCriticalStrain\l- fEbtot\l- fXxii\l- fCtcon\l- fPP\l- fDmat\l- fEP_tan\l- fEcc\l- fStressMatrix\l- fStressArray\l- fStill3D\l- fStress3D\l- fSmlp\l- fSymStress2D\l- fJ\l- fVisc\l- Bvisc\l- fTemperature\l- fSb\l- fEb\l- fEffectiveStrainRate\l- fPressure\l- Temp_0\l- El_E\l- El_V\l- El_K\l- El_G\l- Sb0\l- Rho0\l- Eb0\l- Eb0tot\l- BigN\l- Smm\l- Alpha_T\l- Delta\l- Theta\l- Kappa\l- Cp\l- Chi\l- Ccc\l- Pcp\l- Epsilon_1\l- Epsilon_2\l- Epsilon_rate\l- Gamma_d\l- Mu_d\l- SigCr\l- Xi\l|+ tevp2D()\l+ NeedsPointInitialization()\l+ PointInitialize()\l+ NeedVel()\l+ UpdateHistory()\l+ ResetHistory()\l+ C_IJKL()\l+ S_IJ()\l+ StrainEnergyDensity()\l+ NumOutputVariables()\l+ OutputLabels()\l+ ComputeOutput()\l+ c_ijkl()\l+ s_ij()\l+ Pressure()\l- ComputeF()\l- ComputeD()\l- ComputeSpin()\l- ComputeEbtotCtconXxii()\l- ComputePP()\l- ComputeEcc()\l- ComputeDmat()\l- ComputeEP_tan()\l- ComputeSmlp()\l- AllocateElement()\l- ComputeFluidTemperature()\l- ComputeViscoTemperature()\l- ComputeEffectiveStress()\l- ComputeFluidEffectiveStrain()\l- ComputeViscoEffectiveStrain()\l- CheckCriticalCriteria()\l- CheckIfPlastic()\l- LoadData()\l- Update()\l- Reset()\l- ArrayToMatrix()\l- MatrixToArray()\l- Return3DStress()\l- ArrayToSymMatrix2D()\l* c_ijkl()\l* s_ij()\l* Pressure()\l}"
template "UML:CLASS"
graphics
[
x 701.0
y 487.0
w 20.0
h 20.0
]
]
node
[
id 4
label "{Tahoe::GaussIsokineticT\n||+ GaussIsokineticT()\l+ ~GaussIsokineticT()\l+ ApplyDamping()\l}"
template "UML:CLASS"
graphics
[
x 83.0
y 608.0
w 20.0
h 20.0
]
]
node
[
id 5
label "{Tahoe::BridgingScaleT\n|# fSolid\l# fSolidNodesUsed\l# fParticlesInCell\l# fInverseMapInCell\l# fCoarseScale\l# fFineScale\l# fUx\l# fUy\l# fWtempU\l# fFineScaleU\l# fTotalNodes\l# fConnect\l# fAtomConnect\l# fElMatU\l# fGlobalMass\l# fNodalOutputCodes\l# fElementOutputCodes\l# fLocInitCoords\l# fLocDisp\l# fNEEvec\l# fDOFvec\l- fParticleOutputID\l- fSolidOutputID\l|+ BridgingScaleT()\l+ ~BridgingScaleT()\l+ BridgingFields()\l+ InitialProject()\l+ RegisterOutput()\l+ WriteOutput()\l+ TangentType()\l+ AddNodalForce()\l+ InternalEnergy()\l+ SendOutput()\l+ InitInterpolation()\l+ InterpolateField()\l+ InitProjection()\l+ ProjectField()\l+ CoarseField()\l+ ProjectImagePoints()\l+ CollectProjectedCells()\l+ CollectProjectedNodes()\l+ Compute_B_hatU_U()\l+ DefineParameters()\l+ TakeParameterList()\l# SolidElement()\l# MaptoCells()\l# ShapeFunction()\l# InitialCoordinates()\l# Displacements()\l# EchoConnectivityData()\l# LHSDriver()\l# RHSDriver()\l# SetLocalArrays()\l# CurrElementInfo()\l* fParticleOutputID\l* fSolidOutputID\l* InitInterpolation()\l* InterpolateField()\l* InitProjection()\l* ProjectField()\l* CoarseField()\l* ProjectImagePoints()\l* CollectProjectedCells()\l* CollectProjectedNodes()\l* Compute_B_hatU_U()\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 190.0
y 424.0
w 20.0
h 20.0
]
]
node
[
id 6
label "{Tahoe::LinearSolver\n|- fFormLHS\l|+ LinearSolver()\l+ Initialize()\l+ InitStep()\l+ Solve()\l+ SetTimeStep()\l}"
template "UML:CLASS"
graphics
[
x 432.0
y 604.0
w 20.0
h 20.0
]
]
node
[
id 7
label "{Tahoe::OgdenIsotropicT\n|# fSpectralDecomp\l# fC\l# fEigs\l# fdWdE\l# fddWddE\l# fModMat\l# fModulus\l# fStress\l|+ OgdenIsotropicT()\l+ ~OgdenIsotropicT()\l+ C_IJKL()\l+ S_IJ()\l+ c_ijkl()\l+ s_ij()\l+ Pressure()\l+ TakeParameterList()\l# dWdE()\l# ddWddE()\l# PurePlaneStress()\l- MixedRank4_2D()\l- MixedRank4_3D()\l* c_ijkl()\l* s_ij()\l* Pressure()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 534.0
y 655.0
w 20.0
h 20.0
]
]
node
[
id 8
label "{Tahoe::VIB2D\n|- fCircle\l|+ VIB2D()\l+ ~VIB2D()\l+ SetAngle()\l+ Perturb()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# ComputeModuli()\l# ComputePK2()\l# ComputeEnergyDensity()\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 792.0
y 702.0
w 20.0
h 20.0
]
]
node
[
id 9
label "{Tahoe::HookeanMatT\n|- fModulus\l|+ HookeanMatT()\l+ HookeanMatT()\l+ ~HookeanMatT()\l+ Dimension()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l# SetModulus()\l# Modulus()\l# HookeanStress()\l# HookeanEnergy()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 366.0
y 459.0
w 20.0
h 20.0
]
]
node
[
id 10
label "{Tahoe::MeshfreeBridgingT\n|- fMLS\l- fSupportParams\l- fMeshFreeSupport\l|+ MeshfreeBridgingT()\l+ ~MeshfreeBridgingT()\l+ InitProjection()\l+ ProjectField()\l+ CoarseField()\l+ ProjectImagePoints()\l+ CollectProjectedCells()\l+ CollectProjectedNodes()\l+ Compute_B_hatU_U()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# BuildNodalNeighborhoods()\l# BuildPointNeighborhoods()\l* InitProjection()\l* ProjectField()\l* CoarseField()\l* ProjectImagePoints()\l* CollectProjectedCells()\l* CollectProjectedNodes()\l* Compute_B_hatU_U()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 570.0
y 783.0
w 20.0
h 20.0
]
]
node
[
id 11
label "{Tahoe::SSSolidMatList2DT\n|- fSSMatSupport\l- fGradSSMatSupport\l|+ SSSolidMatList2DT()\l+ SSSolidMatList2DT()\l+ HasPlaneStress()\l+ NewSSSolidMat()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 801.0
y 205.0
w 20.0
h 20.0
]
]
node
[
id 12
label "{Tahoe::TotalLagrangianAxiT\n|# fIPShape\l# fOutputInit\l# fOutputCell\l# fStressMat\l# fStressStiff\l# fGradNa\l# fTemp2\l# fTempMat1\l# fTempMat2\l# fDNa_x\l|+ TotalLagrangianAxiT()\l+ TakeParameterList()\l# FormStiffness()\l# FormKd()\l* fStressMat\l* fStressStiff\l* fGradNa\l* fTemp2\l* fTempMat1\l* fTempMat2\l* fDNa_x\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 933.0
y 446.0
w 20.0
h 20.0
]
]
node
[
id 13
label "{Tahoe::SurfaceShapeT\n|- fTotalNodes\l- fNumFacetNodes\l- fNumFacets\l- fFieldDim\l- fCoords\l- fFacetCoords\l- fNa\l- fjumpNa\l- fgrad_d\l- fgrad_dTgrad_d\l- fgrad_dd\l- fInterp\l- fIPCoord\l- fJacobian\l- fFacetNodes\l- fNodalValues\l- fu_vec\l- fx_vec\l- fM1\l- fM2\l- fdm1_du\l- fdm2_du\l|+ SurfaceShapeT()\l+ SurfaceShapeT()\l+ TotalNodes()\l+ NumFacetNodes()\l+ FieldDim()\l+ Initialize()\l+ NodesOnFacets()\l+ SetJumpVector()\l+ InterpolateJumpU()\l+ InterpolateJump()\l+ Interpolate()\l+ Interpolate()\l+ IPCoords()\l+ Extrapolate()\l+ Grad_d()\l+ Grad_dTGrad_d()\l+ Jacobian()\l+ Jacobian()\l+ Jacobian()\l+ Jacobian()\l+ Shapes()\l+ JumpShapes()\l- Construct()\l- SetNodesOnFacets()\l- ComputeFacetCoords()\l* InterpolateJumpU()\l* InterpolateJump()\l* Interpolate()\l* Interpolate()\l* IPCoords()\l* Extrapolate()\l* Grad_d()\l* Grad_dTGrad_d()\l* Jacobian()\l* Jacobian()\l* Jacobian()\l* Jacobian()\l* Shapes()\l* JumpShapes()\l}"
template "UML:CLASS"
graphics
[
x 270.0
y 806.0
w 20.0
h 20.0
]
]
node
[
id 14
label "{Tahoe::nLinearHHTalpha\n|- dn\l- vn\l- dpred_v\l- dpred_a\l- vpred_a\l- dcorr_d\l- dcorr_dpred\l- dcorr_a\l- vcorr_v\l- vcorr_vpred\l- vcorr_a\l- dalpha_a\l- valpha_a\l- fField\l|+ nLinearHHTalpha()\l+ Dimension()\l+ ConsistentKBC()\l+ ExternalNodeCondition()\l+ Predictor()\l+ Corrector()\l+ Corrector()\l+ MappedCorrector()\l+ MappedCorrectorField()\l# nComputeParameters()\l}"
template "UML:CLASS"
graphics
[
x 971.0
y 653.0
w 20.0
h 20.0
]
]
node
[
id 15
label "{Tahoe::nVerlet\n|- vcorr_a\l- dpred_v\l- dpred_a\l- vpred_a\l|+ nVerlet()\l+ ConsistentKBC()\l+ ExternalNodeCondition()\l+ Predictor()\l+ Corrector()\l+ Corrector()\l+ MappedCorrector()\l+ MappedCorrectorField()\l# nComputeParameters()\l* dpred_v\l* dpred_a\l* vpred_a\l}"
template "UML:CLASS"
graphics
[
x 776.0
y 656.0
w 20.0
h 20.0
]
]
node
[
id 16
label "{Tahoe::MeshFreeCSEAnisoT\n|# fGeometryCode\l# fNumIntPts\l# fOutputArea\l# fMFElementGroup\l# fMFFractureSupport\l# fMFSurfaceShape\l# fSurfacePotential\l# fLocDisp\l# fFractureArea\l# fNEEmat\l# fNEEvec\l# fQ\l# fdelta\l# fT\l# fddU_l\l# fddU_g\l# fdQ\l# fnsd_nee_1\l# fnsd_nee_2\l# fElemEqnosEX\l# fActiveFlag\l# fLocGroup\l# fNEEArray\l# fNEEMatrix\l# fMatrixManager\l# fInitTraction\l# fd_Storage\l# fd_Storage_last\l# fd_Storage_man\l# fd_Storage_last_man\l|+ MeshFreeCSEAnisoT()\l+ ~MeshFreeCSEAnisoT()\l+ TangentType()\l+ InitialCondition()\l+ CloseStep()\l+ ResetStep()\l+ RelaxSystem()\l+ AddNodalForce()\l+ InternalEnergy()\l+ Equations()\l+ RegisterOutput()\l+ WriteOutput()\l+ SendOutput()\l+ ConnectsX()\l+ ConnectsU()\l+ InterpolantDOFs()\l# EchoConnectivityData()\l# LHSDriver()\l# RHSDriver()\l# CurrElementInfo()\l- InitializeNewFacets()\l- SetNumberOfNodes()\l- u_i__Q_ijk()\l- Q_ijk__u_j()\l}"
template "UML:CLASS"
graphics
[
x 958.0
y 213.0
w 20.0
h 20.0
]
]
node
[
id 17
label "{Tahoe::eStaticIntegrator\n|- fLHSMode\l|+ eStaticIntegrator()\l+ FormM()\l+ FormC()\l+ FormK()\l+ FormMa()\l+ FormCv()\l+ FormKd()\l# eComputeParameters()\l* LHSModeT\l* SetLHSMode()\l* FormM()\l* FormC()\l* FormK()\l* FormMa()\l* FormCv()\l* FormKd()\l}"
template "UML:CLASS"
graphics
[
x 219.0
y 903.0
w 20.0
h 20.0
]
]
node
[
id 18
label "{Tahoe::SmallStrainAxiT\n|- fIPInterp\l- fIPShape\l- fStrain2D\l- fStress2D_axi\l|+ SmallStrainAxiT()\l+ CollectMaterialInfo()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# Axisymmetric()\l# NewMaterialSupport()\l# SetLocalArrays()\l# FormKd()\l# FormStiffness()\l# SetGlobalShape()\l- SetMeanGradient()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 491.0
y 619.0
w 20.0
h 20.0
]
]
node
[
id 19
label "{Tahoe::FSSolidMatList1DT\n|- fFSMatSupport\l|+ FSSolidMatList1DT()\l+ FSSolidMatList1DT()\l+ NewFSSolidMat()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 60.0
y 332.0
w 20.0
h 20.0
]
]
node
[
id 20
label "{Tahoe::HarmonicPairT\n|- fR0\l- fK\l- sR0\l- sK\l|+ HarmonicPairT()\l+ HarmonicPairT()\l+ getEnergyFunction()\l+ getForceFunction()\l+ getStiffnessFunction()\l+ DefineParameters()\l+ TakeParameterList()\l- Energy()\l- Force()\l- Stiffness()\l* sR0\l* sK\l* getEnergyFunction()\l* getForceFunction()\l* getStiffnessFunction()\l* DefineParameters()\l* TakeParameterList()\l* Energy()\l* Force()\l* Stiffness()\l}"
template "UML:CLASS"
graphics
[
x 499.0
y 390.0
w 20.0
h 20.0
]
]
node
[
id 21
label "{Tahoe::UpdatedLagrangianT\n|# fLocCurrCoords\l# fCauchyStress\l# fStressStiff\l# fGradNa\l|+ UpdatedLagrangianT()\l+ ~UpdatedLagrangianT()\l+ DefineParameters()\l+ TakeParameterList()\l# SetLocalArrays()\l# SetShape()\l# SetGlobalShape()\l# FormStiffness()\l# FormKd()\l* fCauchyStress\l* fStressStiff\l* fGradNa\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 430.0
y 672.0
w 20.0
h 20.0
]
]
node
[
id 22
label "{Tahoe::SSMatSupportT\n|- fSmallStrain\l- fStrain_List\l- fStrain_last_List\l|+ SSMatSupportT()\l+ ~SSMatSupportT()\l+ LinearStrain()\l+ LinearStrain()\l+ LinearStrain_last()\l+ LinearStrain_last()\l+ SetLinearStrain()\l+ SetLinearStrain_last()\l+ SmallStrain()\l+ SetContinuumElement()\l* fStrain_List\l* fStrain_last_List\l* LinearStrain()\l* LinearStrain()\l* LinearStrain_last()\l* LinearStrain_last()\l* SetLinearStrain()\l* SetLinearStrain_last()\l* SmallStrain()\l* SetContinuumElement()\l}"
template "UML:CLASS"
graphics
[
x 20.0
y 492.0
w 20.0
h 20.0
]
]
node
[
id 23
label "{Tahoe::TensorTransformT\n|- fRank2\l- fRank4\l- fPull\l- fTransform\l- fOuter\l- fRedMat\l|+ TensorTransformT()\l+ TensorTransformT()\l+ Dimension()\l+ PushForward()\l+ PullBack()\l+ PushForward()\l+ PullBack()\l- FFFFC_2D()\l- FFFFC_3D()\l- FFFFC_2D_Z()\l}"
template "UML:CLASS"
graphics
[
x 44.0
y 129.0
w 20.0
h 20.0
]
]
node
[
id 24
label "{Tahoe::EAM\n|# fPairPotential\l# fEmbeddingEnergy\l# fElectronDensity\l- fLattice\l- fBondTensor4\l- fAmn\l- fBondTensor2\l- fBondTensor2b\l- fTensor2Table\l- fIntType\l- fBond1\l- fBond2\l- fBond3\l- fBond4\l- fBond5\l- fBond6\l- fBond7\l- fBond8\l- fBond9\l- fRepRho\l|+ EAM()\l+ ~EAM()\l+ Initialize()\l+ ComputeUnitEnergy()\l+ ComputeUnitSurfaceEnergy()\l+ ComputeUnitStress()\l+ ComputeUnitSurfaceStress()\l+ ComputeUnitModuli()\l+ LatticeParameter()\l+ Mass()\l+ TotalElectronDensity()\l+ ComputeElectronDensity()\l+ PairPotential()\l+ EmbeddingEnergy()\l+ ElectronDensity()\l- FormMixedDerivatives()\l- FormSingleBondContribution()\l- FormMixedBondContribution()\l- SetPairPotential()\l- SetEmbeddingEnergy()\l- SetElectronDensity()\l* fPairPotential\l* fEmbeddingEnergy\l* fElectronDensity\l* PairPotential()\l* EmbeddingEnergy()\l* ElectronDensity()\l}"
template "UML:CLASS"
graphics
[
x 712.0
y 601.0
w 20.0
h 20.0
]
]
node
[
id 25
label "{Tahoe::MeshFreeElementSupportT\n|# fMFShapes\l# fNodalShapes\l# fLocGroup\l# fNumElemenNodes\l# fElemNodesEX\l# fElemEqnosEX\l# fUNodeLists\l# fNEEArray\l# fNEEMatrix\l# fFENodes\l# fEFGNodes\l# fAllFENodes\l# fOffGridNodes\l# fFieldSet\l# fNodalU\l# fGlobalToNodesUsedMap\l# fMapShift\l- fOffGridID\l- fInterpolantID\l- fMeshlessID\l|+ MeshFreeElementSupportT()\l+ ~MeshFreeElementSupportT()\l+ MeshFreeSupport()\l+ SetShape()\l+ MarkActiveCells()\l+ InitSupport()\l+ TraceNode()\l+ WeightNodes()\l+ InterpolantNodes()\l+ OffGridNodes()\l+ ElementNodes()\l+ ElementEquations()\l+ NumElementNodes()\l+ SetElementNodes()\l+ Register()\l+ Register()\l+ Register()\l+ DefineSubs()\l+ TakeParameterList()\l+ SetNodalField()\l+ GetNodalField()\l+ FreeNodalField()\l- CollectNodesData()\l- SetAllFENodes()\l* fOffGridID\l* fInterpolantID\l* fMeshlessID\l* InterpolantNodes()\l* OffGridNodes()\l* ElementNodes()\l* ElementEquations()\l* NumElementNodes()\l* SetElementNodes()\l* Register()\l* Register()\l* Register()\l* DefineSubs()\l* TakeParameterList()\l* SetNodalField()\l* GetNodalField()\l* FreeNodalField()\l}"
template "UML:CLASS"
graphics
[
x 914.0
y 45.0
w 20.0
h 20.0
]
]
node
[
id 26
label "{Tahoe::OgdenIsoVIB2D\n|# fCircle\l|+ OgdenIsoVIB2D()\l+ ~OgdenIsoVIB2D()\l+ StrainEnergyDensity()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# dWdE()\l# ddWddE()\l# PurePlaneStress()\l# ComputeLengths()\l- Construct()\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 808.0
y 109.0
w 20.0
h 20.0
]
]
node
[
id 27
label "{Tahoe::FCCLatticeT\n|- fNumShells\l|+ FCCLatticeT()\l+ NumShells()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l+ SetQ()\l# LoadBondTable()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 995.0
y 624.0
w 20.0
h 20.0
]
]
node
[
id 28
label "{Tahoe::RectGaussianWindowT\n|- fDilationScaling\l- fSharpeningFactor\l- fCutOffFactor\l- fNSD\l- fNSDsym\l- fNSDArray\l|+ RectGaussianWindowT()\l+ SearchType()\l+ Name()\l+ NumberOfSupportParameters()\l+ SynchronizeSupportParameters()\l+ WriteParameters()\l+ Window()\l+ Window()\l+ Covers()\l+ Covers()\l+ SphericalSupportSize()\l+ RectangularSupportSize()\l* Covers()\l* Covers()\l* SphericalSupportSize()\l* RectangularSupportSize()\l}"
template "UML:CLASS"
graphics
[
x 99.0
y 487.0
w 20.0
h 20.0
]
]
node
[
id 29
label "{Tahoe::ParticleT\n|# fOutputID\l# fNeighborDistance\l# fLatticeParameter\l# fNearestNeighborDistance\l# fReNeighborDisp\l# fReNeighborIncr\l# fGrid\l# fReNeighborCoords\l# fDmax\l# fThermostats\l# fID\l# fPointConnectivities\l# fPeriodicSkin\l# fPeriodicBounds\l# fStretchSchedule\l# fTypeNames\l# fType\l# fTypeMessageID\l# fPropertiesMap\l# fParticleProperties\l# fActiveParticles\l# fForce\l# fForce_man\l# fForce_list\l# fForce_list_man\l# fOneOne\l- fReNeighborCounter\l- fhas_periodic\l- fPeriodicLengths\l|+ ParticleT()\l+ ~ParticleT()\l+ TangentType()\l+ AddNodalForce()\l+ InternalEnergy()\l+ RegisterOutput()\l+ WriteOutput()\l+ SendOutput()\l+ RelaxSystem()\l+ SetSkipParticles()\l+ SetConfiguration()\l+ FormStiffness()\l+ InternalForce()\l+ PropertiesMap()\l+ LumpedMass()\l+ SetPeriodicSkin()\l+ AtomicKineticEnergies()\l+ WriteRestart()\l+ ReadRestart()\l+ DefineParameters()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l# GenerateOutputLabels()\l# ChangingGeometry()\l# GenerateNeighborList()\l# AssembleParticleMass()\l# MaxDisplacement()\l# ApplyDamping()\l# SetDamping()\l# New_Thermostat()\l# Calc_Slip_and_Strain()\l# Combination()\l# Calc_CSP()\l# Calc_CN()\l# SetRefNN()\l# ExtractProperties()\l* fID\l* fPointConnectivities\l* fPeriodicSkin\l* fPeriodicBounds\l* fStretchSchedule\l* fTypeNames\l* fType\l* fTypeMessageID\l* fPropertiesMap\l* fParticleProperties\l* fActiveParticles\l* fForce\l* fForce_man\l* fForce_list\l* fForce_list_man\l* fOneOne\l* WriteRestart()\l* ReadRestart()\l* DefineParameters()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l* ExtractProperties()\l}"
template "UML:CLASS"
graphics
[
x 514.0
y 283.0
w 20.0
h 20.0
]
]
node
[
id 30
label "{Tahoe::CSESymAnisoT\n|- fK\l- fSideSet_ID\l|+ CSESymAnisoT()\l+ RegisterOutput()\l+ TakeParameterList()\l# ReadConnectivity()\l# LHSDriver()\l# RHSDriver()\l# ComputeOutput()\l# CurrElementInfo()\l# NumFacetNodes()\l# CollectBlockInfo()\l- u_i__Q_ijk()\l- Q_ijk__u_j()\l* TakeParameterList()\l* CollectBlockInfo()\l}"
template "UML:CLASS"
graphics
[
x 82.0
y 965.0
w 20.0
h 20.0
]
]
node
[
id 31
label "{Tahoe::CSEBaseT\n|# fGeometryCode\l# fNumIntPts\l# fAxisymmetric\l# fCloseSurfaces\l# fOutputArea\l# fShapes\l# fLocInitCoords1\l# fLocCurrCoords\l# fNodes1\l# fNEEvec\l# fNEEmat\l# fOutputBlockID\l# fOutputID\l# fNodalOutputCodes\l# fElementOutputCodes\l# fNodesUsed\l# fOutputGlobalTractions\l# farea_out\l# fFractureArea\l# fpc_AndOr\l# fpc_coordinate\l# fpc_op\l# fpc_value\l# NumNodalOutputCodes\l# NumElementOutputCodes\l|+ CSEBaseT()\l+ ~CSEBaseT()\l+ InitialCondition()\l+ CloseStep()\l+ ResetStep()\l+ AddNodalForce()\l+ InternalEnergy()\l+ RegisterOutput()\l+ WriteOutput()\l+ SendOutput()\l+ ResolveOutputVariable()\l+ Axisymmetric()\l+ DefineParameters()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l# CurrElementInfo()\l# NumFacetNodes()\l# DefaultNumElemNodes()\l# CollectBlockInfo()\l# SetNodalOutputCodes()\l# SetElementOutputCodes()\l# ComputeOutput()\l# GenerateOutputLabels()\l- CloseSurfaces()\l* AndOrT\l* OpT\l* CoordinateT\l* int2AndOrT()\l* int2OpT()\l* int2CoordinateT()\l* fOutputID\l* fNodalOutputCodes\l* fElementOutputCodes\l* fNodesUsed\l* fOutputGlobalTractions\l* farea_out\l* fFractureArea\l* fpc_AndOr\l* fpc_coordinate\l* fpc_op\l* fpc_value\l* DefineParameters()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l* CollectBlockInfo()\l* SetNodalOutputCodes()\l* SetElementOutputCodes()\l* ComputeOutput()\l* GenerateOutputLabels()\l}"
template "UML:CLASS"
graphics
[
x 271.0
y 305.0
w 20.0
h 20.0
]
]
node
[
id 32
label "{Tahoe::SSSolidMatList3DT\n|- fSSMatSupport\l|+ SSSolidMatList3DT()\l+ SSSolidMatList3DT()\l+ NewSSSolidMat()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 879.0
y 518.0
w 20.0
h 20.0
]
]
node
[
id 33
label "{Tahoe::ContactT\n|# fNumFacetNodes\l# fSurfaces\l# fStrikerTags\l# fStrikerArea\l# fStrikerTags_map\l# fStrikerCoords\l# fSurfaceLinks\l# fEqnos_man\l# fStrikerForce2D\l# fActiveMap\l# fActiveStrikers\l# fHitSurface\l# fHitFacets\l- fnum_contact\l- fh_max\l|+ ContactT()\l+ ~ContactT()\l+ TangentType()\l+ InitialCondition()\l+ RelaxSystem()\l+ InitStep()\l+ AddNodalForce()\l+ InternalEnergy()\l+ SendOutput()\l+ RegisterOutput()\l+ WriteOutput()\l+ ConnectsU()\l+ ConnectsX()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# WriteContactInfo()\l# SetContactConfiguration()\l# SetTrackingData()\l# ExtractContactGeometry()\l# SetWorkSpace()\l# SetActiveInteractions()\l# SetConnectivities()\l# InputSideSets()\l# InputBodyBoundary()\l# StrikersFromSurfaces()\l# StrikersFromNodeSets()\l# StrikersFromSideSets()\l* fActiveMap\l* fActiveStrikers\l* fHitSurface\l* fHitFacets\l* fnum_contact\l* fh_max\l* RegisterOutput()\l* WriteOutput()\l* ConnectsU()\l* ConnectsX()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l* ExtractContactGeometry()\l* SetWorkSpace()\l* SetActiveInteractions()\l* SetConnectivities()\l* InputSideSets()\l* InputBodyBoundary()\l* StrikersFromSurfaces()\l* StrikersFromNodeSets()\l* StrikersFromSideSets()\l}"
template "UML:CLASS"
graphics
[
x 626.0
y 388.0
w 20.0
h 20.0
]
]
node
[
id 34
label "{Tahoe::ParadynPairT\n|- fSupport\l- fDescription\l- fCoefficients\l- fAtomicNumber\l- fLatticeParameter\l- fStructure\l- f_cut\l- f_1bydr\l- s_nr\l- s_1bydr\l- s_coeff\l|+ ParadynPairT()\l+ ParadynPairT()\l+ Coefficients()\l+ getEnergyFunction()\l+ getForceFunction()\l+ getStiffnessFunction()\l+ getParadynTable()\l+ DefineParameters()\l+ TakeParameterList()\l- ReadParameters()\l- ComputeCoefficients()\l- Energy()\l- Force()\l- Stiffness()\l* fAtomicNumber\l* fLatticeParameter\l* fStructure\l* f_cut\l* f_1bydr\l* s_nr\l* s_1bydr\l* s_coeff\l* getEnergyFunction()\l* getForceFunction()\l* getStiffnessFunction()\l* getParadynTable()\l* DefineParameters()\l* TakeParameterList()\l* Energy()\l* Force()\l* Stiffness()\l}"
template "UML:CLASS"
graphics
[
x 66.0
y 802.0
w 20.0
h 20.0
]
]
node
[
id 35
label "{Tahoe::SolidMatListT\n|# fSolidMatSupport\l# fHasLocalizers\l# fHasThermal\l|+ SolidMatListT()\l+ SolidMatListT()\l+ HasLocalizingMaterials()\l+ HasThermalStrains()\l+ HasHeatSources()\l+ HasPlaneStress()\l* fHasLocalizers\l* fHasThermal\l}"
template "UML:CLASS"
graphics
[
x 397.0
y 872.0
w 20.0
h 20.0
]
]
node
[
id 36
label "{Tahoe::CommManagerT\n|- fComm\l- fModelManager\l- fSize\l- fRank\l- fSkin\l- fBounds\l- fPartition\l- fNodeManager\l- fFirstConfigure\l- fIsPeriodic\l- fPeriodicBoundaries\l- fPeriodicLength\l- fNumRealNodes\l- fPBCNodes\l- fPBCNodes_ghost\l- fPBCNodes_face\l- fProcessor\l- fNodeMap\l- fPartitionNodes\l- fPartitionNodes_inv\l- fExternalNodes\l- fBorderNodes\l- fNumValues\l- fCommunications\l- fGhostCommunications\l- fd_send_buffer\l- fd_recv_buffer\l- fd_send_buffer_man\l- fd_recv_buffer_man\l- fi_send_buffer\l- fi_recv_buffer\l- fi_send_buffer_man\l- fi_recv_buffer_man\l- fAdjacentCommID\l- fSwap\l- fSendNodes\l- fRecvNodes\l|+ CommManagerT()\l+ ~CommManagerT()\l+ Size()\l+ Rank()\l+ Skin()\l+ SetSkin()\l+ SetPartition()\l+ SetNodeManager()\l+ Communicator()\l+ Configure()\l+ SetPeriodicBoundaries()\l+ ClearPeriodicBoundaries()\l+ PeriodicBoundaries()\l+ EnforcePeriodicBoundaries()\l+ NumRealNodes()\l+ ProcessorMap()\l+ NodeMap()\l+ PartitionNodes()\l+ PartitionNodesChanging()\l+ PartitionNodes_inv()\l+ ExternalNodes()\l+ BorderNodes()\l+ NodesWithGhosts()\l+ GhostNodes()\l+ Init_AllGather()\l+ Init_AllGather()\l+ Init_AllGather()\l+ Clear_AllGather()\l+ AllGather()\l+ AllGather()\l- Partition()\l- NodeManager()\l- CollectPartitionNodes()\l- FirstConfigure()\l- GetProcessorBounds()\l- InitConfigure()\l- Distribute()\l- SetExchange()\l- CloseConfigure()\l- CommManagerT()\l- operator=()\l* fIsPeriodic\l* fPeriodicBoundaries\l* fPeriodicLength\l* fNumRealNodes\l* fPBCNodes\l* fPBCNodes_ghost\l* fPBCNodes_face\l* fProcessor\l* fNodeMap\l* fPartitionNodes\l* fPartitionNodes_inv\l* fExternalNodes\l* fBorderNodes\l* fNumValues\l* fCommunications\l* fGhostCommunications\l* fd_send_buffer\l* fd_recv_buffer\l* fd_send_buffer_man\l* fd_recv_buffer_man\l* fi_send_buffer\l* fi_recv_buffer\l* fi_send_buffer_man\l* fi_recv_buffer_man\l* fAdjacentCommID\l* fSwap\l* fSendNodes\l* fRecvNodes\l* SetPeriodicBoundaries()\l* ClearPeriodicBoundaries()\l* PeriodicBoundaries()\l* EnforcePeriodicBoundaries()\l* NumRealNodes()\l* ProcessorMap()\l* NodeMap()\l* PartitionNodes()\l* PartitionNodesChanging()\l* PartitionNodes_inv()\l* ExternalNodes()\l* BorderNodes()\l* NodesWithGhosts()\l* GhostNodes()\l* Init_AllGather()\l* Init_AllGather()\l* Init_AllGather()\l* Clear_AllGather()\l* AllGather()\l* AllGather()\l* InitConfigure()\l* Distribute()\l* SetExchange()\l* CloseConfigure()\l* CommManagerT()\l* operator=()\l}"
template "UML:CLASS"
graphics
[
x 620.0
y 924.0
w 20.0
h 20.0
]
]
node
[
id 37
label "{Tahoe::UpLagr_ExternalFieldT\n|# fExternalFieldFormat\l# fExternalFieldFile\l# fExternalFieldLabels\l# fFieldVariableIndex\l# fTimeSteps\l# fNodeMap\l# fLocExternalField\l# fExternalField\l# fNodalValues\l|+ UpLagr_ExternalFieldT()\l+ Initialize()\l+ ExternalField()\l+ InitStep()\l# NextElement()\l- AssembleField()\l}"
template "UML:CLASS"
graphics
[
x 739.0
y 66.0
w 20.0
h 20.0
]
]
node
[
id 38
label "{Tahoe::iNLSolver_LS\n|- fFormTangent\l- fLineSearch\l- fIterationStatus\l|+ iNLSolver_LS()\l+ Solve()\l+ iDoCommand()\l- Update()\l- DoStep()\l- DoInitStep()\l- DoIterate()\l}"
template "UML:CLASS"
graphics
[
x 726.0
y 311.0
w 20.0
h 20.0
]
]
node
[
id 39
label "{Tahoe::DiffusionMatListT\n|- fDiffusionMatSupport\l|+ DiffusionMatListT()\l+ DiffusionMatListT()\l+ NewDiffusionMaterial()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 880.0
y 185.0
w 20.0
h 20.0
]
]
node
[
id 40
label "{Tahoe::IsoVIB2D\n|# fCircle\l# fEigs\l# fEigmods\l# fSpectral\l- fb\l- fModulus\l- fStress\l|+ IsoVIB2D()\l+ ~IsoVIB2D()\l+ C_IJKL()\l+ S_IJ()\l+ StrainEnergyDensity()\l+ CurvatureTensor()\l+ c_ijkl()\l+ s_ij()\l+ Pressure()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# ComputeLengths()\l- Construct()\l* c_ijkl()\l* s_ij()\l* Pressure()\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 582.0
y 746.0
w 20.0
h 20.0
]
]
node
[
id 41
label "{Tahoe::CSEAnisoT\n|# fRotate\l# fCurrShapes\l# fNumStateVariables\l# fSurfPots\l# fTiedPots\l# qRetieNodes\l# fRunState\l# fIncrementalHeat\l# fQ\l# fdelta\l# fT\l# fddU\l# fdQ\l# fIPArea\l# fnsd_nee_1\l# fnsd_nee_2\l# fCalcNodalInfo\l# fNodalInfoCode\l# iTiedFlagIndex\l# fNodalQuantities\l# iBulkGroups\l# otherInds\l# freeNodeQ\l# freeNodeQ_last\l# fStateVariables\l# fStateVariables_last\l- fsigma_max\l|+ CSEAnisoT()\l+ ~CSEAnisoT()\l+ GetElemStatus()\l+ TangentType()\l+ InitialCondition()\l+ CloseStep()\l+ ResetStep()\l+ WriteRestart()\l+ ReadRestart()\l+ StateVariables()\l+ StateVariables_Last()\l+ SendOutput()\l+ SetStatus()\l+ Interpolate()\l+ NumIP()\l+ DefineParameters()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l# LHSDriver()\l# RHSDriver()\l# SetNodalOutputCodes()\l# SetElementOutputCodes()\l# ComputeOutput()\l# GenerateOutputLabels()\l# CurrElementInfo()\l# u_i__Q_ijk()\l# Q_ijk__u_j()\l# ComputeFreeNodesForOutput()\l# StoreBulkOutput()\l# SurfaceValuesFromBulk()\l# FromNodesToIPs()\l# UntieOrRetieNodes()\l# InitializeTemperature()\l* fStateVariables\l* fStateVariables_last\l* DefineParameters()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 273.0
y 385.0
w 20.0
h 20.0
]
]
node
[
id 42
label "{Tahoe::VoterChenCu\n||+ VoterChenCu()\l+ LatticeParameter()\l+ Mass()\l- SetPairPotential()\l- SetEmbeddingEnergy()\l- SetElectronDensity()\l}"
template "UML:CLASS"
graphics
[
x 356.0
y 135.0
w 20.0
h 20.0
]
]
node
[
id 43
label "{Tahoe::SimoFiniteStrainT\n|# fIncompressibleMode\l# fModeSolveMethod\l# fLocalIterationMax\l# fAbsTol\l# fRelTol\l# fModesConverged\l# fNumModeShapes\l# fEnhancedModeTags\l# fEnhancedConnectivities\l# fElementModes\l# fCurrElementModes\l# fElementModes_last\l# fCurrElementModes_last\l# fEnhancedShapes\l# fF_enh_List\l# fF_enh_all\l# fF_enh_last_List\l# fF_enh_last_all\l# fF_Galerkin_List\l# fF_Galerkin_all\l# fF_Galerkin_last_List\l# fF_Galerkin_last_all\l# fPK1_storage\l# fPK1_list\l# fc_ijkl_storage\l# fc_ijkl_list\l# fStressMat\l# fStressStiff_11\l# fStressStiff_12\l# fStressStiff_21\l# fStressStiff_22\l# fGradNa\l# fTempMat1\l# fTempMat2\l# fDNa_x\l# fDNa_x_enh\l# fWP_enh\l# fGradNa_enh\l# fRHS_enh\l# fB_enh\l# fK22\l# fK12\l# fK11\l# fK21\l|+ SimoFiniteStrainT()\l+ ~SimoFiniteStrainT()\l+ Initialize()\l+ CloseStep()\l+ ResetStep()\l+ ReadRestart()\l+ WriteRestart()\l+ ConnectsU()\l+ Equations()\l+ SetDOFTags()\l+ DOFTags()\l+ GenerateElementData()\l+ DOFConnects()\l+ ResetDOF()\l+ Reconfigure()\l+ ResetState()\l+ Group()\l# NextElement()\l# SetShape()\l# SetGlobalShape()\l# FormStiffness()\l# FormKd()\l# CurrElementInfo()\l- ModifiedEnhancedDeformation()\l- ComputeEnhancedDeformation()\l- FormStiffness_staggered()\l- FormStiffness_enhanced()\l- FormStiffness_monolithic()\l- FormKd_staggered()\l- FormKd_monolithic()\l- FormKd_enhanced()\l}"
template "UML:CLASS"
graphics
[
x 285.0
y 339.0
w 20.0
h 20.0
]
]
node
[
id 44
label "{Tahoe::SolverT\n|# fFEManager\l# fGroup\l# fNumIteration\l# fEigenSolverParameters\l# fMatrixType\l# fPrintEquationNumbers\l# fLHS\l# fLHS_lock\l# fLHS_update\l# fPerturbation\l# fRHS\l# fRHS_lock\l|+ SolverT()\l+ ~SolverT()\l+ Initialize()\l+ ReceiveEqns()\l+ ReceiveEqns()\l+ IterationNumber()\l+ Check()\l+ RHS()\l+ LHS()\l+ EquationNumberScope()\l+ RenumberEquations()\l+ Group()\l+ InitStep()\l+ Solve()\l+ CloseStep()\l+ ResetStep()\l+ SetTimeStep()\l+ UnlockRHS()\l+ LockRHS()\l+ UnlockLHS()\l+ LockLHS()\l+ AssembleLHS()\l+ AssembleLHS()\l+ AssembleLHS()\l+ OverWriteLHS()\l+ DisassembleLHS()\l+ DisassembleLHSDiagonal()\l+ AssembleRHS()\l+ AssembleRHS()\l+ OverWriteRHS()\l+ DisassembleRHS()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l+ New()\l# Residual()\l# InnerProduct()\l# ApproximateLHS()\l# CompareLHS()\l- CheckMatrixType()\l- SetGlobalMatrix()\l* fMatrixType\l* fPrintEquationNumbers\l* fLHS\l* fLHS_lock\l* fLHS_update\l* fPerturbation\l* fRHS\l* fRHS_lock\l* InitStep()\l* Solve()\l* CloseStep()\l* ResetStep()\l* SetTimeStep()\l* UnlockRHS()\l* LockRHS()\l* UnlockLHS()\l* LockLHS()\l* AssembleLHS()\l* AssembleLHS()\l* AssembleLHS()\l* OverWriteLHS()\l* DisassembleLHS()\l* DisassembleLHSDiagonal()\l* AssembleRHS()\l* AssembleRHS()\l* OverWriteRHS()\l* DisassembleRHS()\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l* ApproximateLHS()\l* CompareLHS()\l}"
template "UML:CLASS"
graphics
[
x 201.0
y 18.0
w 20.0
h 20.0
]
]
node
[
id 45
label "{Tahoe::PeriodicNodesT\n|# fIsPeriodic\l# fPeriodicStride\l|+ PeriodicNodesT()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# InitTiedNodePairs()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 330.0
y 331.0
w 20.0
h 20.0
]
]
node
[
id 46
label "{Tahoe::MeshFreeNodalShapeFunctionT\n|# fMFSupport\l# fSD\l# fNeighbors\l# fNaU\l# fDNaU\l# fNonGridNodes\l|+ MeshFreeNodalShapeFunctionT()\l+ ~MeshFreeNodalShapeFunctionT()\l+ SetSupportSize()\l+ SetNeighborData()\l+ SetSkipNodes()\l+ SetNodalParameters()\l+ GetNodalParameters()\l+ NodalParameters()\l+ NodalVolumes()\l+ SetFieldAt()\l+ SetFieldUsing()\l+ FieldAt()\l+ SetDerivatives()\l+ SetDerivativesAt()\l+ DFieldAt()\l+ UseDerivatives()\l+ SetCuttingFacets()\l+ ResetFacets()\l+ ResetNodes()\l+ ResetCells()\l+ NumberOfNeighbors()\l+ Neighbors()\l+ NodeNeighbors()\l+ SelectedNodalField()\l+ NodalField()\l+ NodalField()\l+ Print()\l+ PrintAt()\l+ WriteParameters()\l+ WriteStatistics()\l+ MeshFreeSupport()\l}"
template "UML:CLASS"
graphics
[
x 25.0
y 518.0
w 20.0
h 20.0
]
]
node
[
id 47
label "{Tahoe::D2MeshFreeSupport2DT\n||+ D2MeshFreeSupport2DT()\l+ D2MeshFreeSupport2DT()\l+ SetCuttingFacets()\l- ProcessBoundaries()\l- Visible()\l- Intersect()\l}"
template "UML:CLASS"
graphics
[
x 770.0
y 613.0
w 20.0
h 20.0
]
]
node
[
id 48
label "{Tahoe::TiedPotentialT\n|- qTv\l- d_n\l- d_t\l- phi_n\l- r_fail\l- fsigma\l- fL_0\l- fL_1\l- fL_2\l- q\l- r\l- fnvec1\l- fnvec2\l- fsigma_critical\l- qRetieNodes\l|+ TiedPotentialT()\l+ NumStateVariables()\l+ InitStateVariables()\l+ FractureEnergy()\l+ Potential()\l+ Traction()\l+ Stiffness()\l+ Status()\l+ NumOutputVariables()\l+ OutputLabels()\l+ ComputeOutput()\l+ NeedsNodalInfo()\l+ NodalQuantityNeeded()\l+ RotateNodalQuantity()\l+ InitiationQ()\l+ NodesMayRetie()\l+ RetieQ()\l+ TiedStatusPosition()\l# CompatibleOutput()\l}"
template "UML:CLASS"
graphics
[
x 17.0
y 890.0
w 20.0
h 20.0
]
]
node
[
id 49
label "{Tahoe::VIB3D\n|- fSphere\l|+ VIB3D()\l+ ~VIB3D()\l+ SetAngles()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l# ComputeModuli()\l# ComputePK2()\l# ComputeEnergyDensity()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 135.0
y 430.0
w 20.0
h 20.0
]
]
node
[
id 50
label "{Tahoe::J2PrimitiveT\n|# fYield\l# fH_bar\l# ftheta\l|+ J2PrimitiveT()\l+ ~J2PrimitiveT()\l+ DefineParameters()\l+ TakeParameterList()\l# YieldCondition()\l# H()\l# dH()\l# K()\l# dK()\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 965.0
y 27.0
w 20.0
h 20.0
]
]
node
[
id 51
label "{Tahoe::GlobalMatrixT\n|# fOut\l# fComm\l# fPrintTag\l# fCheckCode\l# fTotNumEQ\l# fLocNumEQ\l# fStartEQ\l# sOutputCount\l|+ GlobalMatrixT()\l+ GlobalMatrixT()\l+ ~GlobalMatrixT()\l+ SolvePreservesData()\l+ Initialize()\l+ Info()\l+ Clear()\l+ Solve()\l+ AbsRowSum()\l+ operator=()\l+ Clone()\l+ Multx()\l+ MultTx()\l+ MultmBn()\l+ CopyDiagonal()\l+ SetPrintTag()\l+ Communicator()\l+ AddEquationSet()\l+ AddEquationSet()\l+ Assemble()\l+ Assemble()\l+ Assemble()\l+ OverWrite()\l+ Disassemble()\l+ DisassembleDiagonal()\l+ EquationNumberScope()\l+ RenumberEquations()\l+ CheckCode()\l+ NumEquations()\l+ NumTotEquations()\l+ StartEquation()\l+ MatrixType()\l+ PrintAllPivots()\l+ PrintZeroPivots()\l+ PrintLHS()\l# Factorize()\l# BackSubstitute()\l# PrintRHS()\l# PrintSolution()\l# Min()\l# Max()\l# Max()\l# Min()\l# Dot()\l* fCheckCode\l* fTotNumEQ\l* fLocNumEQ\l* fStartEQ\l* AddEquationSet()\l* AddEquationSet()\l* Assemble()\l* Assemble()\l* Assemble()\l* OverWrite()\l* Disassemble()\l* DisassembleDiagonal()\l* EquationNumberScope()\l* RenumberEquations()\l* CheckCode()\l* NumEquations()\l* NumTotEquations()\l* StartEquation()\l* MatrixType()\l* PrintAllPivots()\l* PrintZeroPivots()\l* PrintLHS()\l* PrintRHS()\l* PrintSolution()\l* Min()\l* Max()\l* Max()\l* Min()\l* Dot()\l}"
template "UML:CLASS"
graphics
[
x 302.0
y 414.0
w 20.0
h 20.0
]
]
node
[
id 52
label "{Tahoe::XuNeedleman2DT\n|- q\l- r\l- d_n\l- d_t\l- phi_n\l- r_fail\l- fKratio\l- fK\l|+ XuNeedleman2DT()\l+ NumStateVariables()\l+ FractureEnergy()\l+ Potential()\l+ Traction()\l+ Stiffness()\l+ Status()\l+ DefineParameters()\l+ TakeParameterList()\l* q\l* r\l* d_n\l* d_t\l* phi_n\l* r_fail\l* fKratio\l* fK\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 554.0
y 28.0
w 20.0
h 20.0
]
]
node
[
id 53
label "{Tahoe::QuadLogOgden2DT\n|- flogE\l|+ QuadLogOgden2DT()\l+ StrainEnergyDensity()\l# dWdE()\l# ddWddE()\l}"
template "UML:CLASS"
graphics
[
x 339.0
y 119.0
w 20.0
h 20.0
]
]
node
[
id 54
label "{Tahoe::D2MeshFreeSupportT\n|# fD2EFG\l# fnDDPhiData\l# feDDPhiData\l|+ D2MeshFreeSupportT()\l+ D2MeshFreeSupportT()\l+ InitNeighborData()\l+ LoadNodalData()\l+ LoadElementData()\l+ SetFieldAt()\l+ DDFieldAt()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# SetNodalShapeFunctions()\l# SetElementShapeFunctions()\l# InitNodalShapeData()\l# InitElementShapeData()\l- ComputeNodalData()\l- ComputeElementData()\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 129.0
y 371.0
w 20.0
h 20.0
]
]
node
[
id 55
label "{Tahoe::FSSolidMatT\n|# fFSMatSupport\l# fTemperatureField\l# fTemperature\l# fStress\l# fModulus\l- fQ\l- fF_therm_inv\l- fF_therm_inv_last\l- fF_mechanical\l- F_0_\l- vec_\l- stress_\l|+ FSSolidMatT()\l+ SetFSMatSupport()\l+ FSMatSupport()\l+ ce_ijkl()\l+ S_IJ()\l+ IsLocalized()\l+ IsLocalized()\l+ IsLocalized()\l+ InitStep()\l+ CloseStep()\l+ Need_F()\l+ Need_F_last()\l+ F()\l+ F()\l+ F_total()\l+ F_total()\l+ F_mechanical()\l+ F_mechanical()\l+ F_total_last()\l+ F_total_last()\l+ F_thermal_inverse()\l+ F_thermal_inverse_last()\l+ F_mechanical_last()\l+ F_mechanical_last()\l+ Strain()\l+ Stretch()\l+ c_ijkl()\l+ C_IJKL()\l+ TakeParameterList()\l# Compute_b()\l# Compute_C()\l# Compute_E()\l# Compute_b()\l# Compute_C()\l# Compute_E()\l# Compute_Temperature()\l# Compute_Temperature_last()\l# AcousticalTensor()\l# FiniteStrain()\l- SetInverseThermalTransformation()\l- ComputeQ_2D()\l- ComputeQ_3D()\l* fStress\l* fModulus\l* F_0_\l* vec_\l* stress_\l* c_ijkl()\l* C_IJKL()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 244.0
y 796.0
w 20.0
h 20.0
]
]
node
[
id 56
label "{Tahoe::ElementSupportT\n||+ ElementSupportT()\l+ ~ElementSupportT()\l+ ElementGroupNumber()\l+ AssembleLHS()\l+ AssembleRHS()\l+ AssembleLHS()\l+ AssembleLHS()\l+ ResetAverage()\l+ AssembleAverage()\l+ OutputAverage()\l+ OutputUsedAverage()\l+ WriteOutput()\l+ WriteOutput()\l+ WriteOutput()\l+ WriteOutput()\l+ Argv()\l+ CommandLineOption()\l+ CommandLineOption()\l* AssembleLHS()\l* AssembleRHS()\l* AssembleLHS()\l* AssembleLHS()\l* ResetAverage()\l* AssembleAverage()\l* OutputAverage()\l* OutputUsedAverage()\l* WriteOutput()\l* WriteOutput()\l* WriteOutput()\l* WriteOutput()\l* Argv()\l* CommandLineOption()\l* CommandLineOption()\l}"
template "UML:CLASS"
graphics
[
x 140.0
y 47.0
w 20.0
h 20.0
]
]
node
[
id 57
label "{Tahoe::PointInCellDataT\n|- fContinuumElement\l- fPointInCell\l- fPointInCellCoords\l- fCellNodes\l- fCellConnectivities\l- fPointToNode\l- fPointToPoint\l- fNodeToNode\l- fGlobalToLocal\l- fInterpolatingCell\l- fInterpolationWeights\l|+ PointInCellDataT()\l+ CollectCellNodes()\l+ GenerateCellConnectivities()\l+ CellNodes()\l+ CellConnectivities()\l+ SetContinuumElement()\l+ ContinuumElement()\l+ PointInCell()\l+ PointInCell()\l+ PointInCellCoords()\l+ PointInCellCoords()\l+ InterpolationWeights()\l+ InterpolationWeights()\l+ InterpolatingCell()\l+ InterpolatingCell()\l+ GlobalToLocal()\l+ GlobalToLocal()\l+ InterpolationDataToMatrix()\l+ PointToNode()\l+ PointToNode()\l+ PointToPoint()\l+ PointToPoint()\l+ NodeToNode()\l+ NodeToNode()\l* fGlobalToLocal\l* fInterpolatingCell\l* fInterpolationWeights\l* SetContinuumElement()\l* ContinuumElement()\l* PointInCell()\l* PointInCell()\l* PointInCellCoords()\l* PointInCellCoords()\l* InterpolationWeights()\l* InterpolationWeights()\l* InterpolatingCell()\l* InterpolatingCell()\l* GlobalToLocal()\l* GlobalToLocal()\l* InterpolationDataToMatrix()\l* PointToNode()\l* PointToNode()\l* PointToPoint()\l* PointToPoint()\l* NodeToNode()\l* NodeToNode()\l}"
template "UML:CLASS"
graphics
[
x 519.0
y 253.0
w 20.0
h 20.0
]
]
node
[
id 58
label "{Tahoe::RodMaterialT\n|# fMass\l|+ RodMaterialT()\l+ ~RodMaterialT()\l+ Mass()\l+ Potential()\l+ DPotential()\l+ DDPotential()\l}"
template "UML:CLASS"
graphics
[
x 388.0
y 85.0
w 20.0
h 20.0
]
]
node
[
id 59
label "{Tahoe::Lattice1DT\n|- fNumShells\l|+ Lattice1DT()\l+ NumShells()\l# LoadBondTable()\l}"
template "UML:CLASS"
graphics
[
x 81.0
y 300.0
w 20.0
h 20.0
]
]
node
[
id 60
label "{Tahoe::FSSolidMatList3DT\n|- fFSMatSupport\l|+ FSSolidMatList3DT()\l+ FSSolidMatList3DT()\l+ NewFSSolidMat()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 550.0
y 612.0
w 20.0
h 20.0
]
]
node
[
id 61
label "{Tahoe::NLSolver_LSX\n|- fPuntTol\l- fMinStepRelError\l|+ NLSolver_LSX()\l+ InitStep()\l# ExitIteration()\l}"
template "UML:CLASS"
graphics
[
x 922.0
y 220.0
w 20.0
h 20.0
]
]
node
[
id 62
label "{Tahoe::FBC_CardT\n|- fNode\l- fDOF\l- fValue\l- fSchedule\l|+ FBC_CardT()\l+ SetValues()\l+ SplitForce()\l+ Destination()\l+ Node()\l+ DOF()\l+ Schedule()\l+ CurrentValue()\l}"
template "UML:CLASS"
graphics
[
x 170.0
y 79.0
w 20.0
h 20.0
]
]
node
[
id 63
label "{Tahoe::Verlet\n||+ Verlet()\l+ ImplicitExplicit()\l+ Order()\l+ OrderOfUnknown()\l* ImplicitExplicit()\l* Order()\l* OrderOfUnknown()\l}"
template "UML:CLASS"
graphics
[
x 73.0
y 60.0
w 20.0
h 20.0
]
]
node
[
id 64
label "{Tahoe::DomainIntegrationT\n|# fNumIP\l# fCurrIP\l# fDomain\l# fSurfShapes\l# fDelete\l- fDeleteDomain\l- frefCurrIP\l|+ DomainIntegrationT()\l+ DomainIntegrationT()\l+ ~DomainIntegrationT()\l+ Initialize()\l+ IPWeights()\l+ IPShape()\l+ IPDShape()\l+ IPWeight()\l+ Extrapolate()\l+ ExtrapolateAll()\l+ Extrapolation()\l+ Print()\l+ NumFacets()\l+ NumNodesOnFacets()\l+ NodesOnFacet()\l+ NodesOnEdges()\l+ FacetGeometry()\l+ NeighborNodeMap()\l+ FacetShapeFunction()\l+ ParentDomain()\l+ ParentCoords()\l+ EvaluateShapeFunctions()\l+ NumSD()\l+ NumIP()\l+ GeometryCode()\l+ TopIP()\l+ NextIP()\l+ SetIP()\l+ CurrIP()\l# Na()\l- SetSurfaceShapes()\l* NumSD()\l* NumIP()\l* GeometryCode()\l* TopIP()\l* NextIP()\l* SetIP()\l* CurrIP()\l}"
template "UML:CLASS"
graphics
[
x 258.0
y 29.0
w 20.0
h 20.0
]
]
node
[
id 65
label "{Tahoe::SetOfNodesKBCT\n|# fSchedule\l# fScheduleNum\l# fScale\l# fNodes\l# nIncs\l|+ SetOfNodesKBCT()\l+ ~SetOfNodesKBCT()\l+ Initialize()\l+ InitialCondition()\l# SetBCCards()\l}"
template "UML:CLASS"
graphics
[
x 183.0
y 627.0
w 20.0
h 20.0
]
]
node
[
id 66
label "{Tahoe::HHTalpha\n|# fAuto2ndOrder\l# fgamma\l# fbeta\l# falpha\l|+ HHTalpha()\l+ ImplicitExplicit()\l+ Order()\l+ OrderOfUnknown()\l# Set2ndOrder()\l* fgamma\l* fbeta\l* falpha\l* ImplicitExplicit()\l* Order()\l* OrderOfUnknown()\l}"
template "UML:CLASS"
graphics
[
x 347.0
y 397.0
w 20.0
h 20.0
]
]
node
[
id 67
label "{Tahoe::J2QL2DLinHardT\n|- fb_elastic\l- fEPModuli\l- fa_inverse\l- fMatrixTemp1\l- fMatrixTemp2\l- fMatrixTemp3\l- fdev_beta\l- fb_n\l- fb_tr\l- fbeta_tr\l- fUnitNorm\l- fInternal\l- fFtot\l- ffrel\l- fF_temp\l- fFtot_2D\l- ffrel_2D\l|+ J2QL2DLinHardT()\l+ Need_F_last()\l+ HasHistory()\l+ UpdateHistory()\l+ ResetHistory()\l+ c_ijkl()\l+ s_ij()\l+ StrainEnergyDensity()\l+ NumOutputVariables()\l+ OutputLabels()\l+ ComputeOutput()\l+ DefineParameters()\l+ TakeParameterList()\l# TrialStretch()\l# ReturnMapping()\l# ElastoPlasticCorrection()\l# AllocateElement()\l- ComputeGradients()\l- InitIntermediate()\l- LoadData()\l- PlasticLoading()\l- YieldCondition()\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 984.0
y 840.0
w 20.0
h 20.0
]
]
node
[
id 68
label "{Tahoe::QuadLog2D\n|# fStress2D\l# fModulus2D\l# fb_2D\l|+ QuadLog2D()\l+ c_ijkl()\l+ s_ij()\l+ StrainEnergyDensity()\l+ TakeParameterList()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 662.0
y 237.0
w 20.0
h 20.0
]
]
node
[
id 69
label "{Tahoe::XDOF_ManagerT\n|# fDOFElements\l# fXDOF_Eqnos\l# fXDOFs\l# fStartTag\l# fNumTags\l# fNumTagSets\l# fTagSetLength\l|+ XDOF_ManagerT()\l+ ~XDOF_ManagerT()\l+ XDOF_Register()\l+ XDOF_Eqnos()\l+ XDOF()\l+ XDOF_SetLocalEqnos()\l+ XDOF_SetLocalEqnos()\l+ XDOF_SetLocalEqnos()\l# NumEquations()\l# SetStartTag()\l# ResetState()\l# ResetTags()\l# NumTagSets()\l# Reset()\l# Update()\l# ConfigureElementGroup()\l# SetEquations()\l# CheckEquationNumbers()\l# EquationNumbers()\l# TagSetIndex()\l# ResolveTagSet()\l* fStartTag\l* fNumTags\l* fNumTagSets\l* fTagSetLength\l* XDOF_SetLocalEqnos()\l* XDOF_SetLocalEqnos()\l* XDOF_SetLocalEqnos()\l}"
template "UML:CLASS"
graphics
[
x 237.0
y 799.0
w 20.0
h 20.0
]
]
node
[
id 70
label "{Tahoe::SmallStrainT\n|# fNeedsOffset\l# fStrainDispOpt\l# fSSMatSupport\l# fStrain_List\l# fStrain_last_List\l# fGradU\l# fLocDispTranspose\l# fMeanGradient\l|+ SmallStrainT()\l+ ~SmallStrainT()\l+ CollectMaterialInfo()\l+ LinearStrain()\l+ LinearStrain()\l+ LinearStrain_last()\l+ LinearStrain_last()\l+ DefineParameters()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l# NewMaterialSupport()\l# NewMaterialList()\l# FormKd()\l# FormStiffness()\l# SetGlobalShape()\l# SetMeanGradient()\l* fStrain_List\l* fStrain_last_List\l* fGradU\l* fLocDispTranspose\l* fMeanGradient\l* LinearStrain()\l* LinearStrain()\l* LinearStrain_last()\l* LinearStrain_last()\l* DefineParameters()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 482.0
y 128.0
w 20.0
h 20.0
]
]
node
[
id 71
label "{Tahoe::FieldSupportT\n||+ FieldSupportT()\l+ NewKBC_Controller()\l+ NewFBC_Controller()\l* NewKBC_Controller()\l* NewFBC_Controller()\l}"
template "UML:CLASS"
graphics
[
x 484.0
y 238.0
w 20.0
h 20.0
]
]
node
[
id 72
label "{Tahoe::MLSSolverT\n|# fNumSD\l# fComplete\l# fNumThirdDer\l# fOrder\l# fNumNeighbors\l# fBasis\l# fLocCoords\l# fw\l# fDw\l# fDDw\l# fDDDw\l# fb\l# fDb\l# fDDb\l# fDDDb\l# fMinv\l# fDM\l# fDDM\l# fDDDM\l# fC\l# fDC\l# fDDC\l# fDDDC\l# fphi\l# fDphi\l# fDDphi\l# fDDDphi\l# fArrayGroup\l# fArray2DGroup2\l# fArray2DGroup3\l# fArray2DGroup4\l# fLocCoords_man\l# fWindowType\l# fWindow\l# fOrigin\l- fNSDsym\l- fMtemp\l- fbtemp1\l- fbtemp2\l- fbtemp3\l- fbtemp4\l- fbtemp5\l- fbtemp6\l- fbtemp7\l- fbtemp8\l- fbtemp9\l|+ MLSSolverT()\l+ ~MLSSolverT()\l+ WriteParameters()\l+ Initialize()\l+ SetField()\l+ phi()\l+ Dphi()\l+ DDphi()\l+ DDDphi()\l+ SearchType()\l+ Covers()\l+ BasisDimension()\l+ NumberOfSupportParameters()\l+ SynchronizeSupportParameters()\l+ w()\l+ Dw()\l+ DDw()\l+ DDDw()\l+ b()\l+ Db()\l+ DDb()\l+ DDDb()\l+ C()\l+ DC()\l+ DDC()\l+ DDDC()\l+ SphericalSupportSize()\l+ SphericalSupportSize()\l+ RectangularSupportSize()\l+ RectangularSupportSize()\l- Window()\l- Dimension()\l- SetMomentMatrix()\l- ComputeM()\l- ComputeDM()\l- ComputeDDM()\l- ComputeDDDM()\l- SetCorrectionCoefficient()\l- SetCorrection()\l- SetShapeFunctions()\l- SymmetricInverse3x3()\l- SymmetricInverse4x4()\l* fWindowType\l* fWindow\l* fOrigin\l* SphericalSupportSize()\l* SphericalSupportSize()\l* RectangularSupportSize()\l* RectangularSupportSize()\l}"
template "UML:CLASS"
graphics
[
x 438.0
y 664.0
w 20.0
h 20.0
]
]
node
[
id 73
label "{Tahoe::AugLagContact3DT\n|# fr\l# fXDOFConnectivities\l# fXDOFEqnos\l# fContactDOFtags\l# fLastActiveMap\l# fLastDOF\l- fXDOFConnectivities_man\l- fXDOFEqnos_man\l- fElCoord\l- fElRefCoord\l- fElDisp\l- fdc_du\l- fdn_du\l- fM1\l- fM2\l- fV1\l|+ AugLagContact3DT()\l+ RelaxSystem()\l+ ConnectsU()\l+ Equations()\l+ SetDOFTags()\l+ DOFTags()\l+ GenerateElementData()\l+ DOFConnects()\l+ ResetDOF()\l+ Reconfigure()\l+ ResetState()\l+ Group()\l+ ReadRestart()\l+ WriteRestart()\l+ DefineParameters()\l+ TakeParameterList()\l# SetActiveInteractions()\l# LHSDriver()\l# RHSDriver()\l* fXDOFConnectivities_man\l* fXDOFEqnos_man\l* fElCoord\l* fElRefCoord\l* fElDisp\l* fdc_du\l* fdn_du\l* fM1\l* fM2\l* fV1\l* Equations()\l* SetDOFTags()\l* DOFTags()\l* GenerateElementData()\l* DOFConnects()\l* ResetDOF()\l* Reconfigure()\l* ResetState()\l* Group()\l* ReadRestart()\l* WriteRestart()\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 503.0
y 4.0
w 20.0
h 20.0
]
]
node
[
id 74
label "{Tahoe::FBC_ControllerT\n|# fFieldSupport\l# fField\l# fGroup\l# fIntegrator\l|+ FBC_ControllerT()\l+ ~FBC_ControllerT()\l+ SetField()\l+ TangentType()\l+ Equations()\l+ Connectivities()\l+ InitialCondition()\l+ ReadRestart()\l+ WriteRestart()\l+ InitStep()\l+ CloseStep()\l+ Reset()\l+ ApplyLHS()\l+ ApplyRHS()\l+ RelaxSystem()\l+ RegisterOutput()\l+ WriteOutput()\l+ TakeParameterList()\l+ Code()\l# Field()\l# FieldSupport()\l* ApplyLHS()\l* ApplyRHS()\l* RelaxSystem()\l* RegisterOutput()\l* WriteOutput()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 138.0
y 260.0
w 20.0
h 20.0
]
]
node
[
id 75
label "{Tahoe::ThermalSurfaceT\n|- fLocTemperatures\l- fConduction\l|+ ThermalSurfaceT()\l+ TangentType()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# LHSDriver()\l# RHSDriver()\l# ComputeOutput()\l* fConduction\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 707.0
y 538.0
w 20.0
h 20.0
]
]
node
[
id 76
label "{Tahoe::ElementListT\n|- fSupport\l- fAllElementGroups\l- fHasContact\l|+ ElementListT()\l+ ~ElementListT()\l+ InterpolantDOFs()\l+ HasContact()\l+ SetActiveElementGroupMask()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l- NewElement()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 267.0
y 564.0
w 20.0
h 20.0
]
]
node
[
id 77
label "{Tahoe::SimoQ1P0_inv\n|# fPressure\l# fJacobian\l# fElementVolume\l# fGamma\l# fGamma_last\l# fMeanGradient\l# fF_tmp\l# fNEEmat\l# fdiff_b\l# fb_bar\l# fb_sig\l|+ SimoQ1P0_inv()\l+ CloseStep()\l+ ResetStep()\l+ ReadRestart()\l+ WriteRestart()\l+ TakeParameterList()\l# SetGlobalShape()\l# FormStiffness()\l# FormKd()\l- SetMeanGradient()\l- bSp_bRq_to_KSqRp()\l* fElementVolume\l* fGamma\l* fGamma_last\l* fMeanGradient\l* fF_tmp\l* fNEEmat\l* fdiff_b\l* fb_bar\l* fb_sig\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 624.0
y 80.0
w 20.0
h 20.0
]
]
node
[
id 78
label "{Tahoe::NLSolver\n|# fVerbose\l# fMaxIterations\l# fMinIterations\l# fReformTangentIterations\l# fZeroTolerance\l# fTolerance\l# fDivTolerance\l# fQuickSolveTol\l# fQuickSeriesTol\l# fIterationOutputIncrement\l# fError0\l# fQuickConvCount\l# fIterationOutputCount\l# fRestartIteration\l|+ NLSolver()\l+ SetReferenceError()\l+ InitStep()\l+ Solve()\l+ CloseStep()\l+ ResetStep()\l+ DefineParameters()\l+ TakeParameterList()\l# Update()\l# ExitIteration()\l# Iterate()\l# InitIterationOutput()\l# CloseIterationOutput()\l# DoConverged()\l* fMaxIterations\l* fMinIterations\l* fReformTangentIterations\l* fZeroTolerance\l* fTolerance\l* fDivTolerance\l* fQuickSolveTol\l* fQuickSeriesTol\l* fIterationOutputIncrement\l* fError0\l* fQuickConvCount\l* fIterationOutputCount\l* fRestartIteration\l* InitStep()\l* Solve()\l* CloseStep()\l* ResetStep()\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 666.0
y 667.0
w 20.0
h 20.0
]
]
node
[
id 79
label "{Tahoe::TotalLagrangianT\n|# fStressMat\l# fStressStiff\l# fGradNa\l# fTemp2\l# fTempMat1\l# fTempMat2\l# fDNa_x\l|+ TotalLagrangianT()\l+ TakeParameterList()\l# FormStiffness()\l# FormKd()\l* fStressMat\l* fStressStiff\l* fGradNa\l* fTemp2\l* fTempMat1\l* fTempMat2\l* fDNa_x\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 842.0
y 342.0
w 20.0
h 20.0
]
]
node
[
id 80
label "{Tahoe::BondLatticeT\n|# fBondCounts\l# fBulkCounts\l# fSurf1Counts\l# fSurf2Counts\l# fSurf3Counts\l# fBonds\l# fBulkBonds\l# fSurf1Bonds\l# fSurf2Bonds\l# fSurf3Bonds\l# fDefLength\l# fDefBulk\l# fDefSurf1\l# fDefSurf2\l# fDefSurf3\l# fQ\l# fAtomType\l# fBondSh\l# fBondShB\l# fBondShS1\l# fBondShS2\l# fBondShS3\l# fBondDp\l# fBondDpB\l# fBondDpS1\l# fBondDpS2\l# fBondDpS3\l# fStrain\l# fStretch\l|+ BondLatticeT()\l+ ~BondLatticeT()\l+ Initialize()\l+ ComputeDeformedLengths()\l+ ComputeDeformedBulkBonds()\l+ ComputeDeformedSurf1Bonds()\l+ ComputeDeformedSurf2Bonds()\l+ ComputeDeformedSurf3Bonds()\l+ BondCounts()\l+ BulkCounts()\l+ Surf1Counts()\l+ Surf2Counts()\l+ Surf3Counts()\l+ DeformedLengths()\l+ DeformedBulk()\l+ DeformedSurf1()\l+ DeformedSurf2()\l+ DeformedSurf3()\l+ DeformedLengths()\l+ Bonds()\l+ NumberOfBonds()\l+ Stretch()\l+ AtomTypes()\l# LoadBondTable()\l* fBondSh\l* fBondShB\l* fBondShS1\l* fBondShS2\l* fBondShS3\l* fBondDp\l* fBondDpB\l* fBondDpS1\l* fBondDpS2\l* fBondDpS3\l* fStrain\l* fStretch\l* BondCounts()\l* BulkCounts()\l* Surf1Counts()\l* Surf2Counts()\l* Surf3Counts()\l* DeformedLengths()\l* DeformedBulk()\l* DeformedSurf1()\l* DeformedSurf2()\l* DeformedSurf3()\l* DeformedLengths()\l* Bonds()\l* NumberOfBonds()\l* Stretch()\l* AtomTypes()\l}"
template "UML:CLASS"
graphics
[
x 546.0
y 178.0
w 20.0
h 20.0
]
]
node
[
id 81
label "{Tahoe::SIERRA_Material_BaseT\n|# vars_input\l# fstress_old\l# fstress_new\l# fstate_old\l# fstate_new\l# fmatvals\l# fdstran\l# fstress_old_rotated\l- fDebug\l- fMaterialName\l- fMaterialModelName\l- fTangentType\l- fSIERRA_Material_Data\l- fPressure\l- fDecomp\l- fF_rel\l- fA_nsd\l- fU1\l- fU2\l- fU1U2\l- fdudX\l- fh\l- fhTh\l- fOutputIndex\l- fOutputLabels\l- fArgsArray\l- fBlockSize\l- fNumContinuation\l- fContinuationStep\l- fContinuationPropName\l- fContinuationPropInit\l- fContinuationPropFinal\l- sSIERRA_Material_count\l|+ SIERRA_Material_BaseT()\l+ ~SIERRA_Material_BaseT()\l+ Need_F_last()\l+ TangentType()\l+ InitStep()\l+ RelaxCode()\l+ StrainEnergyDensity()\l+ HasHistory()\l+ NeedsPointInitialization()\l+ PointInitialize()\l+ UpdateHistory()\l+ ResetHistory()\l+ s_ij()\l+ Pressure()\l+ NumOutputVariables()\l+ OutputLabels()\l+ ComputeOutput()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# MaterialData()\l# Register_SIERRA_Material()\l# SetOutputVariables()\l# Load()\l# Store()\l- Set_Calc_Arguments()\l- SIERRA_to_dSymMatrixT()\l- dSymMatrixT_to_SIERRA()\l- Read_SIERRA_Input()\l- Process_SIERRA_Input()\l* vars_input\l* fstress_old\l* fstress_new\l* fstate_old\l* fstate_new\l* fmatvals\l* fdstran\l* fstress_old_rotated\l* fDecomp\l* fF_rel\l* fA_nsd\l* fU1\l* fU2\l* fU1U2\l* fdudX\l* fh\l* fhTh\l* fOutputIndex\l* fOutputLabels\l* fArgsArray\l* fBlockSize\l* fNumContinuation\l* fContinuationStep\l* fContinuationPropName\l* fContinuationPropInit\l* fContinuationPropFinal\l* HasHistory()\l* NeedsPointInitialization()\l* PointInitialize()\l* UpdateHistory()\l* ResetHistory()\l* s_ij()\l* Pressure()\l* NumOutputVariables()\l* OutputLabels()\l* ComputeOutput()\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l* Register_SIERRA_Material()\l* SetOutputVariables()\l* Load()\l* Store()\l* SIERRA_to_dSymMatrixT()\l* dSymMatrixT_to_SIERRA()\l* Read_SIERRA_Input()\l* Process_SIERRA_Input()\l}"
template "UML:CLASS"
graphics
[
x 275.0
y 900.0
w 20.0
h 20.0
]
]
node
[
id 82
label "{Tahoe::FCC3D\n|- fNearestNeighbor\l- fFCCLattice\l- fPairProperty\l- fAtomicVolume\l- fFullDensity\l- fFullDensityForStressOutput\l- fBondTensor4\l- fBondTensor2\l|+ FCC3D()\l+ ~FCC3D()\l+ BondLattice()\l+ CellVolume()\l+ NearestNeighbor()\l+ DefineParameters()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l# ComputeModuli()\l# ComputePK2()\l# ComputeEnergyDensity()\l# ZeroStressStretch()\l* fBondTensor4\l* fBondTensor2\l* BondLattice()\l* CellVolume()\l* NearestNeighbor()\l* DefineParameters()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 913.0
y 337.0
w 20.0
h 20.0
]
]
node
[
id 83
label "{Tahoe::AugLagContact2DT\n|# fr\l# fXDOFConnectivities\l# fXDOFEqnos\l# fContactDOFtags\l# fLastActiveMap\l# fLastDOF\l- fXDOFConnectivities_man\l- fXDOFEqnos_man\l|+ AugLagContact2DT()\l+ RelaxSystem()\l+ ConnectsU()\l+ Equations()\l+ SetDOFTags()\l+ DOFTags()\l+ GenerateElementData()\l+ DOFConnects()\l+ ResetDOF()\l+ Reconfigure()\l+ ResetState()\l+ Group()\l+ ReadRestart()\l+ WriteRestart()\l+ DefineParameters()\l+ TakeParameterList()\l# SetActiveInteractions()\l# LHSDriver()\l# RHSDriver()\l* Equations()\l* SetDOFTags()\l* DOFTags()\l* GenerateElementData()\l* DOFConnects()\l* ResetDOF()\l* Reconfigure()\l* ResetState()\l* Group()\l* ReadRestart()\l* WriteRestart()\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 985.0
y 707.0
w 20.0
h 20.0
]
]
node
[
id 84
label "{Tahoe::ViscousDragT\n|- fViscosity\l- fID\l- fNodesUsed\l- fNodalMass\l- fDragForce\l- fEqnos\l- fGlobalToLocal\l- fOutputID\l- fIncrementalDissipation\l|+ ViscousDragT()\l+ TangentType()\l+ Equations()\l+ AddNodalForce()\l+ InternalEnergy()\l+ RegisterOutput()\l+ WriteOutput()\l+ SendOutput()\l+ DefineParameters()\l+ TakeParameterList()\l# EchoConnectivityData()\l# LHSDriver()\l# RHSDriver()\l* RegisterOutput()\l* WriteOutput()\l* SendOutput()\l* DefineParameters()\l* TakeParameterList()\l* LHSDriver()\l* RHSDriver()\l}"
template "UML:CLASS"
graphics
[
x 13.0
y 447.0
w 20.0
h 20.0
]
]
node
[
id 85
label "{Tahoe::nGear6\n|- F02\l- F12\l- F22\l- F32\l- F42\l- F52\l- fdt2\l- fdt3\l- fdt4\l- fdt5\l|+ nGear6()\l+ ConsistentKBC()\l+ ExternalNodeCondition()\l+ Predictor()\l+ Corrector()\l+ Corrector()\l+ MappedCorrector()\l+ MappedCorrectorField()\l# nComputeParameters()\l* F02\l* F12\l* F22\l* F32\l* F42\l* F52\l* fdt2\l* fdt3\l* fdt4\l* fdt5\l}"
template "UML:CLASS"
graphics
[
x 385.0
y 927.0
w 20.0
h 20.0
]
]
node
[
id 86
label "{Tahoe::NodeManagerT\n|# fFEManager\l# fCommManager\l# fFieldSupport\l# fFields\l# fMessageID\l# fHistoryNodeSetIDs\l# fHistoryOutputID\l# fCoordUpdateIndex\l# fFieldStart\l# fFieldEnd\l- fXDOFRelaxCodes\l- fInitCoords\l- fCoordUpdate\l- fCurrentCoords\l- fCurrentCoords_man\l- fNeedCurrentCoords\l|+ NodeManagerT()\l+ ~NodeManagerT()\l+ RelaxSystem()\l+ SetTimeStep()\l+ InitialCondition()\l+ InitStep()\l+ CloseStep()\l+ Update()\l+ UpdateCurrentCoordinates()\l+ CopyNodeToNode()\l+ ResetStep()\l+ GetUnknowns()\l+ WeightNodalCost()\l+ Schedule()\l+ Rank()\l+ Size()\l+ FEManager()\l+ NumEquations()\l+ NumDOF()\l+ NumNodes()\l+ NumSD()\l+ NumFields()\l+ NumFields()\l+ Field()\l+ Field()\l+ TangentType()\l+ CollectFields()\l+ ConnectsU()\l+ ImplicitExplicit()\l+ RegisterOutput()\l+ WriteOutput()\l+ SetEquationNumbers()\l+ RenumberEquations()\l+ SetEquationNumberScope()\l+ EquationNumber()\l+ WriteEquationNumbers()\l+ Equations()\l+ FormLHS()\l+ FormRHS()\l+ EndRHS()\l+ EndLHS()\l+ PackSize()\l+ Pack()\l+ Unpack()\l+ ReadRestart()\l+ WriteRestart()\l+ InitialCoordinates()\l+ CurrentCoordinates()\l+ Field()\l+ RegisterCoordinates()\l+ CommManager()\l+ CoordinateUpdate()\l+ ResizeNodes()\l+ XDOF_Register()\l+ XDOF_SetLocalEqnos()\l+ XDOF_SetLocalEqnos()\l+ XDOF_SetLocalEqnos()\l+ NewKBC_Controller()\l+ NewFBC_Controller()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# WriteData()\l# SetCoordinates()\l# EquationNumbers()\l# CheckEquationNumbers()\l- WriteNodalHistory()\l- NodeManagerT()\l- operator=()\l* fFieldSupport\l* fFields\l* fMessageID\l* fHistoryNodeSetIDs\l* fHistoryOutputID\l* fCoordUpdateIndex\l* fFieldStart\l* fFieldEnd\l* fInitCoords\l* fCoordUpdate\l* fCurrentCoords\l* fCurrentCoords_man\l* fNeedCurrentCoords\l* Rank()\l* Size()\l* FEManager()\l* NumEquations()\l* NumDOF()\l* NumNodes()\l* NumSD()\l* NumFields()\l* NumFields()\l* Field()\l* Field()\l* TangentType()\l* CollectFields()\l* ConnectsU()\l* ImplicitExplicit()\l* RegisterOutput()\l* WriteOutput()\l* SetEquationNumbers()\l* RenumberEquations()\l* SetEquationNumberScope()\l* EquationNumber()\l* WriteEquationNumbers()\l* Equations()\l* FormLHS()\l* FormRHS()\l* EndRHS()\l* EndLHS()\l* PackSize()\l* Pack()\l* Unpack()\l* ReadRestart()\l* WriteRestart()\l* InitialCoordinates()\l* CurrentCoordinates()\l* Field()\l* RegisterCoordinates()\l* CommManager()\l* CoordinateUpdate()\l* ResizeNodes()\l* XDOF_Register()\l* XDOF_SetLocalEqnos()\l* XDOF_SetLocalEqnos()\l* XDOF_SetLocalEqnos()\l* NewKBC_Controller()\l* NewFBC_Controller()\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l* SetCoordinates()\l* EquationNumbers()\l* CheckEquationNumbers()\l* NodeManagerT()\l* operator=()\l}"
template "UML:CLASS"
graphics
[
x 593.0
y 841.0
w 20.0
h 20.0
]
]
node
[
id 87
label "{Tahoe::PenaltyContact2DT\n|# fK\l# fElCoord\l# fElDisp\l|+ PenaltyContact2DT()\l+ DefineParameters()\l+ TakeParameterList()\l# LHSDriver()\l# RHSDriver()\l* fElCoord\l* fElDisp\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 752.0
y 434.0
w 20.0
h 20.0
]
]
node
[
id 88
label "{Tahoe::NL_E_RotMat2DT\n||+ NL_E_RotMat2DT()\l+ c_ijkl()\l+ s_ij()\l+ StrainEnergyDensity()\l}"
template "UML:CLASS"
graphics
[
x 12.0
y 522.0
w 20.0
h 20.0
]
]
node
[
id 89
label "{Tahoe::FiniteStrainAxiT\n|# fMat2D\l# fLocCurrCoords\l# fRadius_X\l# fRadius_x\l|+ FiniteStrainAxiT()\l+ CollectMaterialInfo()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# Axisymmetric()\l# SetLocalArrays()\l# NewMaterialSupport()\l# SetGlobalShape()\l* fRadius_X\l* fRadius_x\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 51.0
y 898.0
w 20.0
h 20.0
]
]
node
[
id 90
label "{Tahoe::FBD_EAMGlue\n|- fLatticeParameter\l- fMass\l|+ FBD_EAMGlue()\l+ LatticeParameter()\l+ Mass()\l- SetPairPotential()\l- SetEmbeddingEnergy()\l- SetElectronDensity()\l}"
template "UML:CLASS"
graphics
[
x 306.0
y 817.0
w 20.0
h 20.0
]
]
node
[
id 91
label "{Tahoe::ThreeBodyPropertyT\n||+ ThreeBodyPropertyT()\l+ New()\l* EnergyFunction\l* ForceFunction\l* StiffnessFunction\l* getThreeBodyEnergyFunction()\l* getThreeBodyForceFunction()\l* getThreeBodyStiffnessFunction()\l}"
template "UML:CLASS"
graphics
[
x 9.0
y 348.0
w 20.0
h 20.0
]
]
node
[
id 92
label "{Tahoe::nLinearStaticIntegrator\n||+ nLinearStaticIntegrator()\l+ Predictor()\l+ Corrector()\l+ Corrector()\l}"
template "UML:CLASS"
graphics
[
x 533.0
y 927.0
w 20.0
h 20.0
]
]
node
[
id 93
label "{Tahoe::BoyceBaseT\n|# fFv\l# fFv_n\l# fnstatev\l# fstatev\l- fSpectralDecompRef\l|+ BoyceBaseT()\l+ Pressure()\l+ HasHistory()\l+ NeedsPointInitialization()\l+ PointInitialize()\l+ UpdateHistory()\l+ ResetHistory()\l+ InitStep()\l+ TangentType()\l+ Load()\l+ Store()\l+ TakeParameterList()\l# MixedRank4_2D()\l# MixedRank4_3D()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 731.0
y 71.0
w 20.0
h 20.0
]
]
node
[
id 94
label "{Tahoe::UpdatedLagrangianAxiT\n|# fIPShape\l# fStress2D_axi\l# fStressMat\l# fStressStiff\l# fGradNa\l|+ UpdatedLagrangianAxiT()\l+ ~UpdatedLagrangianAxiT()\l+ TakeParameterList()\l# SetShape()\l# SetGlobalShape()\l# FormStiffness()\l# FormKd()\l* fStressMat\l* fStressStiff\l* fGradNa\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 975.0
y 505.0
w 20.0
h 20.0
]
]
node
[
id 95
label "{Tahoe::nNLHHTalpha\n|- dpred_v\l- dpred_a\l- vpred_a\l- dcorr_a\l- vcorr_a\l|+ nNLHHTalpha()\l+ ConsistentKBC()\l+ ExternalNodeCondition()\l+ Predictor()\l+ Corrector()\l+ Corrector()\l+ MappedCorrector()\l+ MappedCorrectorField()\l# nComputeParameters()\l}"
template "UML:CLASS"
graphics
[
x 83.0
y 817.0
w 20.0
h 20.0
]
]
node
[
id 96
label "{Tahoe::MappedPeriodicT\n|# fField\l# fSchedule\l# fFperturb\l# fF\l# fMappedNodeList\l# fSlaveMasterPairs\l# fD_sm\l# fDummySchedule\l# fMappedCards\l# fSlaveCards\l|+ MappedPeriodicT()\l+ InitialCondition()\l+ InitStep()\l+ WriteOutput()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 72.0
y 860.0
w 20.0
h 20.0
]
]
node
[
id 97
label "{Tahoe::NLDiffusionElementT\n|# fField_list\l# fBCFaces\l# fBCEqnos\l# feps\l# fT0\l# falpha\l|+ NLDiffusionElementT()\l+ Equations()\l+ TangentType()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# LHSDriver()\l# RHSDriver()\l# FormKd()\l# FormStiffness()\l# NewMaterialSupport()\l- TakeTractionBC()\l- TractionBC_RHS()\l- TractionBC_LHS()\l* fBCFaces\l* fBCEqnos\l* feps\l* fT0\l* falpha\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l* TakeTractionBC()\l* TractionBC_RHS()\l* TractionBC_LHS()\l}"
template "UML:CLASS"
graphics
[
x 144.0
y 238.0
w 20.0
h 20.0
]
]
node
[
id 98
label "{Tahoe::Gear6Integrator\n||+ Gear6Integrator()\l# ComputeParameters()\l}"
template "UML:CLASS"
graphics
[
x 158.0
y 305.0
w 20.0
h 20.0
]
]
node
[
id 99
label "{Tahoe::TvergHutch3DT\n|- fsigma_max\l- fd_c_n\l- fd_c_t\l- fL_1\l- fL_2\l- fL_fail\l- fpenalty\l- fK\l- fSecantStiffness\l|+ TvergHutch3DT()\l+ TvergHutch3DT()\l+ NumStateVariables()\l+ FractureEnergy()\l+ Potential()\l+ Traction()\l+ Stiffness()\l+ Status()\l+ NumOutputVariables()\l+ OutputLabels()\l+ ComputeOutput()\l+ DefineParameters()\l+ TakeParameterList()\l# CompatibleOutput()\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 351.0
y 694.0
w 20.0
h 20.0
]
]
node
[
id 100
label "{Tahoe::ParticleTersoffT\n|- fTersoffProperties\l- fNeighbors\l- fEqnos\l- fOutputFlags\l- fNearestNeighbors\l- fRefNearestNeighbors\l- fForce_list\l- fForce_list_man\l- fOneOne\l- fopen\l- fout\l- fout2\l- fsummary_file\l- fsummary_file2\l|+ ParticleTersoffT()\l+ Equations()\l+ WriteOutput()\l+ FormStiffness()\l+ Neighbors()\l+ ConnectsX()\l+ ConnectsU()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l# SetConfiguration()\l# ExtractProperties()\l# SetOutputCount()\l# GenerateOutputLabels()\l# LHSDriver()\l# RHSDriver()\l# RHSDriver3D()\l* fNearestNeighbors\l* fRefNearestNeighbors\l* fForce_list\l* fForce_list_man\l* fOneOne\l* fopen\l* fout\l* fout2\l* fsummary_file\l* fsummary_file2\l* ConnectsX()\l* ConnectsU()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l* LHSDriver()\l* RHSDriver()\l* RHSDriver3D()\l}"
template "UML:CLASS"
graphics
[
x 679.0
y 713.0
w 20.0
h 20.0
]
]
node
[
id 101
label "{Tahoe::SSViscoelasticityT\n|# fdevQ_n\l# fdevSin_n\l# fmeanQ_n\l# fmeanSin_n\l# fdevQ\l# fdevSin\l# fmeanQ\l# fmeanSin\l# fnstatev\l# fstatev\l# ftauS\l# ftauB\l# fndtS\l# fndtB\l|+ SSViscoelasticityT()\l+ Pressure()\l+ InitStep()\l+ HasHistory()\l+ NeedsPointInitialization()\l+ PointInitialize()\l+ UpdateHistory()\l+ ResetHistory()\l+ Load()\l+ Store()\l}"
template "UML:CLASS"
graphics
[
x 969.0
y 211.0
w 20.0
h 20.0
]
]
node
[
id 102
label "{Tahoe::BasicFieldT\n|# fFieldName\l# fField\l# fLabels\l# fEqnos\l- fdArray2DGroup\l- fiArray2DGroup\l|+ BasicFieldT()\l+ ~BasicFieldT()\l+ Initialize()\l+ SetLabels()\l+ Dimension()\l+ Clear()\l+ FieldName()\l+ Labels()\l+ operator[]()\l+ operator[]()\l+ NumNodes()\l+ NumDOF()\l+ Order()\l+ EquationNumber()\l+ Equations()\l+ Equations()\l+ WriteEquationNumbers()\l# RegisterArray2D()\l# RegisterArray2D()\l* fdArray2DGroup\l* fiArray2DGroup\l* Initialize()\l* SetLabels()\l* Dimension()\l* Clear()\l* FieldName()\l* Labels()\l* operator[]()\l* operator[]()\l* NumNodes()\l* NumDOF()\l* Order()\l* EquationNumber()\l* Equations()\l* Equations()\l* WriteEquationNumbers()\l* RegisterArray2D()\l* RegisterArray2D()\l}"
template "UML:CLASS"
graphics
[
x 944.0
y 29.0
w 20.0
h 20.0
]
]
node
[
id 103
label "{Tahoe::HexLattice2DT\n|- fNumShells\l|+ HexLattice2DT()\l+ NumShells()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l+ SetQ()\l# LoadBondTable()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 884.0
y 927.0
w 20.0
h 20.0
]
]
node
[
id 104
label "{Tahoe::SimoShapeFunctionT\n|- fElementModes\l- fHas3DIncompressibleMode\l- fDNaX_bubble\l- fDNa_bubble\l- fDNaX_inc\l- fNa_0\l- fDNa_0\l- fJ\l- fJ_0_inv\l|+ SimoShapeFunctionT()\l+ Initialize()\l+ SetDerivatives()\l+ TransformDerivatives_enhanced()\l+ GradNa_enhanced()\l+ GradU_enhanced()\l+ GradU_enhanced()\l+ Print()\l}"
template "UML:CLASS"
graphics
[
x 973.0
y 377.0
w 20.0
h 20.0
]
]
node
[
id 105
label "{Tahoe::MaterialSupportT\n|- fCurrIP\l- fGroupCommunicator\l- fElementCards\l- fContinuumElement\l- fGroup\l- fNumDOF\l- fNumIP\l- fInitCoords\l- fDisp\l|+ MaterialSupportT()\l+ ~MaterialSupportT()\l+ GroupCommunicator()\l+ NumDOF()\l+ NumIP()\l+ CurrIP()\l+ SetCurrIP()\l+ ContinuumElement()\l+ GroupIterationNumber()\l+ NumElements()\l+ CurrElementNumber()\l+ ElementCard()\l+ CurrentElement()\l+ TopElement()\l+ NextElement()\l+ LocalArray()\l+ Interpolate()\l+ Interpolate()\l+ SetContinuumElement()\l+ SetLocalArray()\l+ SetElementCards()\l+ SetGroup()\l* fNumDOF\l* fNumIP\l* fInitCoords\l* fDisp\l* NumDOF()\l* NumIP()\l* CurrIP()\l* SetCurrIP()\l* ContinuumElement()\l* GroupIterationNumber()\l* NumElements()\l* CurrElementNumber()\l* ElementCard()\l* CurrentElement()\l* TopElement()\l* NextElement()\l* LocalArray()\l* Interpolate()\l* Interpolate()\l* SetContinuumElement()\l* SetLocalArray()\l* SetElementCards()\l* SetGroup()\l}"
template "UML:CLASS"
graphics
[
x 695.0
y 580.0
w 20.0
h 20.0
]
]
node
[
id 106
label "{Tahoe::D2FSMatSupportT\n|- fD2MeshFreeFDElastic\l|+ D2FSMatSupportT()\l+ D2MeshFreeFDElastic()\l+ SetContinuumElement()\l* D2MeshFreeFDElastic()\l* SetContinuumElement()\l}"
template "UML:CLASS"
graphics
[
x 746.0
y 955.0
w 20.0
h 20.0
]
]
node
[
id 107
label "{Tahoe::GaussianWindowT\n|- fNSD\l- fNSDsym\l- fNSDArray\l- fSupportSize\l- fDilationScaling\l- fSharpeningFactor\l- fCutOffFactor\l|+ GaussianWindowT()\l+ Name()\l+ SearchType()\l+ NumberOfSupportParameters()\l+ SynchronizeSupportParameters()\l+ WriteParameters()\l+ Window()\l+ Window()\l+ Covers()\l+ Covers()\l+ SphericalSupportSize()\l+ RectangularSupportSize()\l+ SphericalSupportSize()\l* fDilationScaling\l* fSharpeningFactor\l* fCutOffFactor\l* Covers()\l* Covers()\l* SphericalSupportSize()\l* RectangularSupportSize()\l* SphericalSupportSize()\l}"
template "UML:CLASS"
graphics
[
x 248.0
y 101.0
w 20.0
h 20.0
]
]
node
[
id 108
label "{Tahoe::tevp3D\n|# fStress\l# fModulus\l# fStrainEnergyDensity\l# fInternal\l# fTempKirchoff\l# fTempCauchy\l- fDt\l- fFtot\l- fDtot\l- fGradV\l- fLocVel\l- fF_temp\l- fSpin\l- fCriticalStrain\l- fEbtot\l- fXxii\l- fCtcon\l- fPP\l- fDmat\l- fEP_tan\l- fEcc\l- fStressMatrix\l- fStressArray\l- fStill3D\l- fStress3D\l- fSmlp\l- fSymStress3D\l- fJ\l- fVisc\l- fTemperature\l- fSb\l- fEb\l- Temp_0\l- El_E\l- El_V\l- El_K\l- El_G\l- Sb0\l- Rho0\l- Eb0\l- Eb0tot\l- BigN\l- Smm\l- Alpha_T\l- Delta\l- Theta\l- Kappa\l- Cp\l- Chi\l- Ccc\l- Pcp\l- Epsilon_1\l- Epsilon_2\l- Epsilon_rate\l- Gamma_d\l- Mu_d\l- SigCr\l- Xi\l|+ tevp3D()\l+ NeedsPointInitialization()\l+ PointInitialize()\l+ NeedVel()\l+ UpdateHistory()\l+ ResetHistory()\l+ C_IJKL()\l+ S_IJ()\l+ StrainEnergyDensity()\l+ NumOutputVariables()\l+ OutputLabels()\l+ ComputeOutput()\l+ c_ijkl()\l+ s_ij()\l+ Pressure()\l- ComputeF()\l- ComputeD()\l- ComputeSpin()\l- ComputeEbtotCtconXxii()\l- ComputePP()\l- ComputeEcc()\l- ComputeDmat()\l- ComputeEP_tan()\l- ComputeSmlp()\l- AllocateElement()\l- ComputeFluidTemperature()\l- ComputeViscoTemperature()\l- ComputeEffectiveStress()\l- ComputeFluidEffectiveStrain()\l- ComputeViscoEffectiveStrain()\l- CheckCriticalCriteria()\l- CheckIfPlastic()\l- LoadData()\l- Update()\l- Reset()\l- ArrayToMatrix()\l- MatrixToArray()\l- Return3DStress()\l- ArrayToSymMatrix3D()\l* c_ijkl()\l* s_ij()\l* Pressure()\l}"
template "UML:CLASS"
graphics
[
x 402.0
y 256.0
w 20.0
h 20.0
]
]
node
[
id 109
label "{Tahoe::ErcolessiAdamsAl\n||+ ErcolessiAdamsAl()\l+ LatticeParameter()\l+ Mass()\l- SetPairPotential()\l- SetEmbeddingEnergy()\l- SetElectronDensity()\l}"
template "UML:CLASS"
graphics
[
x 661.0
y 408.0
w 20.0
h 20.0
]
]
node
[
id 110
label "{Tahoe::CSEIsoT\n|# fSurfPots\l|+ CSEIsoT()\l+ TangentType()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l# LHSDriver()\l# RHSDriver()\l# ComputeOutput()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 962.0
y 454.0
w 20.0
h 20.0
]
]
node
[
id 111
label "{Tahoe::CCNSMatrixT\n|# famax\l# fEqnos\l# fRaggedEqnos\l# fKU\l# fKL\l# fKD\l# fNumberOfTerms\l# fMatrix\l# fIsFactorized\l# fBand\l# fMeanBand\l- fu\l|+ CCNSMatrixT()\l+ CCNSMatrixT()\l+ ~CCNSMatrixT()\l+ Initialize()\l+ Info()\l+ Clear()\l+ AddEquationSet()\l+ AddEquationSet()\l+ Assemble()\l+ Assemble()\l+ Assemble()\l+ HasNegativePivot()\l+ Element()\l+ EquationNumberScope()\l+ RenumberEquations()\l+ MatrixType()\l+ FindMinMaxPivot()\l+ operator=()\l+ Clone()\l+ CopyDiagonal()\l# Factorize()\l# BackSubstitute()\l# PrintAllPivots()\l# PrintZeroPivots()\l# PrintLHS()\l# InSkyline()\l# BandWidth()\l# operator()()\l- ComputeSize()\l- SetSkylineHeights()\l- SetSkylineHeights()\l- NumberOfFilled()\l- FillWithOnes()\l- FillWithOnes()\l- SolNonSymSysSkyLine()\l- solvLT()\l- solvUT()\l* fEqnos\l* fRaggedEqnos\l* fKU\l* fKL\l* fKD\l* fNumberOfTerms\l* fMatrix\l* fIsFactorized\l* fBand\l* fMeanBand\l}"
template "UML:CLASS"
graphics
[
x 65.0
y 609.0
w 20.0
h 20.0
]
]
node
[
id 112
label "{Tahoe::NLDiffusionMaterialT\n|- fScaledConductivity\l- fConductivityScaleFunction\l- fCpScaleFunction\l|+ NLDiffusionMaterialT()\l+ ~NLDiffusionMaterialT()\l+ k_ij()\l+ q_i()\l+ dq_i_dT()\l+ dk_ij()\l+ SpecificHeat()\l+ dCapacity_dT()\l+ DefineSubs()\l+ NewSub()\l+ DefineInlineSub()\l+ TakeParameterList()\l* fConductivityScaleFunction\l* fCpScaleFunction\l* k_ij()\l* q_i()\l* dq_i_dT()\l* dk_ij()\l* SpecificHeat()\l* dCapacity_dT()\l* DefineSubs()\l* NewSub()\l* DefineInlineSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 699.0
y 866.0
w 20.0
h 20.0
]
]
node
[
id 113
label "{Tahoe::ParticleThreeBodyT\n|- fThreeBodyProperties\l- fNeighbors\l- fEqnos\l- fNearestNeighbors\l- fRefNearestNeighbors\l- fForce_list\l- fForce_list_man\l- fOneOne\l- fopen\l- fout\l- fout2\l- fsummary_file\l- fsummary_file2\l|+ ParticleThreeBodyT()\l+ Equations()\l+ WriteOutput()\l+ FormStiffness()\l+ Neighbors()\l+ ConnectsX()\l+ ConnectsU()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l# SetConfiguration()\l# ExtractProperties()\l# GenerateOutputLabels()\l# LHSDriver()\l# RHSDriver()\l# RHSDriver3D()\l* fNearestNeighbors\l* fRefNearestNeighbors\l* fForce_list\l* fForce_list_man\l* fOneOne\l* fopen\l* fout\l* fout2\l* fsummary_file\l* fsummary_file2\l* ConnectsX()\l* ConnectsU()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l* LHSDriver()\l* RHSDriver()\l* RHSDriver3D()\l}"
template "UML:CLASS"
graphics
[
x 655.0
y 854.0
w 20.0
h 20.0
]
]
node
[
id 114
label "{Tahoe::MeshFreeSupportT\n|# fDextra\l# fStoreShape\l# fScaledSupport\l# fMeshfreeType\l# fNumFacetNodes\l# fCutCoords\l# fCoords\l# fDomain\l# fSD\l# fIP\l# fEFG\l# fRKPM\l# fGrid\l# fSkipNode\l# fnNeighborCount\l# fnNeighborData\l# fSkipElement\l# feNeighborCount\l# feNeighborData\l# fneighbors\l# fvolume\l# fvolume_man\l# fnodal_param\l# fnodal_param_ip\l# fnodal_param_man\l# fcoords\l# fcoords_man\l# fx_ip_table\l# felShapespace\l# fndShapespace\l# fConnects\l# fNonGridNodes\l# fVolume\l# fNodalParameters\l# fNodesUsed\l# fnPhiData\l# fnDPhiData\l# fePhiData\l# feDPhiData\l# fResetNodes\l# fResetElems\l# fReformNode\l# fReformElem\l|+ MeshFreeSupportT()\l+ MeshFreeSupportT()\l+ ~MeshFreeSupportT()\l+ InitSupportParameters()\l+ InitNeighborData()\l+ SetSkipNodes()\l+ SkipNodes()\l+ SetSkipElements()\l+ SkipElements()\l+ SynchronizeSupportParameters()\l+ SetSupportParameters()\l+ GetSupportParameters()\l+ NodalParameters()\l+ NodalVolumes()\l+ SetCuttingFacets()\l+ ResetFacets()\l+ ResetNodes()\l+ ResetCells()\l+ BuildNeighborhood()\l+ ElementNeighborsCounts()\l+ ElementNeighbors()\l+ NodeNeighbors()\l+ NodesUsed()\l+ NodalCoordinates()\l+ WriteParameters()\l+ WriteStatistics()\l+ WriteNodalNeighbors()\l+ WriteNodalShapes()\l+ LoadNodalData()\l+ LoadElementData()\l+ SetFieldAt()\l+ SetFieldUsing()\l+ NeighborsAt()\l+ FieldAt()\l+ DFieldAt()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l+ New_MLSSolverT()\l# SetSearchGrid()\l# SetNodeNeighborData()\l# SetNodeNeighborData_2()\l# SetElementNeighborData()\l# SetElementNeighborData_2()\l# SetNodalShapeFunctions()\l# SetElementShapeFunctions()\l# InitNodalShapeData()\l# InitElementShapeData()\l# ProcessBoundaries()\l# Visible()\l- Covers()\l- ComputeElementData()\l- ComputeNodalData()\l- SetSupport_Spherical_Search()\l- SetSupport_Spherical_Connectivities()\l- SetSupport_Cartesian_Connectivities()\l- SetNodesUsed()\l- SwapData()\l* WriteParameters()\l* WriteStatistics()\l* WriteNodalNeighbors()\l* WriteNodalShapes()\l* LoadNodalData()\l* LoadElementData()\l* SetFieldAt()\l* SetFieldUsing()\l* NeighborsAt()\l* FieldAt()\l* DFieldAt()\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 385.0
y 944.0
w 20.0
h 20.0
]
]
node
[
id 115
label "{Tahoe::RectCubicSplineWindowT\n|- fDilationScaling\l- fNSD\l- fNSDsym\l- fNSDArray\l|+ RectCubicSplineWindowT()\l+ Name()\l+ SearchType()\l+ NumberOfSupportParameters()\l+ SynchronizeSupportParameters()\l+ WriteParameters()\l+ Window()\l+ Window()\l+ Covers()\l+ Covers()\l+ SphericalSupportSize()\l+ RectangularSupportSize()\l* Covers()\l* Covers()\l* SphericalSupportSize()\l* RectangularSupportSize()\l}"
template "UML:CLASS"
graphics
[
x 143.0
y 161.0
w 20.0
h 20.0
]
]
node
[
id 116
label "{Tahoe::Trapezoid\n||+ Trapezoid()\l+ ImplicitExplicit()\l+ Order()\l+ OrderOfUnknown()\l* ImplicitExplicit()\l* Order()\l* OrderOfUnknown()\l}"
template "UML:CLASS"
graphics
[
x 551.0
y 907.0
w 20.0
h 20.0
]
]
node
[
id 117
label "{Tahoe::ElementBaseT\n|# fIntegrator\l# fElementCards\l# fBlockData\l# fOutputID\l# fConnectivities\l# fEqnos\l# fLHS\l# fRHS\l- fSupport\l- fField\l|+ ElementBaseT()\l+ ~ElementBaseT()\l+ FormLHS()\l+ FormRHS()\l+ AddNodalForce()\l+ InitStep()\l+ CloseStep()\l+ ResetStep()\l+ RelaxSystem()\l+ InternalEnergy()\l+ Equations()\l+ InitialCondition()\l+ InterpolantDOFs()\l+ NodalDOFs()\l+ WeightNodalCost()\l+ NodesUsed()\l+ LumpedMass()\l+ InternalForce()\l+ ElementGroupNumber()\l+ NumElements()\l+ NumElementNodes()\l+ TangentType()\l+ Axisymmetric()\l+ ElementBlockID()\l+ ElementSupport()\l+ Field()\l+ RunState()\l+ IterationNumber()\l+ InGroup()\l+ Schedule()\l+ NumSD()\l+ ElementBlockIDs()\l+ BlockData()\l+ NumDOF()\l+ SetStatus()\l+ GetStatus()\l+ RegisterOutput()\l+ WriteOutput()\l+ SendOutput()\l+ ResolveOutputVariable()\l+ ConnectsX()\l+ ConnectsU()\l+ GeometryCode()\l+ WriteRestart()\l+ ReadRestart()\l+ ElementCard()\l+ ElementCard()\l+ CurrElementNumber()\l+ CurrentElement()\l+ CurrentElement()\l+ DefineParameters()\l+ TakeParameterList()\l# BlockToGroupElementNumbers()\l# Group()\l# CurrElementInfo()\l# SetElementCards()\l# CollectBlockInfo()\l# DefineElements()\l# SetLocalX()\l# SetLocalU()\l# LHSDriver()\l# RHSDriver()\l# AssembleRHS()\l# AssembleLHS()\l# Top()\l# NextElement()\l- DefaultNumElemNodes()\l* fConnectivities\l* fEqnos\l* fLHS\l* fRHS\l* fSupport\l* fField\l* ElementGroupNumber()\l* NumElements()\l* NumElementNodes()\l* TangentType()\l* Axisymmetric()\l* ElementBlockID()\l* ElementSupport()\l* Field()\l* RunState()\l* IterationNumber()\l* InGroup()\l* Schedule()\l* NumSD()\l* ElementBlockIDs()\l* BlockData()\l* NumDOF()\l* SetStatus()\l* GetStatus()\l* RegisterOutput()\l* WriteOutput()\l* SendOutput()\l* ResolveOutputVariable()\l* ConnectsX()\l* ConnectsU()\l* GeometryCode()\l* WriteRestart()\l* ReadRestart()\l* ElementCard()\l* ElementCard()\l* CurrElementNumber()\l* CurrentElement()\l* CurrentElement()\l* DefineParameters()\l* TakeParameterList()\l* CollectBlockInfo()\l* DefineElements()\l* SetLocalX()\l* SetLocalU()\l* LHSDriver()\l* RHSDriver()\l* AssembleRHS()\l* AssembleLHS()\l* Top()\l* NextElement()\l}"
template "UML:CLASS"
graphics
[
x 987.0
y 611.0
w 20.0
h 20.0
]
]
node
[
id 118
label "{Tahoe::J2SSKStV\n|- fStress\l- fModulus\l- fElasticIterations\l|+ J2SSKStV()\l+ HasHistory()\l+ UpdateHistory()\l+ ResetHistory()\l+ StrainEnergyDensity()\l+ NumOutputVariables()\l+ OutputLabels()\l+ ComputeOutput()\l+ c_ijkl()\l+ s_ij()\l+ Pressure()\l+ DefineParameters()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l# SetModulus()\l# InternalDOF()\l# InternalStrainVars()\l# InternalStressVars()\l* c_ijkl()\l* s_ij()\l* Pressure()\l* DefineParameters()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 909.0
y 977.0
w 20.0
h 20.0
]
]
node
[
id 119
label "{Tahoe::J2IsoVIB2DLinHardT\n|- fEigs\l- fBeta\l- fddW\l- fb_elastic\l- fEPModuli\l- fMatrixTemp1\l- fMatrixTemp2\l- fdev_beta\l- fb_n\l- fb_tr\l- fbeta_tr\l- flog_e\l- fUnitNorm\l- fInternal\l- fFtot\l- ffrel\l- fF_temp\l- fFtot_2D\l- ffrel_2D\l- fStress2D\l- fb_2D\l|+ J2IsoVIB2DLinHardT()\l+ UpdateHistory()\l+ ResetHistory()\l+ c_ijkl()\l+ s_ij()\l+ C_IJKL()\l+ S_IJ()\l+ StrainEnergyDensity()\l+ NeedLastDisp()\l+ NumOutputVariables()\l+ OutputLabels()\l+ ComputeOutput()\l+ DefineParameters()\l+ TakeParameterList()\l# TrialStretch()\l# ReturnMapping()\l# AllocateElement()\l- ComputeGradients()\l- InitIntermediate()\l- LoadData()\l- PlasticLoading()\l- YieldCondition()\l- ComputeBeta()\l- Computeddw()\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 31.0
y 695.0
w 20.0
h 20.0
]
]
node
[
id 120
label "{Tahoe::LangevinT\n|# fRandom\l|+ LangevinT()\l+ ApplyDamping()\l+ DefineParameters()\l+ TakeParameterList()\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 224.0
y 693.0
w 20.0
h 20.0
]
]
node
[
id 121
label "{Tahoe::J2IsoVIB3DLinHardT\n|- fEigs\l- fBeta\l- fddW\l- fb_elastic\l- fEPModuli\l- fMatrixTemp1\l- fMatrixTemp2\l- fdev_beta\l- fb_n\l- fb_tr\l- fbeta_tr\l- flog_e\l- fUnitNorm\l- fInternal\l- fFtot\l- ffrel\l- fF_temp\l|+ J2IsoVIB3DLinHardT()\l+ UpdateHistory()\l+ ResetHistory()\l+ c_ijkl()\l+ s_ij()\l+ C_IJKL()\l+ S_IJ()\l+ StrainEnergyDensity()\l+ NeedLastDisp()\l+ NumOutputVariables()\l+ OutputLabels()\l+ ComputeOutput()\l+ DefineParameters()\l+ TakeParameterList()\l# TrialStretch()\l# ReturnMapping()\l# AllocateElement()\l- ComputeGradients()\l- InitIntermediate()\l- LoadData()\l- PlasticLoading()\l- YieldCondition()\l- ComputeBeta()\l- Computeddw()\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 347.0
y 384.0
w 20.0
h 20.0
]
]
node
[
id 122
label "{Tahoe::D3MeshFreeSupportT\n|# fNumDeriv\l- fnDDDPhiData\l- feDDDPhiData\l|+ D3MeshFreeSupportT()\l+ D3MeshFreeSupportT()\l+ InitNeighborData()\l+ LoadNodalData()\l+ LoadElementData()\l+ DDDFieldAt()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# SetNodalShapeFunctions()\l# SetElementShapeFunctions()\l# InitNodalShapeData()\l# InitElementShapeData()\l- ComputeNodalData()\l- ComputeElementData()\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 196.0
y 910.0
w 20.0
h 20.0
]
]
node
[
id 123
label "{Tahoe::TvergHutchIrrev3DT\n|- fsigma_max\l- fd_c_n\l- fd_c_t\l- fL_1\l- fL_2\l- fL_fail\l- fpenalty\l- fK\l- fTimeStep\l- fSecantStiffness\l|+ TvergHutchIrrev3DT()\l+ TvergHutchIrrev3DT()\l+ NumStateVariables()\l+ InitStateVariables()\l+ SetTimeStep()\l+ FractureEnergy()\l+ Potential()\l+ Traction()\l+ Stiffness()\l+ Status()\l+ NumOutputVariables()\l+ OutputLabels()\l+ ComputeOutput()\l+ DefineParameters()\l+ TakeParameterList()\l# CompatibleOutput()\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 229.0
y 928.0
w 20.0
h 20.0
]
]
node
[
id 124
label "{Tahoe::RateDep2DT\n|- fsigma_max\l- fd_c_n\l- fd_c_t\l- fL_1\l- fL_2\l- fL_fail\l- fpenalty\l- fK\l- fTimeStep\l- fGroup\l- fNeedsStress\l- L_2_b\l- L_2_m\l- fslope\l|+ RateDep2DT()\l+ SetTimeStep()\l+ InitStateVariables()\l+ NumStateVariables()\l+ FractureEnergy()\l+ Potential()\l+ Traction()\l+ Stiffness()\l+ Status()\l+ NumOutputVariables()\l+ OutputLabels()\l+ ComputeOutput()\l+ DefineParameters()\l+ TakeParameterList()\l# CompatibleOutput()\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 723.0
y 330.0
w 20.0
h 20.0
]
]
node
[
id 125
label "{Tahoe::BasicSupportT\n|- fFEManager\l- fNodeManager\l- fTimeManager\l- fModelManager\l- fCommManager\l- fCommunicator\l- fRunState\l- fNumSD\l|+ BasicSupportT()\l+ SetFEManager()\l+ SetNodeManager()\l+ SetNumSD()\l+ Version()\l+ PrintInput()\l+ Logging()\l+ NumNodes()\l+ NumSD()\l+ InitialCoordinates()\l+ CurrentCoordinates()\l+ RegisterCoordinates()\l+ RunState()\l+ Schedule()\l+ IterationNumber()\l+ IterationNumber()\l+ CurrentGroup()\l+ Time()\l+ StepNumber()\l+ NumberOfSteps()\l+ TimeStep()\l+ Field()\l+ Size()\l+ Rank()\l+ Communicator()\l+ ProcessorMap()\l+ ExternalNodes()\l+ BorderNodes()\l+ NodeMap()\l+ ElementMap()\l+ PartitionNodes()\l+ AssembleLHS()\l+ AssembleLHS()\l+ AssembleLHS()\l+ AssembleRHS()\l+ RHS()\l+ LHS()\l+ InputFile()\l+ Output()\l+ RegisterOutput()\l+ WriteOutput()\l+ WriteOutput()\l+ WriteOutput()\l+ OutputSet()\l+ FEManager()\l+ NodeManager()\l+ XDOF_Manager()\l+ TimeManager()\l+ ModelManager()\l+ CommManager()\l+ NumElementGroups()\l+ ElementGroup()\l* fFEManager\l* fNodeManager\l* fTimeManager\l* fModelManager\l* fCommManager\l* fCommunicator\l* fRunState\l* fNumSD\l* SetFEManager()\l* SetNodeManager()\l* SetNumSD()\l* Version()\l* PrintInput()\l* Logging()\l* NumNodes()\l* NumSD()\l* InitialCoordinates()\l* CurrentCoordinates()\l* RegisterCoordinates()\l* RunState()\l* Schedule()\l* IterationNumber()\l* IterationNumber()\l* CurrentGroup()\l* Time()\l* StepNumber()\l* NumberOfSteps()\l* TimeStep()\l* Field()\l* Size()\l* Rank()\l* Communicator()\l* ProcessorMap()\l* ExternalNodes()\l* BorderNodes()\l* NodeMap()\l* ElementMap()\l* PartitionNodes()\l* AssembleLHS()\l* AssembleLHS()\l* AssembleLHS()\l* AssembleRHS()\l* RHS()\l* LHS()\l* InputFile()\l* Output()\l* RegisterOutput()\l* WriteOutput()\l* WriteOutput()\l* WriteOutput()\l* OutputSet()\l* FEManager()\l* NodeManager()\l* XDOF_Manager()\l* TimeManager()\l* ModelManager()\l* CommManager()\l* NumElementGroups()\l* ElementGroup()\l}"
template "UML:CLASS"
graphics
[
x 702.0
y 49.0
w 20.0
h 20.0
]
]
node
[
id 126
label "{Tahoe::NLSolver_LS\n|- fR\l- fSearchIterations\l- fOrthogTolerance\l- fMaxStepSize\l- fUpdate\l- s_current\l- fSearchData\l|+ NLSolver_LS()\l+ Iterate()\l+ iDoVariable()\l+ DefineParameters()\l+ TakeParameterList()\l# Update()\l- GValue()\l* fSearchIterations\l* fOrthogTolerance\l* fMaxStepSize\l* fUpdate\l* s_current\l* fSearchData\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 263.0
y 180.0
w 20.0
h 20.0
]
]
node
[
id 127
label "{Tahoe::MFPenaltyContact2DT\n|# fNodeToMeshFreePoint\l# fNodeToActiveStriker\l# fElementGroup\l# fMeshFreeSupport\l# fSCNI\l# fStrikerCoords_man\l# fdvT_man\l# fRHS_man\l# fSCNI_tmp\l# fSCNI_LocalID\l# fSCNI_Support\l# fSCNI_Phi\l# fOutputID\l# fOutputForce\l# fNodesUsed\l# fForce\l# fNodesUsed_inv\l|+ MFPenaltyContact2DT()\l+ RegisterOutput()\l+ WriteOutput()\l+ DefineParameters()\l+ TakeParameterList()\l# LHSDriver()\l# RHSDriver()\l# ComputeStrikerCoordinates()\l# SetDerivativeArrays()\l# ExtractContactGeometry()\l# SetActiveInteractions()\l* fElementGroup\l* fMeshFreeSupport\l* fSCNI\l* fStrikerCoords_man\l* fdvT_man\l* fRHS_man\l* fSCNI_tmp\l* fSCNI_LocalID\l* fSCNI_Support\l* fSCNI_Phi\l* fOutputID\l* fOutputForce\l* fNodesUsed\l* fForce\l* fNodesUsed_inv\l* RegisterOutput()\l* WriteOutput()\l* DefineParameters()\l* TakeParameterList()\l* ExtractContactGeometry()\l* SetActiveInteractions()\l}"
template "UML:CLASS"
graphics
[
x 371.0
y 58.0
w 20.0
h 20.0
]
]
node
[
id 128
label "{Tahoe::From2Dto3DT\n|- f2DModel\l|+ From2Dto3DT()\l+ From2Dto3DT()\l+ ~From2Dto3DT()\l+ NumStateVariables()\l+ InitStateVariables()\l+ FractureEnergy()\l+ IncrementalHeat()\l+ Potential()\l+ Traction()\l+ Stiffness()\l+ Status()\l+ NumOutputVariables()\l+ OutputLabels()\l+ ComputeOutput()\l}"
template "UML:CLASS"
graphics
[
x 451.0
y 55.0
w 20.0
h 20.0
]
]
node
[
id 129
label "{Tahoe::KBC_ControllerT\n|# fSupport\l# fKBC_Cards\l|+ KBC_ControllerT()\l+ ~KBC_ControllerT()\l+ KBC_Cards()\l+ KBC_Cards()\l+ SetExternalNodes()\l+ InitialCondition()\l+ RelaxSystem()\l+ WriteOutput()\l+ Connectivities()\l+ Equations()\l+ SetEquations()\l+ IsICController()\l+ ReadRestart()\l+ WriteRestart()\l+ InitStep()\l+ FormRHS()\l+ FormLHS()\l+ Update()\l+ CloseStep()\l+ Reset()\l+ Code()\l# GetNodes()\l- KBC_ControllerT()\l- operator=()\l* ReadRestart()\l* WriteRestart()\l* InitStep()\l* FormRHS()\l* FormLHS()\l* Update()\l* CloseStep()\l* Reset()\l* KBC_ControllerT()\l* operator=()\l}"
template "UML:CLASS"
graphics
[
x 322.0
y 119.0
w 20.0
h 20.0
]
]
node
[
id 130
label "{Tahoe::FS_SCNIMFT\n|# fFSMatSupport\l# fF_list\l# fF_last_list\l|+ FS_SCNIMFT()\l+ FS_SCNIMFT()\l+ ~FS_SCNIMFT()\l+ WriteOutput()\l+ RelaxSystem()\l+ LHSDriver()\l+ RHSDriver()\l+ DefineParameters()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l# TransformModuli()\l# CollectMaterialInfo()\l# NewMaterialList()\l# bVectorToMatrix()\l* fF_list\l* fF_last_list\l* DefineParameters()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 423.0
y 317.0
w 20.0
h 20.0
]
]
node
[
id 131
label "{Tahoe::nExplicitCD\n|- vcorr_a\l- dpred_v\l- dpred_a\l- vpred_a\l|+ nExplicitCD()\l+ ConsistentKBC()\l+ ExternalNodeCondition()\l+ Predictor()\l+ Corrector()\l+ Corrector()\l+ MappedCorrector()\l+ MappedCorrectorField()\l# nComputeParameters()\l* dpred_v\l* dpred_a\l* vpred_a\l}"
template "UML:CLASS"
graphics
[
x 194.0
y 394.0
w 20.0
h 20.0
]
]
node
[
id 132
label "{Tahoe::OgdenIsoVIB3D\n|# fSphere\l|+ OgdenIsoVIB3D()\l+ ~OgdenIsoVIB3D()\l+ StrainEnergyDensity()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# dWdE()\l# ddWddE()\l# ComputeLengths()\l- Construct()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 582.0
y 564.0
w 20.0
h 20.0
]
]
node
[
id 133
label "{Tahoe::QuadLogOgden3DT\n|# flogE\l|+ QuadLogOgden3DT()\l+ StrainEnergyDensity()\l# dWdE()\l# ddWddE()\l}"
template "UML:CLASS"
graphics
[
x 565.0
y 981.0
w 20.0
h 20.0
]
]
node
[
id 134
label "{Tahoe::TorsionKBCT\n|# fStartTime\l# fw\l# fDummySchedule\l# fAxis\l# fPoint\l# fID_List\l# fNodes\l|+ TorsionKBCT()\l+ InitialCondition()\l+ InitStep()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l* fAxis\l* fPoint\l* fID_List\l* fNodes\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 270.0
y 415.0
w 20.0
h 20.0
]
]
node
[
id 135
label "{Tahoe::AdhesionT\n|# fGrid\l# fSurfaceLinks\l# fPenalizePenetration\l# fAllowSameSurface\l# fSurfaces\l# fShapes\l# fCurrShapes\l# fLocInitCoords\l# fLocCurrCoords\l# fScaling\l# fOutputID\l# fFaceForce\l# fCurrentFaceArea\l# fSurface1\l# fSurface2\l# fFaceConnectivities\l# fFaceEquations\l# fCutOff\l# fAdhesion\l# fGrad_d\l# fIPCoords2\l# fIPNorm2\l# fNEE_vec\l# fNEE_mat\l# fNEE_vec_man\l# fNEE_mat_man\l# fGrad_d_man\l# fFace2_man\l|+ AdhesionT()\l+ ~AdhesionT()\l+ TangentType()\l+ RelaxSystem()\l+ AddNodalForce()\l+ InternalEnergy()\l+ RegisterOutput()\l+ WriteOutput()\l+ SendOutput()\l+ Equations()\l+ ConnectsU()\l+ ConnectsX()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# SetConfiguration()\l# NumIP()\l# LHSDriver()\l# RHSDriver()\l# ExtractSurfaces()\l# SetWorkSpace()\l# InputSideSets()\l# InputBodyBoundary()\l* FaceIndexT\l* fFaceCentroids\l* fFaceIndex\l* fPenalizePenetration\l* fAllowSameSurface\l* fSurfaces\l* fShapes\l* fCurrShapes\l* fLocInitCoords\l* fLocCurrCoords\l* fScaling\l* fOutputID\l* fFaceForce\l* fCurrentFaceArea\l* fSurface1\l* fSurface2\l* fFaceConnectivities\l* fFaceEquations\l* fCutOff\l* fAdhesion\l* fGrad_d\l* fIPCoords2\l* fIPNorm2\l* fNEE_vec\l* fNEE_mat\l* fNEE_vec_man\l* fNEE_mat_man\l* fGrad_d_man\l* fFace2_man\l* ConnectsU()\l* ConnectsX()\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l* LHSDriver()\l* RHSDriver()\l* ExtractSurfaces()\l* SetWorkSpace()\l* InputSideSets()\l* InputBodyBoundary()\l}"
template "UML:CLASS"
graphics
[
x 718.0
y 216.0
w 20.0
h 20.0
]
]
node
[
id 136
label "{Tahoe::MeshFreeFSSolidT\n|# fMFShapes\l# fMFFractureSupport\l# fAutoBorder\l# fStressStiff_wrap\l# fB_wrap\l# fGradNa_wrap\l# fDNa_x_wrap\l# fConnectsAll\l# fMeshfreeParameters\l|+ MeshFreeFSSolidT()\l+ ~MeshFreeFSSolidT()\l+ Equations()\l+ ConnectsU()\l+ WriteOutput()\l+ RelaxSystem()\l+ InterpolantDOFs()\l+ NodalDOFs()\l+ WeightNodalCost()\l+ InitStep()\l+ CloseStep()\l+ ResetStep()\l+ MeshFreeSupport()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# SetShape()\l# NextElement()\l# ComputeOutput()\l- WriteField()\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 68.0
y 80.0
w 20.0
h 20.0
]
]
node
[
id 137
label "{Tahoe::Contact3DT\n|# fGrid3D\l# fx1\l# fx2\l# fx3\l# fStriker\l|+ Contact3DT()\l+ ~Contact3DT()\l# ExtractContactGeometry()\l# ConvertQuadToTri()\l# Set_dn_du()\l# DDg_tri_facet()\l# SetActiveInteractions()\l# SetConnectivities()\l- SetActiveStrikers()\l- Intersect()\l* SetActiveInteractions()\l* SetConnectivities()\l}"
template "UML:CLASS"
graphics
[
x 223.0
y 639.0
w 20.0
h 20.0
]
]
node
[
id 138
label "{Tahoe::J2_C0HardeningT\n|# fK\l# fIsLinear\l# fYield\l|+ J2_C0HardeningT()\l+ ~J2_C0HardeningT()\l+ YieldCondition()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l# H()\l# dH()\l# K()\l# dK()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l* H()\l* dH()\l* K()\l* dK()\l}"
template "UML:CLASS"
graphics
[
x 253.0
y 864.0
w 20.0
h 20.0
]
]
node
[
id 139
label "{Tahoe::ConstantVolumeT\n|# fFaceDomain\l# fGrid\l# fSurfaceLinks\l# fDOFConnects\l# fEqnos\l# fPressureTag\l# fPressureLast\l# fSurfaces\l# fShapes\l# fCurrShapes\l# fLocInitCoords\l# fLocCurrCoords\l# fOutputID\l# fFaceForce\l# fCurrentFaceArea\l# fSurface1\l# fSurface2\l# fFaceConnectivities\l# fFaceEquations\l# fGrad_d\l# fIPCoords2\l# fIPNorm2\l# fNEE_vec\l# fNEE_mat\l# fNEE_vec_man\l# fNEE_mat_man\l# fGrad_d_man\l# fFace2_man\l|+ ConstantVolumeT()\l+ ~ConstantVolumeT()\l+ TangentType()\l+ RelaxSystem()\l+ AddNodalForce()\l+ InternalEnergy()\l+ RegisterOutput()\l+ WriteOutput()\l+ SendOutput()\l+ CloseStep()\l+ Equations()\l+ Group()\l+ SetDOFTags()\l+ DOFTags()\l+ GenerateElementData()\l+ DOFConnects()\l+ ResetDOF()\l+ Reconfigure()\l+ ResetState()\l+ ConnectsU()\l+ ConnectsX()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# NumIP()\l# WriteCallLocation()\l# LHSDriver()\l# RHSDriver()\l# ExtractSurfaces()\l# SetWorkSpace()\l# InputSideSets()\l* FaceIndexT\l* fFaceCentroids\l* fFaceIndex\l* fDOFConnects\l* fEqnos\l* fPressureTag\l* fPressureLast\l* fSurfaces\l* fShapes\l* fCurrShapes\l* fLocInitCoords\l* fLocCurrCoords\l* fOutputID\l* fFaceForce\l* fCurrentFaceArea\l* fSurface1\l* fSurface2\l* fFaceConnectivities\l* fFaceEquations\l* fGrad_d\l* fIPCoords2\l* fIPNorm2\l* fNEE_vec\l* fNEE_mat\l* fNEE_vec_man\l* fNEE_mat_man\l* fGrad_d_man\l* fFace2_man\l* ConnectsU()\l* ConnectsX()\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l* LHSDriver()\l* RHSDriver()\l* ExtractSurfaces()\l* SetWorkSpace()\l* InputSideSets()\l}"
template "UML:CLASS"
graphics
[
x 800.0
y 298.0
w 20.0
h 20.0
]
]
node
[
id 140
label "{Tahoe::VIB_E_MatT\n|# fU_0\l|+ VIB_E_MatT()\l# SetReferenceEnergy()\l# VIBEnergyDensity()\l# ComputeLengths()\l# SetStressPointers2D()\l# SetStressPointers3D()\l# SetModuliPointers2D()\l# SetModuliPointers3D()\l}"
template "UML:CLASS"
graphics
[
x 752.0
y 981.0
w 20.0
h 20.0
]
]
node
[
id 141
label "{Tahoe::Chain1D\n|- fNearestNeighbor\l- fLattice1D\l- fPairProperty\l- fAtomicVolume\l- fFullDensity\l- fFullDensityForStressOutput\l- fBondTensor4\l- fBondTensor2\l|+ Chain1D()\l+ ~Chain1D()\l+ BondLattice()\l+ CellVolume()\l+ NearestNeighbor()\l+ DefineParameters()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l# ComputeModuli()\l# ComputePK2()\l# ComputeEnergyDensity()\l# ZeroStressStretch()\l* fBondTensor4\l* fBondTensor2\l* BondLattice()\l* CellVolume()\l* NearestNeighbor()\l* DefineParameters()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 907.0
y 77.0
w 20.0
h 20.0
]
]
node
[
id 142
label "{Tahoe::YoonAllen2DT\n|- fsigma_0\l- fd_c_n\l- fd_c_t\l- fE_infty\l- fE_t\l- ftau\l- fexp_tau\l- idamage\l- falpha_exp\l- flambda_exp\l- falpha_0\l- flambda_0\l- fpenalty\l- fK\l- fTimeStep\l- fCurrentTimeStep\l|+ YoonAllen2DT()\l+ SetTimeStep()\l+ InitStateVariables()\l+ NumStateVariables()\l+ FractureEnergy()\l+ Potential()\l+ Traction()\l+ Stiffness()\l+ Status()\l+ NumOutputVariables()\l+ OutputLabels()\l+ ComputeOutput()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# CompatibleOutput()\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 152.0
y 271.0
w 20.0
h 20.0
]
]
node
[
id 143
label "{Tahoe::PenaltyWallT\n|# fnormal\l# ftangent\l# normal_displ\l# tangent_displ\l# fntforce\l# fnforce\l# ftforce\l# fxyforce\l# fQ\l# fp_i\l# fv_i\l# fprev_i\l# fcurr_i\l# fdispl_i\l# fLHS\l# fd_sh\l# fi_sh\l|+ PenaltyWallT()\l+ ApplyLHS()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l- ComputeContactForce()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 477.0
y 382.0
w 20.0
h 20.0
]
]
node
[
id 144
label "{Tahoe::nTrapezoid\n|- dpred_v\l- dcorr_v\l|+ nTrapezoid()\l+ ConsistentKBC()\l+ ExternalNodeCondition()\l+ Predictor()\l+ Corrector()\l+ Corrector()\l+ MappedCorrector()\l+ MappedCorrectorField()\l# nComputeParameters()\l}"
template "UML:CLASS"
graphics
[
x 207.0
y 706.0
w 20.0
h 20.0
]
]
node
[
id 145
label "{Tahoe::EAMPropertyT\n||+ EAMPropertyT()\l+ getParadynTable()\l+ GetLatticeParameter()\l+ getPairEnergy()\l+ getEmbedEnergy()\l+ getElecDensEnergy()\l+ getPairForce()\l+ getEmbedForce()\l+ getElecDensForce()\l+ getPairStiffness()\l+ getEmbedStiffness()\l+ getElecDensStiffness()\l* PairEnergyFunction\l* EmbedEnergyFunction\l* EDEnergyFunction\l* PairForceFunction\l* EmbedForceFunction\l* EDForceFunction\l* PairStiffnessFunction\l* EmbedStiffnessFunction\l* EDStiffnessFunction\l* getPairEnergy()\l* getEmbedEnergy()\l* getElecDensEnergy()\l* getPairForce()\l* getEmbedForce()\l* getElecDensForce()\l* getPairStiffness()\l* getEmbedStiffness()\l* getElecDensStiffness()\l}"
template "UML:CLASS"
graphics
[
x 970.0
y 14.0
w 20.0
h 20.0
]
]
node
[
id 146
label "{Tahoe::ExplicitCDIntegrator\n||+ ExplicitCDIntegrator()\l# ComputeParameters()\l}"
template "UML:CLASS"
graphics
[
x 218.0
y 858.0
w 20.0
h 20.0
]
]
node
[
id 147
label "{Tahoe::SimoQ1P0Axi_inv\n|# fPressure\l# fJacobian\l# fOutputInit\l# fOutputCell\l# fElementVolume\l# fGamma\l# fGamma_last\l# fMeanGradient\l# fF_tmp\l# fNEEmat\l# fdiff_b\l# fb_bar\l# fb_sig\l|+ SimoQ1P0Axi_inv()\l+ CloseStep()\l+ ResetStep()\l+ ReadRestart()\l+ WriteRestart()\l+ TakeParameterList()\l# SetGlobalShape()\l# FormStiffness()\l# FormKd()\l- SetMeanGradient()\l- bSp_bRq_to_KSqRp()\l* fElementVolume\l* fGamma\l* fGamma_last\l* fMeanGradient\l* fF_tmp\l* fNEEmat\l* fdiff_b\l* fb_bar\l* fb_sig\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 294.0
y 870.0
w 20.0
h 20.0
]
]
node
[
id 148
label "{Tahoe::FSSolidMatList2DT\n|- fFSMatSupport\l|+ FSSolidMatList2DT()\l+ FSSolidMatList2DT()\l+ HasPlaneStress()\l+ NewFSSolidMat()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 178.0
y 948.0
w 20.0
h 20.0
]
]
node
[
id 149
label "{Tahoe::J2Simo3D\n|- fFmech\l- ffrel\l- fF_temp\l|+ J2Simo3D()\l+ TangentType()\l+ UpdateHistory()\l+ ResetHistory()\l+ c_ijkl()\l+ s_ij()\l+ IncrementalHeat()\l+ HasIncrementalHeat()\l+ HasHistory()\l+ StrainEnergyDensity()\l+ Need_F_last()\l+ NumOutputVariables()\l+ OutputLabels()\l+ ComputeOutput()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l- SupportsThermalStrain()\l- ComputeGradients()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 537.0
y 591.0
w 20.0
h 20.0
]
]
node
[
id 150
label "{Tahoe::FullMatrixT\n|# fMatrix\l# fIsFactorized\l|+ FullMatrixT()\l+ FullMatrixT()\l+ Initialize()\l+ Clear()\l+ AddEquationSet()\l+ AddEquationSet()\l+ Assemble()\l+ Assemble()\l+ Assemble()\l+ OverWrite()\l+ Disassemble()\l+ DisassembleDiagonal()\l+ EquationNumberScope()\l+ RenumberEquations()\l+ MatrixType()\l+ operator=()\l+ Clone()\l+ Multx()\l+ MultTx()\l+ MultmBn()\l# BackSubstitute()\l# PrintAllPivots()\l# PrintZeroPivots()\l# PrintLHS()\l}"
template "UML:CLASS"
graphics
[
x 168.0
y 44.0
w 20.0
h 20.0
]
]
node
[
id 151
label "{Tahoe::Tijssens2DT\n|- fTimeStep\l- fk_t0\l- fk_n\l- fc_1\l- fDelta_n_ccr\l- fA_0\l- fA\l- fQ_A\l- fB_0\l- fB\l- fQ_B\l- fGamma_0\l- fDelta_0\l- fsigma_c\l- ftau_c\l- fastar\l- ftemp\l- fGroup\l- fSteps\l|+ Tijssens2DT()\l+ SetTimeStep()\l+ NumStateVariables()\l+ FractureEnergy()\l+ Potential()\l+ Traction()\l+ Stiffness()\l+ Status()\l+ NumOutputVariables()\l+ OutputLabels()\l+ ComputeOutput()\l+ NeedsNodalInfo()\l+ NodalQuantityNeeded()\l+ SetElementGroupsNeeded()\l+ DefineParameters()\l+ TakeParameterList()\l# CompatibleOutput()\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 523.0
y 571.0
w 20.0
h 20.0
]
]
node
[
id 152
label "{Tahoe::MeshFreeFractureSupportT\n|- fNumFacetNodes\l- fFacets\l- fFacetman\l- fResetFacets\l- fInitTractions\l- fInitTractionMan\l- fs_i\l- fda\l- fda_s\l- fcone\l- fn_s\l- fFrontList\l- fSamplingSurfaces\l- fCriterion\l- fs_u\l- fhoop\l- ftmp_nsd\l|+ MeshFreeFractureSupportT()\l+ ~MeshFreeFractureSupportT()\l+ NumFacetNodes()\l+ Facets()\l+ ResetFacets()\l+ InitTractions()\l+ WriteOutput()\l+ InitStep()\l+ CloseStep()\l+ ResetStep()\l+ HasActiveCracks()\l+ FractureCriterion()\l+ CheckGrowth()\l+ InitSupport()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# int2FractureCriterionT()\l- InitCuttingFacetsAndFronts()\l- InitSamplingSurfaces()\l- InitializeFronts()\l- CheckFronts()\l- CheckSurfaces()\l- InitFacetDatabase()\l- ComputeCriterion()\l- SetStreamPrefs()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 84.0
y 226.0
w 20.0
h 20.0
]
]
node
[
id 153
label "{Tahoe::StaticT\n||+ StaticT()\l+ ImplicitExplicit()\l+ Order()\l+ OrderOfUnknown()\l* ImplicitExplicit()\l* Order()\l* OrderOfUnknown()\l}"
template "UML:CLASS"
graphics
[
x 94.0
y 599.0
w 20.0
h 20.0
]
]
node
[
id 154
label "{Tahoe::eExplicitCD\n|- fconstC\l|+ eExplicitCD()\l+ FormM()\l+ FormC()\l+ FormK()\l+ FormMa()\l+ FormCv()\l+ FormKd()\l# eComputeParameters()\l* FormM()\l* FormC()\l* FormK()\l* FormMa()\l* FormCv()\l* FormKd()\l}"
template "UML:CLASS"
graphics
[
x 991.0
y 229.0
w 20.0
h 20.0
]
]
node
[
id 155
label "{Tahoe::PairPropertyT\n||+ PairPropertyT()\l+ New()\l* EnergyFunction\l* ForceFunction\l* StiffnessFunction\l* getEnergyFunction()\l* getForceFunction()\l* getStiffnessFunction()\l* getParadynTable()\l}"
template "UML:CLASS"
graphics
[
x 2.0
y 301.0
w 20.0
h 20.0
]
]
node
[
id 156
label "{Tahoe::FiniteStrainT\n|# fCurrShapes\l# fFSMatSupport\l# fF_List\l# fF_all\l# fF_last_List\l# fF_last_all\l- fNeedsOffset\l|+ FiniteStrainT()\l+ ~FiniteStrainT()\l+ CollectMaterialInfo()\l+ CurrShapeFunction()\l+ DeformationGradient()\l+ DeformationGradient()\l+ DeformationGradient_last()\l+ DeformationGradient_last()\l+ ComputeGradient()\l+ ComputeGradient()\l+ IP_Interpolate_current()\l+ ComputeGradient_reference()\l+ ComputeGradient_reference()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l# NewMaterialSupport()\l# NewMaterialList()\l# SetGlobalShape()\l# Needs_F()\l# Needs_F_last()\l# CurrElementInfo()\l* fF_List\l* fF_all\l* fF_last_List\l* fF_last_all\l* DeformationGradient()\l* DeformationGradient()\l* DeformationGradient_last()\l* DeformationGradient_last()\l* ComputeGradient()\l* ComputeGradient()\l* IP_Interpolate_current()\l* ComputeGradient_reference()\l* ComputeGradient_reference()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 952.0
y 445.0
w 20.0
h 20.0
]
]
node
[
id 157
label "{Tahoe::EAMFCC3DMatT\n|# fEAM\l|+ EAMFCC3DMatT()\l+ ~EAMFCC3DMatT()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# ComputeModuli()\l# ComputePK2()\l# ComputeEnergyDensity()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 24.0
y 312.0
w 20.0
h 20.0
]
]
node
[
id 158
label "{Tahoe::DRSolver\n|- fMass\l- fVel\l- fDisp\l- fDamp\l- fKd\l- fNumEquations\l- fCCSLHS\l|+ DRSolver()\l+ Initialize()\l+ Solve()\l- ComputeMass()\l- ComputeVelocity()\l- ComputeDamping()\l}"
template "UML:CLASS"
graphics
[
x 11.0
y 977.0
w 20.0
h 20.0
]
]
node
[
id 159
label "{Tahoe::SSSolidMatList1DT\n|- fSSMatSupport\l|+ SSSolidMatList1DT()\l+ SSSolidMatList1DT()\l+ NewSSSolidMat()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 365.0
y 840.0
w 20.0
h 20.0
]
]
node
[
id 160
label "{Tahoe::NL_E_MatT\n|# fE\l# fPK2\l# fModuli\l|+ NL_E_MatT()\l+ StrainEnergyDensity()\l+ c_ijkl()\l+ s_ij()\l+ Pressure()\l+ C_IJKL()\l+ S_IJ()\l+ TakeParameterList()\l# ComputeModuli()\l# ComputePK2()\l# ComputeEnergyDensity()\l* c_ijkl()\l* s_ij()\l* Pressure()\l* C_IJKL()\l* S_IJ()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 399.0
y 820.0
w 20.0
h 20.0
]
]
node
[
id 161
label "{Tahoe::TvergHutch2DT\n|- fsigma_max\l- fd_c_n\l- fd_c_t\l- fL_1\l- fL_2\l- fL_fail\l- fpenalty\l- fK\l- fSecantStiffness\l|+ TvergHutch2DT()\l+ NumStateVariables()\l+ FractureEnergy()\l+ FractureStrength()\l+ Potential()\l+ Traction()\l+ Stiffness()\l+ Status()\l+ NumOutputVariables()\l+ OutputLabels()\l+ ComputeOutput()\l+ DefineParameters()\l+ TakeParameterList()\l# CompatibleOutput()\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 36.0
y 619.0
w 20.0
h 20.0
]
]
node
[
id 162
label "{Tahoe::RodT\n|# fMaterialsList\l# fCurrMaterial\l# fGroupNodes\l- fOutputDiagnostic\l- fOneOne\l- fBond\l- fBond0\l- fLocAcc\l- fNEE_vec\l- fKb\l- fInstKE\l- fInstPE\l- fInstTotalE\l- fInstTemp\l- fInstPressure\l- fAvgKE\l- fAvgPE\l- fAvgTotalE\l- fAvgTemp\l- fAvgPressure\l- fSumKE\l- fSumPE\l- fSumTotalE\l- fSumTemp\l- fSumPressure\l- fLocVel\l- fHardyStress\l- fHardyHeatFlux\l|+ RodT()\l+ TangentType()\l+ AddNodalForce()\l+ InternalEnergy()\l+ RegisterOutput()\l+ WriteOutput()\l+ SendOutput()\l+ CloseStep()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# LHSDriver()\l# RHSDriver()\l# NextElement()\l# ChangingGeometry()\l- ComputeInstKE()\l- ComputeAvgKE()\l- ComputeInstPE()\l- ComputeAvgPE()\l- ComputeInstTotalE()\l- ComputeAvgTotalE()\l- ComputeInstTemperature()\l- ComputeAvgTemperature()\l- ComputeInstPressure()\l- ComputeAvgPressure()\l- PrintMDToFile()\l- ComputeHardyStress()\l- ComputeHardyHeatFlux()\l- LocalizationFunction()\l- PairIntegral()\l* fOneOne\l* fBond\l* fBond0\l* fLocAcc\l* fNEE_vec\l* fKb\l* fInstKE\l* fInstPE\l* fInstTotalE\l* fInstTemp\l* fInstPressure\l* fAvgKE\l* fAvgPE\l* fAvgTotalE\l* fAvgTemp\l* fAvgPressure\l* fSumKE\l* fSumPE\l* fSumTotalE\l* fSumTemp\l* fSumPressure\l* fLocVel\l* fHardyStress\l* fHardyHeatFlux\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 8.0
y 632.0
w 20.0
h 20.0
]
]
node
[
id 163
label "{Tahoe::FSIsotropicMatT\n||+ FSIsotropicMatT()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 307.0
y 136.0
w 20.0
h 20.0
]
]
node
[
id 164
label "{Tahoe::ExplicitCD\n||+ ExplicitCD()\l+ ImplicitExplicit()\l+ Order()\l+ OrderOfUnknown()\l* ImplicitExplicit()\l* Order()\l* OrderOfUnknown()\l}"
template "UML:CLASS"
graphics
[
x 686.0
y 786.0
w 20.0
h 20.0
]
]
node
[
id 165
label "{Tahoe::CubicSplineWindowT\n|- fDilationScaling\l- fCutOffFactor\l- fNSD\l- fNSDsym\l- fNSDArray\l|+ CubicSplineWindowT()\l+ Name()\l+ SearchType()\l+ NumberOfSupportParameters()\l+ SynchronizeSupportParameters()\l+ WriteParameters()\l+ Window()\l+ Window()\l+ Covers()\l+ Covers()\l+ SphericalSupportSize()\l+ RectangularSupportSize()\l+ SphericalSupportSize()\l* Covers()\l* Covers()\l* SphericalSupportSize()\l* RectangularSupportSize()\l* SphericalSupportSize()\l}"
template "UML:CLASS"
graphics
[
x 834.0
y 533.0
w 20.0
h 20.0
]
]
node
[
id 166
label "{Tahoe::PenaltyContactDrag3DT\n|# fDrag\l# fGapTolerance\l# fSlipTolerance\l|+ PenaltyContactDrag3DT()\l+ DefineParameters()\l+ TakeParameterList()\l# RHSDriver()\l# LHSDriver()\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 714.0
y 827.0
w 20.0
h 20.0
]
]
node
[
id 167
label "{Tahoe::ScaledVelocityNodesT\n|# fField\l# qIConly\l# qFirstTime\l# qAllNodes\l# qRandomize\l# fIncs\l# fIncCt\l# fTempSchedule\l# fTempScale\l# fDummySchedule\l# fNodeIds\l# fNodes\l# fMass\l# fRandom\l# fT_0\l|+ ScaledVelocityNodesT()\l+ InitStep()\l+ InitialCondition()\l+ IsICController()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# SetBCCards()\l# InitNodeSets()\l* fNodeIds\l* fNodes\l* fMass\l* fRandom\l* fT_0\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l* InitNodeSets()\l}"
template "UML:CLASS"
graphics
[
x 151.0
y 56.0
w 20.0
h 20.0
]
]
node
[
id 168
label "{Tahoe::EAMFCC2D\n|# fEAM\l|+ EAMFCC2D()\l+ ~EAMFCC2D()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# ComputeModuli()\l# ComputePK2()\l# ComputeEnergyDensity()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 917.0
y 281.0
w 20.0
h 20.0
]
]
node
[
id 169
label "{Tahoe::KBC_CardT\n|# fnode\l# fdof\l# fcode\l# fvalue\l# fSchedule\l|+ SetValues()\l+ Value()\l+ KBC_CardT()\l+ KBC_CardT()\l+ Node()\l+ DOF()\l+ Code()\l+ Schedule()\l+ int2CodeT()\l* KBC_CardT()\l* KBC_CardT()\l* Node()\l* DOF()\l* Code()\l* Schedule()\l}"
template "UML:CLASS"
graphics
[
x 561.0
y 586.0
w 20.0
h 20.0
]
]
node
[
id 170
label "{Tahoe::J2Simo2D\n|- fFmech\l- ffrel\l- fF_temp\l- fFmech_2D\l- ffrel_2D\l|+ J2Simo2D()\l+ TangentType()\l+ UpdateHistory()\l+ ResetHistory()\l+ c_ijkl()\l+ s_ij()\l+ StrainEnergyDensity()\l+ IncrementalHeat()\l+ HasIncrementalHeat()\l+ HasHistory()\l+ Need_F_last()\l+ NumOutputVariables()\l+ OutputLabels()\l+ ComputeOutput()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l- SupportsThermalStrain()\l- ComputeGradients()\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 314.0
y 57.0
w 20.0
h 20.0
]
]
node
[
id 171
label "{Tahoe::TiedPotentialBaseT\n|# iBulkGroups\l|+ TiedPotentialBaseT()\l+ ~TiedPotentialBaseT()\l+ NeedsNodalInfo()\l+ NodalQuantityNeeded()\l+ RotateNodalQuantity()\l+ InitiationQ()\l+ BulkGroups()\l+ NodesMayRetie()\l+ RetieQ()\l* kTiedNode\l* kReleaseNextStep\l* kFirstFreeStep\l* kFreeNode\l* kTieNextStep\l* TiedStatusPosition()\l}"
template "UML:CLASS"
graphics
[
x 436.0
y 31.0
w 20.0
h 20.0
]
]
node
[
id 172
label "{Tahoe::Gear6\n||+ Gear6()\l+ ImplicitExplicit()\l+ Order()\l+ OrderOfUnknown()\l* ImplicitExplicit()\l* Order()\l* OrderOfUnknown()\l}"
template "UML:CLASS"
graphics
[
x 904.0
y 699.0
w 20.0
h 20.0
]
]
node
[
id 173
label "{Tahoe::EAMT\n|- fEAMProperties\l- fNeighbors\l- fEqnos\l- fElectronDensity\l- fElectronDensity_man\l- fElectronDensityMessageID\l- fEmbeddingEnergy\l- fEmbeddingEnergy_man\l- fEmbeddingEnergyMessageID\l- fEmbeddingForce\l- fEmbeddingForce_man\l- fEmbeddingForceMessageID\l- fEmbeddingStiff\l- fEmbeddingStiff_man\l- fEmbeddingStiffMessageID\l- frhop_r\l- frhop_r_man\l- frhop_rMessageID\l- fExternalElecDensity\l- fExternalEmbedForce\l- fExternalElecDensityNodes\l- fExternalEmbedForceNodes\l- fOutputFlags\l- fNearestNeighbors\l- fRefNearestNeighbors\l- fForce_list\l- fForce_list_man\l- fOneOne\l|+ EAMT()\l+ Equations()\l+ WriteOutput()\l+ FormStiffness()\l+ SetExternalElecDensity()\l+ SetExternalEmbedForce()\l+ ConnectsX()\l+ ConnectsU()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l# SetConfiguration()\l# ExtractProperties()\l# SetOutputCount()\l# GenerateOutputLabels()\l# New_EAMProperty()\l# LHSDriver()\l# RHSDriver()\l# RHSDriver2D()\l# RHSDriver3D()\l- GetRho2D()\l- GetRho3D()\l- GetRhop_r()\l- GetEmbEnergy()\l- GetEmbForce()\l- GetEmbStiff()\l* fNearestNeighbors\l* fRefNearestNeighbors\l* fForce_list\l* fForce_list_man\l* fOneOne\l* ConnectsX()\l* ConnectsU()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l* LHSDriver()\l* RHSDriver()\l* RHSDriver2D()\l* RHSDriver3D()\l}"
template "UML:CLASS"
graphics
[
x 124.0
y 260.0
w 20.0
h 20.0
]
]
node
[
id 174
label "{Tahoe::nStaticIntegrator\n||+ nStaticIntegrator()\l+ ConsistentKBC()\l+ ExternalNodeCondition()\l+ Predictor()\l+ Corrector()\l+ Corrector()\l+ MappedCorrector()\l+ MappedCorrectorField()\l# nComputeParameters()\l}"
template "UML:CLASS"
graphics
[
x 576.0
y 963.0
w 20.0
h 20.0
]
]
node
[
id 175
label "{Tahoe::YoonAllen3DT\n|- fsigma_0\l- fd_c_n\l- fd_c_t\l- fE_infty\l- fE_t\l- ftau\l- fexp_tau\l- idamage\l- falpha_exp\l- flambda_exp\l- falpha_0\l- flambda_0\l- fpenalty\l- fK\l- fTimeStep\l- fCurrentTimeStep\l|+ YoonAllen3DT()\l+ YoonAllen3DT()\l+ SetTimeStep()\l+ InitStateVariables()\l+ NumStateVariables()\l+ FractureEnergy()\l+ Potential()\l+ Traction()\l+ Stiffness()\l+ Status()\l+ NumOutputVariables()\l+ OutputLabels()\l+ ComputeOutput()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# CompatibleOutput()\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 255.0
y 790.0
w 20.0
h 20.0
]
]
node
[
id 176
label "{Tahoe::PCGSolver_LS\n|- fRestart\l- fRestart_count\l- fOutputFlag\l- fSearchIterations\l- fOrthogTolerance\l- fMaxStepSize\l- fR\l- fR_last\l- fu_last\l- fdiff_R\l- fUpdate\l- s_current\l- fSearchData\l|+ PCGSolver_LS()\l+ Initialize()\l+ Solve()\l+ DefineParameters()\l+ TakeParameterList()\l# Update()\l# Iterate()\l- CGSearch()\l- GValue()\l* fRestart\l* fRestart_count\l* fOutputFlag\l* fSearchIterations\l* fOrthogTolerance\l* fMaxStepSize\l* fR\l* fR_last\l* fu_last\l* fdiff_R\l* fUpdate\l* s_current\l* fSearchData\l* PCGSolver_LS()\l* Initialize()\l* Solve()\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 118.0
y 534.0
w 20.0
h 20.0
]
]
node
[
id 177
label "{Tahoe::SimoIso3D\n|# fb\l# fb_bar\l# fStress\l# fModulus\l- frank4\l- fIdentity\l- fIcrossI\l- fIdentity4\l- fDevOp4\l|+ SimoIso3D()\l+ StrainEnergyDensity()\l+ c_ijkl()\l+ s_ij()\l+ Pressure()\l+ TakeParameterList()\l# ComputeModuli()\l# ComputeCauchy()\l# ComputeEnergy()\l# U()\l# dU()\l# ddU()\l- SupportsThermalStrain()\l* fStress\l* fModulus\l* c_ijkl()\l* s_ij()\l* Pressure()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 534.0
y 742.0
w 20.0
h 20.0
]
]
node
[
id 178
label "{Tahoe::DOFElementT\n||+ DOFElementT()\l+ Group()\l+ SetDOFTags()\l+ DOFTags()\l+ GenerateElementData()\l+ DOFConnects()\l+ ResetDOF()\l+ Reconfigure()\l+ ResetState()\l}"
template "UML:CLASS"
graphics
[
x 784.0
y 248.0
w 20.0
h 20.0
]
]
node
[
id 179
label "{Tahoe::SIERRA_Material_Data\n|- fID\l- fName\l- fModulusFlag\l- fNumStateVars\l- fXMLCommandID\l- fSymbolMap\l- fCheckFunction\l- fCalcFunction\l- fInitFunction\l- fPCFunction\l- fInputVariables\l- fInputVariableSize\l- fStrainMeasure\l- fPropertyNames\l- fPropertyValues\l- fPropertyMap\l- sNextID\l|+ SIERRA_Material_Data()\l+ SetCheckFunction()\l+ SetCalcFunction()\l+ SetInitFunction()\l+ SetPCFunction()\l+ AddXMLCommandID()\l+ AddInputVariable()\l+ SetNumStateVariables()\l+ ID()\l+ Name()\l+ XMLCommandID()\l+ NumStateVariables()\l+ InputVariables()\l+ InputVariableSize()\l+ StrainMeasure()\l+ CheckFunction()\l+ CalcFunction()\l+ InitFunction()\l+ AddProperty()\l+ PropertyNames()\l+ PropertyValues()\l+ Property()\l+ HasProperty()\l+ AddSymbol()\l+ Symbol()\l+ Compare()\l* fCheckFunction\l* fCalcFunction\l* fInitFunction\l* fPCFunction\l* fInputVariables\l* fInputVariableSize\l* fStrainMeasure\l* fPropertyNames\l* fPropertyValues\l* fPropertyMap\l* SetCheckFunction()\l* SetCalcFunction()\l* SetInitFunction()\l* SetPCFunction()\l* AddXMLCommandID()\l* AddInputVariable()\l* SetNumStateVariables()\l* ID()\l* Name()\l* XMLCommandID()\l* NumStateVariables()\l* InputVariables()\l* InputVariableSize()\l* StrainMeasure()\l* CheckFunction()\l* CalcFunction()\l* InitFunction()\l* AddProperty()\l* PropertyNames()\l* PropertyValues()\l* Property()\l* HasProperty()\l* AddSymbol()\l* Symbol()\l}"
template "UML:CLASS"
graphics
[
x 250.0
y 989.0
w 20.0
h 20.0
]
]
node
[
id 180
label "{Tahoe::Anisotropic2DT\n|# fRotator\l|+ Anisotropic2DT()\l+ Anisotropic2DT()\l+ SetRotation()\l+ ~Anisotropic2DT()\l+ Print()\l# Q()\l# TransformIn()\l# TransformOut()\l# TransformIn()\l# TransformOut()\l# TransformIn()\l# TransformOut()\l- Construct()\l}"
template "UML:CLASS"
graphics
[
x 426.0
y 45.0
w 20.0
h 20.0
]
]
node
[
id 181
label "{Tahoe::SIERRA_HypoElasticT\n||+ SIERRA_HypoElasticT()\l+ StrainEnergyDensity()\l# Register_SIERRA_Material()\l# SetOutputVariables()\l* Register_SIERRA_Material()\l* SetOutputVariables()\l}"
template "UML:CLASS"
graphics
[
x 457.0
y 876.0
w 20.0
h 20.0
]
]
node
[
id 182
label "{Tahoe::MaterialListT\n|# fHasHistory\l|+ MaterialListT()\l+ MaterialListT()\l+ ~MaterialListT()\l+ InitStep()\l+ CloseStep()\l+ HasHistoryMaterials()\l}"
template "UML:CLASS"
graphics
[
x 572.0
y 773.0
w 20.0
h 20.0
]
]
node
[
id 183
label "{Tahoe::SimoQ1P0\n|# fPressure\l# fJacobian\l# fElementVolume\l# fElementVolume_last\l# fMeanGradient\l# fF_tmp\l# fNEEmat\l# fdiff_b\l# fb_bar\l# fb_sig\l|+ SimoQ1P0()\l+ CloseStep()\l+ ResetStep()\l+ ReadRestart()\l+ WriteRestart()\l+ TakeParameterList()\l# SetGlobalShape()\l# FormStiffness()\l# FormKd()\l- SetMeanGradient()\l- bSp_bRq_to_KSqRp()\l* fElementVolume\l* fElementVolume_last\l* fMeanGradient\l* fF_tmp\l* fNEEmat\l* fdiff_b\l* fb_bar\l* fb_sig\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 713.0
y 280.0
w 20.0
h 20.0
]
]
node
[
id 184
label "{Tahoe::CirclePointsT\n|# fPoints\l# fAngles\l# fJacobians\l- fQ\l|+ CirclePointsT()\l+ ~CirclePointsT()\l+ CirclePoints()\l+ CircleAngles()\l+ Jacobians()\l+ Jacobians()\l# TransformPoints()\l}"
template "UML:CLASS"
graphics
[
x 895.0
y 307.0
w 20.0
h 20.0
]
]
node
[
id 185
label "{Tahoe::nMixed\n|- dpred_v_\l- dcorr_v_\l|+ nMixed()\l+ Dimension()\l+ ConsistentKBC()\l+ ExternalNodeCondition()\l+ Predictor()\l+ Corrector()\l+ Corrector()\l+ MappedCorrector()\l+ MappedCorrectorField()\l# nComputeParameters()\l}"
template "UML:CLASS"
graphics
[
x 824.0
y 935.0
w 20.0
h 20.0
]
]
node
[
id 186
label "{Tahoe::SimoIso2D\n|# fStress2D\l# fModulus2D\l# fb_2D\l|+ SimoIso2D()\l+ InitStep()\l+ c_ijkl()\l+ s_ij()\l+ StrainEnergyDensity()\l+ TakeParameterList()\l# Compute_b_3D()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 1.0
y 182.0
w 20.0
h 20.0
]
]
node
[
id 187
label "{Tahoe::UnConnectedRodT\n|- fReconnectInc\l- fMaxNeighborCount\l- fNeighborDist\l- fNumNodesUsed\l- fReconnectCount\l|+ UnConnectedRodT()\l+ InitStep()\l+ ResetStep()\l+ RelaxSystem()\l# ReadMaterialData()\l# EchoConnectivityData()\l# ChangingGeometry()\l- ConfigureElementData()\l- PrintConnectivityData()\l}"
template "UML:CLASS"
graphics
[
x 780.0
y 670.0
w 20.0
h 20.0
]
]
node
[
id 188
label "{Tahoe::MeshFreeSupport2DT\n||+ MeshFreeSupport2DT()\l+ MeshFreeSupport2DT()\l+ SetCuttingFacets()\l- ProcessBoundaries()\l- Visible()\l- Intersect()\l- Intersect_2()\l}"
template "UML:CLASS"
graphics
[
x 865.0
y 562.0
w 20.0
h 20.0
]
]
node
[
id 189
label "{Tahoe::VerletIntegrator\n||+ VerletIntegrator()\l# ComputeParameters()\l}"
template "UML:CLASS"
graphics
[
x 392.0
y 561.0
w 20.0
h 20.0
]
]
node
[
id 190
label "{Tahoe::LJTr2D\n|- feps\l|+ LJTr2D()\l+ DefineParameters()\l+ TakeParameterList()\l# ComputeModuli()\l# ComputePK2()\l# ComputeEnergyDensity()\l# LoadBondTable()\l- Ulj()\l- dUlj()\l- ddU()\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 16.0
y 218.0
w 20.0
h 20.0
]
]
node
[
id 191
label "{Tahoe::XuNeedleman3DT\n|- q\l- r\l- d_n\l- d_t\l- phi_n\l- r_fail\l- fKratio\l- fK\l|+ NumStateVariables()\l+ FractureEnergy()\l+ Potential()\l+ Traction()\l+ Stiffness()\l+ Status()\l+ XuNeedleman3DT()\l+ XuNeedleman3DT()\l+ DefineParameters()\l+ TakeParameterList()\l* XuNeedleman3DT()\l* XuNeedleman3DT()\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 494.0
y 591.0
w 20.0
h 20.0
]
]
node
[
id 192
label "{Tahoe::RGViscoelasticityT\n|# fC_v\l# fC_vn\l# fNumProcess\l# fnstatev\l# fstatev\l- fSpectralDecompRef\l|+ RGViscoelasticityT()\l+ Pressure()\l+ HasHistory()\l+ NeedsPointInitialization()\l+ PointInitialize()\l+ UpdateHistory()\l+ ResetHistory()\l+ InitStep()\l+ TangentType()\l+ Compute_Eigs_v()\l+ Compute_Eigs_vn()\l+ Load()\l+ Store()\l+ SetStateVariables()\l+ TakeParameterList()\l# MixedRank4_2D()\l# MixedRank4_3D()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 208.0
y 387.0
w 20.0
h 20.0
]
]
node
[
id 193
label "{Tahoe::TiedNodesT\n|# fField\l# fDummySchedule\l# qNoTiedPotential\l# iElemGroups\l# fLeaderIds\l# fFollowerIds\l# fNodePairs\l# fPairStatus\l# fPairStatus_last\l|+ TiedNodesT()\l+ SetTiedPairs()\l+ SetExternalNodes()\l+ InitialCondition()\l+ RelaxSystem()\l+ Connectivities()\l+ Equations()\l+ SetEquations()\l+ ReadRestart()\l+ WriteRestart()\l+ InitStep()\l+ FormRHS()\l+ CloseStep()\l+ Reset()\l+ DefineParameters()\l+ DefineSubs()\l+ TakeParameterList()\l# ChangeStatus()\l# SetBCCards()\l# Update()\l# CopyKinematics()\l# WriteOutput()\l# InitTiedNodePairs()\l* fLeaderIds\l* fFollowerIds\l* fNodePairs\l* fPairStatus\l* fPairStatus_last\l* ReadRestart()\l* WriteRestart()\l* InitStep()\l* FormRHS()\l* CloseStep()\l* Reset()\l* DefineParameters()\l* DefineSubs()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 413.0
y 742.0
w 20.0
h 20.0
]
]
node
[
id 194
label "{Tahoe::SS_SCNIMFT\n|# fSSMatSupport\l# fssEONI\l# fNeedsOffset\l# fStrain_list\l# fStrain_last_list\l|+ SS_SCNIMFT()\l+ SS_SCNIMFT()\l+ ~SS_SCNIMFT()\l+ WriteOutput()\l+ RelaxSystem()\l+ LHSDriver()\l+ RHSDriver()\l+ DefineParameters()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l# CollectMaterialInfo()\l# NewMaterialList()\l# bVectorToMatrix()\l# bprimeVectorToMatrix()\l* fStrain_list\l* fStrain_last_list\l* DefineParameters()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 173.0
y 960.0
w 20.0
h 20.0
]
]
node
[
id 195
label "{Tahoe::FS_SCNIMF_AxiT\n||+ FS_SCNIMF_AxiT()\l+ WriteOutput()\l+ RelaxSystem()\l+ LHSDriver()\l+ RHSDriver()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# TransformModuli()\l# SetOutputCount()\l# CollectMaterialInfo()\l# GenerateOutputLabels()\l# AssembleParticleMass()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 366.0
y 346.0
w 20.0
h 20.0
]
]
node
[
id 196
label "{Tahoe::CCSMatrixT\n|# fEqnos\l# fRaggedEqnos\l# fDiags\l# fNumberOfTerms\l# fMatrix\l# fIsFactorized\l# fBand\l# fMeanBand\l|+ CCSMatrixT()\l+ CCSMatrixT()\l+ ~CCSMatrixT()\l+ Initialize()\l+ Info()\l+ Clear()\l+ AddEquationSet()\l+ AddEquationSet()\l+ Assemble()\l+ Assemble()\l+ Assemble()\l+ ResidualNorm()\l+ AbsRowSum()\l+ pTKp()\l+ HasNegativePivot()\l+ WriteAztecFormat()\l+ EquationNumberScope()\l+ RenumberEquations()\l+ MatrixType()\l+ FindMinMaxPivot()\l+ operator=()\l+ Clone()\l+ Multx()\l+ MultTx()\l+ CopyDiagonal()\l+ PrintAllPivots()\l+ PrintZeroPivots()\l+ PrintLHS()\l# Factorize()\l# BackSubstitute()\l# ColumnHeight()\l# operator()()\l# operator()()\l- ComputeSize()\l- SetColumnHeights()\l- SetColumnHeights()\l- NumberOfFilled()\l- FillWithOnes()\l- FillWithOnes()\l* fEqnos\l* fRaggedEqnos\l* fDiags\l* fNumberOfTerms\l* fMatrix\l* fIsFactorized\l* fBand\l* fMeanBand\l* PrintAllPivots()\l* PrintZeroPivots()\l* PrintLHS()\l* operator()()\l* operator()()\l}"
template "UML:CLASS"
graphics
[
x 567.0
y 866.0
w 20.0
h 20.0
]
]
node
[
id 197
label "{Tahoe::LJSpringT\n|- f_eps\l- f_sigma\l|+ LJSpringT()\l+ HasInternalStrain()\l+ Potential()\l+ DPotential()\l+ DDPotential()\l* f_eps\l* f_sigma\l}"
template "UML:CLASS"
graphics
[
x 285.0
y 219.0
w 20.0
h 20.0
]
]
node
[
id 198
label "{Tahoe::nIntegratorT\n||+ nIntegratorT()\l+ ~nIntegratorT()\l+ nIntegrator()\l+ Dimension()\l+ ExternalNodeCondition()\l+ Predictor()\l+ Corrector()\l+ Corrector()\l+ MappedCorrector()\l+ MappedCorrectorField()\l+ ConsistentKBC()\l# nComputeParameters()\l}"
template "UML:CLASS"
graphics
[
x 285.0
y 165.0
w 20.0
h 20.0
]
]
node
[
id 199
label "{Tahoe::SSLinearVE2D\n|# fMu\l# fKappa\l# fModulus\l# fStress\l# fModMat\l# fStress3D\l# fStrain3D\l# ftauS\l# ftauB\l# falphaS\l# falphaB\l# fbetaS\l# fbetaB\l|+ SSLinearVE2D()\l+ PointInitialize()\l+ StrainEnergyDensity()\l+ c_ijkl()\l+ s_ij()\l+ C_IJKL()\l+ S_IJ()\l+ NumOutputVariables()\l+ OutputLabels()\l+ ComputeOutput()\l+ DefineParameters()\l+ TakeParameterList()\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 593.0
y 669.0
w 20.0
h 20.0
]
]
node
[
id 200
label "{Tahoe::QuadLog3D\n|# fSpectral\l# fb\l# fStress\l# fModulus\l# fDevOp3\l# fEigs\l# floge\l# fBeta\l# fEigMod\l|+ QuadLog3D()\l+ C_IJKL()\l+ S_IJ()\l+ StrainEnergyDensity()\l+ c_ijkl()\l+ s_ij()\l+ Pressure()\l+ TakeParameterList()\l# ComputeModuli()\l# ComputeCauchy()\l# ComputeEnergy()\l# LogStretches()\l* c_ijkl()\l* s_ij()\l* Pressure()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 229.0
y 628.0
w 20.0
h 20.0
]
]
node
[
id 201
label "{Tahoe::SSIsotropicMatT\n||+ SSIsotropicMatT()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 743.0
y 677.0
w 20.0
h 20.0
]
]
node
[
id 202
label "{Tahoe::MeshFreeT\n||}"
template "UML:CLASS"
graphics
[
x 554.0
y 185.0
w 20.0
h 20.0
]
]
node
[
id 203
label "{Tahoe::FSMatSupportT\n|- fFiniteStrain\l- fF_List\l- fF_last_List\l|+ FSMatSupportT()\l+ Interpolate_current()\l+ DeformationGradient()\l+ DeformationGradient()\l+ DeformationGradient_last()\l+ DeformationGradient_last()\l+ SetDeformationGradient()\l+ SetDeformationGradient_last()\l+ ComputeGradient()\l+ ComputeGradient()\l+ ComputeGradient_reference()\l+ ComputeGradient_reference()\l+ FiniteStrain()\l+ SetContinuumElement()\l* fF_List\l* fF_last_List\l* DeformationGradient()\l* DeformationGradient()\l* DeformationGradient_last()\l* DeformationGradient_last()\l* SetDeformationGradient()\l* SetDeformationGradient_last()\l* ComputeGradient()\l* ComputeGradient()\l* ComputeGradient_reference()\l* ComputeGradient_reference()\l* FiniteStrain()\l* SetContinuumElement()\l}"
template "UML:CLASS"
graphics
[
x 983.0
y 271.0
w 20.0
h 20.0
]
]
node
[
id 204
label "{Tahoe::ConveyorT\n|# fField\l# fMeshRepeatLength\l# fWindowShiftDistance\l# fRightMinSpacing\l# fULBC_Code\l# fULBC_Value\l# fULBC_ScheduleNumber\l# fULBC_Schedule\l# fRightEdge\l# fShiftedNodesU\l# fShiftedNodesL\l# fNumSamples\l# fUy_node_upper\l# fUy_node_lower\l# fUy_samples_upper\l# fUy_samples_lower\l# fTipElementGroup\l# fTipX_0\l# fTipY_0\l# fTrackingType\l# fTipThreshold\l# fTipOutputVariable\l# fTipOutputCode\l# fTipColumnNum\l# fDampingWidth\l# fDampingCoefficient\l# fDampingNodes\l# fDampingForce\l# fDampingCoeff\l# fDampingEqnos\l# fDampingReset\l# fBottomNodes\l# fTopNodes\l# fX_Left\l# fX_Right\l# fX_PeriodicLength\l# fWidthDeadZone\l# fX_Left_last\l# fX_Right_last\l# fTrackingInterval\l# fTrackingOutputInterval\l# fTrackingPoint\l# fTrackingPoint_last\l# fTrackingOutput\l|+ ConveyorT()\l+ Reset()\l+ InitialCondition()\l+ InitStep()\l+ FormRHS()\l+ Update()\l+ CloseStep()\l+ RelaxSystem()\l+ ReadRestart()\l+ WriteRestart()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# TrackPoint()\l# SetSystemFocus()\l# MarkElements()\l# CreatePrecrack()\l# UpperLower()\l* fMeshRepeatLength\l* fWindowShiftDistance\l* fRightMinSpacing\l* fULBC_Code\l* fULBC_Value\l* fULBC_ScheduleNumber\l* fULBC_Schedule\l* fRightEdge\l* fShiftedNodesU\l* fShiftedNodesL\l* fNumSamples\l* fUy_node_upper\l* fUy_node_lower\l* fUy_samples_upper\l* fUy_samples_lower\l* fTipElementGroup\l* fTipX_0\l* fTipY_0\l* fTrackingType\l* fTipThreshold\l* fTipOutputVariable\l* fTipOutputCode\l* fTipColumnNum\l* fDampingWidth\l* fDampingCoefficient\l* fDampingNodes\l* fDampingForce\l* fDampingCoeff\l* fDampingEqnos\l* fDampingReset\l* fBottomNodes\l* fTopNodes\l* fX_Left\l* fX_Right\l* fX_PeriodicLength\l* fWidthDeadZone\l* fX_Left_last\l* fX_Right_last\l* fTrackingInterval\l* fTrackingOutputInterval\l* fTrackingPoint\l* fTrackingPoint_last\l* fTrackingOutput\l* ReadRestart()\l* WriteRestart()\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 419.0
y 682.0
w 20.0
h 20.0
]
]
node
[
id 205
label "{Tahoe::IntegratorT\n|# fdt\l|+ IntegratorT()\l+ ~IntegratorT()\l+ SetTimeStep()\l+ FormNodalForce()\l+ ImplicitExplicit()\l+ Order()\l+ OrderOfUnknown()\l+ eIntegrator()\l+ nIntegrator()\l+ New()\l# ComputeParameters()\l* ImplicitExplicit()\l* Order()\l* OrderOfUnknown()\l* eIntegrator()\l* nIntegrator()\l}"
template "UML:CLASS"
graphics
[
x 63.0
y 656.0
w 20.0
h 20.0
]
]
node
[
id 206
label "{Tahoe::MeshFreeSSSolidT\n|- fMFShapes\l- fMFFractureSupport\l- fAutoBorder\l- fB_wrap\l- fFieldSet\l- fMeshfreeParameters\l|+ MeshFreeSSSolidT()\l+ ~MeshFreeSSSolidT()\l+ Equations()\l+ ConnectsU()\l+ WriteOutput()\l+ RelaxSystem()\l+ InterpolantDOFs()\l+ NodalDOFs()\l+ WeightNodalCost()\l+ InitStep()\l+ CloseStep()\l+ ResetStep()\l+ MeshFreeSupport()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# SetShape()\l# NextElement()\l# ComputeOutput()\l- WriteField()\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 669.0
y 716.0
w 20.0
h 20.0
]
]
node
[
id 207
label "{Tahoe::D3MeshFreeSupport2DT\n||+ D3MeshFreeSupport2DT()\l+ D3MeshFreeSupport2DT()\l+ SetCuttingFacets()\l- ProcessBoundaries()\l- Visible()\l- Intersect()\l}"
template "UML:CLASS"
graphics
[
x 528.0
y 321.0
w 20.0
h 20.0
]
]
node
[
id 208
label "{Tahoe::EAM_particle\n|- fEAMProperty\l- fPairEnergy\l- fPairForce\l- fPairStiffness\l- fEmbedStiffness\l- fEmbedEnergy\l- fEmbedForce\l- fEDEnergy\l- fEDForce\l- fEDStiffness\l- fLattice\l- fLatticeParameter\l- fMass\l- fBondTensor4\l- fAmn\l- fBondTensor2\l- fBondTensor2b\l- fTensor2Table\l- fIntType\l- fBond1\l- fBond2\l- fBond3\l- fBond4\l- fBond5\l- fBond6\l- fRepRho\l|+ EAM_particle()\l+ ~EAM_particle()\l+ Initialize()\l+ ComputeUnitEnergy()\l+ ComputeUnitStress()\l+ ComputeUnitSurfaceStress()\l+ ComputeUnitModuli()\l+ LatticeParameter()\l+ Mass()\l+ TotalElectronDensity()\l+ ComputeElectronDensity()\l+ ReturnEmbeddingForce()\l- FormMixedDerivatives()\l- FormSingleBondContribution()\l- FormMixedBondContribution()\l* fEAMProperty\l* fPairEnergy\l* fPairForce\l* fPairStiffness\l* fEmbedStiffness\l* fEmbedEnergy\l* fEmbedForce\l* fEDEnergy\l* fEDForce\l* fEDStiffness\l* fLattice\l* fLatticeParameter\l* fMass\l* fBondTensor4\l* fAmn\l* fBondTensor2\l* fBondTensor2b\l* fTensor2Table\l* fIntType\l* fBond1\l* fBond2\l* fBond3\l* fBond4\l* fBond5\l* fBond6\l* fRepRho\l}"
template "UML:CLASS"
graphics
[
x 245.0
y 187.0
w 20.0
h 20.0
]
]
node
[
id 209
label "{Tahoe::TersoffPropertyT\n||+ TersoffPropertyT()\l+ New()\l* EnergyFunction\l* ForceFunction\l* StiffnessFunction\l* getEnergyFunction()\l* getForceFunction_ij()\l* getForceFunction_ik()\l* getForceFunction_jk()\l* getStiffnessFunction()\l* GetR()\l* GetS()\l}"
template "UML:CLASS"
graphics
[
x 19.0
y 291.0
w 20.0
h 20.0
]
]
node
[
id 210
label "{Tahoe::MSRMatrixT\n|# fSymmetric\l# fMSRBuilder\l# fupdate\l# fbindx\l# fval\l# fQF_shift\l# fupdate_bin\l# fsrow_dex\l# fsrow_val\l# fRowEqnVec\l# fColEqnVec\l# fRowDexVec\l# fColDexVec\l# fValVec\l# fActiveBlk\l# fActiveBlkMan\l# fActiveDex\l|+ MSRMatrixT()\l+ MSRMatrixT()\l+ ~MSRMatrixT()\l+ Initialize()\l+ CopyDiagonal()\l+ Clear()\l+ EquationNumberScope()\l+ RenumberEquations()\l+ operator=()\l+ AddEquationSet()\l+ AddEquationSet()\l+ Assemble()\l+ Assemble()\l+ Assemble()\l# GenerateRCV()\l# PrintAllPivots()\l# PrintZeroPivots()\l# PrintLHS()\l- AssembleRow()\l- AssembleDiagonals()\l- SetMSRData()\l- SetUpQuickFind()\l- AZ_quick_find()\l- AZ_find_index()\l- AZ_init_quick_find()\l- AZ_sort()\l* fupdate\l* fbindx\l* fval\l* fQF_shift\l* fupdate_bin\l* fsrow_dex\l* fsrow_val\l* fRowEqnVec\l* fColEqnVec\l* fRowDexVec\l* fColDexVec\l* fValVec\l* fActiveBlk\l* fActiveBlkMan\l* fActiveDex\l* AddEquationSet()\l* AddEquationSet()\l* Assemble()\l* Assemble()\l* Assemble()\l* PrintAllPivots()\l* PrintZeroPivots()\l* PrintLHS()\l* AZ_quick_find()\l* AZ_find_index()\l* AZ_init_quick_find()\l* AZ_sort()\l}"
template "UML:CLASS"
graphics
[
x 617.0
y 125.0
w 20.0
h 20.0
]
]
node
[
id 211
label "{Tahoe::eLinearHHTalpha\n|- fconstM\l- fconstC\l- fconstK\l|+ eLinearHHTalpha()\l+ FormM()\l+ FormC()\l+ FormK()\l+ FormMa()\l+ FormCv()\l+ FormKd()\l# eComputeParameters()\l* fconstM\l* fconstC\l* fconstK\l* FormM()\l* FormC()\l* FormK()\l* FormMa()\l* FormCv()\l* FormKd()\l}"
template "UML:CLASS"
graphics
[
x 649.0
y 901.0
w 20.0
h 20.0
]
]
node
[
id 212
label "{Tahoe::FieldT\n|- fFieldSupport\l- fGroup\l- fIntegrator\l- fnIntegrator\l- fField_last\l- fIC\l- fKBC\l- fKBC_Controllers\l- fFBC\l- fFBCValues\l- fFBCEqnos\l- fFBC_Controllers\l- fUpdate\l- fEquationStart\l- fNumEquations\l- fID\l- fSourceOutput\l- fSourceID\l- fSourceBlocks\l- fTrackTotalEnergy\l- fTotalEnergyOutputInc\l- fTotalEnergyOutputID\l- fWork\l- fPointConnect\l- fActiveMass\l- fActiveForce\l- fActiveVel\l|+ FieldT()\l+ ~FieldT()\l+ Connectivities()\l+ SystemType()\l+ InitialConditions()\l+ KinematicBC()\l+ ForceBC()\l+ KBC_Controllers()\l+ FBC_Controllers()\l+ RelaxSystem()\l+ CloseStep()\l+ ResetStep()\l+ RegisterOutput()\l+ WriteOutput()\l+ WriteParameters()\l+ AddKBCController()\l+ AddFBCController()\l+ GetfFBCValues()\l+ GetfFBCEqnos()\l+ Initialize()\l+ RegisterLocal()\l+ SetGroup()\l+ Dimension()\l+ Clear()\l+ FieldSupport()\l+ operator()()\l+ operator()()\l+ Group()\l+ Integrator()\l+ Integrator()\l+ nIntegrator()\l+ nIntegrator()\l+ SetTimeStep()\l+ InitialCondition()\l+ InitStep()\l+ FormLHS()\l+ FormRHS()\l+ EndRHS()\l+ EndLHS()\l+ Update()\l+ AssembleUpdate()\l+ ApplyUpdate()\l+ CopyNodeToNode()\l+ InitEquations()\l+ FinalizeEquations()\l+ NumEquations()\l+ EquationStart()\l+ EquationSets()\l+ SetLocalEqnos()\l+ SetLocalEqnos()\l+ SetLocalEqnos()\l+ SetLocalEqnos()\l+ SetLocalEqnos()\l+ SetLocalEqnos()\l+ WriteRestart()\l+ ReadRestart()\l+ RegisterSource()\l+ Source()\l+ DefineParameters()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l- Apply_IC()\l- SetBCCode()\l- SetFBCEquations()\l- SourceIndex()\l* fIC\l* fKBC\l* fKBC_Controllers\l* fFBC\l* fFBCValues\l* fFBCEqnos\l* fFBC_Controllers\l* fUpdate\l* fEquationStart\l* fNumEquations\l* fID\l* fSourceOutput\l* fSourceID\l* fSourceBlocks\l* fTrackTotalEnergy\l* fTotalEnergyOutputInc\l* fTotalEnergyOutputID\l* fWork\l* fPointConnect\l* fActiveMass\l* fActiveForce\l* fActiveVel\l* Initialize()\l* RegisterLocal()\l* SetGroup()\l* Dimension()\l* Clear()\l* FieldSupport()\l* operator()()\l* operator()()\l* Group()\l* Integrator()\l* Integrator()\l* nIntegrator()\l* nIntegrator()\l* SetTimeStep()\l* InitialCondition()\l* InitStep()\l* FormLHS()\l* FormRHS()\l* EndRHS()\l* EndLHS()\l* Update()\l* AssembleUpdate()\l* ApplyUpdate()\l* CopyNodeToNode()\l* InitEquations()\l* FinalizeEquations()\l* NumEquations()\l* EquationStart()\l* EquationSets()\l* SetLocalEqnos()\l* SetLocalEqnos()\l* SetLocalEqnos()\l* SetLocalEqnos()\l* SetLocalEqnos()\l* SetLocalEqnos()\l* WriteRestart()\l* ReadRestart()\l* RegisterSource()\l* Source()\l* DefineParameters()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 642.0
y 925.0
w 20.0
h 20.0
]
]
node
[
id 213
label "{Tahoe::LinearDamageT\n|- fInitTraction\l- fd_c_n\l- fd_c_t\l- fpenalty\l- fK\l|+ LinearDamageT()\l+ NumStateVariables()\l+ InitStateVariables()\l+ FractureEnergy()\l+ Potential()\l+ Traction()\l+ Stiffness()\l+ Status()\l}"
template "UML:CLASS"
graphics
[
x 343.0
y 79.0
w 20.0
h 20.0
]
]
node
[
id 214
label "{Tahoe::SS_SCNIMF_AxiT\n||+ SS_SCNIMF_AxiT()\l+ SS_SCNIMF_AxiT()\l+ WriteOutput()\l+ RelaxSystem()\l+ LHSDriver()\l+ RHSDriver()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# SetOutputCount()\l# CollectMaterialInfo()\l# GenerateOutputLabels()\l# AssembleParticleMass()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 501.0
y 803.0
w 20.0
h 20.0
]
]
node
[
id 215
label "{Tahoe::NoseHooverT\n|# fBetaOrig\l# fEta\l# fEtaDot\l|+ NoseHooverT()\l+ ApplyDamping()\l+ WriteRestart()\l+ ReadRestart()\l+ TakeParameterList()\l* fBetaOrig\l* fEta\l* fEtaDot\l* WriteRestart()\l* ReadRestart()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 243.0
y 247.0
w 20.0
h 20.0
]
]
node
[
id 216
label "{Tahoe::J2QLLinHardT\n|- fb_elastic\l- fEPModuli\l- fa_inverse\l- fMatrixTemp1\l- fMatrixTemp2\l- fMatrixTemp3\l- fdev_beta\l- fb_n\l- fb_tr\l- fbeta_tr\l- fUnitNorm\l- fInternal\l- fFtot\l- ffrel\l- fF_temp\l|+ J2QLLinHardT()\l+ Need_F_last()\l+ HasHistory()\l+ UpdateHistory()\l+ ResetHistory()\l+ c_ijkl()\l+ s_ij()\l+ StrainEnergyDensity()\l+ NumOutputVariables()\l+ OutputLabels()\l+ ComputeOutput()\l+ DefineParameters()\l+ TakeParameterList()\l# TrialStretch()\l# ReturnMapping()\l# ElastoPlasticCorrection()\l# AllocateElement()\l- ComputeGradients()\l- InitIntermediate()\l- LoadData()\l- PlasticLoading()\l- YieldCondition()\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 665.0
y 494.0
w 20.0
h 20.0
]
]
node
[
id 217
label "{Tahoe::IsoVIB3D\n|# fSpectral\l# fEigs\l# fEigmods\l- fb\l- fSphere\l- fModulus\l- fStress\l|+ IsoVIB3D()\l+ ~IsoVIB3D()\l+ C_IJKL()\l+ S_IJ()\l+ StrainEnergyDensity()\l+ c_ijkl()\l+ s_ij()\l+ Pressure()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# ComputeLengths()\l- Construct()\l* c_ijkl()\l* s_ij()\l* Pressure()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 282.0
y 123.0
w 20.0
h 20.0
]
]
node
[
id 218
label "{Tahoe::HyperbolicDiffusionElementT\n|# fTau\l# fLocAcc\l|+ HyperbolicDiffusionElementT()\l+ DefineParameters()\l+ TakeParameterList()\l# SetLocalArrays()\l# LHSDriver()\l# RHSDriver()\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 527.0
y 79.0
w 20.0
h 20.0
]
]
node
[
id 219
label "{Tahoe::Hex2D\n|- fNearestNeighbor\l- fQ\l- fHexLattice2D\l- fPairProperty\l- fCellVolume\l- fFullDensity\l- fFullDensityForStressOutput\l- fBondTensor4\l- fBondTensor2\l|+ Hex2D()\l+ ~Hex2D()\l+ BondLattice()\l+ CellVolume()\l+ NearestNeighbor()\l+ DefineParameters()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l# ComputeModuli()\l# ComputePK2()\l# ComputeEnergyDensity()\l# ZeroStressStretch()\l* fBondTensor4\l* fBondTensor2\l* BondLattice()\l* CellVolume()\l* NearestNeighbor()\l* DefineParameters()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 109.0
y 451.0
w 20.0
h 20.0
]
]
node
[
id 220
label "{Tahoe::ThermostatBaseT\n|# fTemperatureSchedule\l# fTemperatureScale\l# fSupport\l# fBeta\l# fTemperature\l# fAllNodes\l# fNodes\l# fxmin\l# fxmax\l# nIncs\l|+ ThermostatBaseT()\l+ ~ThermostatBaseT()\l+ Beta()\l+ Temperature()\l+ NodeList()\l+ WriteRestart()\l+ ReadRestart()\l+ ApplyDamping()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# NodesInRegion()\l# InitNodeSets()\l# InitRegion()\l* fSupport\l* fBeta\l* fTemperature\l* fAllNodes\l* fNodes\l* fxmin\l* fxmax\l* nIncs\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l* InitNodeSets()\l* InitRegion()\l}"
template "UML:CLASS"
graphics
[
x 478.0
y 246.0
w 20.0
h 20.0
]
]
node
[
id 221
label "{Tahoe::AugLagCylinderT\n|# fContactEqnos2D\l# fContactTags\l# fContactDOFtags\l# fFloatingDOF\l# fLastDOF\l# fUzawa\l# fPrimalIterations\l# fPenetrationTolerance\l# fDOF\l# fDOFi\l# fIterationi\l# fRecomputeForce\l|+ AugLagCylinderT()\l+ SetEquationNumbers()\l+ Equations()\l+ Connectivities()\l+ ReadRestart()\l+ WriteRestart()\l+ InitStep()\l+ CloseStep()\l+ RelaxSystem()\l+ ApplyLHS()\l+ SetDOFTags()\l+ DOFTags()\l+ GenerateElementData()\l+ DOFConnects()\l+ ResetDOF()\l+ Reconfigure()\l+ ResetState()\l+ Group()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# ComputeContactForce()\l* fContactDOFtags\l* fFloatingDOF\l* fLastDOF\l* fUzawa\l* fPrimalIterations\l* fPenetrationTolerance\l* fDOF\l* fDOFi\l* fIterationi\l* fRecomputeForce\l* SetDOFTags()\l* DOFTags()\l* GenerateElementData()\l* DOFConnects()\l* ResetDOF()\l* Reconfigure()\l* ResetState()\l* Group()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 337.0
y 371.0
w 20.0
h 20.0
]
]
node
[
id 222
label "{Tahoe::SSSolidMatT\n|# fSSMatSupport\l# fModulus\l- fStrainTemp\l- fQ\l- fHasThermalStrain\l- fThermalStrain\l|+ SSSolidMatT()\l+ SetSSMatSupport()\l+ Need_Strain()\l+ Need_Strain_last()\l+ e()\l+ e()\l+ e_last()\l+ e_last()\l+ C_IJKL()\l+ S_IJ()\l+ c_ijkl()\l+ ce_ijkl()\l+ InitStep()\l+ Strain()\l+ HasDissipVar()\l+ SupportsThermalStrain()\l+ IsLocalized()\l+ IsLocalized()\l+ IsLocalized()\l# AcousticalTensor()\l- SetThermalStrain()\l- Q_2D()\l- Q_3D()\l}"
template "UML:CLASS"
graphics
[
x 679.0
y 27.0
w 20.0
h 20.0
]
]
node
[
id 223
label "{Tahoe::AugLagWallT\n|- fContactEqnos2D\l- fContactTags\l- fContactDOFtags\l- fFloatingDOF\l- fLastDOF\l- fUzawa\l- fPrimalIterations\l- fPenetrationTolerance\l- fDOF\l- fDOFi\l- fIterationi\l- fRecomputeForce\l|+ AugLagWallT()\l+ SetEquationNumbers()\l+ Equations()\l+ Connectivities()\l+ ReadRestart()\l+ WriteRestart()\l+ InitStep()\l+ CloseStep()\l+ RelaxSystem()\l+ ApplyLHS()\l+ SetDOFTags()\l+ DOFTags()\l+ GenerateElementData()\l+ DOFConnects()\l+ ResetDOF()\l+ Reconfigure()\l+ ResetState()\l+ Group()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l- ComputeContactForce()\l* fContactDOFtags\l* fFloatingDOF\l* fLastDOF\l* fUzawa\l* fPrimalIterations\l* fPenetrationTolerance\l* fDOF\l* fDOFi\l* fIterationi\l* fRecomputeForce\l* SetDOFTags()\l* DOFTags()\l* GenerateElementData()\l* DOFConnects()\l* ResetDOF()\l* Reconfigure()\l* ResetState()\l* Group()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 151.0
y 126.0
w 20.0
h 20.0
]
]
node
[
id 224
label "{Tahoe::CBLatticeT\n||+ CBLatticeT()\l+ BondComponentTensor4()\l+ BondComponentTensor2()\l+ BatchBondComponentTensor2()\l- BondTensor4_2D()\l- BondTensor4_3D()\l- BondTensor2_2D()\l- BondTensor2_3D()\l- BatchBondTensor2_2D()\l- BatchBondTensor2_3D()\l}"
template "UML:CLASS"
graphics
[
x 749.0
y 520.0
w 20.0
h 20.0
]
]
node
[
id 225
label "{Tahoe::ScheduleT\n|- fFunction\l- fCurrentTime\l- fCurrentValue\l|+ ~ScheduleT()\l+ SetTime()\l+ ScheduleT()\l+ ScheduleT()\l+ Value()\l+ Value()\l+ Time()\l+ Rate()\l+ Rate()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l* fCurrentTime\l* fCurrentValue\l* ScheduleT()\l* ScheduleT()\l* Value()\l* Value()\l* Time()\l* Rate()\l* Rate()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 698.0
y 154.0
w 20.0
h 20.0
]
]
node
[
id 226
label "{Tahoe::Traction_CardT\n|- fElemNum\l- fFacetNum\l- fCoordSystem\l- fLTfPtr\l- fValues\l- fLocNodeNums\l- fNodes\l- fEqnos\l- fTau\l|+ Traction_CardT()\l+ EchoValues()\l+ EchoValues()\l+ SetValues()\l+ SetRelaxationTime()\l+ Destination()\l+ CoordSystem()\l+ CurrentValue()\l+ LocalNodeNumbers()\l+ Nodes()\l+ Eqnos()\l+ Nodes()\l+ Eqnos()\l+ WriteHeader()\l+ int2CoordSystemT()\l}"
template "UML:CLASS"
graphics
[
x 172.0
y 470.0
w 20.0
h 20.0
]
]
node
[
id 227
label "{Tahoe::SimoQ1P0Axi\n|# fPressure\l# fJacobian\l# fOutputInit\l# fOutputCell\l# fElementVolume\l# fElementVolume_last\l# fMeanGradient\l# fF_tmp\l# fNEEmat\l# fdiff_b\l# fb_bar\l# fb_sig\l|+ SimoQ1P0Axi()\l+ CloseStep()\l+ ResetStep()\l+ ReadRestart()\l+ WriteRestart()\l+ TakeParameterList()\l# SetGlobalShape()\l# FormStiffness()\l# FormKd()\l- SetMeanGradient()\l- bSp_bRq_to_KSqRp()\l* fElementVolume\l* fElementVolume_last\l* fMeanGradient\l* fF_tmp\l* fNEEmat\l* fdiff_b\l* fb_bar\l* fb_sig\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 328.0
y 571.0
w 20.0
h 20.0
]
]
node
[
id 228
label "{Tahoe::DiagonalMatrixT\n|- fMatrix\l- fIsFactorized\l- fMode\l|+ DiagonalMatrixT()\l+ DiagonalMatrixT()\l+ SolvePreservesData()\l+ SetAssemblyMode()\l+ Initialize()\l+ Clear()\l+ AddEquationSet()\l+ AddEquationSet()\l+ Assemble()\l+ Assemble()\l+ Assemble()\l+ DisassembleDiagonal()\l+ TheMatrix()\l+ EquationNumberScope()\l+ RenumberEquations()\l+ MatrixType()\l+ operator=()\l+ Clone()\l+ Multx()\l+ MultTx()\l+ MultmBn()\l# Factorize()\l# BackSubstitute()\l# PrintAllPivots()\l# PrintZeroPivots()\l# PrintLHS()\l}"
template "UML:CLASS"
graphics
[
x 114.0
y 902.0
w 20.0
h 20.0
]
]
node
[
id 229
label "{Tahoe::MeshFreeShapeFunctionT\n|# fMFSupport\l# fCurrElement\l# fNeighbors\l# fNaU\l# fDNaU\l# fXConnects\l# fExactNodes\l# fElemHasExactNode\l# fElemFlags\l# fR\l# fDR\l# fNa_tmp\l# fDNa_tmp\l# felSpace\l# fndSpace\l# fNeighExactFlags\l|+ MeshFreeShapeFunctionT()\l+ ~MeshFreeShapeFunctionT()\l+ Initialize()\l+ SetSupportSize()\l+ SetNeighborData()\l+ SetExactNodes()\l+ SetSkipNodes()\l+ SetSkipElements()\l+ SetNodalParameters()\l+ GetNodalParameters()\l+ NodalParameters()\l+ SetDerivatives()\l+ SetDerivativesAt()\l+ UseDerivatives()\l+ SetCuttingFacets()\l+ ResetFacets()\l+ ResetNodes()\l+ ResetCells()\l+ NumberOfNeighbors()\l+ Neighbors()\l+ ElementNeighborsCounts()\l+ ElementNeighbors()\l+ NodeNeighbors()\l+ SelectedNodalField()\l+ NodalField()\l+ NodalField()\l+ Print()\l+ PrintAt()\l+ WriteParameters()\l+ WriteStatistics()\l+ BlendElementData()\l+ BlendNodalData()\l+ MeshFreeSupport()\l- InitBlend()\l}"
template "UML:CLASS"
graphics
[
x 434.0
y 225.0
w 20.0
h 20.0
]
]
node
[
id 230
label "{Tahoe::Mixed\n|# fNumDOF\l|+ Mixed()\l+ ImplicitExplicit()\l+ Order()\l+ OrderOfUnknown()\l* ImplicitExplicit()\l* Order()\l* OrderOfUnknown()\l}"
template "UML:CLASS"
graphics
[
x 953.0
y 325.0
w 20.0
h 20.0
]
]
node
[
id 231
label "{Tahoe::NLSolverX\n|- fMaxNewTangents\l- fMaxTangentReuse\l- fMinFreshTangents\l- fCheckNegPivots\l- fTrustTol\l- fMinResRatio\l- fFormNewTangent\l- fNumNewTangent\l- fLastUpdate\l- pCCS\l- pCCNS\l|+ NLSolverX()\l+ Solve()\l+ ResetStep()\l# DoConverged()\l}"
template "UML:CLASS"
graphics
[
x 601.0
y 553.0
w 20.0
h 20.0
]
]
node
[
id 232
label "{Tahoe::ContinuumElementT\n|# fGroupCommunicator\l# fMaterialList\l# fNodalOutputCodes\l# fElementOutputCodes\l# fBodySchedule\l# fBody\l# fTractionList\l# fTractionBCSet\l# fShapes\l# fStoreShape\l# fLocInitCoords\l# fLocDisp\l# fNEEvec\l# fDOFvec\l- fNumIP\l- fGeometryCode\l|+ ContinuumElementT()\l+ ~ContinuumElementT()\l+ NumIP()\l+ ShapeFunction()\l+ CurrIP()\l+ GroupCommunicator()\l+ IP_Coords()\l+ IP_Interpolate()\l+ IP_Interpolate()\l+ IP_ComputeGradient()\l+ IP_ComputeGradient()\l+ IP_ExtrapolateAll()\l+ InitialCoordinates()\l+ Displacements()\l+ Equations()\l+ TangentType()\l+ ReadRestart()\l+ WriteRestart()\l+ FacetGeometry()\l+ GeometryCode()\l+ SetStatus()\l+ InitialCondition()\l+ MaterialsList()\l+ InitStep()\l+ CloseStep()\l+ ResetStep()\l+ RelaxSystem()\l+ RegisterOutput()\l+ WriteOutput()\l+ ResolveOutputVariable()\l+ DefineParameters()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l+ int2MassTypeT()\l# SetLocalArrays()\l# SetShape()\l# RHSDriver()\l# ApplyTractionBC()\l# SetGlobalShape()\l# FormMass()\l# AddBodyForce()\l# FormMa()\l# TakeNaturalBC()\l# CurrElementInfo()\l# CollectMaterialInfo()\l# NewMaterialList()\l# NewMaterialSupport()\l# CheckMaterialOutput()\l# SetNodalOutputCodes()\l# SetElementOutputCodes()\l# ComputeOutput()\l# GenerateOutputLabels()\l- SetTractionBC()\l- DefaultNumElemNodes()\l* fLocInitCoords\l* fLocDisp\l* fNEEvec\l* fDOFvec\l* InitStep()\l* CloseStep()\l* ResetStep()\l* RelaxSystem()\l* RegisterOutput()\l* WriteOutput()\l* ResolveOutputVariable()\l* DefineParameters()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l* CollectMaterialInfo()\l* NewMaterialList()\l* NewMaterialSupport()\l* CheckMaterialOutput()\l* SetNodalOutputCodes()\l* SetElementOutputCodes()\l* ComputeOutput()\l* GenerateOutputLabels()\l}"
template "UML:CLASS"
graphics
[
x 257.0
y 647.0
w 20.0
h 20.0
]
]
node
[
id 233
label "{Tahoe::PenaltyCylinderT\n|# fRadius\l# fDirection\l# fv_OP\l# fR\l# fLHS\l# fd_sh\l# fi_sh\l|+ PenaltyCylinderT()\l+ TangentType()\l+ ApplyLHS()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# ComputeContactForce()\l* fv_OP\l* fR\l* fLHS\l* fd_sh\l* fi_sh\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 317.0
y 253.0
w 20.0
h 20.0
]
]
node
[
id 234
label "{Tahoe::SolidElementT\n|# fMassType\l# fNormal\l# fv_ss\l# fDensity\l# fCurrMaterial\l# fMaterialNeeds\l# fIncrementalHeat\l# qUseSimo\l# qNoExtrap\l# fEigenvalueInc\l# fLocLastDisp\l# fLocVel\l# fLocAcc\l# fLocTemp\l# fLocTemp_last\l# fElementHeat\l# fD\l# fB\l# fStress\l# fStoreInternalForce\l# fForce\l# NumNodalOutputCodes\l# NumElementOutputCodes\l|+ SolidElementT()\l+ ~SolidElementT()\l+ CloseStep()\l+ TangentType()\l+ AddNodalForce()\l+ AddLinearMomentum()\l+ InternalEnergy()\l+ SendOutput()\l+ SetStoreInternalForce()\l+ InternalForce()\l+ LastDisplacements()\l+ Velocities()\l+ Accelerations()\l+ Temperatures()\l+ LastTemperatures()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l# MaxEigenvalue()\l# SetLocalArrays()\l# SetShape()\l# SetGlobalShape()\l# NewMaterialSupport()\l# Set_B()\l# Set_B_axi()\l# Set_B_bar()\l# Set_B_bar_axi()\l# NextElement()\l# FormStiffness()\l# FormKd()\l# StructuralMaterialList()\l# ComputeOutput()\l# LHSDriver()\l# ElementLHSDriver()\l# RHSDriver()\l# ElementRHSDriver()\l# SetNodalOutputCodes()\l# SetElementOutputCodes()\l# GenerateOutputLabels()\l* fLocLastDisp\l* fLocVel\l* fLocAcc\l* fLocTemp\l* fLocTemp_last\l* fElementHeat\l* fD\l* fB\l* fStress\l* fStoreInternalForce\l* fForce\l* LastDisplacements()\l* Velocities()\l* Accelerations()\l* Temperatures()\l* LastTemperatures()\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l* LHSDriver()\l* ElementLHSDriver()\l* RHSDriver()\l* ElementRHSDriver()\l* SetNodalOutputCodes()\l* SetElementOutputCodes()\l* GenerateOutputLabels()\l}"
template "UML:CLASS"
graphics
[
x 884.0
y 815.0
w 20.0
h 20.0
]
]
node
[
id 235
label "{Tahoe::SurfacePotentialT\n|# fTraction\l# fStiffness\l|+ SurfacePotentialT()\l+ ~SurfacePotentialT()\l+ SetTimeStep()\l+ NumStateVariables()\l+ InitStateVariables()\l+ UpdateStateVariables()\l+ FractureEnergy()\l+ FractureStrength()\l+ IncrementalHeat()\l+ Potential()\l+ Traction()\l+ Stiffness()\l+ TangentType()\l+ Status()\l+ NumOutputVariables()\l+ OutputLabels()\l+ ComputeOutput()\l+ New()\l+ CompatibleOutput()\l# CompatibleOutput()\l}"
template "UML:CLASS"
graphics
[
x 828.0
y 11.0
w 20.0
h 20.0
]
]
node
[
id 236
label "{Tahoe::DiffusionMaterialT\n|# fDiffusionMatSupport\l# fDensity\l# fSpecificHeat\l# fConductivity\l# fq_i\l# fdq_i\l# fdk_ij\l|+ DiffusionMaterialT()\l+ SetDiffusionMatSupport()\l+ k_ij()\l+ q_i()\l+ dq_i_dT()\l+ dk_ij()\l+ Density()\l+ SpecificHeat()\l+ Capacity()\l+ dCapacity_dT()\l+ DefineParameters()\l+ TakeParameterList()\l* fDensity\l* fSpecificHeat\l* fConductivity\l* fq_i\l* fdq_i\l* fdk_ij\l* k_ij()\l* q_i()\l* dq_i_dT()\l* dk_ij()\l* Density()\l* SpecificHeat()\l* Capacity()\l* dCapacity_dT()\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 187.0
y 900.0
w 20.0
h 20.0
]
]
node
[
id 237
label "{Tahoe::VoterChenAl\n||+ VoterChenAl()\l+ LatticeParameter()\l+ Mass()\l- SetPairPotential()\l- SetEmbeddingEnergy()\l- SetElectronDensity()\l}"
template "UML:CLASS"
graphics
[
x 959.0
y 736.0
w 20.0
h 20.0
]
]
node
[
id 238
label "{Tahoe::PenaltyContact3DT\n|# fK\l# fElCoord\l# fElRefCoord\l# fElDisp\l# fdc_du\l# fdn_du\l# fM1\l# fM2\l# fV1\l|+ PenaltyContact3DT()\l+ DefineParameters()\l+ TakeParameterList()\l# LHSDriver()\l# RHSDriver()\l* fElCoord\l* fElRefCoord\l* fElDisp\l* fdc_du\l* fdn_du\l* fM1\l* fM2\l* fV1\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 911.0
y 722.0
w 20.0
h 20.0
]
]
node
[
id 239
label "{Tahoe::InterpolationDataT\n|- fMap\l- fNeighbors\l- fNeighborWeights\l|+ InterpolationDataT()\l+ Free()\l+ GenerateRCV()\l+ ToMatrix()\l+ Neighbors()\l+ Neighbors()\l+ NeighborWeights()\l+ NeighborWeights()\l+ Map()\l+ Map()\l+ Transpose()\l+ Transpose()\l* Neighbors()\l* Neighbors()\l* NeighborWeights()\l* NeighborWeights()\l* Map()\l* Map()\l* Transpose()\l* Transpose()\l}"
template "UML:CLASS"
graphics
[
x 314.0
y 137.0
w 20.0
h 20.0
]
]
node
[
id 240
label "{Tahoe::PenaltyContactDrag2DT\n|# fDrag\l# fGapTolerance\l# fSlipTolerance\l|+ PenaltyContactDrag2DT()\l+ DefineParameters()\l+ TakeParameterList()\l# RHSDriver()\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 766.0
y 68.0
w 20.0
h 20.0
]
]
node
[
id 241
label "{Tahoe::ShapeFunctionT\n|# fCoords\l- fDet\l- fDNaX\l- fDDNaX\l- pNaU\l- pDNaU\l- pDDNaU\l- fGrad_x_temp\l- fv1\l- fv2\l- fStore\l- fCurrElementNumber\l- fDet_store\l- fDNaX_store\l|+ ShapeFunctionT()\l+ ShapeFunctionT()\l+ ShapeFunctionT()\l+ DomainCoordType()\l+ Coordinates()\l+ SetDerivatives()\l+ SetDerivatives_DN_DDN()\l+ IPDets()\l+ IPCoords()\l+ IPCoords()\l+ InterpolateU()\l+ InterpolateU()\l+ InitializeDomain()\l+ CurlU()\l+ CurlU()\l+ Print()\l+ Derivatives_X()\l+ Derivatives_X()\l+ Derivatives_U()\l+ Derivatives_U()\l+ IPShapeX()\l+ IPShapeX()\l+ IPShapeU()\l+ IPShapeU()\l+ GradU()\l+ GradU()\l+ GradU()\l+ TransformDerivatives()\l+ TransformDerivatives()\l+ GradNa()\l+ GradNa()\l+ Grad_GradNa()\l+ Grad_GradNa()\l+ InitStore()\l+ Store()\l+ CloseStore()\l# SetGrad_x()\l# SetUShapeFunctions()\l# DNaX()\l# DDNaX()\l- Construct()\l- Construct_DN_DDN()\l- IPShape()\l* fStore\l* fCurrElementNumber\l* fDet_store\l* fDNaX_store\l* Derivatives_X()\l* Derivatives_X()\l* Derivatives_U()\l* Derivatives_U()\l* IPShapeX()\l* IPShapeX()\l* IPShapeU()\l* IPShapeU()\l* GradU()\l* GradU()\l* GradU()\l* TransformDerivatives()\l* TransformDerivatives()\l* GradNa()\l* GradNa()\l* Grad_GradNa()\l* Grad_GradNa()\l* InitStore()\l* Store()\l* CloseStore()\l}"
template "UML:CLASS"
graphics
[
x 26.0
y 559.0
w 20.0
h 20.0
]
]
node
[
id 242
label "{Tahoe::DetCheckT\n|- fs_jl\l- fc_ijkl\l- fce_ijkl\l- phi2\l- phi4\l- A0\l- A2\l- A4\l- fStructuralMatSupport\l- normal_out\l- fFirstPass\l- fDeBug\l|+ DetCheckT()\l+ IsLocalized()\l+ IsLocalized_SS()\l+ IsLocalized_SS()\l+ SetfStructuralMatSupport()\l- SPINLOC_localize()\l- DetCheck2D_SS()\l- DetCheck3D_SS()\l- FormAcousticTensor2D()\l- RefineNormal2D()\l- IndexConversion2D()\l- FindApproxLocalMins()\l- ChooseNewNormal()\l- ChooseNormalFromNormalSet()\l- DetCheck2D()\l- ComputeCoefficients()\l- det()\l- ddet()\l- dddet()\l}"
template "UML:CLASS"
graphics
[
x 719.0
y 373.0
w 20.0
h 20.0
]
]
node
[
id 243
label "{Tahoe::VIB\n|# fNumSD\l# fPotential\l# fLengths\l# fU\l# fdU\l# fddU\l# fjacobian\l# fNumStress\l# fStressTable\l# fNumModuli\l# fModuliTable\l|+ VIB()\l+ ~VIB()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l# Dimension()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 748.0
y 324.0
w 20.0
h 20.0
]
]
node
[
id 244
label "{Tahoe::eIntegratorT\n||+ eIntegratorT()\l+ ~eIntegratorT()\l+ eIntegrator()\l+ FormM()\l+ FormC()\l+ FormK()\l+ FormMa()\l+ FormCv()\l+ FormKd()\l# eComputeParameters()\l* FormM()\l* FormC()\l* FormK()\l* FormMa()\l* FormCv()\l* FormKd()\l}"
template "UML:CLASS"
graphics
[
x 247.0
y 114.0
w 20.0
h 20.0
]
]
node
[
id 245
label "{Tahoe::eGear6\n|- fconstC\l|+ eGear6()\l+ FormM()\l+ FormC()\l+ FormK()\l+ FormMa()\l+ FormCv()\l+ FormKd()\l# eComputeParameters()\l* FormM()\l* FormC()\l* FormK()\l* FormMa()\l* FormCv()\l* FormKd()\l}"
template "UML:CLASS"
graphics
[
x 580.0
y 760.0
w 20.0
h 20.0
]
]
node
[
id 246
label "{Tahoe::SSLinearVE3D\n|# fMu\l# fKappa\l# fe\l# fModulus\l# fStress\l# fModMat\l# ftauS\l# ftauB\l# falphaS\l# falphaB\l# fbetaS\l# fbetaB\l|+ SSLinearVE3D()\l+ StrainEnergyDensity()\l+ c_ijkl()\l+ s_ij()\l+ C_IJKL()\l+ S_IJ()\l+ NumOutputVariables()\l+ OutputLabels()\l+ ComputeOutput()\l+ DefineParameters()\l+ TakeParameterList()\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 434.0
y 247.0
w 20.0
h 20.0
]
]
node
[
id 247
label "{Tahoe::ParticlePropertyT\n|# fMass\l# fRange\l# fNearestNeighbor\l|+ ParticlePropertyT()\l+ ~ParticlePropertyT()\l+ Range()\l+ Mass()\l+ NearestNeighbor()\l+ DefineParameters()\l+ TakeParameterList()\l# SetMass()\l# SetRange()\l# SetNearestNeighbor()\l* fMass\l* fRange\l* fNearestNeighbor\l* DefineParameters()\l* TakeParameterList()\l* SetMass()\l* SetRange()\l* SetNearestNeighbor()\l}"
template "UML:CLASS"
graphics
[
x 802.0
y 839.0
w 20.0
h 20.0
]
]
node
[
id 248
label "{Tahoe::SolidMaterialT\n|# fThermal\l# fDensity\l# fConstraint\l# fCTE\l|+ Constraint()\l+ IncrementalHeat()\l+ StrainEnergyDensity()\l+ AcousticalTensor()\l+ WaveSpeeds()\l+ Strain()\l+ HasThermalStrain()\l+ ThermalStrainSchedule()\l+ SetThermalSchedule()\l+ ThermalElongation()\l+ Density()\l+ IsLocalized()\l+ IsLocalized()\l+ IsLocalized()\l+ SupportsThermalStrain()\l+ c_ijkl()\l+ ce_ijkl()\l+ s_ij()\l+ Pressure()\l+ C_IJKL()\l+ S_IJ()\l+ HasLocalization()\l+ HasIncrementalHeat()\l+ NeedDisp()\l+ NeedLastDisp()\l+ NeedVel()\l+ HasChangingDensity()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l+ InternalDOF()\l+ InternalStressVars()\l+ InternalStrainVars()\l* ConstraintT\l* int2ConstraintT()\l* SolidMaterialT()\l* ~SolidMaterialT()\l* c_ijkl()\l* ce_ijkl()\l* s_ij()\l* Pressure()\l* C_IJKL()\l* S_IJ()\l* HasLocalization()\l* HasIncrementalHeat()\l* NeedDisp()\l* NeedLastDisp()\l* NeedVel()\l* HasChangingDensity()\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l* InternalDOF()\l* InternalStressVars()\l* InternalStrainVars()\l}"
template "UML:CLASS"
graphics
[
x 407.0
y 917.0
w 20.0
h 20.0
]
]
node
[
id 249
label "{Tahoe::povirk2D\n|# fStress\l# fModulus\l# fStrainEnergyDensity\l# fInternal\l# fTempKirchoff\l# fTempCauchy\l- fDt\l- fFtot_2D\l- fFtot\l- fDtot\l- fGradV_2D\l- fGradV\l- fLocVel\l- fLocDisp\l- fF_temp\l- fSpin\l- fCriticalStrain\l- fEbtot\l- fXxii\l- fCtcon\l- fPP\l- fDmat\l- fEP_tan\l- fEcc\l- fStressMatrix\l- fStressArray\l- fStill3D\l- fStress3D\l- fSmlp\l- fSymStress2D\l- fJ\l- fVisc\l- fTemperature\l- fSb\l- fEb\l- fEbRate\l- Temp_0\l- El_E\l- El_V\l- El_K\l- El_G\l- Sb0\l- Rho0\l- Eb0\l- Eb0tot\l- BigN\l- Smm\l- Alpha_T\l- Delta\l- Theta\l- Kappa\l- Cp\l- Chi\l- Ccc\l- Pcp\l- Epsilon_1\l- Epsilon_2\l- Epsilon_rate\l- Gamma_d\l- Mu_d\l- SigCr\l- Xi\l|+ povirk2D()\l+ NeedsPointInitialization()\l+ PointInitialize()\l+ NeedVel()\l+ UpdateHistory()\l+ ResetHistory()\l+ C_IJKL()\l+ S_IJ()\l+ StrainEnergyDensity()\l+ NumOutputVariables()\l+ OutputLabels()\l+ ComputeOutput()\l+ c_ijkl()\l+ s_ij()\l+ Pressure()\l- ComputeF()\l- ComputeD()\l- ComputeSpin()\l- ComputeEbtotCtconXxii()\l- ComputePP()\l- ComputeEcc()\l- ComputeDmat()\l- ComputeEP_tan()\l- ComputeSmlp()\l- AllocateElement()\l- ComputeViscoTemperature()\l- ComputeEffectiveStress()\l- ComputeViscoEffectiveStrain()\l- CheckIfPlastic()\l- LoadData()\l- Update()\l- Reset()\l- ArrayToMatrix()\l- MatrixToArray()\l- Return3DStress()\l- ArrayToSymMatrix2D()\l* c_ijkl()\l* s_ij()\l* Pressure()\l}"
template "UML:CLASS"
graphics
[
x 871.0
y 890.0
w 20.0
h 20.0
]
]
node
[
id 250
label "{Tahoe::J2SimoC0HardeningT\n|# fInternal\l# fb_bar\l# fbeta_bar\l# fStressCorr\l# fModuliCorr\l# fRelStress\l# f_f_bar\l# fb_bar_trial\l# ftrace_beta_trial\l# fbeta_bar_trial\l# fMatrixTemp1\l# fMatrixTemp2\l# fRed2Temp\l# fRed4Temp1\l# fRed4Temp2\l# fUnitNorm\l# fb_bar_trial_\l# fbeta_bar_trial_\l# kNumInternal\l|+ J2SimoC0HardeningT()\l# TrialElasticState()\l# PlasticLoading()\l# StressCorrection()\l# ModuliCorrection()\l# AllocateElement()\l# Update()\l# Reset()\l- InitIntermediate()\l- LoadData()\l}"
template "UML:CLASS"
graphics
[
x 341.0
y 990.0
w 20.0
h 20.0
]
]
node
[
id 251
label "{Tahoe::ExpCD_DRSolver\n|# fMaxIterations\l# fTolerance\l# fMass_scaling\l# fDamp_scaling\l# fOutputDOF\l# fhist_out\l# fError0\l- fDis\l- fVel\l- fAcc\l- pMass\l- fdt\l- fK_0\l|+ ExpCD_DRSolver()\l+ Initialize()\l+ Solve()\l# ExitIteration()\l# ExitRelaxation()\l# SolveAndForm()\l- SetMass()\l- SetDamping()\l}"
template "UML:CLASS"
graphics
[
x 974.0
y 629.0
w 20.0
h 20.0
]
]
node
[
id 252
label "{Tahoe::TvergHutchRigid2DT\n|- fsigma_max\l- fd_c_n\l- fd_c_t\l- fL_1\l- fL_2\l- fT_2\l- fL_fail\l- fpenalty\l- fK\l|+ TvergHutchRigid2DT()\l+ NumStateVariables()\l+ InitStateVariables()\l+ FractureEnergy()\l+ Potential()\l+ Traction()\l+ Stiffness()\l+ Status()\l+ NumOutputVariables()\l+ OutputLabels()\l+ ComputeOutput()\l# CompatibleOutput()\l* fsigma_max\l* fd_c_n\l* fd_c_t\l* fL_1\l* fL_2\l* fT_2\l* fL_fail\l* fpenalty\l* fK\l}"
template "UML:CLASS"
graphics
[
x 736.0
y 529.0
w 20.0
h 20.0
]
]
node
[
id 253
label "{Tahoe::J2SSC0HardeningT\n|# fPlasticStrain\l# fUnitNorm\l# fBeta\l# fInternal\l# fInternalStressVars\l# fInternalStrainVars\l# fInternalDOF\l- fElasticStrain\l- fStressCorr\l- fModuliCorr\l- fRelStress\l- fDevStrain\l- fTensorTemp\l|+ J2SSC0HardeningT()\l# ElasticStrain()\l# StressCorrection()\l# ModuliCorrection()\l# AllocateElement()\l# Update()\l# Reset()\l# Get_PlasticStrain()\l# Get_Beta()\l# Get_Internal()\l- LoadData()\l- PlasticLoading()\l- RelativeStress()\l}"
template "UML:CLASS"
graphics
[
x 68.0
y 748.0
w 20.0
h 20.0
]
]
node
[
id 254
label "{Tahoe::ContinuumMaterialT\n|# fMaterialSupport\l# fNumDOF\l# fNumSD\l# fNumIP\l|+ ContinuumMaterialT()\l+ ~ContinuumMaterialT()\l+ SetMaterialSupport()\l+ TangentType()\l+ RelaxCode()\l+ MaterialSupport()\l+ ContinuumElement()\l+ NumElementNodes()\l+ NumDOF()\l+ NumSD()\l+ NumIP()\l+ CurrIP()\l+ NumElements()\l+ CurrElementNumber()\l+ ElementCard()\l+ CurrentElement()\l+ InitStep()\l+ CloseStep()\l+ HasHistory()\l+ NeedsPointInitialization()\l+ PointInitialize()\l+ UpdateHistory()\l+ ResetHistory()\l* HasHistory()\l* NeedsPointInitialization()\l* PointInitialize()\l* UpdateHistory()\l* ResetHistory()\l* NumOutputVariables()\l* OutputLabels()\l* ComputeOutput()\l* CompatibleOutput()\l}"
template "UML:CLASS"
graphics
[
x 808.0
y 928.0
w 20.0
h 20.0
]
]
node
[
id 255
label "{Tahoe::eVerlet\n|- fconstC\l|+ eVerlet()\l+ FormM()\l+ FormC()\l+ FormK()\l+ FormMa()\l+ FormCv()\l+ FormKd()\l# eComputeParameters()\l* FormM()\l* FormC()\l* FormK()\l* FormMa()\l* FormCv()\l* FormKd()\l}"
template "UML:CLASS"
graphics
[
x 843.0
y 898.0
w 20.0
h 20.0
]
]
node
[
id 256
label "{Tahoe::PenaltySphereT\n|# fRadius\l# fv_OP\l# fLHS\l# fd_sh\l# fi_sh\l|+ PenaltySphereT()\l+ TangentType()\l+ ApplyLHS()\l+ DefineParameters()\l+ TakeParameterList()\l# ComputeContactForce()\l* fv_OP\l* fLHS\l* fd_sh\l* fi_sh\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 107.0
y 165.0
w 20.0
h 20.0
]
]
node
[
id 257
label "{Tahoe::LennardJonesPairT\n|- f_eps\l- f_sigma\l- f_alpha\l- f_phi_rc\l- f_dphi_rc\l- s_eps\l- s_sigma\l- s_alpha\l- s_phi_rc\l- s_dphi_rc\l|+ LennardJonesPairT()\l+ LennardJonesPairT()\l+ getEnergyFunction()\l+ getForceFunction()\l+ getStiffnessFunction()\l+ DefineParameters()\l+ TakeParameterList()\l- Energy()\l- Force()\l- Stiffness()\l* f_eps\l* f_sigma\l* f_alpha\l* f_phi_rc\l* f_dphi_rc\l* s_eps\l* s_sigma\l* s_alpha\l* s_phi_rc\l* s_dphi_rc\l* getEnergyFunction()\l* getForceFunction()\l* getStiffnessFunction()\l* DefineParameters()\l* TakeParameterList()\l* Energy()\l* Force()\l* Stiffness()\l}"
template "UML:CLASS"
graphics
[
x 250.0
y 531.0
w 20.0
h 20.0
]
]
node
[
id 258
label "{Tahoe::SCNIMFT\n|# fMeshfreeParameters\l# fPointConnectivities\l# fNodes\l# fNodes_inv\l# fNodalCoordinates\l# fCellGeometry\l# fNumIP\l# fCellVolumes\l# fCellCentroids\l# fMaterialList\l# nodalCellSupports\l# bVectorArray\l# bprimeVectorArray\l# Ymatrices\l# fNodalPhi\l# fBoundaryPhi\l# fNodalSupports\l# fBoundarySupports\l# fBodySchedule\l# fBody\l# fNodalShapes\l# fElementConnectivities\l# fEqnos\l# fTractionVectors\l# fTractionBoundaryCondition\l# fBoundaryFacetNormals\l# qIsAxisymmetric\l# circumferential_B\l# fOutputFlags\l# fForce\l# fForce_man\l|+ SCNIMFT()\l+ SCNIMFT()\l+ ~SCNIMFT()\l+ TangentType()\l+ AddNodalForce()\l+ InternalEnergy()\l+ RegisterOutput()\l+ SendOutput()\l+ RelaxSystem()\l+ InterpolantDOFs()\l+ NodalDOFs()\l+ LHSDriver()\l+ RHSDriver()\l+ Equations()\l+ InitStep()\l+ CloseStep()\l+ ResetStep()\l+ WriteRestart()\l+ ReadRestart()\l+ NodesUsed()\l+ GlobalToLocalNumbering()\l+ GlobalToLocalNumbering()\l+ InterpolatedFieldAtNodes()\l+ NodalSupportAndPhi()\l+ SupportSize()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l+ TakeNaturalBC()\l# SetOutputCount()\l# DefineElements()\l# CollectMaterialInfo()\l# NewMaterialList()\l# GenerateOutputLabels()\l# ChangingGeometry()\l# AssembleParticleMass()\l* fForce\l* fForce_man\l* InitStep()\l* CloseStep()\l* ResetStep()\l* WriteRestart()\l* ReadRestart()\l* NodesUsed()\l* GlobalToLocalNumbering()\l* GlobalToLocalNumbering()\l* InterpolatedFieldAtNodes()\l* NodalSupportAndPhi()\l* SupportSize()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l* TakeNaturalBC()\l}"
template "UML:CLASS"
graphics
[
x 660.0
y 988.0
w 20.0
h 20.0
]
]
node
[
id 259
label "{Tahoe::WindowT\n||+ WindowT()\l+ ~WindowT()\l+ SearchType()\l+ NumberOfSupportParameters()\l+ SynchronizeSupportParameters()\l+ Name()\l+ WriteParameters()\l+ Window()\l+ Covers()\l+ SphericalSupportSize()\l+ RectangularSupportSize()\l+ Window()\l+ Covers()\l+ SphericalSupportSize()\l+ RectangularSupportSize()\l* Window()\l* Covers()\l* SphericalSupportSize()\l* RectangularSupportSize()\l* Window()\l* Covers()\l* SphericalSupportSize()\l* RectangularSupportSize()\l}"
template "UML:CLASS"
graphics
[
x 280.0
y 658.0
w 20.0
h 20.0
]
]
node
[
id 260
label "{Tahoe::DiffusionElementT\n|# fCurrMaterial\l# fLocVel\l# fGradient_list\l# fD\l# fB\l# fq\l# NumNodalOutputCodes\l- fDiffusionMatSupport\l|+ DiffusionElementT()\l+ ~DiffusionElementT()\l+ AddNodalForce()\l+ InternalEnergy()\l+ SendOutput()\l# LHSDriver()\l# RHSDriver()\l# B()\l# NextElement()\l# FormStiffness()\l# FormKd()\l# NewMaterialSupport()\l# NewMaterialList()\l# ComputeOutput()\l# CollectMaterialInfo()\l# SetLocalArrays()\l# SetShape()\l# DefineSubs()\l# NewSub()\l# TakeParameterList()\l- SetNodalOutputCodes()\l- SetElementOutputCodes()\l- GenerateOutputLabels()\l* fD\l* fB\l* fq\l* SetLocalArrays()\l* SetShape()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l* SetNodalOutputCodes()\l* SetElementOutputCodes()\l* GenerateOutputLabels()\l}"
template "UML:CLASS"
graphics
[
x 521.0
y 181.0
w 20.0
h 20.0
]
]
node
[
id 261
label "{Tahoe::MeshFreeSupport3DT\n||+ MeshFreeSupport3DT()\l+ MeshFreeSupport3DT()\l+ SetCuttingFacets()\l- ProcessBoundaries()\l- Visible()\l- Intersect()\l- CutCircle()\l- CutEllipse()\l}"
template "UML:CLASS"
graphics
[
x 702.0
y 456.0
w 20.0
h 20.0
]
]
node
[
id 262
label "{Tahoe::ViscTvergHutch2DT\n|- fTimeStep\l- fsigma_max\l- fd_c_n\l- fd_c_t\l- fL_1\l- fL_2\l- fL_fail\l- fbeta\l- feta0\l- fpenalty\l- fK\l|+ ViscTvergHutch2DT()\l+ SetTimeStep()\l+ NumStateVariables()\l+ IncrementalHeat()\l+ FractureEnergy()\l+ Potential()\l+ Traction()\l+ Stiffness()\l+ Status()\l+ NumOutputVariables()\l+ OutputLabels()\l+ ComputeOutput()\l+ DefineParameters()\l+ TakeParameterList()\l# CompatibleOutput()\l* DefineParameters()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 645.0
y 323.0
w 20.0
h 20.0
]
]
node
[
id 263
label "{Tahoe::SolidMatSupportT\n|- fSolidElement\l- fLastDisp\l- fVel\l- fAcc\l- fTemperatures\l- fLastTemperatures\l|+ SolidMatSupportT()\l+ SolidElement()\l+ LocalArray()\l+ Temperatures()\l+ LastTemperatures()\l+ SetContinuumElement()\l+ SetLocalArray()\l+ SetTemperatures()\l+ SetLastTemperatures()\l* fLastDisp\l* fVel\l* fAcc\l* fTemperatures\l* fLastTemperatures\l* SolidElement()\l* LocalArray()\l* Temperatures()\l* LastTemperatures()\l* SetContinuumElement()\l* SetLocalArray()\l* SetTemperatures()\l* SetLastTemperatures()\l}"
template "UML:CLASS"
graphics
[
x 728.0
y 857.0
w 20.0
h 20.0
]
]
node
[
id 264
label "{Tahoe::ParticlePairT\n|- fPairProperties\l- fNeighbors\l- fEqnos\l- fOutputFlags\l- fNearestNeighbors\l- fRefNearestNeighbors\l- fForce_list\l- fForce_list_man\l- fOneOne\l- fopen\l- fout\l- fout2\l- fsummary_file\l- fsummary_file2\l|+ ParticlePairT()\l+ Equations()\l+ WriteOutput()\l+ FormStiffness()\l+ Neighbors()\l+ ConnectsX()\l+ ConnectsU()\l+ DefineSubs()\l+ DefineInlineSub()\l+ NewSub()\l+ TakeParameterList()\l# SetConfiguration()\l# ExtractProperties()\l# SetOutputCount()\l# GenerateOutputLabels()\l# LHSDriver()\l# RHSDriver()\l# RHSDriver1D()\l# RHSDriver2D()\l# RHSDriver3D()\l* fNearestNeighbors\l* fRefNearestNeighbors\l* fForce_list\l* fForce_list_man\l* fOneOne\l* fopen\l* fout\l* fout2\l* fsummary_file\l* fsummary_file2\l* ConnectsX()\l* ConnectsU()\l* DefineSubs()\l* DefineInlineSub()\l* NewSub()\l* TakeParameterList()\l* LHSDriver()\l* RHSDriver()\l* RHSDriver1D()\l* RHSDriver2D()\l* RHSDriver3D()\l}"
template "UML:CLASS"
graphics
[
x 1000.0
y 515.0
w 20.0
h 20.0
]
]
node
[
id 265
label "{Tahoe::DiffusionMatSupportT\n|- fField_list\l- fGradient_list\l- fDiffusion\l|+ DiffusionMatSupportT()\l+ Field()\l+ Field()\l+ SetField()\l+ Gradient()\l+ Gradient()\l+ SetGradient()\l+ Diffusion()\l+ SetContinuumElement()\l* Field()\l* Field()\l* SetField()\l* Gradient()\l* Gradient()\l* SetGradient()\l* Diffusion()\l* SetContinuumElement()\l}"
template "UML:CLASS"
graphics
[
x 461.0
y 867.0
w 20.0
h 20.0
]
]
node
[
id 266
label "{Tahoe::PenaltyRegionT\n|# fTempNumNodes\l# fNodalAreas\l# fGlobal2Local\l# fContactArea\l# fx0\l# fv0\l# fk\l# fmu\l# fSlow\l# fMass\l# fLTf\l# fNumContactNodes\l# fx\l# fv\l# fxlast\l# fvlast\l# fRollerDirection\l# fSecantSearch\l# fContactNodes\l# fContactEqnos\l# fContactForce\l# fGap\l# fContactForce2D\l# fOutputID\l|+ PenaltyRegionT()\l+ ~PenaltyRegionT()\l+ TangentType()\l+ Equations()\l+ InitialCondition()\l+ ReadRestart()\l+ WriteRestart()\l+ ApplyRHS()\l+ InitStep()\l+ CloseStep()\l+ Reset()\l+ RelaxSystem()\l+ RegisterOutput()\l+ WriteOutput()\l+ DefineParameters()\l+ DefineSubs()\l+ NewSub()\l+ TakeParameterList()\l- ComputeContactForce()\l* fx0\l* fv0\l* fk\l* fmu\l* fSlow\l* fMass\l* fLTf\l* fNumContactNodes\l* fx\l* fv\l* fxlast\l* fvlast\l* fRollerDirection\l* fSecantSearch\l* fContactNodes\l* fContactEqnos\l* fContactForce\l* fGap\l* fContactForce2D\l* fOutputID\l* RegisterOutput()\l* WriteOutput()\l* DefineParameters()\l* DefineSubs()\l* NewSub()\l* TakeParameterList()\l}"
template "UML:CLASS"
graphics
[
x 550.0
y 915.0
w 20.0
h 20.0
]
]
edge
[
source 24
target 90
generalization 1
]
edge
[
source 89
target 94
generalization 1
]
edge
[
source 241
target 104
generalization 1
]
edge
[
source 178
target 83
generalization 1
]
edge
[
source 29
target 264
generalization 1
]
edge
[
source 224
target 59
generalization 1
]
edge
[
source 225
target 220
generalization 0
]
edge
[
source 66
target 14
generalization 1
]
edge
[
source 222
target 118
generalization 1
]
edge
[
source 31
target 110
generalization 1
]
edge
[
source 15
target 189
generalization 1
]
edge
[
source 152
target 16
generalization 0
]
edge
[
source 203
target 130
generalization 0
]
edge
[
source 145
target 208
generalization 0
]
edge
[
source 198
target 15
generalization 1
]
edge
[
source 212
target 204
generalization 0
]
edge
[
source 205
target 172
generalization 1
]
edge
[
source 13
target 31
generalization 0
]
edge
[
source 50
target 216
generalization 1
]
edge
[
source 220
target 215
generalization 1
]
edge
[
source 258
target 194
generalization 1
]
edge
[
source 24
target 42
generalization 1
]
edge
[
source 198
target 144
generalization 1
]
edge
[
source 260
target 97
generalization 1
]
edge
[
source 235
target 213
generalization 1
]
edge
[
source 51
target 44
generalization 0
]
edge
[
source 126
target 61
generalization 1
]
edge
[
source 259
target 165
generalization 1
]
edge
[
source 55
target 160
generalization 1
]
edge
[
source 131
target 146
generalization 1
]
edge
[
source 238
target 166
generalization 1
]
edge
[
source 198
target 212
generalization 0
]
edge
[
source 102
target 167
generalization 0
]
edge
[
source 212
target 117
generalization 0
]
edge
[
source 22
target 32
generalization 0
]
edge
[
source 263
target 22
generalization 1
]
edge
[
source 266
target 143
generalization 1
]
edge
[
source 205
target 164
generalization 1
]
edge
[
source 102
target 96
generalization 0
]
edge
[
source 51
target 212
generalization 0
]
edge
[
source 243
target 217
generalization 1
]
edge
[
source 85
target 98
generalization 1
]
edge
[
source 160
target 88
generalization 1
]
edge
[
source 260
target 218
generalization 1
]
edge
[
source 21
target 37
generalization 1
]
edge
[
source 125
target 129
generalization 0
]
edge
[
source 198
target 95
generalization 1
]
edge
[
source 232
target 234
generalization 1
]
edge
[
source 160
target 8
generalization 1
]
edge
[
source 243
target 26
generalization 1
]
edge
[
source 44
target 251
generalization 1
]
edge
[
source 102
target 193
generalization 0
]
edge
[
source 155
target 82
generalization 0
]
edge
[
source 247
target 145
generalization 1
]
edge
[
source 78
target 126
generalization 1
]
edge
[
source 155
target 257
generalization 1
]
edge
[
source 224
target 190
generalization 1
]
edge
[
source 232
target 57
generalization 0
]
edge
[
source 54
target 122
generalization 1
]
edge
[
source 235
target 52
generalization 1
]
edge
[
source 205
target 198
generalization 1
]
edge
[
source 29
target 113
generalization 1
]
edge
[
source 64
target 241
generalization 1
]
edge
[
source 205
target 116
generalization 1
]
edge
[
source 102
target 14
generalization 0
]
edge
[
source 266
target 256
generalization 1
]
edge
[
source 200
target 68
generalization 1
]
edge
[
source 241
target 156
generalization 0
]
edge
[
source 244
target 154
generalization 1
]
edge
[
source 205
target 244
generalization 1
]
edge
[
source 184
target 26
generalization 0
]
edge
[
source 232
target 260
generalization 1
]
edge
[
source 105
target 265
generalization 1
]
edge
[
source 81
target 181
generalization 1
]
edge
[
source 163
target 108
generalization 1
]
edge
[
source 184
target 8
generalization 0
]
edge
[
source 22
target 11
generalization 0
]
edge
[
source 12
target 2
generalization 1
]
edge
[
source 138
target 253
generalization 1
]
edge
[
source 244
target 17
generalization 1
]
edge
[
source 205
target 212
generalization 0
]
edge
[
source 229
target 206
generalization 0
]
edge
[
source 263
target 35
generalization 0
]
edge
[
source 114
target 188
generalization 1
]
edge
[
source 203
target 60
generalization 0
]
edge
[
source 71
target 86
generalization 0
]
edge
[
source 224
target 103
generalization 1
]
edge
[
source 117
target 5
generalization 1
]
edge
[
source 155
target 34
generalization 1
]
edge
[
source 129
target 134
generalization 1
]
edge
[
source 155
target 91
generalization 1
]
edge
[
source 255
target 189
generalization 1
]
edge
[
source 224
target 24
generalization 0
]
edge
[
source 155
target 141
generalization 0
]
edge
[
source 117
target 139
generalization 1
]
edge
[
source 162
target 187
generalization 1
]
edge
[
source 163
target 177
generalization 1
]
edge
[
source 186
target 170
generalization 1
]
edge
[
source 66
target 95
generalization 1
]
edge
[
source 78
target 231
generalization 1
]
edge
[
source 55
target 93
generalization 1
]
edge
[
source 22
target 159
generalization 0
]
edge
[
source 125
target 56
generalization 1
]
edge
[
source 129
target 204
generalization 1
]
edge
[
source 160
target 157
generalization 1
]
edge
[
source 7
target 132
generalization 1
]
edge
[
source 217
target 121
generalization 1
]
edge
[
source 137
target 238
generalization 1
]
edge
[
source 225
target 232
generalization 0
]
edge
[
source 155
target 20
generalization 1
]
edge
[
source 203
target 148
generalization 0
]
edge
[
source 248
target 222
generalization 1
]
edge
[
source 234
target 156
generalization 1
]
edge
[
source 117
target 135
generalization 1
]
edge
[
source 160
target 82
generalization 1
]
edge
[
source 225
target 96
generalization 0
]
edge
[
source 235
target 123
generalization 1
]
edge
[
source 155
target 219
generalization 0
]
edge
[
source 225
target 167
generalization 0
]
edge
[
source 117
target 31
generalization 1
]
edge
[
source 229
target 2
generalization 0
]
edge
[
source 248
target 55
generalization 1
]
edge
[
source 35
target 159
generalization 1
]
edge
[
source 243
target 40
generalization 1
]
edge
[
source 202
target 114
generalization 1
]
edge
[
source 196
target 158
generalization 0
]
edge
[
source 156
target 21
generalization 1
]
edge
[
source 205
target 66
generalization 1
]
edge
[
source 22
target 222
generalization 0
]
edge
[
source 225
target 65
generalization 0
]
edge
[
source 74
target 266
generalization 1
]
edge
[
source 182
target 258
generalization 0
]
edge
[
source 205
target 153
generalization 1
]
edge
[
source 184
target 40
generalization 0
]
edge
[
source 163
target 249
generalization 1
]
edge
[
source 9
target 118
generalization 1
]
edge
[
source 79
target 136
generalization 1
]
edge
[
source 35
target 148
generalization 1
]
edge
[
source 105
target 263
generalization 1
]
edge
[
source 236
target 112
generalization 1
]
edge
[
source 261
target 10
generalization 0
]
edge
[
source 248
target 234
generalization 0
]
edge
[
source 117
target 127
generalization 0
]
edge
[
source 66
target 211
generalization 1
]
edge
[
source 179
target 81
generalization 0
]
edge
[
source 114
target 261
generalization 1
]
edge
[
source 7
target 53
generalization 1
]
edge
[
source 117
target 258
generalization 1
]
edge
[
source 230
target 185
generalization 1
]
edge
[
source 24
target 109
generalization 1
]
edge
[
source 44
target 78
generalization 1
]
edge
[
source 198
target 14
generalization 1
]
edge
[
source 78
target 158
generalization 1
]
edge
[
source 140
target 49
generalization 1
]
edge
[
source 114
target 127
generalization 0
]
edge
[
source 235
target 262
generalization 1
]
edge
[
source 21
target 183
generalization 1
]
edge
[
source 243
target 140
generalization 1
]
edge
[
source 235
target 124
generalization 1
]
edge
[
source 222
target 101
generalization 1
]
edge
[
source 235
target 191
generalization 1
]
edge
[
source 46
target 25
generalization 0
]
edge
[
source 250
target 149
generalization 1
]
edge
[
source 101
target 246
generalization 1
]
edge
[
source 156
target 203
generalization 0
]
edge
[
source 130
target 195
generalization 1
]
edge
[
source 129
target 96
generalization 1
]
edge
[
source 160
target 219
generalization 1
]
edge
[
source 103
target 219
generalization 0
]
edge
[
source 111
target 231
generalization 0
]
edge
[
source 260
target 265
generalization 0
]
edge
[
source 33
target 137
generalization 1
]
edge
[
source 104
target 43
generalization 0
]
edge
[
source 171
target 48
generalization 1
]
edge
[
source 36
target 125
generalization 0
]
edge
[
source 225
target 226
generalization 0
]
edge
[
source 143
target 223
generalization 1
]
edge
[
source 46
target 258
generalization 0
]
edge
[
source 164
target 131
generalization 1
]
edge
[
source 263
target 203
generalization 1
]
edge
[
source 35
target 11
generalization 1
]
edge
[
source 117
target 33
generalization 1
]
edge
[
source 259
target 115
generalization 1
]
edge
[
source 235
target 128
generalization 1
]
edge
[
source 117
target 162
generalization 1
]
edge
[
source 220
target 1
generalization 1
]
edge
[
source 160
target 49
generalization 1
]
edge
[
source 258
target 127
generalization 0
]
edge
[
source 243
target 132
generalization 1
]
edge
[
source 224
target 208
generalization 0
]
edge
[
source 22
target 70
generalization 0
]
edge
[
source 266
target 233
generalization 1
]
edge
[
source 55
target 40
generalization 1
]
edge
[
source 234
target 263
generalization 0
]
edge
[
source 163
target 7
generalization 1
]
edge
[
source 70
target 18
generalization 1
]
edge
[
source 35
target 60
generalization 1
]
edge
[
source 178
target 73
generalization 1
]
edge
[
source 198
target 185
generalization 1
]
edge
[
source 35
target 19
generalization 1
]
edge
[
source 125
target 105
generalization 1
]
edge
[
source 51
target 196
generalization 1
]
edge
[
source 125
target 34
generalization 0
]
edge
[
source 51
target 150
generalization 1
]
edge
[
source 244
target 255
generalization 1
]
edge
[
source 235
target 16
generalization 0
]
edge
[
source 117
target 232
generalization 1
]
edge
[
source 254
target 236
generalization 1
]
edge
[
source 117
target 29
generalization 1
]
edge
[
source 212
target 74
generalization 0
]
edge
[
source 129
target 65
generalization 1
]
edge
[
source 56
target 76
generalization 0
]
edge
[
source 196
target 231
generalization 0
]
edge
[
source 72
target 114
generalization 0
]
edge
[
source 224
target 27
generalization 1
]
edge
[
source 31
target 41
generalization 1
]
edge
[
source 253
target 118
generalization 1
]
edge
[
source 232
target 105
generalization 0
]
edge
[
source 225
target 266
generalization 0
]
edge
[
source 265
target 236
generalization 0
]
edge
[
source 225
target 193
generalization 0
]
edge
[
source 5
target 10
generalization 1
]
edge
[
source 25
target 152
generalization 1
]
edge
[
source 36
target 86
generalization 0
]
edge
[
source 86
target 36
generalization 0
]
edge
[
source 27
target 82
generalization 0
]
edge
[
source 156
target 43
generalization 1
]
edge
[
source 72
target 10
generalization 0
]
edge
[
source 125
target 220
generalization 0
]
edge
[
source 24
target 237
generalization 1
]
edge
[
source 138
target 250
generalization 1
]
edge
[
source 203
target 156
generalization 0
]
edge
[
source 225
target 134
generalization 0
]
edge
[
source 241
target 232
generalization 0
]
edge
[
source 160
target 168
generalization 1
]
edge
[
source 220
target 4
generalization 1
]
edge
[
source 59
target 141
generalization 0
]
edge
[
source 114
target 46
generalization 0
]
edge
[
source 54
target 47
generalization 1
]
edge
[
source 70
target 206
generalization 1
]
edge
[
source 180
target 88
generalization 1
]
edge
[
source 220
target 120
generalization 1
]
edge
[
source 250
target 170
generalization 1
]
edge
[
source 116
target 144
generalization 1
]
edge
[
source 235
target 175
generalization 1
]
edge
[
source 56
target 117
generalization 0
]
edge
[
source 154
target 146
generalization 1
]
edge
[
source 80
target 224
generalization 1
]
edge
[
source 21
target 77
generalization 1
]
edge
[
source 163
target 200
generalization 1
]
edge
[
source 225
target 62
generalization 0
]
edge
[
source 205
target 63
generalization 1
]
edge
[
source 233
target 221
generalization 1
]
edge
[
source 203
target 55
generalization 0
]
edge
[
source 259
target 107
generalization 1
]
edge
[
source 105
target 254
generalization 0
]
edge
[
source 235
target 161
generalization 1
]
edge
[
source 50
target 121
generalization 1
]
edge
[
source 94
target 147
generalization 1
]
edge
[
source 102
target 212
generalization 1
]
edge
[
source 114
target 54
generalization 1
]
edge
[
source 178
target 43
generalization 1
]
edge
[
source 152
target 2
generalization 0
]
edge
[
source 87
target 240
generalization 1
]
edge
[
source 114
target 229
generalization 0
]
edge
[
source 244
target 245
generalization 1
]
edge
[
source 247
target 209
generalization 1
]
edge
[
source 177
target 149
generalization 1
]
edge
[
source 241
target 229
generalization 1
]
edge
[
source 55
target 217
generalization 1
]
edge
[
source 160
target 141
generalization 1
]
edge
[
source 55
target 163
generalization 1
]
edge
[
source 182
target 35
generalization 1
]
edge
[
source 51
target 111
generalization 1
]
edge
[
source 174
target 92
generalization 1
]
edge
[
source 71
target 212
generalization 0
]
edge
[
source 263
target 242
generalization 0
]
edge
[
source 217
target 119
generalization 1
]
edge
[
source 235
target 48
generalization 1
]
edge
[
source 140
target 8
generalization 1
]
edge
[
source 160
target 190
generalization 1
]
edge
[
source 164
target 154
generalization 1
]
edge
[
source 245
target 98
generalization 1
]
edge
[
source 78
target 176
generalization 1
]
edge
[
source 156
target 79
generalization 1
]
edge
[
source 70
target 22
generalization 0
]
edge
[
source 239
target 57
generalization 0
]
edge
[
source 86
target 125
generalization 0
]
edge
[
source 229
target 136
generalization 0
]
edge
[
source 200
target 216
generalization 1
]
edge
[
source 41
target 30
generalization 1
]
edge
[
source 225
target 169
generalization 0
]
edge
[
source 193
target 45
generalization 1
]
edge
[
source 235
target 142
generalization 1
]
edge
[
source 58
target 162
generalization 0
]
edge
[
source 203
target 106
generalization 1
]
edge
[
source 254
target 248
generalization 1
]
edge
[
source 152
target 136
generalization 0
]
edge
[
source 137
target 73
generalization 1
]
edge
[
source 58
target 197
generalization 1
]
edge
[
source 64
target 13
generalization 1
]
edge
[
source 265
target 260
generalization 0
]
edge
[
source 235
target 151
generalization 1
]
edge
[
source 69
target 86
generalization 1
]
edge
[
source 235
target 99
generalization 1
]
edge
[
source 94
target 227
generalization 1
]
edge
[
source 229
target 25
generalization 0
]
edge
[
source 117
target 84
generalization 1
]
edge
[
source 182
target 39
generalization 1
]
edge
[
source 194
target 214
generalization 1
]
edge
[
source 225
target 204
generalization 0
]
edge
[
source 234
target 5
generalization 0
]
edge
[
source 156
target 89
generalization 1
]
edge
[
source 234
target 70
generalization 1
]
edge
[
source 153
target 174
generalization 1
]
edge
[
source 101
target 199
generalization 1
]
edge
[
source 212
target 86
generalization 0
]
edge
[
source 63
target 15
generalization 1
]
edge
[
source 68
target 67
generalization 1
]
edge
[
source 129
target 167
generalization 1
]
edge
[
source 247
target 155
generalization 1
]
edge
[
source 178
target 139
generalization 1
]
edge
[
source 203
target 19
generalization 0
]
edge
[
source 122
target 207
generalization 1
]
edge
[
source 244
target 117
generalization 0
]
edge
[
source 50
target 67
generalization 1
]
edge
[
source 205
target 230
generalization 1
]
edge
[
source 258
target 130
generalization 1
]
edge
[
source 7
target 133
generalization 1
]
edge
[
source 23
target 55
generalization 1
]
edge
[
source 172
target 245
generalization 1
]
edge
[
source 44
target 6
generalization 1
]
edge
[
source 225
target 258
generalization 0
]
edge
[
source 71
target 74
generalization 0
]
edge
[
source 31
target 75
generalization 1
]
edge
[
source 129
target 193
generalization 1
]
edge
[
source 153
target 17
generalization 1
]
edge
[
source 63
target 255
generalization 1
]
edge
[
source 89
target 12
generalization 1
]
edge
[
source 117
target 16
generalization 1
]
edge
[
source 198
target 85
generalization 1
]
edge
[
source 236
target 260
generalization 0
]
edge
[
source 198
target 174
generalization 1
]
edge
[
source 51
target 210
generalization 1
]
edge
[
source 7
target 26
generalization 1
]
edge
[
source 235
target 252
generalization 1
]
edge
[
source 172
target 85
generalization 1
]
edge
[
source 259
target 72
generalization 0
]
edge
[
source 177
target 186
generalization 1
]
edge
[
source 87
target 127
generalization 1
]
edge
[
source 55
target 192
generalization 1
]
edge
[
source 125
target 71
generalization 1
]
edge
[
source 244
target 74
generalization 0
]
edge
[
source 35
target 32
generalization 1
]
edge
[
source 265
target 39
generalization 0
]
edge
[
source 13
target 41
generalization 0
]
edge
[
source 163
target 81
generalization 1
]
edge
[
source 29
target 173
generalization 1
]
edge
[
source 51
target 228
generalization 1
]
edge
[
source 196
target 5
generalization 0
]
edge
[
source 259
target 28
generalization 1
]
edge
[
source 50
target 119
generalization 1
]
edge
[
source 178
target 223
generalization 1
]
edge
[
source 198
target 131
generalization 1
]
edge
[
source 244
target 211
generalization 1
]
edge
[
source 222
target 201
generalization 1
]
edge
[
source 152
target 206
generalization 0
]
edge
[
source 22
target 194
generalization 0
]
edge
[
source 178
target 221
generalization 1
]
edge
[
source 126
target 38
generalization 1
]
edge
[
source 182
target 232
generalization 0
]
edge
[
source 163
target 3
generalization 1
]
edge
[
source 29
target 100
generalization 1
]
]
