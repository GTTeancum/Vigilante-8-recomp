// addr: 0x8001d370  name: FUN_8001d370

void FUN_8001d370(void)

{
  puRamffff9638 = &UNK_8005693c + iRam00000004 * 0x4000;
  puRamffff963c = &UNK_8005a93c + iRam00000004 * 0x4000;
  SetColorMatrix((MATRIX *)&DAT_8006f760);
  SetBackColor(0x40,0x40,0x40);
  SetFogNearFar(0x800,0x2000,lRam000006d4);
  return;
}

