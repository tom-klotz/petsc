#include <../src/snes/impls/richardson/snesrichardsonimpl.h>


#undef __FUNCT__
#define __FUNCT__ "SNESReset_NRichardson"
PetscErrorCode SNESReset_NRichardson(SNES snes)
{
  SNES_NRichardson        *neP = (SNES_NRichardson *)snes->data;
  PetscErrorCode          ierr;

  PetscFunctionBegin;
  ierr = LineSearchDestroy(&neP->linesearch);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

/*
  SNESDestroy_NRichardson - Destroys the private SNES_NRichardson context that was created with SNESCreate_NRichardson().

  Input Parameter:
. snes - the SNES context

  Application Interface Routine: SNESDestroy()
*/
#undef __FUNCT__
#define __FUNCT__ "SNESDestroy_NRichardson"
PetscErrorCode SNESDestroy_NRichardson(SNES snes)
{
  PetscErrorCode   ierr;

  PetscFunctionBegin;
  ierr = SNESReset_NRichardson(snes);CHKERRQ(ierr);
  ierr = PetscFree(snes->data);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

/*
   SNESSetUp_NRichardson - Sets up the internal data structures for the later use
   of the SNESNRICHARDSON nonlinear solver.

   Input Parameters:
+  snes - the SNES context
-  x - the solution vector

   Application Interface Routine: SNESSetUp()
 */
#undef __FUNCT__
#define __FUNCT__ "SNESSetUp_NRichardson"
PetscErrorCode SNESSetUp_NRichardson(SNES snes)
{
  const char              *optionsprefix;
  SNES_NRichardson        *neP = (SNES_NRichardson *)snes->data;
  PetscErrorCode          ierr;

  PetscFunctionBegin;

  ierr = SNESGetOptionsPrefix(snes, &optionsprefix);CHKERRQ(ierr);
  ierr = LineSearchCreate(((PetscObject)snes)->comm, &neP->linesearch);CHKERRQ(ierr);
  ierr = LineSearchSetSNES(neP->linesearch, snes);CHKERRQ(ierr);
  ierr = LineSearchSetType(neP->linesearch, LINESEARCHL2);CHKERRQ(ierr);
  ierr = LineSearchAppendOptionsPrefix(neP->linesearch, optionsprefix);CHKERRQ(ierr);
  ierr = LineSearchSetFromOptions(neP->linesearch);CHKERRQ(ierr);

  PetscFunctionReturn(0);
}

/*
  SNESSetFromOptions_NRichardson - Sets various parameters for the SNESLS method.

  Input Parameter:
. snes - the SNES context

  Application Interface Routine: SNESSetFromOptions()
*/
#undef __FUNCT__
#define __FUNCT__ "SNESSetFromOptions_NRichardson"
static PetscErrorCode SNESSetFromOptions_NRichardson(SNES snes)
{
  PetscErrorCode ierr;
  PetscFunctionBegin;
    ierr = PetscOptionsHead("SNES Richardson options");CHKERRQ(ierr);
    ierr = PetscOptionsTail();CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

/*
  SNESView_NRichardson - Prints info from the SNESRichardson data structure.

  Input Parameters:
+ SNES - the SNES context
- viewer - visualization context

  Application Interface Routine: SNESView()
*/
#undef __FUNCT__
#define __FUNCT__ "SNESView_NRichardson"
static PetscErrorCode SNESView_NRichardson(SNES snes, PetscViewer viewer)
{
  PetscBool        iascii;
  PetscErrorCode   ierr;
  SNES_NRichardson *neP = (SNES_NRichardson *)snes->data;

  PetscFunctionBegin;
  ierr = PetscTypeCompare((PetscObject) viewer, PETSCVIEWERASCII, &iascii);CHKERRQ(ierr);
  if (iascii) {
    ierr = PetscViewerASCIIPrintf(viewer,"  Line search variant: %s\n", ((PetscObject)neP->linesearch)->type_name);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

/*
  SNESSolve_NRichardson - Solves a nonlinear system with the Richardson method.

  Input Parameters:
. snes - the SNES context

  Output Parameter:
. outits - number of iterations until termination

  Application Interface Routine: SNESSolve()
*/
#undef __FUNCT__
#define __FUNCT__ "SNESSolve_NRichardson"
PetscErrorCode SNESSolve_NRichardson(SNES snes)
{
  SNES_NRichardson    *neP = (SNES_NRichardson *)snes->data;
  Vec                 X, Y, F;
  PetscReal           xnorm, fnorm, ynorm;
  PetscInt            maxits, i;
  PetscErrorCode      ierr;
  PetscBool           lsSuccess;
  SNESConvergedReason reason;

  PetscFunctionBegin;
  snes->reason = SNES_CONVERGED_ITERATING;

  maxits = snes->max_its;        /* maximum number of iterations */
  X      = snes->vec_sol;        /* X^n */
  Y      = snes->vec_sol_update; /* \tilde X */
  F      = snes->vec_func;       /* residual vector */

  ierr = PetscObjectTakeAccess(snes);CHKERRQ(ierr);
  snes->iter = 0;
  snes->norm = 0.;
  ierr = PetscObjectGrantAccess(snes);CHKERRQ(ierr);
  ierr = SNESComputeFunction(snes,X,F);CHKERRQ(ierr);
  if (snes->domainerror) {
    snes->reason = SNES_DIVERGED_FUNCTION_DOMAIN;
    PetscFunctionReturn(0);
  }
  ierr = VecNorm(F, NORM_2, &fnorm);CHKERRQ(ierr); /* fnorm <- ||F||  */
  if (PetscIsInfOrNanReal(fnorm)) SETERRQ(PETSC_COMM_SELF,PETSC_ERR_FP,"Infinite or not-a-number generated in norm");
  ierr = PetscObjectTakeAccess(snes);CHKERRQ(ierr);
  snes->norm = fnorm;
  ierr = PetscObjectGrantAccess(snes);CHKERRQ(ierr);
  SNESLogConvHistory(snes,fnorm,0);
  ierr = SNESMonitor(snes,0,fnorm);CHKERRQ(ierr);

  /* set parameter for default relative tolerance convergence test */
  snes->ttol = fnorm*snes->rtol;
  /* test convergence */
  ierr = (*snes->ops->converged)(snes,0,0.0,0.0,fnorm,&snes->reason,snes->cnvP);CHKERRQ(ierr);
  if (snes->reason) PetscFunctionReturn(0);

  for(i = 0; i < maxits; i++) {
    lsSuccess = PETSC_TRUE;
    /* Call general purpose update function */
    if (snes->ops->update) {
      ierr = (*snes->ops->update)(snes, snes->iter);CHKERRQ(ierr);
    }
    else if (snes->pc) {
      ierr = VecCopy(X,Y);CHKERRQ(ierr);
      ierr = SNESSolve(snes->pc, snes->vec_rhs, Y);CHKERRQ(ierr);
      ierr = SNESGetConvergedReason(snes->pc,&reason);CHKERRQ(ierr);
      if (reason < 0  && reason != SNES_DIVERGED_MAX_IT) {
        snes->reason = SNES_DIVERGED_INNER;
        PetscFunctionReturn(0);
      }
      ierr = VecAYPX(Y,-1.0,X);CHKERRQ(ierr);
    } else {
      ierr = VecCopy(F,Y);CHKERRQ(ierr);
    }
    ierr = LineSearchApply(neP->linesearch, X, F, &fnorm, Y);CHKERRQ(ierr);
    ierr = LineSearchGetNorms(neP->linesearch, &xnorm, &fnorm, &ynorm);CHKERRQ(ierr);
    ierr = LineSearchGetSuccess(neP->linesearch, &lsSuccess);CHKERRQ(ierr);
    if (!lsSuccess) {
      if (++snes->numFailures >= snes->maxFailures) {
        snes->reason = SNES_DIVERGED_LINE_SEARCH;
        break;
      }
    }
    if (snes->nfuncs >= snes->max_funcs) {
      snes->reason = SNES_DIVERGED_FUNCTION_COUNT;
      break;
    }
    if (snes->domainerror) {
      snes->reason = SNES_DIVERGED_FUNCTION_DOMAIN;
      PetscFunctionReturn(0);
    }
    /* Monitor convergence */
    ierr = PetscObjectTakeAccess(snes);CHKERRQ(ierr);
    snes->iter = i+1;
    snes->norm = fnorm;
    ierr = PetscObjectGrantAccess(snes);CHKERRQ(ierr);
    SNESLogConvHistory(snes,snes->norm,0);
    ierr = SNESMonitor(snes,snes->iter,snes->norm);CHKERRQ(ierr);
    /* Test for convergence */
    ierr = (*snes->ops->converged)(snes,snes->iter,xnorm,ynorm,fnorm,&snes->reason,snes->cnvP);CHKERRQ(ierr);
    if (snes->reason) break;
  }
  if (i == maxits) {
    ierr = PetscInfo1(snes, "Maximum number of iterations has been reached: %D\n", maxits);CHKERRQ(ierr);
    if (!snes->reason) snes->reason = SNES_DIVERGED_MAX_IT;
  }
  PetscFunctionReturn(0);
}

/*MC
  SNESNRICHARDSON - Richardson nonlinear solver that uses successive substitutions, also sometimes known as Picard iteration.

  Level: beginner

  Options Database:
.   -snes_ls <basic,basicnormnorms,quadratic,critical> Line search type.

  Notes: If no inner nonlinear preconditioner is provided then solves F(x) - b = 0 using x^{n+1} = x^{n} - lambda
            (F(x^n) - b) where lambda is obtained either SNESLineSearchSetDamping(), -snes_damping or a line search.  If
            an inner nonlinear preconditioner is provided (either with -npc_snes_type or SNESSetPC()) then the inner
            solver is called an initial solution x^n and the nonlinear Richardson uses x^{n+1} = x^{n} + lambda d^{n}
            where d^{n} = \hat{x}^{n} - x^{n} where \hat{x}^{n} is the solution returned from the inner solver.

     This uses no derivative information thus will be much slower then Newton's method obtained with -snes_type ls

.seealso:  SNESCreate(), SNES, SNESSetType(), SNESLS, SNESTR, SNESNGMRES, SNESQN, SNESNCG
M*/
EXTERN_C_BEGIN
#undef __FUNCT__
#define __FUNCT__ "SNESCreate_NRichardson"
PetscErrorCode  SNESCreate_NRichardson(SNES snes)
{
  PetscErrorCode          ierr;
  SNES_NRichardson        *neP;
  PetscFunctionBegin;
  snes->ops->destroy         = SNESDestroy_NRichardson;
  snes->ops->setup           = SNESSetUp_NRichardson;
  snes->ops->setfromoptions  = SNESSetFromOptions_NRichardson;
  snes->ops->view            = SNESView_NRichardson;
  snes->ops->solve           = SNESSolve_NRichardson;
  snes->ops->reset           = SNESReset_NRichardson;

  snes->usesksp              = PETSC_FALSE;
  snes->usespc               = PETSC_TRUE;

  ierr = PetscNewLog(snes, SNES_NRichardson, &neP);CHKERRQ(ierr);
  snes->data = (void*) neP;

  snes->max_funcs = 30000;
  snes->max_its   = 10000;

  PetscFunctionReturn(0);
}
EXTERN_C_END