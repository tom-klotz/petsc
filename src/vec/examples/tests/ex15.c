/*$Id: ex15.c,v 1.12 2001/01/15 21:45:13 bsmith Exp bsmith $*/

static char help[] = "Tests VecSetValuesBlocked() on Seq vectors\n\n";

#include "petscvec.h"
#include "petscsys.h"

#undef __FUNC__
#define __FUNC__ "main"
int main(int argc,char **argv)
{
  int          n = 9,ierr,size,bs = 3,indices[2],i;
  Scalar       values[6];
  Vec          x;

  PetscInitialize(&argc,&argv,(char*)0,help);
  ierr = MPI_Comm_size(PETSC_COMM_WORLD,&size);CHKERRQ(ierr);

  if (size != 1) SETERRQ(1,"Must be run with one processor");

  /* create vector */
  ierr = VecCreateSeq(PETSC_COMM_SELF,n,&x);CHKERRQ(ierr);
  ierr = VecSetBlockSize(x,bs);CHKERRQ(ierr);

  for (i=0; i<6; i++) values[i] = 4.0*i;
  indices[0] = 0; indices[1] = 2;
  ierr = VecSetValuesBlocked(x,2,indices,values,INSERT_VALUES);CHKERRQ(ierr);

  ierr = VecAssemblyBegin(x);CHKERRQ(ierr);
  ierr = VecAssemblyEnd(x);CHKERRQ(ierr);

  /* 
      Resulting vector should be 0 4 8  0 0 0 12 16 20
  */
  ierr = VecView(x,PETSC_VIEWER_STDOUT_WORLD);CHKERRQ(ierr);

  ierr = VecDestroy(x);CHKERRQ(ierr);

  PetscFinalize();
  return 0;
}
 
