/*$Id: ex12.c,v 1.12 2001/01/15 21:45:20 bsmith Exp bsmith $*/

/* Program usage:  mpirun ex1 [-help] [all PETSc options] */

static char help[] = "Demonstrates VecStrideScatter() and VecStrideGather().\n\n";

/*T
   Concepts: vectors^sub-vectors;
   Processors: n
T*/

/* 
  Include "petscvec.h" so that we can use vectors.  Note that this file
  automatically includes:
     petsc.h       - base PETSc routines   petscis.h     - index sets
     petscsys.h    - system routines       petscviewer.h - viewers
*/

#include "petscvec.h"

#undef __FUNC__
#define __FUNC__ "main"
int main(int argc,char **argv)
{
  Vec      v,s;               /* vectors */
  int      n = 20,ierr;
  Scalar   one = 1.0;

  PetscInitialize(&argc,&argv,(char*)0,help);
  ierr = PetscOptionsGetInt(PETSC_NULL,"-n",&n,PETSC_NULL);CHKERRQ(ierr);

  /* 
      Create multi-component vector with 2 components
  */
  ierr = VecCreate(PETSC_COMM_WORLD,PETSC_DECIDE,n,&v);CHKERRQ(ierr);
  ierr = VecSetFromOptions(v);CHKERRQ(ierr);
  ierr = VecSetBlockSize(v,2);CHKERRQ(ierr);

  /* 
      Create single-component vector
  */
  ierr = VecCreate(PETSC_COMM_WORLD,PETSC_DECIDE,n/2,&s);CHKERRQ(ierr);
  ierr = VecSetFromOptions(s);CHKERRQ(ierr);

  /*
     Set the vectors to entries to a constant value.
  */
  ierr = VecSet(&one,v);CHKERRQ(ierr);

  /*
     Get the first component from the multi-component vector to the single vector
  */
  ierr = VecStrideGather(v,0,s,INSERT_VALUES);CHKERRQ(ierr);

  ierr = VecView(s,PETSC_VIEWER_STDOUT_WORLD);CHKERRQ(ierr);

  /*
     Put the values back into the second component 
  */
  ierr = VecStrideScatter(s,1,v,ADD_VALUES);CHKERRQ(ierr);

  ierr = VecView(v,PETSC_VIEWER_STDOUT_WORLD);CHKERRQ(ierr);

  /* 
     Free work space.  All PETSc objects should be destroyed when they
     are no longer needed.
  */
  ierr = VecDestroy(v);CHKERRQ(ierr);
  ierr = VecDestroy(s);CHKERRQ(ierr);
  PetscFinalize();
  return 0;
}
 
