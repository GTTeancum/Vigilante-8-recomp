// addr: 0x8004d504  name: SetFarColor

void SetFarColor(long rfc,long gfc,long bfc)

{
  gte_ldfcdir(rfc << 4,gfc << 4,bfc << 4);
  return;
}

