/*$Id: ex58.c,v 1.11 2001/01/15 21:46:09 bsmith Exp bsmith $*/

static char help[] = "Tests MatTranspose() and MatEqual() for MPIAIJ matrices.\n\n";

#include "petscmat.h"


#undef __FUNC__
#define __FUNC__ "main"
int main(int argc,char **argv)
{
  Mat        A,B;
  int        m = 7,n,i,ierr,rstart,rend,cols[3];
  Scalar     v[3];
  PetscTruth equal;
  char       *eq[2];

  PetscInitialize(&argc,&argv,(char*)0,help);
  ierr = PetscViewerSetFormat(PETSC_VIEWER_STDOUT_WORLD,PETSC_VIEWER_FORMAT_ASCII_COMMON,0);CHKERRQ(ierr);
  ierr = PetscOptionsGetInt(PETSC_NULL,"-m",&m,PETSC_NULL);CHKERRQ(ierr);
  n = m;

  /* ------- Assemble matrix, test MatValid() --------- */

  ierr = MatCreateMPIAIJ(PETSC_COMM_WORLD,PETSC_DECIDE,PETSC_DECIDE,m,n,0,0,0,0,&A);CHKERRQ(ierr);
  ierr = MatGetOwnershipRange(A,&rstart,&rend);CHKERRQ(ierr);
  if (rstart == 0) {
    cols[0] = 0;
    cols[1] = 1;
    v[0]    = 2.0; v[1] = -1.0;
    ierr = MatSetValues(A,1,&rstart,2,cols,v,INSERT_VALUES);CHKERRQ(ierr);
    rstart++;
  }
  if (rend == m) {
    rend--;
    cols[0] = rend-1;
    cols[1] = rend;
    v[0]    = -1.0; v[1] = 2.0;
    ierr = MatSetValues(A,1,&rend,2,cols,v,INSERT_VALUES);CHKERRQ(ierr);
  }
  v[0] = -1.0; v[1] = 2.0; v[2] = -1.0;
  for (i=rstart; i<rend; i++) { 
    cols[0] = i-1;
    cols[1] = i;
    cols[2] = i+1;
    ierr = MatSetValues(A,1,&i,3,cols,v,INSERT_VALUES);CHKERRQ(ierr);
  }
  ierr = MatAssemblyBegin(A,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  ierr = MatAssemblyEnd(A,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);

  ierr = MatTranspose(A,&B);CHKERRQ(ierr);

  ierr = MatEqual(A,B,&equal);

  eq[0] = "not equal";
  eq[1] = "equal";
  ierr = PetscPrintf(PETSC_COMM_WORLD,"Matrices are %s\n",eq[equal]);CHKERRQ(ierr);

  /* Free data structures */  
  ierr = MatDestroy(A);CHKERRQ(ierr);
  ierr = MatDestroy(B);CHKERRQ(ierr);


  PetscFinalize();
  return 0;
}
 
