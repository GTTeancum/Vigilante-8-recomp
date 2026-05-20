// addr: 0x8001a2cc  name: FUN_8001a2cc

int * FUN_8001a2cc(RECT *param_1,int param_2,int param_3,int param_4)

{
  short sVar1;
  int *piVar2;
  int iVar3;
  DR_ENV *dr_env;
  DRAWENV DStack_80;
  
  piVar2 = (int *)FUN_800116f4(0x88);
  *piVar2 = (int)param_1->x;
  sVar1 = param_1->y;
  piVar2[2] = param_2;
  piVar2[3] = param_3;
  piVar2[1] = (int)sVar1;
  iVar3 = FUN_80018124((int)param_1->w,(int)param_1->h,0x40,1,0x40,1);
  dr_env = (DR_ENV *)(piVar2 + 0x12);
  piVar2[5] = iVar3;
  ClearOTag((u_long *)dr_env,1);
  SetDefDrawEnv(&DStack_80,(int)*(short *)piVar2[5],(int)((short *)piVar2[5])[1],(int)param_1->w,
                (int)param_1->h);
  DStack_80.dfe = '\x01';
  if (param_4 < 0) {
    iVar3 = FUN_80018124((int)param_1->w,(int)param_1->h,1,1,1,1);
    piVar2[4] = iVar3;
    SetDrawEnv(dr_env,&DStack_80);
    SetDrawMove((DR_MOVE *)(piVar2 + 6),(RECT *)piVar2[5],(int)param_1->x,(int)param_1->y);
    SetDrawMove((DR_MOVE *)(piVar2 + 0xc),(RECT *)piVar2[4],(int)*(short *)piVar2[5],
                (int)((short *)piVar2[5])[1]);
    MargePrim((DR_MOVE *)(piVar2 + 6),(DR_MOVE *)(piVar2 + 0xc));
    piVar2[6] = piVar2[6] & 0xff000000U | (uint)dr_env & 0xffffff;
    MoveImage(param_1,(int)*(short *)piVar2[4],(int)((short *)piVar2[4])[1]);
  }
  else {
    piVar2[4] = 0;
    DStack_80.g0 = (u_char)((uint)param_4 >> 8);
    DStack_80.isbg = '\x01';
    DStack_80.r0 = (u_char)param_4;
    DStack_80.b0 = (u_char)((uint)param_4 >> 0x10);
    SetDrawEnv(dr_env,&DStack_80);
    SetDrawMove((DR_MOVE *)(piVar2 + 6),(RECT *)piVar2[5],(int)param_1->x,(int)param_1->y);
    piVar2[6] = piVar2[6] & 0xff000000U | (uint)dr_env & 0xffffff;
  }
  return piVar2;
}

