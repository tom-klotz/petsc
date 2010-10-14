#define PETSCDM_DLL

/* 
   Plots vectors obtained with DACreate2d()
*/

#include "private/daimpl.h"      /*I  "petscda.h"   I*/
#include "private/vecimpl.h" 

/*
        The data that is passed into the graphics callback
*/
typedef struct {
  PetscInt     m,n,step,k;
  PetscReal    min,max,scale;
  PetscScalar  *xy,*v;
  PetscBool    showgrid;
} ZoomCtx;

/*
       This does the drawing for one particular field 
    in one particular set of coordinates. It is a callback
    called from PetscDrawZoom()
*/
#undef __FUNCT__  
#define __FUNCT__ "VecView_MPI_Draw_DA2d_Zoom"
PetscErrorCode VecView_MPI_Draw_DA2d_Zoom(PetscDraw draw,void *ctx)
{
  ZoomCtx        *zctx = (ZoomCtx*)ctx;
  PetscErrorCode ierr;
  PetscInt       m,n,i,j,k,step,id,c1,c2,c3,c4;
  PetscReal      s,min,x1,x2,x3,x4,y_1,y2,y3,y4;
  PetscScalar   *v,*xy;

  PetscFunctionBegin; 
  m    = zctx->m;
  n    = zctx->n;
  step = zctx->step;
  k    = zctx->k;
  v    = zctx->v;
  xy   = zctx->xy;
  s    = zctx->scale;
  min  = zctx->min;
   
  /* PetscDraw the contour plot patch */
  for (j=0; j<n-1; j++) {
    for (i=0; i<m-1; i++) {
#if !defined(PETSC_USE_COMPLEX)
      id = i+j*m;    x1 = xy[2*id];y_1 = xy[2*id+1];c1 = (int)(PETSC_DRAW_BASIC_COLORS+s*(v[k+step*id]-min));
      id = i+j*m+1;  x2 = xy[2*id];y2  = y_1;       c2 = (int)(PETSC_DRAW_BASIC_COLORS+s*(v[k+step*id]-min));
      id = i+j*m+1+m;x3 = x2;      y3  = xy[2*id+1];c3 = (int)(PETSC_DRAW_BASIC_COLORS+s*(v[k+step*id]-min));
      id = i+j*m+m;  x4 = x1;      y4  = y3;        c4 = (int)(PETSC_DRAW_BASIC_COLORS+s*(v[k+step*id]-min));
#else
      id = i+j*m;    x1 = PetscRealPart(xy[2*id]);y_1 = PetscRealPart(xy[2*id+1]);c1 = (int)(PETSC_DRAW_BASIC_COLORS+s*(PetscRealPart(v[k+step*id])-min));
      id = i+j*m+1;  x2 = PetscRealPart(xy[2*id]);y2  = y_1;       c2 = (int)(PETSC_DRAW_BASIC_COLORS+s*(PetscRealPart(v[k+step*id])-min));
      id = i+j*m+1+m;x3 = x2;      y3  = PetscRealPart(xy[2*id+1]);c3 = (int)(PETSC_DRAW_BASIC_COLORS+s*(PetscRealPart(v[k+step*id])-min));
      id = i+j*m+m;  x4 = x1;      y4  = y3;        c4 = (int)(PETSC_DRAW_BASIC_COLORS+s*(PetscRealPart(v[k+step*id])-min));
#endif
      ierr = PetscDrawTriangle(draw,x1,y_1,x2,y2,x3,y3,c1,c2,c3);CHKERRQ(ierr);
      ierr = PetscDrawTriangle(draw,x1,y_1,x3,y3,x4,y4,c1,c3,c4);CHKERRQ(ierr);
      if (zctx->showgrid) {
        ierr = PetscDrawLine(draw,x1,y_1,x2,y2,PETSC_DRAW_BLACK);CHKERRQ(ierr);
        ierr = PetscDrawLine(draw,x2,y2,x3,y3,PETSC_DRAW_BLACK);CHKERRQ(ierr);
        ierr = PetscDrawLine(draw,x3,y3,x4,y4,PETSC_DRAW_BLACK);CHKERRQ(ierr);
        ierr = PetscDrawLine(draw,x4,y4,x1,y_1,PETSC_DRAW_BLACK);CHKERRQ(ierr);
      }
    }
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "VecView_MPI_Draw_DA2d"
PetscErrorCode VecView_MPI_Draw_DA2d(Vec xin,PetscViewer viewer)
{
  DM                 da,dac,dag;
  PetscErrorCode     ierr;
  PetscMPIInt        rank;
  PetscInt           igstart,N,s,M,istart,isize,jgstart,w;
  const PetscInt     *lx,*ly;
  PetscReal          coors[4],ymin,ymax,xmin,xmax;
  PetscDraw          draw,popup;
  PetscBool          isnull,useports = PETSC_FALSE;
  MPI_Comm           comm;
  Vec                xlocal,xcoor,xcoorl;
  DAPeriodicType     periodic;
  DAStencilType      st;
  ZoomCtx            zctx;
  PetscDrawViewPorts *ports;
  PetscViewerFormat  format;

  PetscFunctionBegin;
  zctx.showgrid = PETSC_FALSE;
  ierr = PetscViewerDrawGetDraw(viewer,0,&draw);CHKERRQ(ierr);
  ierr = PetscDrawIsNull(draw,&isnull);CHKERRQ(ierr); if (isnull) PetscFunctionReturn(0);

  ierr = PetscObjectQuery((PetscObject)xin,"DA",(PetscObject*)&da);CHKERRQ(ierr);
  if (!da) SETERRQ(((PetscObject)xin)->comm,PETSC_ERR_ARG_WRONG,"Vector not generated from a DA");

  ierr = PetscObjectGetComm((PetscObject)xin,&comm);CHKERRQ(ierr);
  ierr = MPI_Comm_rank(comm,&rank);CHKERRQ(ierr);

  ierr = DAGetInfo(da,0,&M,&N,0,&zctx.m,&zctx.n,0,&w,&s,&periodic,&st);CHKERRQ(ierr);
  ierr = DAGetOwnershipRanges(da,&lx,&ly,PETSC_NULL);CHKERRQ(ierr);

  /* 
        Obtain a sequential vector that is going to contain the local values plus ONE layer of 
     ghosted values to draw the graphics from. We also need its corresponding DA (dac) that will
     update the local values pluse ONE layer of ghost values. 
  */
  ierr = PetscObjectQuery((PetscObject)da,"GraphicsGhosted",(PetscObject*)&xlocal);CHKERRQ(ierr);
  if (!xlocal) {
    if (periodic != DA_NONPERIODIC || s != 1 || st != DA_STENCIL_BOX) {
      /* 
         if original da is not of stencil width one, or periodic or not a box stencil then
         create a special DA to handle one level of ghost points for graphics
      */
      ierr = DACreate2d(comm,DA_NONPERIODIC,DA_STENCIL_BOX,M,N,zctx.m,zctx.n,w,1,lx,ly,&dac);CHKERRQ(ierr); 
      ierr = PetscInfo(da,"Creating auxilary DA for managing graphics ghost points\n");CHKERRQ(ierr);
    } else {
      /* otherwise we can use the da we already have */
      dac = da;
    }
    /* create local vector for holding ghosted values used in graphics */
    ierr = DACreateLocalVector(dac,&xlocal);CHKERRQ(ierr);
    if (dac != da) {
      /* don't keep any public reference of this DA, is is only available through xlocal */
      ierr = DMDestroy(dac);CHKERRQ(ierr);
    } else {
      /* remove association between xlocal and da, because below we compose in the opposite
         direction and if we left this connect we'd get a loop, so the objects could 
         never be destroyed */
      ierr = PetscObjectCompose((PetscObject)xlocal,"DA",0);CHKERRQ(ierr);
    }
    ierr = PetscObjectCompose((PetscObject)da,"GraphicsGhosted",(PetscObject)xlocal);CHKERRQ(ierr);
    ierr = PetscObjectDereference((PetscObject)xlocal);CHKERRQ(ierr);
  } else {
    if (periodic == DA_NONPERIODIC && s == 1 && st == DA_STENCIL_BOX) {
      dac = da;
    } else {
      ierr = PetscObjectQuery((PetscObject)xlocal,"DA",(PetscObject*)&dac);CHKERRQ(ierr);
    }
  }

  /*
      Get local (ghosted) values of vector
  */
  ierr = DMGlobalToLocalBegin(dac,xin,INSERT_VALUES,xlocal);CHKERRQ(ierr);
  ierr = DMGlobalToLocalEnd(dac,xin,INSERT_VALUES,xlocal);CHKERRQ(ierr);
  ierr = VecGetArray(xlocal,&zctx.v);CHKERRQ(ierr);

  /* get coordinates of nodes */
  ierr = DAGetCoordinates(da,&xcoor);CHKERRQ(ierr);
  if (!xcoor) {
    ierr = DASetUniformCoordinates(da,0.0,1.0,0.0,1.0,0.0,0.0);CHKERRQ(ierr);
    ierr = DAGetCoordinates(da,&xcoor);CHKERRQ(ierr);
  }

  /*
      Determine the min and max  coordinates in plot 
  */
  ierr = VecStrideMin(xcoor,0,PETSC_NULL,&xmin);CHKERRQ(ierr);
  ierr = VecStrideMax(xcoor,0,PETSC_NULL,&xmax);CHKERRQ(ierr);
  ierr = VecStrideMin(xcoor,1,PETSC_NULL,&ymin);CHKERRQ(ierr);
  ierr = VecStrideMax(xcoor,1,PETSC_NULL,&ymax);CHKERRQ(ierr);
  coors[0] = xmin - .05*(xmax- xmin); coors[2] = xmax + .05*(xmax - xmin);
  coors[1] = ymin - .05*(ymax- ymin); coors[3] = ymax + .05*(ymax - ymin);
  ierr = PetscInfo4(da,"Preparing DA 2d contour plot coordinates %G %G %G %G\n",coors[0],coors[1],coors[2],coors[3]);CHKERRQ(ierr);

  /*
       get local ghosted version of coordinates 
  */
  ierr = PetscObjectQuery((PetscObject)da,"GraphicsCoordinateGhosted",(PetscObject*)&xcoorl);CHKERRQ(ierr);
  if (!xcoorl) {
    /* create DA to get local version of graphics */
    ierr = DACreate2d(comm,DA_NONPERIODIC,DA_STENCIL_BOX,M,N,zctx.m,zctx.n,2,1,lx,ly,&dag);CHKERRQ(ierr); 
    ierr = PetscInfo(dag,"Creating auxilary DA for managing graphics coordinates ghost points\n");CHKERRQ(ierr);
    ierr = DACreateLocalVector(dag,&xcoorl);CHKERRQ(ierr);
    ierr = PetscObjectCompose((PetscObject)da,"GraphicsCoordinateGhosted",(PetscObject)xcoorl);CHKERRQ(ierr);
    ierr = DMDestroy(dag);CHKERRQ(ierr);/* dereference dag */
    ierr = PetscObjectDereference((PetscObject)xcoorl);CHKERRQ(ierr);
  } else {
    ierr = PetscObjectQuery((PetscObject)xcoorl,"DA",(PetscObject*)&dag);CHKERRQ(ierr);
  }
  ierr = DMGlobalToLocalBegin(dag,xcoor,INSERT_VALUES,xcoorl);CHKERRQ(ierr);
  ierr = DMGlobalToLocalEnd(dag,xcoor,INSERT_VALUES,xcoorl);CHKERRQ(ierr);
  ierr = VecGetArray(xcoorl,&zctx.xy);CHKERRQ(ierr);
  
  /*
        Get information about size of area each processor must do graphics for
  */
  ierr = DAGetInfo(dac,0,&M,&N,0,0,0,0,&zctx.step,0,&periodic,0);CHKERRQ(ierr);
  ierr = DAGetGhostCorners(dac,&igstart,&jgstart,0,&zctx.m,&zctx.n,0);CHKERRQ(ierr);
  ierr = DAGetCorners(dac,&istart,0,0,&isize,0,0);CHKERRQ(ierr);

  ierr = PetscOptionsGetTruth(PETSC_NULL,"-draw_contour_grid",&zctx.showgrid,PETSC_NULL);CHKERRQ(ierr);

  ierr = PetscViewerGetFormat(viewer,&format);CHKERRQ(ierr);
  ierr = PetscOptionsGetTruth(PETSC_NULL,"-draw_ports",&useports,PETSC_NULL);CHKERRQ(ierr);
  if (useports || format == PETSC_VIEWER_DRAW_PORTS){
    ierr = PetscDrawSynchronizedClear(draw);CHKERRQ(ierr);
    ierr = PetscDrawViewPortsCreate(draw,zctx.step,&ports);CHKERRQ(ierr);
  }
  /*
     Loop over each field; drawing each in a different window
  */
  for (zctx.k=0; zctx.k<zctx.step; zctx.k++) {
    if (useports) {
      ierr = PetscDrawViewPortsSet(ports,zctx.k);CHKERRQ(ierr);
    } else {
      ierr = PetscViewerDrawGetDraw(viewer,zctx.k,&draw);CHKERRQ(ierr);
      ierr = PetscDrawSynchronizedClear(draw);CHKERRQ(ierr);
    }

    /*
        Determine the min and max color in plot 
    */
    ierr = VecStrideMin(xin,zctx.k,PETSC_NULL,&zctx.min);CHKERRQ(ierr);
    ierr = VecStrideMax(xin,zctx.k,PETSC_NULL,&zctx.max);CHKERRQ(ierr);
    if (zctx.min == zctx.max) {
      zctx.min -= 1.e-12;
      zctx.max += 1.e-12;
    }

    if (!rank) {
      const char *title;

      ierr = DAGetFieldName(da,zctx.k,&title);CHKERRQ(ierr);
      if (title) {
        ierr = PetscDrawSetTitle(draw,title);CHKERRQ(ierr);
      }
    }
    ierr = PetscDrawSetCoordinates(draw,coors[0],coors[1],coors[2],coors[3]);CHKERRQ(ierr);
    ierr = PetscInfo2(da,"DA 2d contour plot min %G max %G\n",zctx.min,zctx.max);CHKERRQ(ierr);

    ierr = PetscDrawGetPopup(draw,&popup);CHKERRQ(ierr);
    if (popup) {ierr = PetscDrawScalePopup(popup,zctx.min,zctx.max);CHKERRQ(ierr);}

    zctx.scale = (245.0 - PETSC_DRAW_BASIC_COLORS)/(zctx.max - zctx.min);

    ierr = PetscDrawZoom(draw,VecView_MPI_Draw_DA2d_Zoom,&zctx);CHKERRQ(ierr);
  }
  if (useports){
    ierr = PetscDrawViewPortsDestroy(ports);CHKERRQ(ierr);
  }

  ierr = VecRestoreArray(xcoorl,&zctx.xy);CHKERRQ(ierr);
  ierr = VecRestoreArray(xlocal,&zctx.v);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#if defined(PETSC_HAVE_HDF5)
#undef __FUNCT__  
#define __FUNCT__ "VecView_MPI_HDF5_DA"
PetscErrorCode VecView_MPI_HDF5_DA(Vec xin,PetscViewer viewer)
{
  PetscErrorCode ierr;
  DM             da;
  hsize_t        dim,dims[5];
  hsize_t        count[5];
  hsize_t        offset[5];
  PetscInt       cnt = 0;
  PetscScalar    *x;
  const char     *vecname;
  hid_t          filespace; /* file dataspace identifier */
  hid_t	         plist_id;  /* property list identifier */
  hid_t          dset_id;   /* dataset identifier */
  hid_t          memspace;  /* memory dataspace identifier */
  hid_t          file_id;
  herr_t         status;

  PetscFunctionBegin;
  ierr = PetscViewerHDF5GetFileId(viewer, &file_id);CHKERRQ(ierr);
  ierr = PetscObjectQuery((PetscObject)xin,"DA",(PetscObject*)&da);CHKERRQ(ierr);
  if (!da) SETERRQ(((PetscObject)xin)->comm,PETSC_ERR_ARG_WRONG,"Vector not generated from a DA");

  /* Create the dataspace for the dataset */
  dim       = PetscHDF5IntCast(da->dim + ((da->w == 1) ? 0 : 1));
  if (da->dim == 3) dims[cnt++]   = PetscHDF5IntCast(da->P);
  if (da->dim > 1)  dims[cnt++]   = PetscHDF5IntCast(da->N);
  dims[cnt++]   = PetscHDF5IntCast(da->M);
  if (da->w > 1) dims[cnt++] = PetscHDF5IntCast(da->w);
#if defined(PETSC_USE_COMPLEX)
  dim++;
  dims[cnt++] = 2;
#endif
  filespace = H5Screate_simple(dim, dims, NULL); 
  if (filespace == -1) SETERRQ(PETSC_COMM_SELF,PETSC_ERR_LIB,"Cannot H5Screate_simple()");

  /* Create the dataset with default properties and close filespace */
  ierr = PetscObjectGetName((PetscObject)xin,&vecname);CHKERRQ(ierr);
#if (H5_VERS_MAJOR * 10000 + H5_VERS_MINOR * 100 + H5_VERS_RELEASE >= 10800)
  dset_id = H5Dcreate2(file_id, vecname, H5T_NATIVE_DOUBLE, filespace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
#else
  dset_id = H5Dcreate(file_id, vecname, H5T_NATIVE_DOUBLE, filespace, H5P_DEFAULT);
#endif
  if (dset_id == -1) SETERRQ(PETSC_COMM_SELF,PETSC_ERR_LIB,"Cannot H5Dcreate2()");
  status = H5Sclose(filespace);CHKERRQ(status);

  /* Each process defines a dataset and writes it to the hyperslab in the file */
  cnt = 0; 
  if (da->dim == 3) offset[cnt++] = PetscHDF5IntCast(da->zs);
  if (da->dim > 1)  offset[cnt++] = PetscHDF5IntCast(da->ys);
  offset[cnt++] = PetscHDF5IntCast(da->xs/da->w);
  if (da->w > 1) offset[cnt++] = 0;
#if defined(PETSC_USE_COMPLEX)
  offset[cnt++] = 0;
#endif
  cnt = 0; 
  if (da->dim == 3) count[cnt++] = PetscHDF5IntCast(da->ze - da->zs);
  if (da->dim > 1)  count[cnt++] = PetscHDF5IntCast(da->ye - da->ys);
  count[cnt++] = PetscHDF5IntCast((da->xe - da->xs)/da->w);
  if (da->w > 1) count[cnt++] = PetscHDF5IntCast(da->w);
#if defined(PETSC_USE_COMPLEX)
  count[cnt++] = 2;
#endif
  memspace = H5Screate_simple(dim, count, NULL);
  if (memspace == -1) SETERRQ(PETSC_COMM_SELF,PETSC_ERR_LIB,"Cannot H5Screate_simple()");


  filespace = H5Dget_space(dset_id);
  if (filespace == -1) SETERRQ(PETSC_COMM_SELF,PETSC_ERR_LIB,"Cannot H5Dget_space()");
  status = H5Sselect_hyperslab(filespace, H5S_SELECT_SET, offset, NULL, count, NULL);CHKERRQ(status);

  /* Create property list for collective dataset write */
  plist_id = H5Pcreate(H5P_DATASET_XFER);
  if (plist_id == -1) SETERRQ(PETSC_COMM_SELF,PETSC_ERR_LIB,"Cannot H5Pcreate()");
#if defined(PETSC_HAVE_H5PSET_FAPL_MPIO)
  status = H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_COLLECTIVE);CHKERRQ(status);
#endif
  /* To write dataset independently use H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_INDEPENDENT) */

  ierr = VecGetArray(xin, &x);CHKERRQ(ierr);
  status = H5Dwrite(dset_id, H5T_NATIVE_DOUBLE, memspace, filespace, plist_id, x);CHKERRQ(status);
  status = H5Fflush(file_id, H5F_SCOPE_GLOBAL);CHKERRQ(status);
  ierr = VecRestoreArray(xin, &x);CHKERRQ(ierr);

  /* Close/release resources */
  status = H5Pclose(plist_id);CHKERRQ(status);
  status = H5Sclose(filespace);CHKERRQ(status);
  status = H5Sclose(memspace);CHKERRQ(status);
  status = H5Dclose(dset_id);CHKERRQ(status);
  ierr = PetscInfo1(xin,"Wrote Vec object with name %s\n",vecname);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
#endif

EXTERN PetscErrorCode VecView_MPI_Draw_DA1d(Vec,PetscViewer);

#if defined(PETSC_HAVE_MPIIO)
#undef __FUNCT__  
#define __FUNCT__ "DAArrayMPIIO"
static PetscErrorCode DAArrayMPIIO(DM da,PetscViewer viewer,Vec xin,PetscBool  write)
{
  PetscErrorCode    ierr;
  MPI_File          mfdes;
  PetscMPIInt       gsizes[4],lsizes[4],lstarts[4],asiz,dof;
  MPI_Datatype      view;
  const PetscScalar *array;
  MPI_Offset        off;
  MPI_Aint          ub,ul;
  PetscInt          type,rows,vecrows,tr[2];
  DM_DA             *dd = (DM_DA*)da->data;

  PetscFunctionBegin;
  ierr = VecGetSize(xin,&vecrows);CHKERRQ(ierr);
  if (!write) {
    /* Read vector header. */
    ierr = PetscViewerBinaryRead(viewer,tr,2,PETSC_INT);CHKERRQ(ierr);
    type = tr[0];
    rows = tr[1];
    if (type != VEC_FILE_CLASSID) SETERRQ(((PetscObject)da)->comm,PETSC_ERR_ARG_WRONG,"Not vector next in file");
    if (rows != vecrows) SETERRQ(((PetscObject)da)->comm,PETSC_ERR_ARG_SIZ,"Vector in file not same size as DA vector");
  } else {
    tr[0] = VEC_FILE_CLASSID;
    tr[1] = vecrows;
    ierr = PetscViewerBinaryWrite(viewer,tr,2,PETSC_INT,PETSC_TRUE);CHKERRQ(ierr);
  }

  dof = PetscMPIIntCast(dd->w);
  gsizes[0]  = dof; gsizes[1] = PetscMPIIntCast(dd->M); gsizes[2] = PetscMPIIntCast(dd->N); gsizes[3] = PetscMPIIntCast(dd->P);
  lsizes[0]  = dof;lsizes[1] = PetscMPIIntCast((dd->xe-dd->xs)/dof); lsizes[2] = PetscMPIIntCast(dd->ye-dd->ys); lsizes[3] = PetscMPIIntCast(dd->ze-dd->zs);
  lstarts[0] = 0;  lstarts[1] = PetscMPIIntCast(dd->xs/dof); lstarts[2] = PetscMPIIntCast(dd->ys); lstarts[3] = PetscMPIIntCast(dd->zs);
  ierr = MPI_Type_create_subarray(dd->dim+1,gsizes,lsizes,lstarts,MPI_ORDER_FORTRAN,MPIU_SCALAR,&view);CHKERRQ(ierr);
  ierr = MPI_Type_commit(&view);CHKERRQ(ierr);
  
  ierr = PetscViewerBinaryGetMPIIODescriptor(viewer,&mfdes);CHKERRQ(ierr);
  ierr = PetscViewerBinaryGetMPIIOOffset(viewer,&off);CHKERRQ(ierr);
  ierr = MPI_File_set_view(mfdes,off,MPIU_SCALAR,view,(char *)"native",MPI_INFO_NULL);CHKERRQ(ierr);
  ierr = VecGetArrayRead(xin,&array);CHKERRQ(ierr);
  asiz = lsizes[1]*(lsizes[2] > 0 ? lsizes[2] : 1)*(lsizes[3] > 0 ? lsizes[3] : 1)*dof;
  if (write) {
    ierr = MPIU_File_write_all(mfdes,(PetscScalar*)array,asiz,MPIU_SCALAR,MPI_STATUS_IGNORE);CHKERRQ(ierr);
  } else {
    ierr = MPIU_File_read_all(mfdes,(PetscScalar*)array,asiz,MPIU_SCALAR,MPI_STATUS_IGNORE);CHKERRQ(ierr);
  }
  ierr = MPI_Type_get_extent(view,&ul,&ub);CHKERRQ(ierr);
  ierr = PetscViewerBinaryAddMPIIOOffset(viewer,ub);CHKERRQ(ierr);
  ierr = VecRestoreArrayRead(xin,&array);CHKERRQ(ierr);
  ierr = MPI_Type_free(&view);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
#endif

EXTERN_C_BEGIN
#undef __FUNCT__  
#define __FUNCT__ "VecView_MPI_DA"
PetscErrorCode PETSCDM_DLLEXPORT VecView_MPI_DA(Vec xin,PetscViewer viewer)
{
  DM             da;
  PetscErrorCode ierr;
  PetscInt       dim;
  Vec            natural;
  PetscBool      isdraw;
#if defined(PETSC_HAVE_HDF5)
  PetscBool      ishdf5;
#endif
  const char     *prefix;

  PetscFunctionBegin;
  ierr = PetscObjectQuery((PetscObject)xin,"DA",(PetscObject*)&da);CHKERRQ(ierr);
  if (!da) SETERRQ(((PetscObject)xin)->comm,PETSC_ERR_ARG_WRONG,"Vector not generated from a DA");
  ierr = PetscTypeCompare((PetscObject)viewer,PETSCVIEWERDRAW,&isdraw);CHKERRQ(ierr);
#if defined(PETSC_HAVE_HDF5)
  ierr = PetscTypeCompare((PetscObject)viewer,PETSCVIEWERHDF5,&ishdf5);CHKERRQ(ierr);
#endif
  if (isdraw) {
    ierr = DAGetInfo(da,&dim,0,0,0,0,0,0,0,0,0,0);CHKERRQ(ierr);
    if (dim == 1) {
      ierr = VecView_MPI_Draw_DA1d(xin,viewer);CHKERRQ(ierr);
    } else if (dim == 2) {
      ierr = VecView_MPI_Draw_DA2d(xin,viewer);CHKERRQ(ierr);
    } else {
      SETERRQ1(((PetscObject)da)->comm,PETSC_ERR_SUP,"Cannot graphically view vector associated with this dimensional DA %D",dim);
    }
#if defined(PETSC_HAVE_HDF5)
  } else if (ishdf5) {
    ierr = VecView_MPI_HDF5_DA(xin,viewer);CHKERRQ(ierr);
#endif
  } else {
#if defined(PETSC_HAVE_MPIIO)
    PetscBool  isbinary,isMPIIO;

    ierr = PetscTypeCompare((PetscObject)viewer,PETSCVIEWERBINARY,&isbinary);CHKERRQ(ierr);
    if (isbinary) {
      ierr = PetscViewerBinaryGetMPIIO(viewer,&isMPIIO);CHKERRQ(ierr);
      if (isMPIIO) {
       ierr = DAArrayMPIIO(da,viewer,xin,PETSC_TRUE);CHKERRQ(ierr);
       PetscFunctionReturn(0);
      }
    }
#endif
    
    /* call viewer on natural ordering */
    ierr = PetscObjectGetOptionsPrefix((PetscObject)xin,&prefix);CHKERRQ(ierr);
    ierr = DACreateNaturalVector(da,&natural);CHKERRQ(ierr);
    ierr = PetscObjectSetOptionsPrefix((PetscObject)natural,prefix);CHKERRQ(ierr);
    ierr = DAGlobalToNaturalBegin(da,xin,INSERT_VALUES,natural);CHKERRQ(ierr);
    ierr = DAGlobalToNaturalEnd(da,xin,INSERT_VALUES,natural);CHKERRQ(ierr);
    ierr = PetscObjectSetName((PetscObject)natural,((PetscObject)xin)->name);CHKERRQ(ierr);
    ierr = VecView(natural,viewer);CHKERRQ(ierr);
    ierr = VecDestroy(natural);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}
EXTERN_C_END

#if defined(PETSC_HAVE_HDF5)
#undef __FUNCT__  
#define __FUNCT__ "VecLoad_HDF5_DA"
PetscErrorCode VecLoad_HDF5_DA(Vec xin, PetscViewer viewer)
{
  DM             da;
  PetscErrorCode ierr;
  hsize_t        dim,dims[5];
  hsize_t        count[5];
  hsize_t        offset[5];
  PetscInt       cnt = 0;
  PetscScalar    *x;
  const char     *vecname;
  hid_t          filespace; /* file dataspace identifier */
  hid_t	         plist_id;  /* property list identifier */
  hid_t          dset_id;   /* dataset identifier */
  hid_t          memspace;  /* memory dataspace identifier */
  hid_t          file_id;
  herr_t         status;
  DM_DA          *dd = (DM_DA*)da->data;

  PetscFunctionBegin;
  ierr = PetscViewerHDF5GetFileId(viewer, &file_id);CHKERRQ(ierr);
  ierr = PetscObjectQuery((PetscObject)xin,"DA",(PetscObject*)&da);CHKERRQ(ierr);

  /* Create the dataspace for the dataset */
  dim       = PetscHDF5IntCast(dd->dim + ((dd->w == 1) ? 0 : 1));
  if (dd->dim == 3) dims[cnt++]   = PetscHDF5IntCast(dd->P);
  if (dd->dim > 1)  dims[cnt++]   = PetscHDF5IntCast(dd->N);
  dims[cnt++]     = PetscHDF5IntCast(dd->M);
  if (dd->w > 1) PetscHDF5IntCast(dims[cnt++] = dd->w);
#if defined(PETSC_USE_COMPLEX)
  dim++;
  dims[cnt++] = 2;
#endif

  /* Create the dataset with default properties and close filespace */
  ierr = PetscObjectGetName((PetscObject)xin,&vecname);CHKERRQ(ierr);
#if (H5_VERS_MAJOR * 10000 + H5_VERS_MINOR * 100 + H5_VERS_RELEASE >= 10800)
  dset_id = H5Dopen2(file_id, vecname, H5P_DEFAULT);
#else
  dset_id = H5Dopen(file_id, vecname);
#endif
  if (dset_id == -1) SETERRQ1(PETSC_COMM_SELF,PETSC_ERR_LIB,"Cannot H5Dopen2() with Vec named %s",vecname);
  filespace = H5Dget_space(dset_id);

  /* Each process defines a dataset and reads it from the hyperslab in the file */
  cnt = 0; 
  if (dd->dim == 3) offset[cnt++] = PetscHDF5IntCast(dd->zs);
  if (dd->dim > 1)  offset[cnt++] = PetscHDF5IntCast(dd->ys);
  offset[cnt++] = PetscHDF5IntCast(dd->xs/dd->w);
  if (dd->w > 1) offset[cnt++] = 0;
#if defined(PETSC_USE_COMPLEX)
  offset[cnt++] = 0;
#endif
  cnt = 0; 
  if (dd->dim == 3) count[cnt++] = PetscHDF5IntCast(dd->ze - dd->zs);
  if (dd->dim > 1)  count[cnt++] = PetscHDF5IntCast(dd->ye - dd->ys);
  count[cnt++] = PetscHDF5IntCast((dd->xe - dd->xs)/dd->w);
  if (dd->w > 1) count[cnt++] = PetscHDF5IntCast(dd->w);
#if defined(PETSC_USE_COMPLEX)
  count[cnt++] = 2;
#endif
  memspace = H5Screate_simple(dim, count, NULL);
  if (memspace == -1) SETERRQ(PETSC_COMM_SELF,PETSC_ERR_LIB,"Cannot H5Screate_simple()");

  status = H5Sselect_hyperslab(filespace, H5S_SELECT_SET, offset, NULL, count, NULL);CHKERRQ(status);

  /* Create property list for collective dataset write */
  plist_id = H5Pcreate(H5P_DATASET_XFER);
  if (plist_id == -1) SETERRQ(PETSC_COMM_SELF,PETSC_ERR_LIB,"Cannot H5Pcreate()");
#if defined(PETSC_HAVE_H5PSET_FAPL_MPIO)
  status = H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_COLLECTIVE);CHKERRQ(status);
#endif
  /* To write dataset independently use H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_INDEPENDENT) */

  ierr = VecGetArray(xin, &x);CHKERRQ(ierr);
  status = H5Dread(dset_id, H5T_NATIVE_DOUBLE, memspace, filespace, plist_id, x);CHKERRQ(status);
  ierr = VecRestoreArray(xin, &x);CHKERRQ(ierr);

  /* Close/release resources */
  status = H5Pclose(plist_id);CHKERRQ(status);
  status = H5Sclose(filespace);CHKERRQ(status);
  status = H5Sclose(memspace);CHKERRQ(status);
  status = H5Dclose(dset_id);CHKERRQ(status);
  PetscFunctionReturn(0);
}
#endif

#undef __FUNCT__  
#define __FUNCT__ "VecLoad_Binary_DA"
PetscErrorCode VecLoad_Binary_DA(Vec xin, PetscViewer viewer)
{
  DM             da;
  PetscErrorCode ierr;
  Vec            natural;
  const char     *prefix;
  PetscInt       bs;
  PetscBool      flag;
  DM_DA          *dd;
#if defined(PETSC_HAVE_MPIIO)
  PetscBool      isMPIIO;
#endif

  PetscFunctionBegin;
  ierr = PetscObjectQuery((PetscObject)xin,"DA",(PetscObject*)&da);CHKERRQ(ierr);
  dd   = (DM_DA*)da->data;
#if defined(PETSC_HAVE_MPIIO)
  ierr = PetscViewerBinaryGetMPIIO(viewer,&isMPIIO);CHKERRQ(ierr);
  if (isMPIIO) {
    ierr = DAArrayMPIIO(da,viewer,xin,PETSC_FALSE);CHKERRQ(ierr);
    PetscFunctionReturn(0);
  }
#endif

  ierr = PetscObjectGetOptionsPrefix((PetscObject)xin,&prefix);CHKERRQ(ierr);
  ierr = DACreateNaturalVector(da,&natural);CHKERRQ(ierr);
  ierr = PetscObjectSetName((PetscObject)natural,((PetscObject)xin)->name);CHKERRQ(ierr);
  ierr = PetscObjectSetOptionsPrefix((PetscObject)natural,prefix);CHKERRQ(ierr);
  ierr = VecLoad_Binary(natural,viewer);CHKERRQ(ierr);
  ierr = DANaturalToGlobalBegin(da,natural,INSERT_VALUES,xin);CHKERRQ(ierr);
  ierr = DANaturalToGlobalEnd(da,natural,INSERT_VALUES,xin);CHKERRQ(ierr);
  ierr = VecDestroy(natural);CHKERRQ(ierr);
  ierr = PetscInfo(xin,"Loading vector from natural ordering into DA\n");CHKERRQ(ierr);
  ierr = PetscOptionsGetInt(((PetscObject)xin)->prefix,"-vecload_block_size",&bs,&flag);CHKERRQ(ierr);
  if (flag && bs != dd->w) {
    ierr = PetscInfo2(xin,"Block size in file %D not equal to DA's dof %D\n",bs,dd->w);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

EXTERN_C_BEGIN
#undef __FUNCT__
#define __FUNCT__ "VecLoad_Default_DA"
PetscErrorCode PETSCDM_DLLEXPORT VecLoad_Default_DA(Vec xin, PetscViewer viewer)
{
  PetscErrorCode ierr;
  DM             da;
  PetscBool      isbinary;
#if defined(PETSC_HAVE_HDF5)
  PetscBool      ishdf5;
#endif

  PetscFunctionBegin;
  ierr = PetscObjectQuery((PetscObject)xin,"DA",(PetscObject*)&da);CHKERRQ(ierr);
  if (!da) SETERRQ(((PetscObject)xin)->comm,PETSC_ERR_ARG_WRONG,"Vector not generated from a DA");

#if defined(PETSC_HAVE_HDF5)
  ierr = PetscTypeCompare((PetscObject)viewer,PETSCVIEWERHDF5,&ishdf5);CHKERRQ(ierr);
#endif
  ierr = PetscTypeCompare((PetscObject)viewer,PETSCVIEWERBINARY,&isbinary);CHKERRQ(ierr);

  if (isbinary) {
    ierr = VecLoad_Binary_DA(xin,viewer);CHKERRQ(ierr);
#if defined(PETSC_HAVE_HDF5)
  } else if (ishdf5) {
    ierr = VecLoad_HDF5_DA(xin,viewer);CHKERRQ(ierr);
#endif
  } else {
    SETERRQ1(PETSC_COMM_SELF,PETSC_ERR_SUP,"Viewer type %s not supported for vector loading", ((PetscObject)viewer)->type_name);
  }
  PetscFunctionReturn(0);
}
EXTERN_C_END