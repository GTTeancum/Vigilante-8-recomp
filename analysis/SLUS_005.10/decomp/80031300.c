// addr: 0x80031300  name: FUN_80031300

undefined4 * FUN_80031300(int param_1,int param_2,ushort param_3,undefined4 param_4,int param_5)

{
  undefined2 uVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  
  iVar2 = FUN_8001b038(param_2,0x8000);
  if ((int)((uint)param_3 << 0x10) < 0) {
    puVar3 = (undefined4 *)FUN_8001d470(param_4);
  }
  else {
    puVar3 = (undefined4 *)FUN_8001ac44(*(undefined4 *)(param_2 + 0x58),param_3,param_4,8);
  }
  puVar3[0x20] = param_1;
  *puVar3 = 0x800000;
  uVar1 = *(undefined2 *)(param_1 + 6);
  *(undefined1 *)(puVar3 + 1) = 7;
  *(undefined2 *)((int)puVar3 + 6) = uVar1;
  if (iVar2 == 0) {
    puVar4 = (undefined4 *)FUN_8001d624(param_2);
    uVar5 = puVar4[1];
    uVar6 = puVar4[2];
    uVar7 = puVar4[3];
    puVar3[4] = *puVar4;
    puVar3[5] = uVar5;
    puVar3[6] = uVar6;
    puVar3[7] = uVar7;
    uVar5 = puVar4[5];
    uVar6 = puVar4[6];
    uVar7 = puVar4[7];
    puVar3[8] = puVar4[4];
    puVar3[9] = uVar5;
    puVar3[10] = uVar6;
    puVar3[0xb] = uVar7;
  }
  else {
    FUN_8001d68c(puVar3 + 4,param_2,iVar2);
  }
  puVar3[0x12] = puVar3[9];
  puVar3[0x13] = puVar3[10];
  puVar3[0x14] = puVar3[0xb];
  if (param_5 != 0) {
    FUN_8001b2fc(param_2,iVar2,param_5);
  }
  return puVar3;
}

