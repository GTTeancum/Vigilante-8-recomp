// addr: 0x801012f8  name: FUN_801012f8

/* WARNING: Removing unreachable block (ram,0x8010136c) */
/* WARNING: Removing unreachable block (ram,0x80101394) */
/* WARNING: Removing unreachable block (ram,0x801013cc) */
/* WARNING: Removing unreachable block (ram,0x80101404) */
/* WARNING: Removing unreachable block (ram,0x8010143c) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint FUN_801012f8(void)

{
  int in_v0;
  uint uVar1;
  
  uVar1 = (uint)(in_v0 != 3) << 4 | 0x7c00;
  if ((_DAT_80065908 & 0x40) != 0) {
    uVar1 = 0x7000;
  }
  if ((_DAT_80065908 & 0x80) != 0) {
    uVar1 = uVar1 & 0xffffefff;
  }
  if ((_DAT_80065908 & 0x100) != 0) {
    uVar1 = uVar1 & 0xffff9fff;
  }
  if ((_DAT_80065908 & 0x200) != 0) {
    uVar1 = 0;
  }
  return uVar1;
}

