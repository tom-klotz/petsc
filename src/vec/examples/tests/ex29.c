/*$Id: ex29.c,v 1.13 2001/01/16 18:17:02 balay Exp bsmith $*/

static char help[] = "Tests VecSetValues and VecSetValuesBlocked() on MPI vectors\n\
where atleast a couple of mallocs will occur in the stash code.\n\n";

#include "petscvec.h"
#include "petscsys.h"

#undef __FUNC__
#define __FUNC__ "main"
int main(int argc,char **argv)
{
  int          i,j,n = 50,ierr,bs,size;
  Scalar       val,*vals,zero=0.0;
  Vec          x;

  PetscInitialize(&argc,&argv,(char*)0,help);
  ierr = MPI_Comm_size(PETSC_COMM_WORLD,&size);CHKERRQ(ierr);
  bs = size;

  ierr = PetscOptionsGetInt(PETSC_NULL,"-n",&n,PETSC_NULL);CHKERRQ(ierr);
  ierr = VecCreateMPI(PETSC_COMM_WORLD,PETSC_DECIDE,n*bs,&x);CHKERRQ(ierr);
  ierr = VecSetBlockSize(x,bs);CHKERRQ(ierr);

  for (i=0; i<n*bs; i++) {
    val  = i*1.0;
    ierr = VecSetValues(x,1,&i,&val,INSERT_VALUES);CHKERRQ(ierr);
  }
  ierr = VecAssemblyBegin(x);CHKERRQ(ierr);
  ierr = VecAssemblyEnd(x);CHKERRQ(ierr);

  ierr = VecView(x,PETSC_VIEWER_STDOUT_WORLD);CHKERRQ(ierr);

  /* Now do the blocksetvalues */
  ierr = VecSet(&zero,x);CHKERRQ(ierr);
  ierr = PetscMalloc(bs*sizeof(Scalar),&vals);CHKERRQ(ierr);
  for (i=0; i<n; i++) {
    for (j=0; j<bs; j++) {
      vals[j] = (i*bs+j)*1.0;
    }
    ierr = VecSetValuesBlocked(x,1,&i,vals,INSERT_VALUES);CHKERRQ(ierr);
  }

  ierr = VecAssemblyBegin(x);CHKERRQ(ierr);
  ierr = VecAssemblyEnd(x);CHKERRQ(ierr);

  ierr = VecView(x,PETSC_VIEWER_STDOUT_WORLD);CHKERRQ(ierr);

  ierr = VecDestroy(x);CHKERRQ(ierr);
  ierr = PetscFree(vals);CHKERRQ(ierr);
  PetscFinalize();
  return 0;
}
 
