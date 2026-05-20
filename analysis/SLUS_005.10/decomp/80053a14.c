// addr: 0x80053a14  name: HookEntryInt

void HookEntryInt(void)

{
  (*(code *)&SUB_000000b0)();
  return;
}

