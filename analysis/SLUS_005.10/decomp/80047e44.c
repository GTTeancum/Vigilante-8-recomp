// addr: 0x80047e44  name: VSync

int VSync(int mode)

{
  dword dVar1;
  uint uVar2;
  int iVar3;
  
  uVar2 = TMR_HRETRACE_VAL - DAT_8005ee84;
  if (mode < 0) {
    iVar3 = VSYNC_OBJ_164();
    return iVar3;
  }
  if (mode != 1) {
    if (0 < mode) {
      iVar3 = VSYNC_OBJ_A4();
      return iVar3;
    }
    iVar3 = 0;
    if (0 < mode) {
      iVar3 = mode + -1;
    }
    VSYNC_OBJ_178(DAT_8005ee88,iVar3);
    dVar1 = GPU_REG1;
    VSYNC_OBJ_178(DAT_8005ffb4 + 1,1);
    if (((dVar1 & 0x400000) != 0) && (-1 < (int)(dVar1 ^ GPU_REG1))) {
      do {
      } while (((dVar1 ^ GPU_REG1) & 0x80000000) == 0);
    }
    DAT_8005ee88 = DAT_8005ffb4;
    DAT_8005ee84 = TMR_HRETRACE_VAL;
  }
  return uVar2 & 0xffff;
}

