// addr: 0x80019960  name: FUN_80019960

void FUN_80019960(int *param_1,char *param_2,undefined4 param_3,int param_4)

{
  char cVar1;
  undefined4 uVar2;
  char *pcVar3;
  undefined1 auStack_40 [32];
  
  cVar1 = *param_2;
  uVar2 = param_3;
  while (cVar1 != '\0') {
    pcVar3 = param_2 + 1;
    if (cVar1 == '\n') {
      param_4 = param_4 + (uint)*(byte *)(*param_1 + 7);
      uVar2 = param_3;
    }
    else if (cVar1 == '\x01') {
      *(char *)(param_1 + 1) = *pcVar3;
      *(char *)((int)param_1 + 5) = param_2[2];
      pcVar3 = param_2 + 4;
      *(char *)((int)param_1 + 6) = param_2[3];
    }
    else {
      DrawSync(0);
      uVar2 = FUN_80019370(param_1,auStack_40,cVar1,uVar2,param_4);
      DrawPrim(auStack_40);
    }
    param_2 = pcVar3;
    cVar1 = *pcVar3;
  }
  return;
}

