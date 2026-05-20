// addr: 0x80045d7c  name: _spu_Fr

undefined4 _spu_Fr(undefined4 param_1,undefined4 param_2)

{
  _spu_t(2,(uint)DAT_8005edec << (DAT_8005edfc & 0x1f));
  _spu_t(0);
  _spu_t(3,param_1,param_2);
  return param_2;
}

