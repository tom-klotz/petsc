#ifdef PETSC_RCS_HEADER
static char vcid[] = "$Id: dpause.c,v 1.12 1997/10/19 03:27:39 bsmith Exp bsmith $";
#endif
/*
       Provides the calling sequences for all the basic Draw routines.
*/
#include "src/draw/drawimpl.h"  /*I "draw.h" I*/

#undef __FUNC__  
#define __FUNC__ "DrawPause" 
/*@
   DrawPause - Waits n seconds or until user input, depending on input 
               to DrawSetPause().

   Input Parameter:
.  draw - the drawing context

   Collective operation on Draw object.

.keywords: draw, pause

.seealso: DrawSetPause(), DrawGetPause()
@*/
int DrawPause(Draw draw)
{
  int ierr;
  PetscFunctionBegin;
  PetscValidHeaderSpecific(draw,DRAW_COOKIE);
  if (draw->type == DRAW_NULLWINDOW) PetscFunctionReturn(0);
  if (draw->ops.pause) {
    ierr = (*draw->ops.pause)(draw);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}
