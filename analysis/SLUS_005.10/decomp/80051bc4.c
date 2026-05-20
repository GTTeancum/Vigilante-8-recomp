// addr: 0x80051bc4  name: SYS_OBJ_29C0

undefined4 SYS_OBJ_29C0(void)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  
  iVar2 = VSync(-1);
  iVar1 = DAT_80065164;
  if ((iVar2 <= DAT_80065160) && (DAT_80065164 = DAT_80065164 + 1, iVar1 < 0xf0001)) {
    return 0;
  }
  printf("GPU timeout:que=%d,stat=%08x,chcr=%08x,madr=%08x\n",DAT_8006514c - DAT_80065150 & 0x3f,
         GPU_REG1,DMA_GPU_CHCR,DMA_GPU_MADR);
  DAT_8006515c = SetIntrMask(0);
  DAT_80065150 = 0;
  DAT_8006514c = 0;
  DMA_GPU_CHCR = 0x401;
  DMA_DPCR = DMA_DPCR | 0x800;
  GPU_REG1 = 0x1000000;
  SetIntrMask(DAT_8006515c);
  uVar3 = SYS_OBJ_2AF4();
  return uVar3;
}

