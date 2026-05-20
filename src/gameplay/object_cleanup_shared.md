# Shared "generic cleanup" idiom across all terrain DLLs

Every terrain DLL contains a function with this exact body
(modulo address):

```
uint32_t Cleanup(void *obj, int mode)
{
    if (mode != 1) {
        if (mode != 7) return 0;
        *(void **)(_DAT_800659fc + 0x58) = obj;
        Object_Pool_Alloc(0x80);   // 0x80-byte FX prim
    }
    Heap_Free(obj);
    return 0xffffffffu;
}
```

Confirmed sites:
| DLL          | Address     | Hand-cleaned name |
|--------------|-------------|-------------------|
| HOOVRDAM.DLL | 0x8010036c  | `HD_TransformerCleanup` |
| CASNOCTY.DLL | 0x80101ca8  | `CC_GenericCleanup`     |
| VALLYFRM.DLL | 0x801002c4  | `VF_GenericCleanup`     |
| OILFIELD.DLL | (TBD)       | (per-overlay byte-identical copy expected) |
| CANYNLND.DLL | (TBD)       | (likewise)              |
| etc.         |             |                         |

## Why ten copies?

V8's overlay loader (`Overlay_LoadAndRelocate`) relocates each DLL
into a new memory region per level. Cross-overlay calls cost a
PSY-Q reloc table entry per call. To avoid the indirection cost,
the DLL build script inlines this 3-line cleanup into every overlay
rather than linking to a single shared copy in the main EXE.

## Pass 3 cleanup target

Once pass 3 starts, hoist a single canonical implementation to
`src/gameplay/object_cleanup.c` and rewrite each per-DLL entry as a
thin wrapper. The bit-exact contract is preserved as long as the
inlined body matches.
