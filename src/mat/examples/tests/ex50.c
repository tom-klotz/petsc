/*$Id: ex50.c,v 1.22 2001/01/15 21:46:09 bsmith Exp bsmith $*/

static char help[] = "Reads in a matrix and vector in ASCII format and writes\n\
them using the PETSc sparse format. Input parameters are:\n\
  -fin <filename> : input file\n\
  -fout <filename> : output file\n\n";

#include "petscmat.h"

#undef __FUNC__
#define __FUNC__ "main"
int main(int argc,char **args)
{
  Mat        A;
  Vec        b;
  char       filein[256],finname[256],fileout[256];
  int        n,ierr,col,row;
  int        rowin;
  PetscTruth flg;
  Scalar     val,*array;
  FILE*      file;
  PetscViewer     view;

  PetscInitialize(&argc,&args,(char *)0,help);

  /* Read in matrix and RHS */
  ierr = PetscOptionsGetString(PETSC_NULL,"-fin",filein,255,&flg);CHKERRQ(ierr);
  if (!flg) SETERRQ(1,"Must indicate file for reading");
  ierr = PetscOptionsGetString(PETSC_NULL,"-fout",fileout,255,&flg);CHKERRQ(ierr);
  if (!flg) SETERRQ(1,"Must indicate file for writing");

  ierr = PetscFixFilename(filein,finname);CHKERRQ(ierr);
  if (!(file = fopen(finname,"r"))) {
    SETERRQ(1,"cannot open input file\n");
  }
  fscanf(file,"%d\n",&n);

  ierr = MatCreate(PETSC_COMM_WORLD,PETSC_DECIDE,PETSC_DECIDE,n,n,&A);CHKERRQ(ierr);
  ierr = MatSetFromOptions(A);CHKERRQ(ierr);
  ierr = VecCreate(PETSC_COMM_WORLD,PETSC_DECIDE,n,&b);CHKERRQ(ierr);
  ierr = VecSetFromOptions(b);CHKERRQ(ierr);

  for (row=0; row<n; row++) {
    fscanf(file,"row %d:",&rowin);
    if (rowin != row) SETERRQ(1,"Bad file");
    while (fscanf(file," %d %le",&col,&val)) {
      ierr = MatSetValues(A,1,&row,1,&col,&val,INSERT_VALUES);CHKERRQ(ierr);
    }  
  }
  ierr = MatAssemblyBegin(A,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  ierr = MatAssemblyEnd(A,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  ierr = VecGetArray(b,&array);CHKERRQ(ierr);
  for (row=0; row<n; row++) {
    fscanf(file," ii= %d %le",&col,array+row);
  }
  ierr = VecRestoreArray(b,&array);CHKERRQ(ierr);

  fclose(file);

  ierr = PetscPrintf(PETSC_COMM_SELF,"Reading matrix complete.\n");CHKERRQ(ierr);
  ierr = PetscViewerBinaryOpen(PETSC_COMM_WORLD,fileout,PETSC_BINARY_CREATE,&view);CHKERRQ(ierr);
  ierr = MatView(A,view);CHKERRQ(ierr);
  ierr = VecView(b,view);CHKERRQ(ierr);
  ierr = PetscViewerDestroy(view);CHKERRQ(ierr);

  ierr = VecDestroy(b);CHKERRQ(ierr);
  ierr = MatDestroy(A);CHKERRQ(ierr);

  PetscFinalize();
  return 0;
}

