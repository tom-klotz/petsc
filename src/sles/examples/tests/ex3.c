/*$Id: ex3.c,v 1.65 2001/01/16 18:19:49 balay Exp bsmith $*/

static char help[] = 
"This example solves a linear system in parallel with SLES.  The matrix\n\
uses simple bilinear elements on the unit square.  To test the parallel\n\
matrix assembly, the matrix is intentionally laid out across processors\n\
differently from the way it is assembled.  Input arguments are:\n\
  -m <size> : problem size\n\n";

#include "petscsles.h"

#undef __FUNC__
#define __FUNC__ "FormElementStiffness"
int FormElementStiffness(double H,Scalar *Ke)
{
  PetscFunctionBegin;
  Ke[0]  = H/6.0;    Ke[1]  = -.125*H; Ke[2]  = H/12.0;   Ke[3]  = -.125*H;
  Ke[4]  = -.125*H;  Ke[5]  = H/6.0;   Ke[6]  = -.125*H;  Ke[7]  = H/12.0;
  Ke[8]  = H/12.0;   Ke[9]  = -.125*H; Ke[10] = H/6.0;    Ke[11] = -.125*H;
  Ke[12] = -.125*H;  Ke[13] = H/12.0;  Ke[14] = -.125*H;  Ke[15] = H/6.0;
  PetscFunctionReturn(0);
}
#undef __FUNC__
#define __FUNC__ "FormElementRhs"
int FormElementRhs(double x,double y,double H,Scalar *r)
{
  PetscFunctionBegin;
  r[0] = 0.; r[1] = 0.; r[2] = 0.; r[3] = 0.0; 
  PetscFunctionReturn(0);
}

#undef __FUNC__
#define __FUNC__ "main"
int main(int argc,char **args)
{
  Mat         C; 
  int         i,m = 5,rank,size,N,start,end,M,its;
  Scalar      val,zero = 0.0,one = 1.0,none = -1.0,Ke[16],r[4];
  double      x,y,h,norm;
  int         ierr,idx[4],count,*rows;
  Vec         u,ustar,b;
  SLES        sles;
  KSP         ksp;
  IS          is;

  PetscInitialize(&argc,&args,(char *)0,help);
  ierr = PetscOptionsGetInt(PETSC_NULL,"-m",&m,PETSC_NULL);CHKERRQ(ierr);
  N = (m+1)*(m+1); /* dimension of matrix */
  M = m*m; /* number of elements */
  h = 1.0/m;       /* mesh width */
  ierr = MPI_Comm_rank(PETSC_COMM_WORLD,&rank);CHKERRQ(ierr);
  ierr = MPI_Comm_size(PETSC_COMM_WORLD,&size);CHKERRQ(ierr);

  /* Create stiffness matrix */
  ierr = MatCreate(PETSC_COMM_WORLD,PETSC_DECIDE,PETSC_DECIDE,N,N,&C);CHKERRQ(ierr);
  ierr = MatSetFromOptions(C);CHKERRQ(ierr);
  start = rank*(M/size) + ((M%size) < rank ? (M%size) : rank);
  end   = start + M/size + ((M%size) > rank); 

  /* Assemble matrix */
  ierr = FormElementStiffness(h*h,Ke);   /* element stiffness for Laplacian */
  for (i=start; i<end; i++) {
     /* location of lower left corner of element */
     x = h*(i % m); y = h*(i/m); 
     /* node numbers for the four corners of element */
     idx[0] = (m+1)*(i/m) + (i % m);
     idx[1] = idx[0]+1; idx[2] = idx[1] + m + 1; idx[3] = idx[2] - 1;
     ierr = MatSetValues(C,4,idx,4,idx,Ke,ADD_VALUES);CHKERRQ(ierr);
  }
  ierr = MatAssemblyBegin(C,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  ierr = MatAssemblyEnd(C,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);

  /* Create right-hand-side and solution vectors */
  ierr = VecCreate(PETSC_COMM_WORLD,PETSC_DECIDE,N,&u);CHKERRQ(ierr); 
  ierr = VecSetFromOptions(u);CHKERRQ(ierr);
  ierr = PetscObjectSetName((PetscObject)u,"Approx. Solution");CHKERRQ(ierr);
  ierr = VecDuplicate(u,&b);CHKERRQ(ierr);
  ierr = PetscObjectSetName((PetscObject)b,"Right hand side");CHKERRQ(ierr);
  ierr = VecDuplicate(b,&ustar);CHKERRQ(ierr);
  ierr = VecSet(&zero,u);CHKERRQ(ierr);
  ierr = VecSet(&zero,b);CHKERRQ(ierr);

  /* Assemble right-hand-side vector */
  for (i=start; i<end; i++) {
     /* location of lower left corner of element */
     x = h*(i % m); y = h*(i/m); 
     /* node numbers for the four corners of element */
     idx[0] = (m+1)*(i/m) + (i % m);
     idx[1] = idx[0]+1; idx[2] = idx[1] + m + 1; idx[3] = idx[2] - 1;
     ierr = FormElementRhs(x,y,h*h,r);CHKERRQ(ierr);
     ierr = VecSetValues(b,4,idx,r,ADD_VALUES);CHKERRQ(ierr);
  }
  ierr = VecAssemblyBegin(b);CHKERRQ(ierr);
  ierr = VecAssemblyEnd(b);CHKERRQ(ierr);

  /* Modify matrix and right-hand-side for Dirichlet boundary conditions */
  ierr = PetscMalloc(4*m*sizeof(int),&rows);CHKERRQ(ierr);
  for (i=0; i<m+1; i++) {
    rows[i] = i; /* bottom */
    rows[3*m - 1 +i] = m*(m+1) + i; /* top */
  }
  count = m+1; /* left side */
  for (i=m+1; i<m*(m+1); i+= m+1) {
    rows[count++] = i;
  }
  count = 2*m; /* left side */
  for (i=2*m+1; i<m*(m+1); i+= m+1) {
    rows[count++] = i;
  }
  ierr = ISCreateGeneral(PETSC_COMM_SELF,4*m,rows,&is);CHKERRQ(ierr);
  for (i=0; i<4*m; i++) {
     x = h*(rows[i] % (m+1)); y = h*(rows[i]/(m+1)); 
     val = y;
     ierr = VecSetValues(u,1,&rows[i],&val,INSERT_VALUES);CHKERRQ(ierr);
     ierr = VecSetValues(b,1,&rows[i],&val,INSERT_VALUES);CHKERRQ(ierr);
  }    
  ierr = PetscFree(rows);CHKERRQ(ierr);
  ierr = VecAssemblyBegin(u);CHKERRQ(ierr);
  ierr = VecAssemblyEnd(u);CHKERRQ(ierr);
  ierr = VecAssemblyBegin(b);CHKERRQ(ierr); 
  ierr = VecAssemblyEnd(b);CHKERRQ(ierr);

  ierr = MatZeroRows(C,is,&one);CHKERRQ(ierr);
  ierr = ISDestroy(is);CHKERRQ(ierr);


  { Mat A;
  ierr = MatConvert(C,MATSAME,&A);CHKERRQ(ierr);
  ierr = MatDestroy(C);CHKERRQ(ierr);
  ierr = MatConvert(A,MATSAME,&C);CHKERRQ(ierr);
  ierr = MatDestroy(A);CHKERRQ(ierr);
  }

  /* Solve linear system */
  ierr = SLESCreate(PETSC_COMM_WORLD,&sles);CHKERRQ(ierr);
  ierr = SLESSetOperators(sles,C,C,DIFFERENT_NONZERO_PATTERN);CHKERRQ(ierr);
  ierr = SLESSetFromOptions(sles);CHKERRQ(ierr);
  ierr = SLESGetKSP(sles,&ksp);CHKERRQ(ierr);
  ierr = KSPSetInitialGuessNonzero(ksp);CHKERRQ(ierr);
  ierr = SLESSolve(sles,b,u,&its);CHKERRQ(ierr);

  /* Check error */
  ierr = VecGetOwnershipRange(ustar,&start,&end);CHKERRQ(ierr);
  for (i=start; i<end; i++) {
     x = h*(i % (m+1)); y = h*(i/(m+1)); 
     val = y;
     ierr = VecSetValues(ustar,1,&i,&val,INSERT_VALUES);CHKERRQ(ierr);
  }
  ierr = VecAssemblyBegin(ustar);CHKERRQ(ierr);
  ierr = VecAssemblyEnd(ustar);CHKERRQ(ierr);
  ierr = VecAXPY(&none,ustar,u);CHKERRQ(ierr);
  ierr = VecNorm(u,NORM_2,&norm);CHKERRQ(ierr);
  ierr = PetscPrintf(PETSC_COMM_WORLD,"Norm of error %A Iterations %d\n",norm*h,its);CHKERRQ(ierr);

  /* Free work space */
  ierr = SLESDestroy(sles);CHKERRQ(ierr);
  ierr = VecDestroy(ustar);CHKERRQ(ierr);
  ierr = VecDestroy(u);CHKERRQ(ierr);
  ierr = VecDestroy(b);CHKERRQ(ierr);
  ierr = MatDestroy(C);CHKERRQ(ierr);
  PetscFinalize();
  return 0;
}


