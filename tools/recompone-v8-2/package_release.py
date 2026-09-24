"""Build a clean release ZIP from explicit runtime paths, never a loose-game tree."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import zipfile

ROOT = Path(__file__).resolve().parents[2]
MOD_PATHS = {
    'enhanced_textures_2x': ['manifest.json','images/**/*.dds','loading_cards/*.ppm',
        'files/SHELL/VEHICLES.EXP','files/SHARED/COMMON.EXP'],
    'v8_classic_menu': ['manifest.json','README.md','images/**/*.dds','files/**/*'],
    'ttf_game_font': ['mod.json','files/SHARED/GAME.FNT'],
    'v8_to_v82_guest_roster': ['CUSTOM.EXP','VEHICLES.V8R','quests.json','SHELL/SELECTOR_*.PPM','files/**/*'],
    'v82_n64_super_dreamland': ['ARENAS.V8R','files/**/*','loading_cards/*.ppm'],
}
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--exe',type=Path,required=True)
parser.add_argument('--output',type=Path,required=True)
parser.add_argument('--release',default='2026-09-24-rc')
args=parser.parse_args()
out=args.output.resolve()
package=out/'Vigilante8Classic'
package.mkdir(parents=True,exist_ok=False)
shutil.copy2(args.exe,package/'Vigilante8PC.exe')
docs=ROOT/'notes/release'
for f in docs.rglob('*'):
    if f.is_file():
        dest=package/f.relative_to(docs);dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(f,dest)
for mod,patterns in MOD_PATHS.items():
    source=ROOT/'V8_2_LOOSE/mods'/mod
    files=set()
    for pattern in patterns:
        found=[f for f in source.glob(pattern) if f.is_file()]
        if not found:raise RuntimeError(f'Missing runtime mod content: {mod}/{pattern}')
        files.update(found)
    for f in sorted(files):
        relative=f.relative_to(source)
        if any(part.lower() in ('proof','font_work','font_sources','userdata') for part in relative.parts):
            raise RuntimeError(f'Test/production data selected: {f}')
        dest=package/'mods'/mod/relative;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(f,dest)
# A flat glob silently omits nested terrain/route/font atlases. Validate every
# manifest-owned asset path before producing any distributable archive.
def asset_paths(value):
    if isinstance(value,dict):
        for child in value.values():yield from asset_paths(child)
    elif isinstance(value,list):
        for child in value:yield from asset_paths(child)
    elif isinstance(value,str) and value.startswith(('images/','loading_cards/','files/')):
        yield value
for mod in MOD_PATHS:
    for filename in ('manifest.json','mod.json'):
        manifest_file=package/'mods'/mod/filename
        if manifest_file.exists():
            for relative in asset_paths(json.loads(manifest_file.read_text())):
                if not (manifest_file.parent/relative).is_file():
                    raise RuntimeError(f'Missing manifest dependency: {mod}/{relative}')
files=[]
for f in sorted(package.rglob('*')):
    if not f.is_file():continue
    relative=f.relative_to(package).as_posix()
    if f.suffix.lower() in ('.bin','.cue','.sav','.log') or f.name.lower()=='settings.json':
        raise RuntimeError(f'Forbidden release content: {relative}')
    files.append(dict(path=relative,bytes=f.stat().st_size,sha256=hashlib.sha256(f.read_bytes()).hexdigest()))
manifest=dict(format=4,release=args.release,packageRoot=package.name,
    sourceCommit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
    includesUncommittedSetupSource=True,mods=list(MOD_PATHS),
    excludes=['base-game installation','disc images','saves','settings','test proofs','logs'],files=files)
(package/'release-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
archive=out/f'Vigilante8Classic-{args.release}-win-x64.zip'
with zipfile.ZipFile(archive,'x',compression=zipfile.ZIP_DEFLATED,compresslevel=6) as z:
    for f in sorted(package.rglob('*')):
        if f.is_file():z.write(f,f.relative_to(out).as_posix())
with zipfile.ZipFile(archive) as z:
    bad=z.testzip()
    if bad:raise RuntimeError(f'Bad ZIP CRC: {bad}')
digest=hashlib.sha256(archive.read_bytes()).hexdigest()
archive.with_suffix('.zip.sha256').write_text(f'{digest}  {archive.name}\n')
print(json.dumps(dict(archive=str(archive),sha256=digest,files=len(files)+1,
    unpacked_bytes=sum(f['bytes'] for f in files),zip_bytes=archive.stat().st_size),indent=2))
