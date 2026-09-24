"""Stage the guest mod's runtime dependencies from the prepared development install.
No whole base-game directory is packaged; quest media comes from explicit metadata.
"""
from pathlib import Path
import json, shutil

def stage(root):
    mod=root/'mods/v8_to_v82_guest_roster'
    paths=['SHELL/SOUNDS.SND','SHELL/V8VOICES.SND']
    paths += [f'SHARED/V8VOICE/{outcome}{character:02d}.XA' for outcome in ('D','V') for character in range(13)]
    paths += [c['endingMovie'] for c in json.loads((mod/'quests.json').read_text())['campaigns'] if c.get('endingMovie')]
    if len(paths)!=40: raise RuntimeError(f'Incomplete guest dependencies: {len(paths)}')
    for name in paths:
        source=root/name
        target=mod/'files'/name
        target.parent.mkdir(parents=True,exist_ok=True)
        shutil.copy2(source,target)
    # Authored light palettes are mod assets, too. The executable's palette
    # metadata must be paired with these modified banks in a clean install.
    for name in ('SHELL/VEHICLES.EXP','SHARED/COMMON.EXP'):
        target=root/'mods/enhanced_textures_2x/files'/name
        target.parent.mkdir(parents=True,exist_ok=True)
        shutil.copy2(root/name,target)
    return paths

if __name__=='__main__':
    root=Path(__file__).resolve().parents[2]
    print(json.dumps(stage(root/'V8_2_LOOSE'),indent=2))
