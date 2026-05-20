// addr: 0x80052ce4  name: toupper

char toupper(char param_1)

{
  if (((&DAT_80065175)[(byte)param_1] & 2) != 0) {
    param_1 = param_1 + -0x20;
  }
  return param_1;
}

