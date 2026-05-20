// addr: 0x80045914  name: _spu_FiDMA

/* WARNING: Removing unreachable block (ram,0x80045960) */
/* WARNING: Removing unreachable block (ram,0x80045964) */
/* WARNING: Removing unreachable block (ram,0x80045970) */

void _spu_FiDMA(void)

{
  if (DAT_8005ee24 == 0) {
    _spu_Fw1ts();
  }
  SPU_CTRL_REG_CPUCNT = SPU_CTRL_REG_CPUCNT & 0xffcf;
  if (DAT_8005ee0c == (code *)0x0) {
    DeliverEvent(0xf0000009,0x20);
    return;
  }
  (*DAT_8005ee0c)();
  SPU_OBJ_4EC();
  return;
}

