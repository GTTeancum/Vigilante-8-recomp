// addr: 0x80048118  name: VSyncCallbacks

void VSyncCallbacks(void)

{
  (*DAT_8005ff68)();
  return;
}

