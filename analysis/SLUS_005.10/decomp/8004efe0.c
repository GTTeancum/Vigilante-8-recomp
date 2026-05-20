// addr: 0x8004efe0  name: EXT_OBJ_AC

u_short EXT_OBJ_AC(RECT *param_1)

{
  u_short uVar1;
  u_long *in_t0;
  int unaff_s0;
  int unaff_s1;
  int unaff_s2;
  int unaff_s3;
  
  LoadImage(param_1,in_t0);
  uVar1 = GetTPage(unaff_s0,unaff_s2,unaff_s1,unaff_s3);
  return uVar1;
}

