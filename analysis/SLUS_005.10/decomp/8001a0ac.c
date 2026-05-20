// addr: 0x8001a0ac  name: FUN_8001a0ac

void FUN_8001a0ac(undefined2 *param_1,undefined4 param_2)

{
  undefined1 auStack_18 [3];
  undefined1 local_15;
  undefined1 local_14;
  undefined1 local_13;
  undefined1 local_12;
  undefined1 local_11;
  undefined2 local_10;
  undefined2 local_e;
  undefined2 local_c;
  undefined2 local_a;
  
  local_15 = 3;
  local_11 = 0x60;
  local_14 = (undefined1)param_2;
  local_13 = (undefined1)((uint)param_2 >> 8);
  local_12 = (undefined1)((uint)param_2 >> 0x10);
  local_10 = *param_1;
  local_e = param_1[1];
  local_c = param_1[2];
  local_a = param_1[3];
  DrawPrim(auStack_18);
  return;
}

