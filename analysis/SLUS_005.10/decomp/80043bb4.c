// addr: 0x80043bb4  name: FUN_80043bb4

byte FUN_80043bb4(void)

{
  byte local_10 [8];
  
  CdControlB('\x01',(u_char *)0x0,local_10);
  return local_10[0] >> 4 & 1;
}

