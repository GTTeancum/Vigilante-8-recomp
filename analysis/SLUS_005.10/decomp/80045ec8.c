// addr: 0x80045ec8  name: _spu_FgetRXXa

uint _spu_FgetRXXa(int param_1,int param_2)

{
  uint uVar1;
  
  if (param_2 != -1) {
    uVar1 = SPU_OBJ_A28();
    return uVar1;
  }
  return (uint)*(ushort *)((int)&VOICE_00_LEFT_RIGHT + param_1 * 2);
}

