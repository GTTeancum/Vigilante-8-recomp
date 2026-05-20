// addr: 0x80046904  name: S_SCA_OBJ_70

void S_SCA_OBJ_70(int param_1)

{
  ushort uVar1;
  ushort in_t0;
  uint in_t1;
  int in_t2;
  
  uVar1 = 0x7f;
  if ((*(short *)(param_1 + 4) < 0x80) && (uVar1 = 0, -1 < *(short *)(param_1 + 4))) {
    uVar1 = *(ushort *)(param_1 + 4);
  }
  SPU_MAIN_VOL_L = uVar1 & 0x7fff | 0xb000;
  if (in_t2 == 0) {
    if ((in_t1 & 2) == 0) goto S_SCA_OBJ_194;
    if ((in_t1 & 8) != 0) goto S_SCA_OBJ_EC;
S_SCA_OBJ_150:
    in_t0 = *(ushort *)(param_1 + 6);
    uVar1 = 0;
  }
  else {
S_SCA_OBJ_EC:
    switch(*(undefined2 *)(param_1 + 10)) {
    default:
      goto S_SCA_OBJ_150;
    case 1:
      uVar1 = 0x8000;
      break;
    case 2:
      uVar1 = 0x9000;
      break;
    case 3:
      uVar1 = 0xa000;
      break;
    case 4:
      uVar1 = 0xb000;
      break;
    case 5:
      uVar1 = 0xc000;
      break;
    case 6:
      uVar1 = 0xd000;
      break;
    case 7:
      uVar1 = 0xe000;
    }
  }
  if (uVar1 != 0) {
    in_t0 = 0x7f;
    if ((*(short *)(param_1 + 6) < 0x80) && (in_t0 = 0, -1 < *(short *)(param_1 + 6))) {
      in_t0 = *(ushort *)(param_1 + 6);
    }
  }
  SPU_MAIN_VOL_R = in_t0 & 0x7fff | uVar1;
S_SCA_OBJ_194:
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

