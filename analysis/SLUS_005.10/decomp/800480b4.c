// addr: 0x800480b4  name: DMACallback

void DMACallback(void)

{
  (*DAT_8005ff58)();
  return;
}

