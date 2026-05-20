// addr: 0x8004d4e4  name: SetBackColor

void SetBackColor(long rbk,long gbk,long bbk)

{
  gte_ldbkdir(rbk << 4,gbk << 4,bbk << 4);
  return;
}

