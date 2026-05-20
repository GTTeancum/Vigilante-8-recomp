// addr: 0x80018618  name: FUN_80018618

undefined4 * FUN_80018618(undefined4 param_1,u_short *param_2,u_short *param_3,ushort *param_4)

{
  u_short uVar1;
  RECT *rect;
  int iVar2;
  RECT *unaff_s1;
  
  FUN_800185cc(&DAT_8006f628,param_1);
  if (DAT_8006f62c != (RECT *)0x0) {
    if ((DAT_8006f628 & 0x10) == 0) {
      iVar2 = (int)*(short *)((int)DAT_8006f62c + 4);
    }
    else {
      iVar2 = 0x10;
      if (0x10 < *(short *)((int)DAT_8006f62c + 4)) {
        iVar2 = 0x100;
      }
    }
    unaff_s1 = (RECT *)FUN_80018124(iVar2,(int)*(short *)((int)DAT_8006f62c + 6),0x10,1,
                                    (int)*(short *)((int)DAT_8006f62c + 4),1);
    if (unaff_s1 != (RECT *)0x0) {
      LoadImage(unaff_s1,DAT_8006f630);
      if (param_3 != (u_short *)0x0) {
        uVar1 = GetClut((int)unaff_s1->x,(int)unaff_s1->y);
        *param_3 = uVar1;
      }
      goto LAB_80018714;
    }
  }
  if (param_3 != (u_short *)0x0) {
    *param_3 = 0;
  }
LAB_80018714:
  rect = (RECT *)FUN_80018124((int)*(short *)((int)DAT_8006f634 + 4),
                              (int)*(short *)((int)DAT_8006f634 + 6),0x40,0x100,
                              0x40 << (DAT_8006f628 & 3),0x100);
  if (rect != (RECT *)0x0) {
    LoadImage(rect,DAT_8006f638);
  }
  DAT_8006f62c = unaff_s1;
  DAT_8006f634 = rect;
  if (param_2 != (u_short *)0x0) {
    uVar1 = GetTPage(DAT_8006f628 & 3,0,(int)rect->x,(int)rect->y);
    *param_2 = uVar1;
  }
  if (param_4 != (ushort *)0x0) {
    *param_4 = (ushort)(((ushort)rect->x & 0x3f) << (2 - (DAT_8006f628 & 3) & 0x1f)) |
               (ushort)(byte)rect->y << 8;
  }
  return &DAT_8006f628;
}

