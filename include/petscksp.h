/* $Id: ksp.h,v 1.62 1998/01/27 23:47:29 curfman Exp bsmith $ */
/*
   Defines the interface functions for the Krylov subspace accelerators.
*/
#ifndef __KSP_PACKAGE
#define __KSP_PACKAGE
#include "petsc.h"
#include "vec.h"
#include "mat.h"
#include "pc.h"

#define KSP_COOKIE  PETSC_COOKIE+8

typedef struct _p_KSP*     KSP;

#define KSPRICHARDSON "richardson"
#define KSPCHEBYCHEV  "chebychev"
#define KSPCG         "cg"
#define KSPGMRES      "gmres"
#define KSPTCQMR      "tcqmr"
#define KSPBCGS       "bcgs"
#define KSPCGS        "cgs"
#define KSPTFQMR      "tfqmr"
#define KSPCR         "cr"
#define KSPLSQR       "lsqr"
#define KSPPREONLY    "preonly"
#define KSPQCG        "qcg"
#define KSPTRLS       "trls"
typedef char * KSPType;

extern int KSPCreate(MPI_Comm,KSP *);
extern int KSPSetType(KSP,KSPType);
extern int KSPSetUp(KSP);
extern int KSPSolve(KSP,int *);
extern int KSPDestroy(KSP);

extern DLList KSPList;
extern int KSPRegisterAll(char *);
extern int KSPRegisterDestroy();


extern int KSPGetType(KSP, KSPType *);
extern int KSPSetPreconditionerSide(KSP,PCSide);
extern int KSPGetPreconditionerSide(KSP,PCSide*);
extern int KSPGetTolerances(KSP,double*,double*,double*,int*);
extern int KSPSetTolerances(KSP,double,double,double,int);
extern int KSPSetComputeResidual(KSP,PetscTruth);
extern int KSPSetUsePreconditionedResidual(KSP);
extern int KSPSetInitialGuessNonzero(KSP);
extern int KSPSetComputeEigenvalues(KSP);
extern int KSPSetComputeSingularValues(KSP);
extern int KSPSetRhs(KSP,Vec);
extern int KSPGetRhs(KSP,Vec *);
extern int KSPSetSolution(KSP,Vec);
extern int KSPGetSolution(KSP,Vec *);
extern int KSPGetResidualNorm(KSP,double*);

extern int KSPSetPC(KSP,PC);
extern int KSPGetPC(KSP,PC*);

extern int KSPSetMonitor(KSP,int (*)(KSP,int,double, void*), void *);
extern int KSPGetMonitorContext(KSP,void **);
extern int KSPSetResidualHistory(KSP, double *,int);

extern int KSPSetConvergenceTest(KSP,int (*)(KSP,int,double, void*), void *);
extern int KSPGetConvergenceContext(KSP,void **);

extern int KSPBuildSolution(KSP, Vec,Vec *);
extern int KSPBuildResidual(KSP, Vec, Vec,Vec *);

extern int KSPRichardsonSetScale(KSP , double);
extern int KSPChebychevSetEigenvalues(KSP , double, double);
extern int KSPComputeExtremeSingularValues(KSP, double*,double*);
extern int KSPComputeEigenvalues(KSP,int,double*,double*);
extern int KSPComputeEigenvaluesExplicitly(KSP,int,double*,double*);

extern int KSPGMRESSetRestart(KSP, int);
extern int KSPGMRESSetPreAllocateVectors(KSP);
extern int KSPGMRESSetOrthogonalization(KSP,int (*)(KSP,int));
extern int KSPGMRESUnmodifiedGramSchmidtOrthogonalization(KSP,int);
extern int KSPGMRESModifiedGramSchmidtOrthogonalization(KSP,int);
extern int KSPGMRESIROrthogonalization(KSP,int);
extern int KSPGMRESDGKSOrthogonalization(KSP,int);

extern int KSPSetFromOptions(KSP);
extern int KSPAddOptionsChecker(int (*)(KSP));

extern int KSPSingularValueMonitor(KSP,int,double, void * );
extern int KSPDefaultMonitor(KSP,int,double, void *);
extern int KSPTrueMonitor(KSP,int,double, void *);
extern int KSPDefaultSMonitor(KSP,int,double, void *);

extern int KSPDefaultConverged(KSP,int,double, void *);

extern int KSPResidual(KSP,Vec,Vec,Vec,Vec,Vec,Vec);
extern int KSPUnwindPreconditioner(KSP,Vec,Vec);
extern int KSPDefaultBuildSolution(KSP,Vec,Vec*);
extern int KSPDefaultBuildResidual(KSP,Vec,Vec,Vec *);

extern int KSPPrintHelp(KSP);

extern int KSPSetOptionsPrefix(KSP,char*);
extern int KSPAppendOptionsPrefix(KSP,char*);
extern int KSPGetOptionsPrefix(KSP,char**);

extern int KSPView(KSP,Viewer);

extern int KSPComputeExplicitOperator(KSP,Mat *);

typedef enum {KSP_CG_SYMMETRIC=1, KSP_CG_HERMITIAN=2} KSPCGType;
extern int KSPCGSetType(KSP,KSPCGType);

extern int PCPreSolve(PC,KSP);
extern int PCPostSolve(PC,KSP);

extern int KSPLGMonitorCreate(char*,char*,int,int,int,int,DrawLG*);
extern int KSPLGMonitor(KSP,int,double,void*);
extern int KSPLGMonitorDestroy(DrawLG);
extern int KSPLGTrueMonitorCreate(MPI_Comm,char*,char*,int,int,int,int,DrawLG*);
extern int KSPLGTrueMonitor(KSP,int,double,void*);
extern int KSPLGTrueMonitorDestroy(DrawLG);

#endif


