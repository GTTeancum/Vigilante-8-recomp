// addr: 0x80101424  name: FUN_80101424

undefined4 FUN_80101424(int param_1,int param_2)

{
  if (param_2 == 1) {
    *(undefined1 **)(param_1 + 100) = &SUB_800223dc;
  }
  return 0;
}

