#define PETSCDM_DLL

/*
  Code for manipulating distributed regular arrays in parallel.
*/

#include "private/daimpl.h"    /*I   "petscda.h"   I*/

/* Logging support */
PetscClassId PETSCDM_DLLEXPORT DM_CLASSID;
PetscClassId PETSCDM_DLLEXPORT ADDA_CLASSID;
PetscLogEvent  DA_GlobalToLocal, DA_LocalToGlobal, DA_LocalADFunction;

#undef __FUNCT__  
#define __FUNCT__ "DMDestroy_Private"
/*
   DMDestroy_Private - handles the work vectors created by DMGetGlobalVector() and DMGetLocalVector()

*/
PetscErrorCode PETSCDM_DLLEXPORT DMDestroy_Private(DM dm,PetscBool  *done)
{
  PetscErrorCode ierr;
  PetscErrorCode i,cnt = 0;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  *done = PETSC_FALSE;

  for (i=0; i<DM_MAX_WORK_VECTORS; i++) {
    if (dm->localin[i])  {cnt++;}
    if (dm->globalin[i]) {cnt++;}
  }

  if (--((PetscObject)dm)->refct - cnt > 0) PetscFunctionReturn(0);

  /*
         Need this test because the dm references the vectors that 
     reference the dm, so destroying the dm calls destroy on the 
     vectors that cause another destroy on the dm
  */
  if (((PetscObject)dm)->refct < 0) PetscFunctionReturn(0);
  ((PetscObject)dm)->refct = 0;

  for (i=0; i<DM_MAX_WORK_VECTORS; i++) {
    if (dm->localout[i]) SETERRQ(PETSC_COMM_SELF,PETSC_ERR_ARG_WRONGSTATE,"Destroying a DM that has a local vector obtained with DMGetLocalVector()");
    if (dm->localin[i]) {ierr = VecDestroy(dm->localin[i]);CHKERRQ(ierr);}
    if (dm->globalout[i]) SETERRQ(PETSC_COMM_SELF,PETSC_ERR_ARG_WRONGSTATE,"Destroying a DM that has a global vector obtained with DMGetGlobalVector()");
    if (dm->globalin[i]) {ierr = VecDestroy(dm->globalin[i]);CHKERRQ(ierr);}
  }
  *done = PETSC_TRUE;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "DMDestroy_DA"
PetscErrorCode PETSCDM_DLLEXPORT DMDestroy_DA(DM da)
{
  PetscErrorCode ierr;
  PetscErrorCode i;
  PetscBool      done;
  DM_DA          *dd = (DM_DA*)da->data;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(da,DM_CLASSID,1);

  ierr = DMDestroy_Private((DM)da,&done);CHKERRQ(ierr);
  if (!done) PetscFunctionReturn(0);

  /* destroy the external/common part */
  for (i=0; i<DA_MAX_AD_ARRAYS; i++) {
    ierr = PetscFree(dd->adstartghostedout[i]);CHKERRQ(ierr);
    ierr = PetscFree(dd->adstartghostedin[i]);CHKERRQ(ierr);
    ierr = PetscFree(dd->adstartout[i]);CHKERRQ(ierr);
    ierr = PetscFree(dd->adstartin[i]);CHKERRQ(ierr);
  }
  for (i=0; i<DA_MAX_AD_ARRAYS; i++) {
    ierr = PetscFree(dd->admfstartghostedout[i]);CHKERRQ(ierr);
    ierr = PetscFree(dd->admfstartghostedin[i]);CHKERRQ(ierr);
    ierr = PetscFree(dd->admfstartout[i]);CHKERRQ(ierr);
    ierr = PetscFree(dd->admfstartin[i]);CHKERRQ(ierr);
  }
  for (i=0; i<DA_MAX_WORK_ARRAYS; i++) {
    ierr = PetscFree(dd->startghostedout[i]);CHKERRQ(ierr);
    ierr = PetscFree(dd->startghostedin[i]);CHKERRQ(ierr);
    ierr = PetscFree(dd->startout[i]);CHKERRQ(ierr);
    ierr = PetscFree(dd->startin[i]);CHKERRQ(ierr);
  }

  /* if memory was published with AMS then destroy it */
  ierr = PetscObjectDepublish(da);CHKERRQ(ierr);

  if (dd->ltog)   {ierr = VecScatterDestroy(dd->ltog);CHKERRQ(ierr);}
  if (dd->gtol)   {ierr = VecScatterDestroy(dd->gtol);CHKERRQ(ierr);}
  if (dd->ltol)   {ierr = VecScatterDestroy(dd->ltol);CHKERRQ(ierr);}
  if (dd->natural){
    ierr = VecDestroy(dd->natural);CHKERRQ(ierr);
  }
  if (dd->gton) {
    ierr = VecScatterDestroy(dd->gton);CHKERRQ(ierr);
  }

  if (dd->ao) {
    ierr = AODestroy(dd->ao);CHKERRQ(ierr);
  }
  if (dd->ltogmap) {
    ierr = ISLocalToGlobalMappingDestroy(dd->ltogmap);CHKERRQ(ierr);
  }
  if (dd->ltogmapb) {
    ierr = ISLocalToGlobalMappingDestroy(dd->ltogmapb);CHKERRQ(ierr);
  }

  ierr = PetscFree(dd->lx);CHKERRQ(ierr);
  ierr = PetscFree(dd->ly);CHKERRQ(ierr);
  ierr = PetscFree(dd->lz);CHKERRQ(ierr);
  ierr = PetscFree(da->vectype);CHKERRQ(ierr);

  if (dd->fieldname) {
    for (i=0; i<dd->w; i++) {
      ierr = PetscFree(dd->fieldname[i]);CHKERRQ(ierr);
    }
    ierr = PetscFree(dd->fieldname);CHKERRQ(ierr);
  }

  if (dd->localcoloring) {
    ierr = ISColoringDestroy(dd->localcoloring);CHKERRQ(ierr);
  }
  if (dd->ghostedcoloring) {
    ierr = ISColoringDestroy(dd->ghostedcoloring);CHKERRQ(ierr);
  }

  if (dd->coordinates) {ierr = VecDestroy(dd->coordinates);CHKERRQ(ierr);}
  if (dd->ghosted_coordinates) {ierr = VecDestroy(dd->ghosted_coordinates);CHKERRQ(ierr);}
  if (dd->da_coordinates && da != dd->da_coordinates) {ierr = DMDestroy(dd->da_coordinates);CHKERRQ(ierr);}

  ierr = PetscFree(dd->neighbors);CHKERRQ(ierr);
  ierr = PetscFree(dd->dfill);CHKERRQ(ierr);
  ierr = PetscFree(dd->ofill);CHKERRQ(ierr);
  ierr = PetscFree(dd->e);CHKERRQ(ierr);

  ierr = PetscHeaderDestroy(da);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "DAGetISLocalToGlobalMapping"
/*@
   DAGetISLocalToGlobalMapping - Accesses the local-to-global mapping in a DA.

   Not Collective

   Input Parameter:
.  da - the distributed array that provides the mapping 

   Output Parameter:
.  ltog - the mapping

   Level: intermediate

   Notes:
   This mapping can them be used by VecSetLocalToGlobalMapping() or 
   MatSetLocalToGlobalMapping().

   Essentially the same data is returned in the form of an integer array
   with the routine DAGetGlobalIndices().

.keywords: distributed array, destroy

.seealso: DACreate1d(), DACreate2d(), DACreate3d(), VecSetLocalToGlobalMapping(),
          MatSetLocalToGlobalMapping(), DAGetGlobalIndices(), DAGetISLocalToGlobalMappingBlck()
@*/
PetscErrorCode PETSCDM_DLLEXPORT DAGetISLocalToGlobalMapping(DM da,ISLocalToGlobalMapping *map)
{
  DM_DA *dd = (DM_DA*)da->data;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(da,DM_CLASSID,1);
  PetscValidPointer(map,2);
  *map = dd->ltogmap;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "DAGetISLocalToGlobalMappingBlck"
/*@
   DAGetISLocalToGlobalMappingBlck - Accesses the local-to-global mapping in a DA.

   Not Collective

   Input Parameter:
.  da - the distributed array that provides the mapping 

   Output Parameter:
.  ltog - the mapping

   Level: intermediate

   Notes:
   This mapping can them be used by VecSetLocalToGlobalMappingBlock() or 
   MatSetLocalToGlobalMappingBlock().

   Essentially the same data is returned in the form of an integer array
   with the routine DAGetGlobalIndices().

.keywords: distributed array, destroy

.seealso: DACreate1d(), DACreate2d(), DACreate3d(), VecSetLocalToGlobalMapping(),
          MatSetLocalToGlobalMapping(), DAGetGlobalIndices(), DAGetISLocalToGlobalMapping()
@*/
PetscErrorCode PETSCDM_DLLEXPORT DAGetISLocalToGlobalMappingBlck(DM da,ISLocalToGlobalMapping *map)
{
  DM_DA *dd = (DM_DA*)da->data;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(da,DM_CLASSID,1);
  PetscValidPointer(map,2);
  *map = dd->ltogmapb;
  PetscFunctionReturn(0);
}

