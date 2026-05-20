// addr: 0x80048f68  name: CdSync

/* Possible GS_106.OBJ/GsSetProjection
   Possible _OP_VDEL.OBJ/__builtin_vec_delete
   Possible _OP_VNEW.OBJ/__builtin_vec_new
   Possible SSNOFF.OBJ/SsSetNoiseOff
   Possible SSQUIT.OBJ/SsQuit */

int CdSync(int mode,u_char *result)

{
  int iVar1;
  
  iVar1 = CD_sync();
  return iVar1;
}

