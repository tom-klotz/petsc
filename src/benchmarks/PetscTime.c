/*$Id: PetscTime.c,v 1.17 2001/01/15 21:49:39 bsmith Exp bsmith $*/

#include "petsc.h"
#include "petscfix.h"
#include "src/sys/src/PetscLog/ptime.h"

#undef __FUNC__
#define __FUNC__ "main"
int main(int argc,char **argv)
{
  PetscLogDouble x,y;
  int        i;

  PetscInitialize(&argc,&argv,0,0);
  /* To take care of paging effects */
  PetscTime(y);

  for (i=0; i<2; i++) { 
    PetscTime(x);
    PetscTime(y); 
    PetscTime(y);
    PetscTime(y);
    PetscTime(y);
    PetscTime(y);
    PetscTime(y); 
    PetscTime(y);
    PetscTime(y);
    PetscTime(y);
    PetscTime(y);

    fprintf(stdout,"%-15s : %e sec\n","PetscTime",(y-x)/10.0);
  }
  PetscTime(x);
  ierr = PetscSleep(10);CHKERRQ(ierr);
  PetscTime(y); 
  fprintf(stdout,"%-15s : %e sec - Slept for 10 sec \n","PetscTime",(y-x));

  PetscFinalize();
  PetscFunctionReturn(0);
}
