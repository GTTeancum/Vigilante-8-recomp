// addr: 0x80052274  name: AddPrim

void AddPrim(void *ot,void *p)

{
                    /* Probable PsyQ macro: addPrim(). */
  *(uint *)p = *(uint *)p & 0xff000000 | *(uint *)ot & 0xffffff;
  *(uint *)ot = *(uint *)ot & 0xff000000 | (uint)p & 0xffffff;
  return;
}

