// addr: 0x8001e1c0  name: FUN_8001e1c0

bool FUN_8001e1c0(int *param_1,int param_2,int *param_3,MATRIX *param_4)

{
  bool bVar1;
  MATRIX MStack_68;
  int local_48;
  int local_44;
  int local_40;
  int local_38;
  int local_34;
  int local_30;
  int local_28;
  int local_24;
  int local_20;
  
  local_48 = param_4->t[0] - *(int *)(param_2 + 0x14);
  local_44 = param_4->t[1] - *(int *)(param_2 + 0x18);
  local_40 = param_4->t[2] - *(int *)(param_2 + 0x1c);
  local_38 = (*param_3 + param_3[3]) / 2;
  local_34 = (param_3[1] + param_3[4]) / 2;
  local_30 = (param_3[2] + param_3[5]) / 2;
  local_28 = (param_3[3] - *param_3) / 2;
  local_24 = (param_3[4] - param_3[1]) / 2;
  local_20 = (param_3[5] - param_3[2]) / 2;
  FUN_8004366c();
  FUN_80043248(&local_48,&local_48);
  MulRotMatrix0(param_4,&MStack_68);
  FUN_80043358(&MStack_68,&local_38,&local_38);
  local_38 = local_38 + local_48;
  local_34 = local_34 + local_44;
  local_30 = local_30 + local_40;
  FUN_800436c8(&MStack_68);
  FUN_80043248(&local_28,&local_28);
  bVar1 = false;
  if ((((local_38 - local_28 <= param_1[3]) && (*param_1 <= local_38 + local_28)) &&
      (local_34 - local_24 <= param_1[4])) &&
     ((param_1[1] <= local_34 + local_24 && (local_30 - local_20 <= param_1[5])))) {
    bVar1 = param_1[2] <= local_30 + local_20;
  }
  return bVar1;
}

