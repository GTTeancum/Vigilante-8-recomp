// addr: 0x80107584  name: FUN_80107584

void FUN_80107584(int param_1,int param_2,int param_3,int param_4)

{
  short sVar1;
  short sVar2;
  short local_20;
  short local_1e;
  short local_1c;
  short local_1a;
  
  sVar1 = (short)param_3;
  local_1c = ((short)param_1 - sVar1) + 1;
  sVar2 = (short)param_2;
  local_1e = sVar2;
  if (param_4 < param_2) {
    local_1e = sVar2 + -1;
  }
  local_1a = 2;
  local_20 = sVar1;
  FUN_8001a0ac/*0x8001a0ac*/(&local_20,0xc0f9);
  local_1e = (short)param_4;
  local_1a = (sVar2 - local_1e) + 1;
  local_20 = sVar1;
  if (param_1 < param_3) {
    local_20 = sVar1 + -1;
  }
  local_1c = 2;
  FUN_8001a0ac/*0x8001a0ac*/(&local_20,0xc0f9);
  return;
}

