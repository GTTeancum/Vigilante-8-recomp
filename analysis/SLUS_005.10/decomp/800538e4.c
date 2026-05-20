// addr: 0x800538e4  name: setjmp

int setjmp(__jmp_buf_tag *__env)

{
  int unaff_s0;
  int unaff_s1;
  int unaff_s2;
  int unaff_s3;
  ulong unaff_s4;
  ulong unaff_s5;
  ulong unaff_s6;
  ulong unaff_s7;
  int unaff_s8;
  int unaff_retaddr;
  undefined1 auStackX_0 [16];
  
  __env->__jmpbuf[0] = unaff_retaddr;
  (__env->__saved_mask).__val[4] = 0;
  __env->__jmpbuf[1] = (int)register0x00000074;
  __env->__jmpbuf[2] = unaff_s8;
  __env->__jmpbuf[3] = unaff_s0;
  __env->__jmpbuf[4] = unaff_s1;
  __env->__jmpbuf[5] = unaff_s2;
  __env->__mask_was_saved = unaff_s3;
  (__env->__saved_mask).__val[0] = unaff_s4;
  (__env->__saved_mask).__val[1] = unaff_s5;
  (__env->__saved_mask).__val[2] = unaff_s6;
  (__env->__saved_mask).__val[3] = unaff_s7;
  return 0;
}

