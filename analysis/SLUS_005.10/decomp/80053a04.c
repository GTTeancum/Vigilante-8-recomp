// addr: 0x80053a04  name: ResetEntryInt

void ResetEntryInt(void)

{
  (*(code *)&SUB_000000b0)();
  return;
}

