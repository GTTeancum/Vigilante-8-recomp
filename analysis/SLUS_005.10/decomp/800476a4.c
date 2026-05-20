// addr: 0x800476a4  name: PCcreat

int PCcreat(char *name,int perms)

{
  int in_v0;
  int in_v1;
  
  trap(0x102);
  if (in_v0 != 0) {
    in_v1 = -1;
  }
  return in_v1;
}

