/*$Id: ex13.c,v 1.10 2001/01/15 21:49:08 bsmith Exp bsmith $*/

static char help[] = "Tests loading DA vector from file\n\n";

#include "petscda.h"
#include "petscsys.h"

#undef __FUNC__
#define __FUNC__ "main"
int main(int argc,char **argv)
{
  int      ierr,M = PETSC_DECIDE,N = PETSC_DECIDE;
  DA       da;
  Vec      global;
  PetscViewer   bviewer;

  PetscInitialize(&argc,&argv,(char*)0,help);

  /* Read options */
  ierr = PetscOptionsGetInt(PETSC_NULL,"-M",&M,PETSC_NULL);CHKERRQ(ierr);
  ierr = PetscOptionsGetInt(PETSC_NULL,"-N",&N,PETSC_NULL);CHKERRQ(ierr);

  ierr = PetscViewerBinaryOpen(PETSC_COMM_WORLD,"daoutput",PETSC_BINARY_RDONLY,&bviewer);CHKERRQ(ierr);
  ierr = DALoad(bviewer,M,N,PETSC_DECIDE,&da);CHKERRQ(ierr);
  ierr = DACreateGlobalVector(da,&global);CHKERRQ(ierr); 
  ierr = VecLoadIntoVector(bviewer,global);CHKERRQ(ierr); 
  ierr = PetscViewerDestroy(bviewer);CHKERRQ(ierr);


  ierr = VecView(global,PETSC_VIEWER_DRAW_WORLD);CHKERRQ(ierr);


  /* Free memory */
  ierr = VecDestroy(global);CHKERRQ(ierr); 
  ierr = DADestroy(da);CHKERRQ(ierr);
  PetscFinalize();
  return 0;
}
 
