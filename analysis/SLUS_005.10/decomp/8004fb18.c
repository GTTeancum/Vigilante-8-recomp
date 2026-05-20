// addr: 0x8004fb18  name: DrawPrim

void DrawPrim(void *p)

{
  undefined1 uVar1;
  
  uVar1 = *(undefined1 *)((int)p + 3);
  SYS_OBJ_2850(0);
  SYS_OBJ_2114((int)p + 4,uVar1);
  return;
}

