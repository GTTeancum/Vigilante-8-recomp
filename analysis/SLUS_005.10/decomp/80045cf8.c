// addr: 0x80045cf8  name: _spu_Fw

undefined4 _spu_Fw(undefined4 param_1,undefined4 param_2)

{
  undefined4 uVar1;
  
  if (DAT_8005edf0 == 0) {
    _spu_t(2,(uint)DAT_8005edec << (DAT_8005edfc & 0x1f));
    _spu_t(1);
    _spu_t(3,param_1,param_2);
    uVar1 = SPU_OBJ_894();
    return uVar1;
  }
  SPU_OBJ_280(param_1,param_2);
  return param_2;
}

