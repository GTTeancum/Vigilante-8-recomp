// addr: 0x80052d54  name: memchr

void * memchr(uchar *param_1,uchar param_2,int param_3)

{
  void *pvVar1;
  
  if ((param_1 != (uchar *)0x0) && (0 < param_3)) {
    pvVar1 = (void *)MEMCHR_OBJ_20(param_1,param_2,param_3 + -1);
    return pvVar1;
  }
  return (void *)0x0;
}

