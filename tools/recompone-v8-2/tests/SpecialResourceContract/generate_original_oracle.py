"""Extract the original instruction-level projectile as a differential test oracle."""
from pathlib import Path
import re, hashlib

ROOT=Path(__file__).resolve().parents[4]
source=(ROOT/'reference/generated/recompiled/main.cs').read_text(encoding='utf-8-sig')
body=re.search(r'    public static void LAB_800359c0\(CpuContext c, IMemory m\)\n    \{.*?\n    \}',source,re.S).group()
digest=hashlib.sha256(body.encode()).hexdigest()
body=re.sub(r'Vigilante8PC\.FUN_([0-9a-f]{8})\(c, m\);',r'Dispatcher.Call(c, m, 0x\1u);',body)
Path(__file__).with_name('OriginalProjectileOracle.cs').write_text(
    '// Generated from original SLUS_005.10, not from the port under test.\n'
    f'// Source body SHA256: {digest}\n'
    'using RecompOne.Runtime.Context;\nusing RecompOne.Runtime.Dispatch;\nusing RecompOne.Runtime.Memory;\n'
    'static class OriginalProjectileOracle\n{\n'+body+'\n}\n')

parts=[]
for name in ['FUN_80030c08','FUN_80017324','FUN_800439b8']:
    body=re.search(r'    public static void '+name+r'\(CpuContext c, IMemory m\)\n    \{.*?\n    \}',source,re.S).group()
    body=re.sub(r'Vigilante8PC\.FUN_([0-9a-f]{8})\(c, m\);',r'Dispatcher.Call(c, m, 0x\1u);',body)
    body=body.replace('Vigilante8PC.MatrixNormal(c, m);','Dispatcher.Call(c, m, 0x80059A0Cu);')
    parts.append(body)
Path(__file__).with_name('OriginalFlightOracle.cs').write_text(
    '// Generated original V8 instructions; MatrixNormal links to the shared native PSYQ service.\n'
    'using RecompOne.Runtime.Context;\nusing RecompOne.Runtime.Dispatch;\nusing RecompOne.Runtime.Memory;\n'
    'static class OriginalFlightOracle\n{\n'+'\n'.join(parts)+'\n}\n')
