// addr: 0x80018cb8  name: FUN_80018cb8

void FUN_80018cb8(int param_1,int param_2)

{
                    /* Probable PsyQ macro: setDrawTPage() if setlen(p, 1), setDrawMode() if
                       setlen(p, 2). */
  *(uint *)(param_1 + 4) = *(ushort *)(param_2 + 8) & 0x9ff | 0xe1000400;
  *(undefined2 *)(param_1 + 0x16) = *(undefined2 *)(param_2 + 10);
  *(undefined2 *)(param_1 + 0x18) = *(undefined2 *)(param_2 + 2);
  *(undefined2 *)(param_1 + 0x1a) = *(undefined2 *)(param_2 + 4);
  *(undefined2 *)(param_1 + 0x14) = *(undefined2 *)(param_2 + 6);
  return;
}

