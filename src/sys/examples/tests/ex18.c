
static char help[] = "Tests PetscContainerCreate() and PetscContainerDestroy().\n\n";

#include <petscsys.h>

#undef __FUNCT__
#define __FUNCT__ "main"
int main(int argc,char **argv)
{
  PetscErrorCode ierr;
  PetscContainer container;

  ierr = PetscInitialize(&argc,&argv,(char*)0,help);if (ierr) return ierr;
  ierr = PetscContainerCreate(PETSC_COMM_SELF,&container);CHKERRQ(ierr);
  ierr = PetscContainerDestroy(container);CHKERRQ(ierr);
  ierr = PetscFinalize();
  return ierr;
}
