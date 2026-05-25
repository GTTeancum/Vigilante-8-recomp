/* quest_data.c -- Quest definition file loader and free.
 *
 * Source: SLUS_005.10
 *   FUN_800128d4  Quest_Load()   (~37 instr)
 *   FUN_80012980  Quest_Free()   (~8 instr)
 *
 * FUN_800128d4 (Quest_Load):
 *   Loads "Quest.bin" via FUN_80015f80 (Asset_LoadFileOrDie).
 *   The binary layout is a self-describing array-of-arrays: the first
 *   word gives the outer entry count.  Each outer entry begins with:
 *     [0] outer count  (int, already at *piVar1)
 *     [1] inner count  (int)
 *     [2..] relative offset -> absolute pointer (fixed up to piVar1 base)
 *   The inner fixup adds piVar1 to each piVar3[0] and piVar3[1] pointer
 *   within the sub-array (stride 4 ints = 16 bytes per inner element).
 *   Stores the base pointer in piRam00000608.
 *
 * FUN_80012980 (Quest_Free):
 *   Frees the quest data block via FUN_80045088 if piRam00000608 != NULL.
 *
 * HIGH confidence: standard PSX relocatable-data fixup pattern.
 */
#include <stdint.h>

/* FUN_80015f80: Asset_LoadFileOrDie -- load named file or halt on failure.
 * Returns a pointer to the loaded data.
 * PSX calling convention: path string is passed in a0; the function
 * passes it through to Asset_LoadFromOpened via the a0 register. */
extern void *FUN_80015f80(const char *path);

/* FUN_80045088: Heap_Free -- free a heap block by its handle. */
extern void FUN_80045088(int handle);

/* piRam00000608: pointer to loaded quest data base.
 * NULL if quest data is not loaded. */
extern int *piRam00000608;

/* ================================================================
 * FUN_800128d4 -- Quest_Load
 *
 * Loads "Quest.bin" and performs pointer fixup on the data blob.
 * The file begins with an int[] table: the first word is the number
 * of outer entries.  Each outer entry is a pair (relative-offset, inner-count).
 * The relative offset is converted to an absolute pointer by adding piVar1.
 * Within each sub-array (starting at relative+8), each pair of ints
 * [0] and [1] are similarly made absolute.
 * ================================================================ */
void FUN_800128d4(void)
{
    int *piVar1;
    int  iVar2;
    int *piVar3;
    int  iVar4;
    int *piVar5;
    int *piVar6;
    int  iVar7;

    piVar1 = (int *)FUN_80015f80("Quest.bin");
    piVar6 = piVar1 + 1;
    iVar7  = 0;
    piVar5 = piVar1;
    piRam00000608 = piVar1;
    if (0 < *piVar1) {
        do {
            piVar5  = piVar5 + 2;
            iVar2   = *piVar5;
            *piVar5 = iVar2 + (int)(uintptr_t)piVar1;
            iVar4   = 0;
            if (0 < *piVar6) {
                piVar3 = (int *)((uintptr_t)(iVar2 + (int)(uintptr_t)piVar1) + 8u);
                do {
                    piVar3[1] = piVar3[1] + (int)(uintptr_t)piVar1;
                    *piVar3   = *piVar3   + (int)(uintptr_t)piVar1;
                    iVar4++;
                    piVar3 += 4;
                } while (iVar4 < *piVar6);
            }
            iVar7++;
            piVar6 = piVar6 + 2;
        } while (iVar7 < *piVar1);
    }
}

/* ================================================================
 * FUN_80012980 -- Quest_Free
 *
 * If quest data is loaded (piRam00000608 != NULL), free it and
 * clear the pointer.
 * ================================================================ */
void FUN_80012980(void)
{
    if (piRam00000608 != (int *)0) {
        FUN_80045088((int)(uintptr_t)piRam00000608);
        piRam00000608 = (int *)0;
    }
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_800128d4  (from analysis/SLUS_005.10/decomp/800128d4.c) --- */
// addr: 0x800128d4  name: FUN_800128d4

void FUN_800128d4(void)

{
  int *piVar1;
  int iVar2;
  int *piVar3;
  int iVar4;
  int *piVar5;
  int *piVar6;
  int iVar7;

  piVar1 = (int *)FUN_80015f80("Quest.bin");
  piVar6 = piVar1 + 1;
  iVar7 = 0;
  piVar5 = piVar1;
  piRam00000608 = piVar1;
  if (0 < *piVar1) {
    do {
      piVar5 = piVar5 + 2;
      iVar2 = *piVar5;
      *piVar5 = iVar2 + (int)piVar1;
      iVar4 = 0;
      if (0 < *piVar6) {
        piVar3 = (int *)(iVar2 + (int)piVar1 + 8);
        do {
          piVar3[1] = piVar3[1] + (int)piVar1;
          *piVar3 = *piVar3 + (int)piVar1;
          iVar4 = iVar4 + 1;
          piVar3 = piVar3 + 4;
        } while (iVar4 < *piVar6);
      }
      iVar7 = iVar7 + 1;
      piVar6 = piVar6 + 2;
    } while (iVar7 < *piVar1);
  }
  return;
}

/* --- SLUS_005.10 FUN_80012980  (from analysis/SLUS_005.10/decomp/80012980.c) --- */
// addr: 0x80012980  name: FUN_80012980

void FUN_80012980(void)

{
  if (iRam00000608 != 0) {
    FUN_80045088();
    iRam00000608 = 0;
  }
  return;
}

#endif  /* GHIDRA REF */
