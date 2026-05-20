// addr: 0x8002cce8  name: FUN_8002cce8

void FUN_8002cce8(int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  undefined2 *puVar6;
  
  uVar5 = 0;
  puVar6 = &DAT_80010534;
  do {
    if ((param_2 >> (uVar5 & 0x1f) & 1U) != 0) {
      if ((int)uVar5 < 6) {
        iVar1 = FUN_8001ac44(DAT_800737dc,*puVar6,0x80,8);
        *(undefined4 *)(iVar1 + 100) = *(undefined4 *)(puVar6 + 2);
      }
      else {
        iVar1 = FUN_8001b038(param_1,0x801f,0x80);
        iVar1 = FUN_8001ac44(*(undefined4 *)(param_1 + 0x58),*(undefined2 *)(iVar1 + 0x1a),0x80,8);
        uVar2 = FUN_8003d1e8(*(undefined1 *)(param_1 + 0xd0));
        *(undefined4 *)(iVar1 + 100) = uVar2;
      }
      *(undefined2 *)(iVar1 + 6) = 0;
      if (*(code **)(iVar1 + 100) != (code *)0x0) {
        (**(code **)(iVar1 + 100))(iVar1,1,0);
      }
      iVar3 = FUN_8003d188(param_1,iVar1);
      if (iVar3 == 0) {
        FUN_80015368("Nowhere to put weapon");
      }
      else {
        FUN_8001b2fc(param_1,iVar3,iVar1);
        iVar3 = 8;
        if (uVar5 != 0) {
          iVar3 = 0;
          if (*(int *)(param_1 + 0x110) != 0) {
            iVar4 = 0x24;
            do {
              iVar4 = iVar4 + 4;
              if (3 < iVar3) break;
              iVar3 = iVar3 + 1;
            } while (*(int *)(param_1 + iVar4 + 0xec) != 0);
          }
          iVar3 = iVar3 + 9;
        }
        *(int *)(param_1 + iVar3 * 4 + 0xec) = iVar1;
      }
    }
    uVar5 = uVar5 + 1;
    puVar6 = puVar6 + 4;
    if (6 < (int)uVar5) {
      return;
    }
  } while( true );
}

