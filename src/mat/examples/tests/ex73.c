/*$Id: ex73.c,v 1.7 2001/01/16 18:18:12 balay Exp bsmith $*/

static char help[] = 
"Reads a PETSc matrix from a file partitions it\n\n";

/*T
   Concepts: partitioning
   Processors: n
T*/

/* 
  Include "petscmat.h" so that we can use matrices.  Note that this file
  automatically includes:
     petsc.h       - base PETSc routines   petscvec.h - vectors
     petscsys.h    - system routines       petscmat.h - matrices
     petscis.h     - index sets            
     petscviewer.h - viewers               
*/
#include "petscsles.h"

#undef __FUNC__
#define __FUNC__ "main"
int main(int argc,char **args)
{
  MatType         mtype = MATSEQSBAIJ;            /* matrix format */
  Mat             A,B;                /* matrix */
  PetscViewer          fd;               /* viewer */
  char            file[128];        /* input file name */
  PetscTruth      flg;
  int             ierr,*nlocal,rank,size;
  MatPartitioning part;
  IS              is,isn;

  PetscInitialize(&argc,&args,(char *)0,help);
  ierr = MPI_Comm_size(PETSC_COMM_WORLD,&size);CHKERRQ(ierr);
  ierr = MPI_Comm_rank(PETSC_COMM_WORLD,&rank);CHKERRQ(ierr);

  /* 
     Determine file from which we read the matrix
  */
  ierr = PetscOptionsGetString(PETSC_NULL,"-f",file,127,&flg);CHKERRQ(ierr);

  /* 
       Open binary file.  Note that we use PETSC_BINARY_RDONLY to indicate
       reading from this file.
  */
  ierr = PetscViewerBinaryOpen(PETSC_COMM_WORLD,file,PETSC_BINARY_RDONLY,&fd);CHKERRQ(ierr);

  /*
      Load the matrix and vector; then destroy the viewer.
  */
  ierr = MatLoad(fd,mtype,&A);CHKERRQ(ierr);
  ierr = PetscViewerDestroy(fd);CHKERRQ(ierr);

  ierr = MatView(A,PETSC_VIEWER_DRAW_WORLD);CHKERRQ(ierr);

  /*
       Partition the graph of the matrix 
  */
  ierr = MatPartitioningCreate(PETSC_COMM_WORLD,&part);CHKERRQ(ierr);
  ierr = MatPartitioningSetAdjacency(part,A);CHKERRQ(ierr);
  ierr = MatPartitioningSetFromOptions(part);CHKERRQ(ierr);
  /* get new processor owner number of each vertex */
  ierr = MatPartitioningApply(part,&is);CHKERRQ(ierr);
  /* get new global number of each old global number */
  ierr = ISPartitioningToNumbering(is,&isn);CHKERRQ(ierr);
  ierr = PetscMalloc(size*sizeof(int),&nlocal);CHKERRQ(ierr);
  /* get number of new vertices for each processor */
  ierr = ISPartitioningCount(is,nlocal);CHKERRQ(ierr); 
  ierr = ISDestroy(is);CHKERRQ(ierr);

  /* get old global number of each new global number */
  ierr = ISInvertPermutation(isn,nlocal[rank],&is);CHKERRQ(ierr);
  ierr = PetscFree(nlocal);CHKERRQ(ierr);
  ierr = ISDestroy(isn);CHKERRQ(ierr);
  ierr = MatPartitioningDestroy(part);CHKERRQ(ierr);

  ierr = ISSort(is);CHKERRQ(ierr);
  ierr = ISAllGather(is,&isn);CHKERRQ(ierr);


  ierr = MatGetSubMatrix(A,is,isn,PETSC_DECIDE,MAT_INITIAL_MATRIX,&B);CHKERRQ(ierr);
  ierr = ISDestroy(is);CHKERRQ(ierr);
  ierr = ISDestroy(isn);CHKERRQ(ierr);

  ierr = MatView(B,PETSC_VIEWER_DRAW_WORLD);CHKERRQ(ierr);

  /*
       Free work space.  All PETSc objects should be destroyed when they
       are no longer needed.
  */
  ierr = MatDestroy(A);CHKERRQ(ierr); 
  ierr = MatDestroy(B);CHKERRQ(ierr); 


  PetscFinalize();
  return 0;
}

