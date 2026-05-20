// addr: 0x80047ee8  name: VSYNC_OBJ_A4

void VSYNC_OBJ_A4(void)

{
  dword dVar1;
  
  VSYNC_OBJ_178();
  dVar1 = GPU_REG1;
  VSYNC_OBJ_178(DAT_8005ffb4 + 1,1);
  if (((dVar1 & 0x400000) != 0) && (-1 < (int)(dVar1 ^ GPU_REG1))) {
    do {
    } while (((dVar1 ^ GPU_REG1) & 0x80000000) == 0);
  }
  DAT_8005ee88 = DAT_8005ffb4;
  DAT_8005ee84 = TMR_HRETRACE_VAL;
  return;
}

