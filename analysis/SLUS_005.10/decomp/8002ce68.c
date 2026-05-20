// addr: 0x8002ce68  name: FUN_8002ce68

void FUN_8002ce68(uint *param_1,int param_2)

{
  undefined2 uVar1;
  int iVar2;
  int iVar3;
  uint *puVar4;
  
  puVar4 = (uint *)param_1[*(byte *)((int)param_1 + 0xb3) + 0x44];
  if (puVar4 != (uint *)0x0) {
    if (((param_2 == 0) || ((short)puVar4[3] == 0)) || (*(short *)((int)puVar4 + 6) != 0)) {
      if ((code *)puVar4[0x19] != (code *)0x0) {
        (*(code *)puVar4[0x19])(puVar4,0,param_1);
      }
    }
    else {
      if ((code *)puVar4[0x19] == (code *)0x0) {
        *(undefined2 *)((int)puVar4 + 6) = 0;
      }
      else {
        uVar1 = (*(code *)puVar4[0x19])(puVar4,0xb,param_1);
        *(undefined2 *)((int)puVar4 + 6) = uVar1;
      }
      if ((0 < *(short *)((int)param_1 + 6)) && ((*puVar4 & 0x40000) == 0)) {
        if ((*param_1 & 0x40000) == 0) {
          iVar3 = (2 - cRam00000016) * 0x40;
        }
        else {
          iVar3 = (2 - cRam00000016) * 0x20;
        }
        iVar2 = FUN_80017160();
        *(short *)((int)puVar4 + 6) =
             *(short *)((int)puVar4 + 6) + (short)iVar3 + (short)(iVar2 * iVar3 >> 0xf);
      }
    }
  }
  return;
}

