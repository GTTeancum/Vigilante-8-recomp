// addr: 0x800507fc  name: SYS_OBJ_15F8

void SYS_OBJ_15F8(void)

{
  undefined2 in_v0;
  int in_t0;
  int unaff_s0;
  int unaff_s1;
  short sStack00000010;
  short sStack00000012;
  ushort in_stack_00000014;
  
  _in_stack_00000014 = CONCAT22(in_v0,in_stack_00000014);
  if (((_sStack00000010 & 0x3f) == 0) && ((in_stack_00000014 & 0x3f) == 0)) {
    *(uint *)(in_t0 * 4 + unaff_s1) =
         (uint)*(byte *)(unaff_s0 + 0x1b) << 0x10 |
         (uint)*(byte *)(unaff_s0 + 0x1a) << 8 | 0x2000000 | (uint)*(byte *)(unaff_s0 + 0x19);
    *(uint *)((in_t0 + 1) * 4 + unaff_s1) = _sStack00000010;
    *(undefined4 *)((in_t0 + 2) * 4 + unaff_s1) = _in_stack_00000014;
    *(char *)(unaff_s1 + 3) = (char)in_t0 + '\x02';
    return;
  }
  _sStack00000010 =
       CONCAT22(sStack00000012 - *(short *)(unaff_s0 + 10),sStack00000010 - *(short *)(unaff_s0 + 8)
               );
  *(uint *)(in_t0 * 4 + unaff_s1) =
       (uint)*(byte *)(unaff_s0 + 0x1b) << 0x10 | (uint)*(byte *)(unaff_s0 + 0x1a) << 8 | 0x60000000
       | (uint)*(byte *)(unaff_s0 + 0x19);
  *(uint *)((in_t0 + 1) * 4 + unaff_s1) = _sStack00000010;
  *(undefined4 *)((in_t0 + 2) * 4 + unaff_s1) = _in_stack_00000014;
  SYS_OBJ_16F0();
  return;
}

