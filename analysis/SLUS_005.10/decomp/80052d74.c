// addr: 0x80052d74  name: MEMCHR_OBJ_20

undefined4 MEMCHR_OBJ_20(char *param_1,char param_2,int param_3)

{
  char cVar1;
  undefined4 uVar2;
  
  while( true ) {
    if (param_3 < 0) {
      return 0;
    }
    cVar1 = *param_1;
    param_1 = param_1 + 1;
    if (cVar1 == param_2) break;
    param_3 = param_3 + -1;
  }
  uVar2 = MEMCHR_OBJ_48();
  return uVar2;
}

