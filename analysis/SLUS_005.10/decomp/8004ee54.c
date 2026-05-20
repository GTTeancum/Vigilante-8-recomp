// addr: 0x8004ee54  name: _patch_gte

void _patch_gte(void)

{
  int iVar1;
  int *piVar2;
  int *piVar3;
  int *piVar4;
  code *pcVar5;
  int iVar6;
  undefined4 unaff_retaddr;
  
  DAT_800a32e8 = unaff_retaddr;
  FUN_80053a24();
  iVar1 = (*(code *)&SUB_000000b0)();
  piVar2 = (int *)(*(int *)(iVar1 + 0x18) + 0x28);
  piVar4 = &PATCHGTE_OBJ_AC;
  piVar3 = piVar2;
  do {
    iVar1 = *piVar4;
    iVar6 = *piVar3;
    piVar4 = piVar4 + 1;
    piVar3 = piVar3 + 1;
    if (iVar1 != iVar6) goto PATCHGTE_OBJ_88;
  } while (piVar4 != &PATCHGTE_OBJ_C4);
  pcVar5 = (code *)&PATCHGTE_OBJ_C4;
  do {
    *piVar2 = *(int *)pcVar5;
    pcVar5 = pcVar5 + 4;
    piVar2 = piVar2 + 1;
  } while (pcVar5 != PATCHGTE_OBJ_DC);
PATCHGTE_OBJ_88:
  FlushCache();
  FUN_80053a34();
  return;
}

