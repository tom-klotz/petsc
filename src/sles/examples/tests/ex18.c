/*$Id: ex18.c,v 1.20 2001/01/15 21:47:31 bsmith Exp bsmith $*/

#if !defined(PETSC_USE_COMPLEX)

static char help[] = 
"Reads a PETSc matrix and vector from a file and solves a linear system.\n\
Input arguments are:\n\
  -f <input_file> : file to load.  For a 5X5 example of the 5-pt. stencil,\n\
                    use the file petsc/src/mat/examples/matbinary.ex\n\n";

#include "petscmat.h"
#include "petscsles.h"

#undef __FUNC__
#define __FUNC__ "main"
int main(int argc,char **args)
{
  int        ierr,its,m,n,mvec;
  PetscLogDouble time1,time2,time;
  double     norm;
  Scalar     zero = 0.0,none = -1.0;
  Vec        x,b,u;
  Mat        A;
  SLES       sles;
  char       file[128]; 
  PetscViewer     fd;

  PetscInitialize(&argc,&args,(char *)0,help);

  /* Read matrix and RHS */
  ierr = PetscOptionsGetString(PETSC_NULL,"-f",file,127,PETSC_NULL);CHKERRQ(ierr);
  ierr = PetscViewerBinaryOpen(PETSC_COMM_WORLD,file,PETSC_BINARY_RDONLY,&fd);CHKERRQ(ierr);
  ierr = MatLoad(fd,MATSEQAIJ,&A);CHKERRQ(ierr);
  ierr = VecLoad(fd,&b);CHKERRQ(ierr);
  ierr = PetscViewerDestroy(fd);CHKERRQ(ierr);

  /* 
     If the load matrix is larger then the vector, due to being padded 
     to match the blocksize then create a new padded vector
  */
  ierr = MatGetSize(A,&m,&n);CHKERRQ(ierr);
  ierr = VecGetSize(b,&mvec);CHKERRQ(ierr);
  if (m > mvec) {
    Vec    tmp;
    Scalar *bold,*bnew;
    /* create a new vector b by padding the old one */
    ierr = VecCreate(PETSC_COMM_WORLD,PETSC_DECIDE,m,&tmp);CHKERRQ(ierr);
    ierr = VecSetFromOptions(tmp);CHKERRQ(ierr);
    ierr = VecGetArray(tmp,&bnew);CHKERRQ(ierr);
    ierr = VecGetArray(b,&bold);CHKERRQ(ierr);
    ierr = PetscMemcpy(bnew,bold,mvec*sizeof(Scalar));CHKERRQ(ierr);
    ierr = VecDestroy(b);CHKERRQ(ierr);
    b = tmp;
  }

  /* Set up solution */
  ierr = VecDuplicate(b,&x);CHKERRQ(ierr);
  ierr = VecDuplicate(b,&u);CHKERRQ(ierr);
  ierr = VecSet(&zero,x);CHKERRQ(ierr);

  /* Solve system */
  PetscLogStagePush(1);
  ierr = SLESCreate(PETSC_COMM_WORLD,&sles);CHKERRQ(ierr);
  ierr = SLESSetOperators(sles,A,A,DIFFERENT_NONZERO_PATTERN);CHKERRQ(ierr);
  ierr = SLESSetFromOptions(sles);CHKERRQ(ierr);
  ierr = PetscGetTime(&time1);CHKERRQ(ierr);
  ierr = SLESSolve(sles,b,x,&its);CHKERRQ(ierr);
  ierr = PetscGetTime(&time2);CHKERRQ(ierr);
  time = time2 - time1;
  PetscLogStagePop();

  /* Show result */
  ierr = MatMult(A,x,u);
  ierr = VecAXPY(&none,b,u);CHKERRQ(ierr);
  ierr = VecNorm(u,NORM_2,&norm);CHKERRQ(ierr);
  ierr = PetscPrintf(PETSC_COMM_WORLD,"Number of iterations = %3d\n",its);CHKERRQ(ierr);
  ierr = PetscPrintf(PETSC_COMM_WORLD,"Residual norm %A\n",norm);CHKERRQ(ierr);
  ierr = PetscPrintf(PETSC_COMM_WORLD,"Time for solve = %5.2f seconds\n",time);CHKERRQ(ierr);

  /* Cleanup */
  ierr = SLESDestroy(sles);CHKERRQ(ierr);
  ierr = VecDestroy(x);CHKERRQ(ierr);
  ierr = VecDestroy(b);CHKERRQ(ierr);
  ierr = VecDestroy(u);CHKERRQ(ierr);
  ierr = MatDestroy(A);CHKERRQ(ierr);

  PetscFinalize();
  return 0;
}

#else
#include <stdio.h>
int main(int argc,char **args)
{
  fprintf(stdout,"This example does not work for complex numbers.\n");
  return 0;
}
#endif
