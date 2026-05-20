// addr: 0x800469d4  name: S_SCA_OBJ_140

void S_SCA_OBJ_140(int param_1)

{
  ushort uVar1;
  uint in_t1;
  int in_t2;
  
  uVar1 = 0x7f;
  if ((*(short *)(param_1 + 6) < 0x80) && (uVar1 = 0, -1 < *(short *)(param_1 + 6))) {
    uVar1 = *(ushort *)(param_1 + 6);
  }
  SPU_MAIN_VOL_R = uVar1 & 0x7fff | 0xd000;
  if ((in_t2 != 0) || ((in_t1 & 0x40) != 0)) {
    CD_VOL_L = *(word *)(param_1 + 0x10);
  }
  if ((in_t2 != 0) || ((in_t1 & 0x80) != 0)) {
    CD_VOL_R = *(word *)(param_1 + 0x12);
  }
  if ((in_t2 != 0) || ((in_t1 & 0x400) != 0)) {
    EXT_VOL_L = *(word *)(param_1 + 0x1c);
  }
  if ((in_t2 != 0) || ((in_t1 & 0x800) != 0)) {
    EXT_VOL_R = *(word *)(param_1 + 0x1e);
  }
  if ((in_t2 != 0) || ((in_t1 & 0x100) != 0)) {
    if (*(int *)(param_1 + 0x14) == 0) {
      S_SCA_OBJ_274();
      return;
    }
    SPU_CTRL_REG_CPUCNT = SPU_CTRL_REG_CPUCNT | 4;
  }
  if ((in_t2 != 0) || ((in_t1 & 0x200) != 0)) {
    if (*(int *)(param_1 + 0x18) == 0) {
      S_SCA_OBJ_2C8();
      return;
    }
    SPU_CTRL_REG_CPUCNT = SPU_CTRL_REG_CPUCNT | 1;
  }
  if ((in_t2 != 0) || ((in_t1 & 0x1000) != 0)) {
    if (*(int *)(param_1 + 0x20) == 0) {
      S_SCA_OBJ_31C();
      return;
    }
    SPU_CTRL_REG_CPUCNT = SPU_CTRL_REG_CPUCNT | 8;
  }
  if ((in_t2 != 0) || ((in_t1 & 0x2000) != 0)) {
    if (*(int *)(param_1 + 0x24) == 0) {
      S_SCA_OBJ_370();
      return;
    }
    SPU_CTRL_REG_CPUCNT = SPU_CTRL_REG_CPUCNT | 2;
  }
  return;
}

