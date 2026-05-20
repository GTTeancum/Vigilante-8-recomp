// addr: 0x80018bd0  name: FUN_80018bd0

void FUN_80018bd0(void *param_1)

{
  *(undefined1 *)((int)param_1 + 3) = 1;
  *(undefined1 *)((int)param_1 + 0xb) = 4;
                    /* Probable PsyQ macro: setDrawTPage() if setlen(p, 1), setDrawMode() if
                       setlen(p, 2). */
  *(undefined4 *)((int)param_1 + 4) = 0xe1000400;
                    /* Possible PsyQ macro: setSprt() + setShadeTex(sprt, 1) */
  *(undefined1 *)((int)param_1 + 0xf) = 0x65;
  MargePrim(param_1,(void *)((int)param_1 + 8));
  return;
}

