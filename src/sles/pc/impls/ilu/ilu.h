/* $Id: ilu.h,v 1.4 1995/11/19 00:24:20 bsmith Exp bsmith $ */

/* 
   Private data structure for ILU preconditioner.
*/
#if !defined(__ILU_H)
#define __ILU_H

#include "mat.h"

typedef struct {
  Mat           fact;            /* factored matrix */
  MatReordering ordering;        /* matrix reordering */
  int           levels;          /* levels of fill */
  IS            row, col;        /* row and column permutations for reordering */
  void          *implctx;        /* private implementation context */
  int           inplace;         /* in-place ILU factorization */
  int           reusereordering; /* reuses previous reordering computed */

  int           usedt;           /* use drop tolerance form of ILU */
  double        dt;              /* drop tolerance */
  int           dtcount;         /* max number nonzeros per row in iludt */
  int           reusefill;       /* reuse fill from previous ILUDT */
} PC_ILU;

#endif
