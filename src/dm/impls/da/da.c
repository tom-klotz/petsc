#define PETSCDM_DLL
#include "private/daimpl.h"    /*I   "petscda.h"   I*/


#undef __FUNCT__  
#define __FUNCT__ "DASetDim"
/*@
  DASetDim - Sets the dimension

  Collective on DA

  Input Parameters:
+ da - the DA
- dim - the dimension (or PETSC_DECIDE)

  Level: intermediate

.seealso: DaGetDim(), DASetSizes()
@*/
PetscErrorCode PETSCDM_DLLEXPORT DASetDim(DM da, PetscInt dim)
{
  DM_DA *dd = (DM_DA*)da->data;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(da, DM_CLASSID, 1);
  if (dd->dim > 0 && dim != dd->dim) SETERRQ2(((PetscObject)da)->comm,PETSC_ERR_ARG_WRONGSTATE,"Cannot change DA dim from %D after it was set to %D",dd->dim,dim);
  dd->dim = dim;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "DASetSizes"
/*@
  DASetSizes - Sets the global sizes

  Logically Collective on DA

  Input Parameters:
+ da - the DA
. M - the global X size (or PETSC_DECIDE)
. N - the global Y size (or PETSC_DECIDE)
- P - the global Z size (or PETSC_DECIDE)

  Level: intermediate

.seealso: DAGetSize(), PetscSplitOwnership()
@*/
PetscErrorCode PETSCDM_DLLEXPORT DASetSizes(DM da, PetscInt M, PetscInt N, PetscInt P)
{
  DM_DA *dd = (DM_DA*)da->data;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(da, DM_CLASSID, 1);
  PetscValidLogicalCollectiveInt(da,M,2);
  PetscValidLogicalCollectiveInt(da,N,3);
  PetscValidLogicalCollectiveInt(da,P,4);

  dd->M = M;
  dd->N = N;
  dd->P = P;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "DASetNumProcs"
/*@
  DASetNumProcs - Sets the number of processes in each dimension

  Logically Collective on DA

  Input Parameters:
+ da - the DA
. m - the number of X procs (or PETSC_DECIDE)
. n - the number of Y procs (or PETSC_DECIDE)
- p - the number of Z procs (or PETSC_DECIDE)

  Level: intermediate

.seealso: DASetSizes(), DAGetSize(), PetscSplitOwnership()
@*/
PetscErrorCode PETSCDM_DLLEXPORT DASetNumProcs(DM da, PetscInt m, PetscInt n, PetscInt p)
{
  DM_DA *dd = (DM_DA*)da->data;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(da, DM_CLASSID, 1);
  PetscValidLogicalCollectiveInt(da,m,2);
  PetscValidLogicalCollectiveInt(da,n,3);
  PetscValidLogicalCollectiveInt(da,p,4);
  dd->m = m;
  dd->n = n;
  dd->p = p;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "DASetPeriodicity"
/*@
  DASetPeriodicity - Sets the type of periodicity

  Not collective

  Input Parameter:
+ da    - The DA
- ptype - One of DA_NONPERIODIC, DA_XPERIODIC, DA_YPERIODIC, DA_ZPERIODIC, DA_XYPERIODIC, DA_XZPERIODIC, DA_YZPERIODIC, or DA_XYZPERIODIC

  Level: intermediate

.keywords:  distributed array, periodicity
.seealso: DACreate(), DMDestroy(), DA, DAPeriodicType
@*/
PetscErrorCode PETSCDM_DLLEXPORT DASetPeriodicity(DM da, DAPeriodicType ptype)
{
  DM_DA *dd = (DM_DA*)da->data;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(da,DM_CLASSID,1);
  dd->wrap = ptype;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "DASetDof"
/*@
  DASetDof - Sets the number of degrees of freedom per vertex

  Not collective

  Input Parameter:
+ da  - The DA
- dof - Number of degrees of freedom

  Level: intermediate

.keywords:  distributed array, degrees of freedom
.seealso: DACreate(), DMDestroy(), DA
@*/
PetscErrorCode PETSCDM_DLLEXPORT DASetDof(DM da, int dof)
{
  DM_DA *dd = (DM_DA*)da->data;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(da,DM_CLASSID,1);
  dd->w = dof;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "DASetStencilType"
/*@
  DASetStencilType - Sets the type of the communication stencil

  Logically Collective on DA

  Input Parameter:
+ da    - The DA
- stype - The stencil type, use either DA_STENCIL_BOX or DA_STENCIL_STAR.

  Level: intermediate

.keywords:  distributed array, stencil
.seealso: DACreate(), DMDestroy(), DA
@*/
PetscErrorCode PETSCDM_DLLEXPORT DASetStencilType(DM da, DAStencilType stype)
{
  DM_DA *dd = (DM_DA*)da->data;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(da,DM_CLASSID,1);
  PetscValidLogicalCollectiveEnum(da,stype,2);
  dd->stencil_type = stype;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "DASetStencilWidth"
/*@
  DASetStencilWidth - Sets the width of the communication stencil

  Logically Collective on DA

  Input Parameter:
+ da    - The DA
- width - The stencil width

  Level: intermediate

.keywords:  distributed array, stencil
.seealso: DACreate(), DMDestroy(), DA
@*/
PetscErrorCode PETSCDM_DLLEXPORT DASetStencilWidth(DM da, PetscInt width)
{
  DM_DA *dd = (DM_DA*)da->data;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(da,DM_CLASSID,1);
  PetscValidLogicalCollectiveInt(da,width,2);
  dd->s = width;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "DACheckOwnershipRanges_Private"
static PetscErrorCode DACheckOwnershipRanges_Private(DM da,PetscInt M,PetscInt m,const PetscInt lx[])
{
  PetscInt i,sum;

  PetscFunctionBegin;
  if (M < 0) SETERRQ(((PetscObject)da)->comm,PETSC_ERR_ARG_WRONGSTATE,"Global dimension not set");
  for (i=sum=0; i<m; i++) sum += lx[i];
  if (sum != M) SETERRQ2(((PetscObject)da)->comm,PETSC_ERR_ARG_INCOMP,"Ownership ranges sum to %D but global dimension is %D",sum,M);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "DASetOwnershipRanges"
/*@
  DASetOwnershipRanges - Sets the number of nodes in each direction on each process

  Logically Collective on DA

  Input Parameter:
+ da - The DA
. lx - array containing number of nodes in the X direction on each process, or PETSC_NULL. If non-null, must be of length da->m
. ly - array containing number of nodes in the Y direction on each process, or PETSC_NULL. If non-null, must be of length da->n
- lz - array containing number of nodes in the Z direction on each process, or PETSC_NULL. If non-null, must be of length da->p.

  Level: intermediate

.keywords:  distributed array
.seealso: DACreate(), DMDestroy(), DA
@*/
PetscErrorCode PETSCDM_DLLEXPORT DASetOwnershipRanges(DM da, const PetscInt lx[], const PetscInt ly[], const PetscInt lz[])
{
  PetscErrorCode ierr;
  DM_DA          *dd = (DM_DA*)da->data;
  
  PetscFunctionBegin;
  PetscValidHeaderSpecific(da,DM_CLASSID,1);
  if (lx) {
    if (dd->m < 0) SETERRQ(((PetscObject)da)->comm,PETSC_ERR_ARG_WRONGSTATE,"Cannot set ownership ranges before setting number of procs");
    ierr = DACheckOwnershipRanges_Private(da,dd->M,dd->m,lx);CHKERRQ(ierr);
    if (!dd->lx) {
      ierr = PetscMalloc(dd->m*sizeof(PetscInt), &dd->lx);CHKERRQ(ierr);
    }
    ierr = PetscMemcpy(dd->lx, lx, dd->m*sizeof(PetscInt));CHKERRQ(ierr);
  }
  if (ly) {
    if (dd->n < 0) SETERRQ(((PetscObject)da)->comm,PETSC_ERR_ARG_WRONGSTATE,"Cannot set ownership ranges before setting number of procs");
    ierr = DACheckOwnershipRanges_Private(da,dd->N,dd->n,ly);CHKERRQ(ierr);
    if (!dd->ly) {
      ierr = PetscMalloc(dd->n*sizeof(PetscInt), &dd->ly);CHKERRQ(ierr);
    }
    ierr = PetscMemcpy(dd->ly, ly, dd->n*sizeof(PetscInt));CHKERRQ(ierr);
  }
  if (lz) {
    if (dd->p < 0) SETERRQ(((PetscObject)da)->comm,PETSC_ERR_ARG_WRONGSTATE,"Cannot set ownership ranges before setting number of procs");
    ierr = DACheckOwnershipRanges_Private(da,dd->P,dd->p,lz);CHKERRQ(ierr);
    if (!dd->lz) {
      ierr = PetscMalloc(dd->p*sizeof(PetscInt), &dd->lz);CHKERRQ(ierr);
    }
    ierr = PetscMemcpy(dd->lz, lz, dd->p*sizeof(PetscInt));CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DACreateOwnershipRanges"
/*
 Ensure that da->lx, ly, and lz exist.  Collective on DA.
*/
PetscErrorCode PETSCDM_DLLEXPORT DACreateOwnershipRanges(DM da)
{
  DM_DA          *dd = (DM_DA*)da->data;
  PetscErrorCode ierr;
  PetscInt       n;
  MPI_Comm       comm;
  PetscMPIInt    size;

  PetscFunctionBegin;
  if (!dd->lx) {
    ierr = PetscMalloc(dd->m*sizeof(PetscInt),&dd->lx);CHKERRQ(ierr);
    ierr = DAGetProcessorSubset(da,DA_X,dd->xs,&comm);CHKERRQ(ierr);
    ierr = MPI_Comm_size(comm,&size);CHKERRQ(ierr);
    n = (dd->xe-dd->xs)/dd->w;
    ierr = MPI_Allgather(&n,1,MPIU_INT,dd->lx,1,MPIU_INT,comm);CHKERRQ(ierr);
  }
  if (dd->dim > 1 && !dd->ly) {
    ierr = PetscMalloc(dd->n*sizeof(PetscInt),&dd->ly);CHKERRQ(ierr);
    ierr = DAGetProcessorSubset(da,DA_Y,dd->ys,&comm);CHKERRQ(ierr);
    n = dd->ye-dd->ys;
    ierr = MPI_Allgather(&n,1,MPIU_INT,dd->ly,1,MPIU_INT,comm);CHKERRQ(ierr);
  }
  if (dd->dim > 2 && !dd->lz) {
    ierr = PetscMalloc(dd->p*sizeof(PetscInt),&dd->lz);CHKERRQ(ierr);
    ierr = DAGetProcessorSubset(da,DA_Z,dd->zs,&comm);CHKERRQ(ierr);
    n = dd->ze-dd->zs;
    ierr = MPI_Allgather(&n,1,MPIU_INT,dd->lz,1,MPIU_INT,comm);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "DASetInterpolationType"
/*@
       DASetInterpolationType - Sets the type of interpolation that will be 
          returned by DAGetInterpolation()

   Logically Collective on DA

   Input Parameter:
+  da - initial distributed array
.  ctype - DA_Q1 and DA_Q0 are currently the only supported forms

   Level: intermediate

   Notes: you should call this on the coarser of the two DAs you pass to DAGetInterpolation()

.keywords:  distributed array, interpolation

.seealso: DACreate1d(), DACreate2d(), DACreate3d(), DMDestroy(), DA, DAInterpolationType
@*/
PetscErrorCode PETSCDM_DLLEXPORT DASetInterpolationType(DM da,DAInterpolationType ctype)
{
  DM_DA *dd = (DM_DA*)da->data;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(da,DM_CLASSID,1);
  PetscValidLogicalCollectiveEnum(da,ctype,2);
  dd->interptype = ctype;
  PetscFunctionReturn(0);
}


#undef __FUNCT__  
#define __FUNCT__ "DAGetNeighbors"
/*@C
      DAGetNeighbors - Gets an array containing the MPI rank of all the current
        processes neighbors.

    Not Collective

   Input Parameter:
.     da - the DA object

   Output Parameters:
.     ranks - the neighbors ranks, stored with the x index increasing most rapidly.
              this process itself is in the list

   Notes: In 2d the array is of length 9, in 3d of length 27
          Not supported in 1d
          Do not free the array, it is freed when the DA is destroyed.

   Fortran Notes: In fortran you must pass in an array of the appropriate length.

   Level: intermediate

@*/
PetscErrorCode PETSCDM_DLLEXPORT DAGetNeighbors(DM da,const PetscMPIInt *ranks[])
{
  DM_DA *dd = (DM_DA*)da->data;
  PetscFunctionBegin;
  PetscValidHeaderSpecific(da,DM_CLASSID,1);
  *ranks = dd->neighbors;
  PetscFunctionReturn(0);
}

/*@C
      DASetElementType - Sets the element type to be returned by DMGetElements()

    Not Collective

   Input Parameter:
.     da - the DA object

   Output Parameters:
.     etype - the element type, currently either DA_ELEMENT_P1 or ELEMENT_Q1

   Level: intermediate

.seealso: DAElementType, DAGetElementType(), DMGetElements(), DMRestoreElements()
@*/
#undef __FUNCT__
#define __FUNCT__ "DASetElementType"
PetscErrorCode PETSCDM_DLLEXPORT DASetElementType(DM da, DAElementType etype)
{
  DM_DA          *dd = (DM_DA*)da->data;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(da,DM_CLASSID,1);
  PetscValidLogicalCollectiveEnum(da,etype,2);
  if (dd->elementtype != etype) {
    ierr = PetscFree(dd->e);CHKERRQ(ierr);
    dd->elementtype = etype;
    dd->ne          = 0; 
    dd->e           = PETSC_NULL;
  }
  PetscFunctionReturn(0);
}

/*@C
      DAGetElementType - Gets the element type to be returned by DMGetElements()

    Not Collective

   Input Parameter:
.     da - the DA object

   Output Parameters:
.     etype - the element type, currently either DA_ELEMENT_P1 or ELEMENT_Q1

   Level: intermediate

.seealso: DAElementType, DASetElementType(), DMGetElements(), DMRestoreElements()
@*/
#undef __FUNCT__
#define __FUNCT__ "DAGetElementType"
PetscErrorCode PETSCDM_DLLEXPORT DAGetElementType(DM da, DAElementType *etype)
{
  DM_DA *dd = (DM_DA*)da->data;
  PetscFunctionBegin;
  PetscValidHeaderSpecific(da,DM_CLASSID,1);
  PetscValidPointer(etype,2);
  *etype = dd->elementtype;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "DMGetElements"
/*@C
      DMGetElements - Gets an array containing the indices (in local coordinates) 
                 of all the local elements

    Not Collective

   Input Parameter:
.     dm - the DM object

   Output Parameters:
+     n - number of local elements
-     e - the indices of the elements vertices

   Level: intermediate

.seealso: DMElementType, DMSetElementType(), DMRestoreElements()
@*/
PetscErrorCode PETSCDM_DLLEXPORT DMGetElements(DM dm,PetscInt *n,const PetscInt *e[])
{
  PetscErrorCode ierr;
  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  ierr = (dm->ops->getelements)(dm,n,e);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "DMRestoreElements"
/*@C
      DMRestoreElements - Returns an array containing the indices (in local coordinates) 
                 of all the local elements obtained with DMGetElements()

    Not Collective

   Input Parameter:
+     dm - the DM object
.     n - number of local elements
-     e - the indices of the elements vertices

   Level: intermediate

.seealso: DMElementType, DMSetElementType(), DMGetElements()
@*/
PetscErrorCode PETSCDM_DLLEXPORT DMRestoreElements(DM dm,PetscInt *n,const PetscInt *e[])
{
  PetscErrorCode ierr;
  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  if (dm->ops->restoreelements) {
    ierr = (dm->ops->restoreelements)(dm,n,e);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "DAGetOwnershipRanges"
/*@C
      DAGetOwnershipRanges - Gets the ranges of indices in the x, y and z direction that are owned by each process

    Not Collective

   Input Parameter:
.     da - the DA object

   Output Parameter:
+     lx - ownership along x direction (optional)
.     ly - ownership along y direction (optional)
-     lz - ownership along z direction (optional)

   Level: intermediate

    Note: these correspond to the optional final arguments passed to DACreate(), DACreate2d(), DACreate3d()

    In Fortran one must pass in arrays lx, ly, and lz that are long enough to hold the values; the sixth, seventh and
    eighth arguments from DAGetInfo()

     In C you should not free these arrays, nor change the values in them. They will only have valid values while the
    DA they came from still exists (has not been destroyed).

.seealso: DAGetCorners(), DAGetGhostCorners(), DACreate(), DACreate1d(), DACreate2d(), DACreate3d(), VecGetOwnershipRanges()
@*/
PetscErrorCode PETSCDM_DLLEXPORT DAGetOwnershipRanges(DM da,const PetscInt *lx[],const PetscInt *ly[],const PetscInt *lz[])
{
  DM_DA *dd = (DM_DA*)da->data;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(da,DM_CLASSID,1);
  if (lx) *lx = dd->lx;
  if (ly) *ly = dd->ly;
  if (lz) *lz = dd->lz;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "DASetRefinementFactor"
/*@
     DASetRefinementFactor - Set the ratios that the DA grid is refined

    Logically Collective on DA

  Input Parameters:
+    da - the DA object
.    refine_x - ratio of fine grid to coarse in x direction (2 by default)
.    refine_y - ratio of fine grid to coarse in y direction (2 by default)
-    refine_z - ratio of fine grid to coarse in z direction (2 by default)

  Options Database:
+  -da_refine_x - refinement ratio in x direction
.  -da_refine_y - refinement ratio in y direction
-  -da_refine_z - refinement ratio in z direction

  Level: intermediate

    Notes: Pass PETSC_IGNORE to leave a value unchanged

.seealso: DARefine(), DAGetRefinementFactor()
@*/
PetscErrorCode PETSCDM_DLLEXPORT DASetRefinementFactor(DM da, PetscInt refine_x, PetscInt refine_y,PetscInt refine_z)
{
  DM_DA *dd = (DM_DA*)da->data;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(da,DM_CLASSID,1);
  PetscValidLogicalCollectiveInt(da,refine_x,2);
  PetscValidLogicalCollectiveInt(da,refine_y,3);
  PetscValidLogicalCollectiveInt(da,refine_z,4);

  if (refine_x > 0) dd->refine_x = refine_x;
  if (refine_y > 0) dd->refine_y = refine_y;
  if (refine_z > 0) dd->refine_z = refine_z;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "DAGetRefinementFactor"
/*@C
     DAGetRefinementFactor - Gets the ratios that the DA grid is refined

    Not Collective

  Input Parameter:
.    da - the DA object

  Output Parameters:
+    refine_x - ratio of fine grid to coarse in x direction (2 by default)
.    refine_y - ratio of fine grid to coarse in y direction (2 by default)
-    refine_z - ratio of fine grid to coarse in z direction (2 by default)

  Level: intermediate

    Notes: Pass PETSC_NULL for values you do not need

.seealso: DARefine(), DASetRefinementFactor()
@*/
PetscErrorCode PETSCDM_DLLEXPORT DAGetRefinementFactor(DM da, PetscInt *refine_x, PetscInt *refine_y,PetscInt *refine_z)
{
  DM_DA *dd = (DM_DA*)da->data;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(da,DM_CLASSID,1);
  if (refine_x) *refine_x = dd->refine_x;
  if (refine_y) *refine_y = dd->refine_y;
  if (refine_z) *refine_z = dd->refine_z;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "DASetGetMatrix"
/*@C
     DASetGetMatrix - Sets the routine used by the DA to allocate a matrix.

    Logically Collective on DA

  Input Parameters:
+    da - the DA object
-    f - the function that allocates the matrix for that specific DA

  Level: developer

   Notes: See DASetBlockFills() that provides a simple way to provide the nonzero structure for 
       the diagonal and off-diagonal blocks of the matrix

.seealso: DAGetMatrix(), DASetBlockFills()
@*/
PetscErrorCode PETSCDM_DLLEXPORT DASetGetMatrix(DM da,PetscErrorCode (*f)(DM, const MatType,Mat*))
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(da,DM_CLASSID,1);
  da->ops->getmatrix = f;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DARefineOwnershipRanges"
/*
  Creates "balanced" ownership ranges after refinement, constrained by the need for the
  fine grid boundaries to fall within one stencil width of the coarse partition.

  Uses a greedy algorithm to handle non-ideal layouts, could probably do something better.
*/
static PetscErrorCode DARefineOwnershipRanges(DM da,PetscBool periodic,PetscInt stencil_width,PetscInt ratio,PetscInt m,const PetscInt lc[],PetscInt lf[])
{
  PetscInt i,totalc = 0,remaining,startc = 0,startf = 0;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (ratio < 1) SETERRQ1(((PetscObject)da)->comm,PETSC_ERR_USER,"Requested refinement ratio %D must be at least 1",ratio);
  if (ratio == 1) {
    ierr = PetscMemcpy(lf,lc,m*sizeof(lc[0]));CHKERRQ(ierr);
    PetscFunctionReturn(0);
  }
  for (i=0; i<m; i++) totalc += lc[i];
  remaining = (!periodic) + ratio * (totalc - (!periodic));
  for (i=0; i<m; i++) {
    PetscInt want = remaining/(m-i) + !!(remaining%(m-i));
    if (i == m-1) lf[i] = want;
    else {
      PetscInt diffc = (startf+want)/ratio - (startc + lc[i]);
      while (PetscAbs(diffc) > stencil_width) {
        want += (diffc < 0);
        diffc = (startf+want)/ratio - (startc + lc[i]);
      }
    }
    lf[i] = want;
    startc += lc[i];
    startf += lf[i];
    remaining -= lf[i];
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "DARefine"
/*@
   DARefine - Creates a new distributed array that is a refinement of a given
   distributed array.

   Collective on DA

   Input Parameter:
+  da - initial distributed array
-  comm - communicator to contain refined DA, must be either same as the da communicator or include the 
          da communicator and be 2, 4, or 8 times larger. Currently ignored

   Output Parameter:
.  daref - refined distributed array

   Level: advanced

   Note:
   Currently, refinement consists of just doubling the number of grid spaces
   in each dimension of the DA.

.keywords:  distributed array, refine

.seealso: DACreate1d(), DACreate2d(), DACreate3d(), DMDestroy(), DAGetOwnershipRanges()
@*/
PetscErrorCode PETSCDM_DLLEXPORT DARefine(DM da,MPI_Comm comm,DM *daref)
{
  PetscErrorCode ierr;
  PetscInt       M,N,P;
  DM             da2;
  DM_DA          *dd = (DM_DA*)da->data,*dd2;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(da,DM_CLASSID,1);
  PetscValidPointer(daref,3);

  if (DAXPeriodic(dd->wrap) || dd->interptype == DA_Q0){
    M = dd->refine_x*dd->M;
  } else {
    M = 1 + dd->refine_x*(dd->M - 1);
  }
  if (DAYPeriodic(dd->wrap) || dd->interptype == DA_Q0){
    N = dd->refine_y*dd->N;
  } else {
    N = 1 + dd->refine_y*(dd->N - 1);
  }
  if (DAZPeriodic(dd->wrap) || dd->interptype == DA_Q0){
    P = dd->refine_z*dd->P;
  } else {
    P = 1 + dd->refine_z*(dd->P - 1);
  }
  ierr = DACreateOwnershipRanges(da);CHKERRQ(ierr);
  if (dd->dim == 3) {
    PetscInt *lx,*ly,*lz;
    ierr = PetscMalloc3(dd->m,PetscInt,&lx,dd->n,PetscInt,&ly,dd->p,PetscInt,&lz);CHKERRQ(ierr);
    ierr = DARefineOwnershipRanges(da,(PetscBool)(DAXPeriodic(dd->wrap) || dd->interptype == DA_Q0),dd->s,dd->refine_x,dd->m,dd->lx,lx);CHKERRQ(ierr);
    ierr = DARefineOwnershipRanges(da,(PetscBool)(DAYPeriodic(dd->wrap) || dd->interptype == DA_Q0),dd->s,dd->refine_y,dd->n,dd->ly,ly);CHKERRQ(ierr);
    ierr = DARefineOwnershipRanges(da,(PetscBool)(DAZPeriodic(dd->wrap) || dd->interptype == DA_Q0),dd->s,dd->refine_z,dd->p,dd->lz,lz);CHKERRQ(ierr);
    ierr = DACreate3d(((PetscObject)da)->comm,dd->wrap,dd->stencil_type,M,N,P,dd->m,dd->n,dd->p,dd->w,dd->s,lx,ly,lz,&da2);CHKERRQ(ierr);
    ierr = PetscFree3(lx,ly,lz);CHKERRQ(ierr);
  } else if (dd->dim == 2) {
    PetscInt *lx,*ly;
    ierr = PetscMalloc2(dd->m,PetscInt,&lx,dd->n,PetscInt,&ly);CHKERRQ(ierr);
    ierr = DARefineOwnershipRanges(da,(PetscBool)(DAXPeriodic(dd->wrap) || dd->interptype == DA_Q0),dd->s,dd->refine_x,dd->m,dd->lx,lx);CHKERRQ(ierr);
    ierr = DARefineOwnershipRanges(da,(PetscBool)(DAYPeriodic(dd->wrap) || dd->interptype == DA_Q0),dd->s,dd->refine_y,dd->n,dd->ly,ly);CHKERRQ(ierr);
    ierr = DACreate2d(((PetscObject)da)->comm,dd->wrap,dd->stencil_type,M,N,dd->m,dd->n,dd->w,dd->s,lx,ly,&da2);CHKERRQ(ierr);
    ierr = PetscFree2(lx,ly);CHKERRQ(ierr);
  } else if (dd->dim == 1) {
    PetscInt *lx;
    ierr = PetscMalloc(dd->m*sizeof(PetscInt),&lx);CHKERRQ(ierr);
    ierr = DARefineOwnershipRanges(da,(PetscBool)(DAXPeriodic(dd->wrap) || dd->interptype == DA_Q0),dd->s,dd->refine_x,dd->m,dd->lx,lx);CHKERRQ(ierr);
    ierr = DACreate1d(((PetscObject)da)->comm,dd->wrap,M,dd->w,dd->s,lx,&da2);CHKERRQ(ierr);
    ierr = PetscFree(lx);CHKERRQ(ierr);
  }
  dd2 = (DM_DA*)da2->data;

  /* allow overloaded (user replaced) operations to be inherited by refinement clones */
  da2->ops->getmatrix        = da->ops->getmatrix;
  da2->ops->getinterpolation = da->ops->getinterpolation;
  da2->ops->getcoloring      = da->ops->getcoloring;
  dd2->interptype            = dd->interptype;
  
  /* copy fill information if given */
  if (dd->dfill) {
    ierr = PetscMalloc((dd->dfill[dd->w]+dd->w+1)*sizeof(PetscInt),&dd2->dfill);CHKERRQ(ierr);
    ierr = PetscMemcpy(dd2->dfill,dd->dfill,(dd->dfill[dd->w]+dd->w+1)*sizeof(PetscInt));CHKERRQ(ierr);
  }
  if (dd->ofill) {
    ierr = PetscMalloc((dd->ofill[dd->w]+dd->w+1)*sizeof(PetscInt),&dd2->ofill);CHKERRQ(ierr);
    ierr = PetscMemcpy(dd2->ofill,dd->ofill,(dd->ofill[dd->w]+dd->w+1)*sizeof(PetscInt));CHKERRQ(ierr);
  }
  /* copy the refine information */
  dd2->refine_x = dd->refine_x;
  dd2->refine_y = dd->refine_y;
  dd2->refine_z = dd->refine_z;

  /* copy vector type information */
  ierr = PetscFree(da2->vectype);CHKERRQ(ierr);
  ierr = PetscStrallocpy(da->vectype,&da2->vectype);CHKERRQ(ierr);
  *daref = da2;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "DACoarsen"
/*@
   DACoarsen - Creates a new distributed array that is a coarsenment of a given
   distributed array.

   Collective on DA

   Input Parameter:
+  da - initial distributed array
-  comm - communicator to contain coarsend DA. Currently ignored

   Output Parameter:
.  daref - coarsend distributed array

   Level: advanced

   Note:
   Currently, coarsenment consists of just dividing the number of grid spaces
   in each dimension of the DA by refinex_x, refinex_y, ....

.keywords:  distributed array, coarsen

.seealso: DACreate1d(), DACreate2d(), DACreate3d(), DMDestroy(), DAGetOwnershipRanges()
@*/
PetscErrorCode PETSCDM_DLLEXPORT DACoarsen(DM da, MPI_Comm comm,DM *daref)
{
  PetscErrorCode ierr;
  PetscInt       M,N,P;
  DM             da2;
  DM_DA          *dd = (DM_DA*)da->data,*dd2;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(da,DM_CLASSID,1);
  PetscValidPointer(daref,3);

  if (DAXPeriodic(dd->wrap) || dd->interptype == DA_Q0){
    if(dd->refine_x)
      M = dd->M / dd->refine_x;
    else
      M = dd->M;
  } else {
    if(dd->refine_x)
      M = 1 + (dd->M - 1) / dd->refine_x;
    else
      M = dd->M;
  }
  if (DAYPeriodic(dd->wrap) || dd->interptype == DA_Q0){
    if(dd->refine_y)
      N = dd->N / dd->refine_y;
    else
      N = dd->N;
  } else {
    if(dd->refine_y)
      N = 1 + (dd->N - 1) / dd->refine_y;
    else
      N = dd->M;
  }
  if (DAZPeriodic(dd->wrap) || dd->interptype == DA_Q0){
    if(dd->refine_z)
      P = dd->P / dd->refine_z;
    else
      P = dd->P;
  } else {
    if(dd->refine_z)
      P = 1 + (dd->P - 1) / dd->refine_z;
    else
      P = dd->P;
  }
  if (dd->dim == 3) {
    ierr = DACreate3d(((PetscObject)da)->comm,dd->wrap,dd->stencil_type,M,N,P,dd->m,dd->n,dd->p,dd->w,dd->s,0,0,0,&da2);CHKERRQ(ierr);
  } else if (dd->dim == 2) {
    ierr = DACreate2d(((PetscObject)da)->comm,dd->wrap,dd->stencil_type,M,N,dd->m,dd->n,dd->w,dd->s,0,0,&da2);CHKERRQ(ierr);
  } else if (dd->dim == 1) {
    ierr = DACreate1d(((PetscObject)da)->comm,dd->wrap,M,dd->w,dd->s,0,&da2);CHKERRQ(ierr);
  }
  dd2 = (DM_DA*)da2->data;

  /* allow overloaded (user replaced) operations to be inherited by refinement clones */
  da2->ops->getmatrix        = da->ops->getmatrix;
  da2->ops->getinterpolation = da->ops->getinterpolation;
  da2->ops->getcoloring      = da->ops->getcoloring;
  dd2->interptype            = dd->interptype;
  
  /* copy fill information if given */
  if (dd->dfill) {
    ierr = PetscMalloc((dd->dfill[dd->w]+dd->w+1)*sizeof(PetscInt),&dd2->dfill);CHKERRQ(ierr);
    ierr = PetscMemcpy(dd2->dfill,dd->dfill,(dd->dfill[dd->w]+dd->w+1)*sizeof(PetscInt));CHKERRQ(ierr);
  }
  if (dd->ofill) {
    ierr = PetscMalloc((dd->ofill[dd->w]+dd->w+1)*sizeof(PetscInt),&dd2->ofill);CHKERRQ(ierr);
    ierr = PetscMemcpy(dd2->ofill,dd->ofill,(dd->ofill[dd->w]+dd->w+1)*sizeof(PetscInt));CHKERRQ(ierr);
  }
  /* copy the refine information */
  dd2->refine_x = dd->refine_x;
  dd2->refine_y = dd->refine_y;
  dd2->refine_z = dd->refine_z;

  /* copy vector type information */
  ierr = PetscFree(da2->vectype);CHKERRQ(ierr);
  ierr = PetscStrallocpy(da->vectype,&da2->vectype);CHKERRQ(ierr);

  *daref = da2;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "DARefineHierarchy"
/*@
   DARefineHierarchy - Perform multiple levels of refinement.

   Collective on DA

   Input Parameter:
+  da - initial distributed array
-  nlevels - number of levels of refinement to perform

   Output Parameter:
.  daf - array of refined DAs

   Options Database:
+  -da_refine_hierarchy_x - list of refinement ratios in x direction
.  -da_refine_hierarchy_y - list of refinement ratios in y direction
-  -da_refine_hierarchy_z - list of refinement ratios in z direction

   Level: advanced

.keywords: distributed array, refine

.seealso: DARefine(), DACoarsenHierarchy()
@*/
PetscErrorCode PETSCDM_DLLEXPORT DARefineHierarchy(DM da,PetscInt nlevels,DM daf[])
{
  PetscErrorCode ierr;
  PetscInt       i,n,*refx,*refy,*refz;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(da,DM_CLASSID,1);
  if (nlevels < 0) SETERRQ(((PetscObject)da)->comm,PETSC_ERR_ARG_OUTOFRANGE,"nlevels cannot be negative");
  if (nlevels == 0) PetscFunctionReturn(0);
  PetscValidPointer(daf,3);

  /* Get refinement factors, defaults taken from the coarse DA */
  ierr = PetscMalloc3(nlevels,PetscInt,&refx,nlevels,PetscInt,&refy,nlevels,PetscInt,&refz);CHKERRQ(ierr);
  for (i=0; i<nlevels; i++) {
    ierr = DAGetRefinementFactor(da,&refx[i],&refy[i],&refz[i]);CHKERRQ(ierr);
  }
  n = nlevels;
  ierr = PetscOptionsGetIntArray(((PetscObject)da)->prefix,"-da_refine_hierarchy_x",refx,&n,PETSC_NULL);CHKERRQ(ierr);
  n = nlevels;
  ierr = PetscOptionsGetIntArray(((PetscObject)da)->prefix,"-da_refine_hierarchy_y",refy,&n,PETSC_NULL);CHKERRQ(ierr);
  n = nlevels;
  ierr = PetscOptionsGetIntArray(((PetscObject)da)->prefix,"-da_refine_hierarchy_z",refz,&n,PETSC_NULL);CHKERRQ(ierr);

  ierr = DASetRefinementFactor(da,refx[0],refy[0],refz[0]);CHKERRQ(ierr);
  ierr = DARefine(da,((PetscObject)da)->comm,&daf[0]);CHKERRQ(ierr);
  for (i=1; i<nlevels; i++) {
    ierr = DASetRefinementFactor(daf[i-1],refx[i],refy[i],refz[i]);CHKERRQ(ierr);
    ierr = DARefine(daf[i-1],((PetscObject)da)->comm,&daf[i]);CHKERRQ(ierr);
  }
  ierr = PetscFree3(refx,refy,refz);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "DACoarsenHierarchy"
/*@
   DACoarsenHierarchy - Perform multiple levels of coarsening

   Collective on DA

   Input Parameter:
+  da - initial distributed array
-  nlevels - number of levels of coarsening to perform

   Output Parameter:
.  dac - array of coarsened DAs

   Level: advanced

.keywords: distributed array, coarsen

.seealso: DACoarsen(), DARefineHierarchy()
@*/
PetscErrorCode PETSCDM_DLLEXPORT DACoarsenHierarchy(DM da,PetscInt nlevels,DM dac[])
{
  PetscErrorCode ierr;
  PetscInt i;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(da,DM_CLASSID,1);
  if (nlevels < 0) SETERRQ(((PetscObject)da)->comm,PETSC_ERR_ARG_OUTOFRANGE,"nlevels cannot be negative");
  if (nlevels == 0) PetscFunctionReturn(0);
  PetscValidPointer(dac,3);
  ierr = DACoarsen(da,((PetscObject)da)->comm,&dac[0]);CHKERRQ(ierr);
  for (i=1; i<nlevels; i++) {
    ierr = DACoarsen(dac[i-1],((PetscObject)da)->comm,&dac[i]);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}