"""Exercise the generated native allocator, with only host malloc/free stubs."""
from pathlib import Path
import sys
import struct

root = Path(__file__).resolve().parents[4]
exe = (root / 'V8_2_LOOSE/SLUS_008.68').read_bytes()
base = struct.unpack_from('<I', exe, 0x18)[0]
assert struct.unpack_from('<5I', exe, 0x800 + 0x80010598 - base) == (
    0x80020B04, 0x80020CD4, 0x80020D00, 0x80020D50, 0x80020D50)
s = (root / 'reference-v8-2/generated/recompiled/main.cs').read_text()
a = s.index('    public static void func_80020700(')
b = s.index('    public static void func_80020FF8(')
s = s[a:b]
s = s[:s.rfind('    [System.Runtime')]
s = s.replace('        c.S0 = RecompOne.Runtime.Sdk.V82Compat.SelectMatchVramFreeLeaf(c, m, c.S0);\n', '')
needle = '        c.LoadWord(16, m, (c.GP + 0xE80u));'
assert s.count(needle) == 1
s = s.replace(needle, needle + '\n        if (Improved) c.S0 = RecompOne.Runtime.Sdk.NativeVramPlacement.FindFreeLeaf(c,m,c.S0);')
output = Path(sys.argv[1]); output.parent.mkdir(parents=True, exist_ok=True)
output.write_text('using RecompOne.Runtime.Context;\nusing RecompOne.Runtime.Memory;\nusing RecompOne.Runtime.Dispatch;\npublic static class Vigilante82PC {\n' + s + '''
public static uint Heap=0x80100000;
public static int Failures;
public static bool Improved;
public static readonly HashSet<uint> Live=new();
public static void func_80052060(CpuContext c,IMemory m){ c.V0=Heap; Heap+=(c.A0+7)&~7u; Live.Add(c.V0); }
public static void func_800520D8(CpuContext c,IMemory m){ if(!Live.Remove(c.A0))throw new Exception($"Invalid native free {c.A0:X8}"); }
public static void func_800206A8(CpuContext c,IMemory m){}
public static void func_80015D9C(CpuContext c,IMemory m){ Failures++; }
}
''')
