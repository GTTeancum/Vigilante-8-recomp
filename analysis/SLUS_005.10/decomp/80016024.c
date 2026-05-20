// addr: 0x80016024  name: FUN_80016024

int FUN_80016024(int param_1)

{
  ushort uVar1;
  byte bVar2;
  char cVar3;
  char cVar4;
  int iVar5;
  int iVar6;
  undefined2 *puVar7;
  int iVar8;
  undefined1 uVar9;
  undefined2 uVar10;
  int iVar11;
  int iVar12;
  uint uVar13;
  undefined2 uVar14;
  uint uVar15;
  int iVar16;
  DISPENV DStack_c0;
  undefined1 auStack_a8 [3];
  undefined1 local_a5;
  undefined1 local_a1;
  undefined2 local_a0;
  undefined2 local_9e;
  undefined1 local_9c;
  undefined1 local_9b;
  undefined2 local_98;
  undefined2 local_96;
  undefined1 local_94;
  undefined1 local_93;
  ushort local_92;
  undefined2 local_90;
  undefined2 local_8e;
  undefined1 local_8c;
  undefined1 local_8b;
  undefined2 local_88;
  undefined2 local_86;
  undefined1 local_84;
  undefined1 local_83;
  DRAWENV DStack_80;
  
  SetDefDispEnv(&DStack_c0,0x280,0,0x140,0xf0);
  DStack_c0.screen.x = (short)cRam00000018;
  DStack_c0.screen.y = (short)cRam00000019;
  if (param_1 == 0) {
    MoveImage((RECT *)&DAT_80065660,0x280,0);
  }
  else {
    SetDefDrawEnv(&DStack_80,0x280,0,0x140,0xf0);
    DStack_80.isbg = '\x01';
    PutDrawEnv(&DStack_80);
    local_a5 = 9;
    local_a1 = 0x2d;
    uVar15 = 0;
    do {
      uVar1 = (ushort)(uVar15 >> 5) & 0xf;
      local_92 = uVar1 | 0x100;
      uVar13 = uVar15 + 0x40;
      local_9c = 0;
      local_9b = 0;
      local_94 = 0x80;
      local_93 = 0;
      local_8c = 0;
      local_8b = 0xff;
      local_84 = 0x80;
      local_83 = 0xff;
      uVar14 = (undefined2)uVar15;
      local_9e = 0;
      uVar10 = (undefined2)uVar13;
      local_96 = 0;
      local_8e = 0x80;
      local_86 = 0x80;
      local_a0 = uVar14;
      local_98 = uVar10;
      local_90 = uVar14;
      local_88 = uVar10;
      DrawPrim(auStack_a8);
      local_92 = uVar1 | 0x110;
      local_8b = 0xe0;
      local_83 = 0xe0;
      local_9c = 0;
      local_9b = 0;
      local_94 = 0x80;
      local_93 = 0;
      local_8c = 0;
      local_84 = 0x80;
      local_9e = 0x80;
      local_96 = 0x80;
      local_8e = 0xf0;
      local_86 = 0xf0;
      local_a0 = uVar14;
      local_98 = uVar10;
      local_90 = uVar14;
      local_88 = uVar10;
      DrawPrim(auStack_a8);
      uVar15 = uVar13;
    } while ((int)uVar13 < 0x140);
  }
  iVar5 = FUN_8001178c(1,0x5dd4);
  iVar12 = 0;
  iVar11 = iVar5;
  do {
    iVar8 = 0;
    do {
      cVar3 = (char)iVar8;
      puVar7 = (undefined2 *)(iVar11 + 0x22);
      iVar16 = 0;
      do {
        *(undefined1 *)((int)puVar7 + -0x1f) = 9;
                    /* Possible PsyQ macro: setPolyFT4() */
        *(undefined1 *)((int)puVar7 + -0x1b) = 0x2c;
        puVar7[-6] = (ushort)((int)(iVar16 + 0x280U & 0x3ff) >> 6) | 0x100;
        bVar2 = (byte)iVar16;
        *(undefined1 *)(puVar7 + -0xf) = 0x80;
        *(undefined1 *)((int)puVar7 + -0x1d) = 0x80;
        *(undefined1 *)(puVar7 + -0xe) = 0x80;
        *(byte *)(puVar7 + -0xb) = bVar2 & 0x3f;
        *(char *)((int)puVar7 + -0x15) = cVar3;
        if (iVar16 == 0x130) {
          cVar4 = '?';
        }
        else {
          cVar4 = (bVar2 & 0x3f) + 0x10;
        }
        *(char *)(puVar7 + -7) = cVar4;
        *(char *)((int)puVar7 + -0xd) = cVar3;
        *(byte *)(puVar7 + -3) = bVar2 & 0x3f;
        uVar9 = (undefined1)(iVar8 + 0x10);
        if ((iVar8 == 0xf0) || (iVar8 == 0xe0)) {
          *(char *)((int)puVar7 + -5) = cVar3 + '\x0f';
        }
        else {
          *(undefined1 *)((int)puVar7 + -5) = uVar9;
        }
        if (iVar16 == 0x130) {
          cVar4 = '?';
        }
        else {
          cVar4 = (bVar2 & 0x3f) + 0x10;
        }
        *(char *)(puVar7 + 1) = cVar4;
        if ((iVar8 == 0xf0) || (iVar8 == 0xe0)) {
          *(char *)((int)puVar7 + 3) = cVar3 + '\x0f';
        }
        else {
          *(undefined1 *)((int)puVar7 + 3) = uVar9;
        }
        iVar6 = iVar16 + 0x10;
        puVar7[-0xd] = (short)iVar16;
        puVar7[-0xc] = (short)iVar8;
        puVar7[-9] = (short)iVar6;
        puVar7[-8] = (short)iVar8;
        puVar7[-5] = (short)iVar16;
        uVar10 = (undefined2)(iVar8 + 0x10);
        puVar7[-4] = uVar10;
        puVar7[-1] = (short)iVar6;
        *puVar7 = uVar10;
        puVar7 = puVar7 + 0x14;
        iVar11 = iVar11 + 0x28;
        iVar16 = iVar6;
      } while (iVar6 < 0x140);
      iVar8 = iVar8 + 0x10;
    } while (iVar8 < 0xf0);
    iVar12 = iVar12 + 1;
  } while (iVar12 < 2);
  FUN_8004d524(0xa0,0x78);
  FUN_8004d544(0x100);
  PutDispEnv(&DStack_c0);
  PutDrawEnv((DRAWENV *)(&DAT_8006f208 + iRam00000004 * 0x5c));
  return iVar5;
}

