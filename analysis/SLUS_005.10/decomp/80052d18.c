// addr: 0x80052d18  name: tolower

char tolower(char param_1)

{
  if (((&DAT_80065175)[(byte)param_1] & 1) != 0) {
    param_1 = param_1 + ' ';
  }
  return param_1;
}

