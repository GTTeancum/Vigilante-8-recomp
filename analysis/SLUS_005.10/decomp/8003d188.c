// addr: 0x8003d188  name: FUN_8003d188

undefined4 FUN_8003d188(undefined4 param_1,int param_2)

{
  uint uVar1;
  undefined4 uVar2;
  
  if (*(code **)(param_2 + 100) == (code *)0x0) {
    uVar1 = 0;
  }
  else {
    uVar1 = (**(code **)(param_2 + 100))(param_2,0xe,0);
  }
  if (uVar1 == 0) {
    uVar2 = 0;
  }
  else {
    uVar2 = FUN_8001b038(param_1,uVar1 & 0xffff);
  }
  return uVar2;
}

