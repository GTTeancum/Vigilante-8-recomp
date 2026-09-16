"""Reproduce the Arcade guest VRAM regression with process-local input only.

Uses isolated settings and the installed HD mods. Inspect the two native
gameplay captures after the log checks; successful rendering alone is not QA.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--install', type=Path, default=ROOT/'V8_2_LOOSE')
parser.add_argument('--output', type=Path, required=True)
parser.add_argument('--roster', default='1,8,6,7')
args = parser.parse_args()
install, output = args.install.resolve(), args.output.resolve()
output.mkdir(parents=True, exist_ok=False)
exe = install/'Vigilante8PC.exe'
fixture = output/'input.txt'
fixture.write_text('''1+2=LEFT
480+2=LEFT
540+3=START
900+3=START
[after:v82_main_menu]
100+1=NONE
340+3=P1:PHYS:A
440+3=P1:PHYS:A
[after:choose_location_1]
160+3=P1:PHYS:A
[after:choose_player]
80+3=P1:PHYS:LSLEFT
140+3=P1:PHYS:LSLEFT
200+3=P1:PHYS:LSLEFT
260+3=P1:PHYS:LSLEFT
400+3=P1:PHYS:A
600+3=P1:PHYS:A
[after:choose_enemies]
300+3=P1:PHYS:A
[after:v82_ready_prompt]
40+120=P1:CROSS
[gameplay]
120+1=NONE
300+1=NONE
''')
userdata = output/'userdata'
userdata.mkdir()
(userdata/'settings.json').write_text(json.dumps({
    'V82CheatFlags':1048640, 'InputProfile':'Modern', 'V82Transformations':0}))
options = dict(DISABLE_LIVE_INPUT='1', WINDOW_VISIBLE='0', GPU_HLE='1',
    GRAPHICS_PRESET='Enhanced', SUPPRESS_RUMBLE='1', UNTHROTTLED='1',
    DISABLE_SCRIPT_STAGE_CAPTURES='1', DISPLAY_PROBE_IMAGES='0',
    LOG_PATH=str(output/'runtime.log'), PRESENTATION_CAPTURE='1',
    PRESENTATION_RESOLUTION='1280x720', CAPTURE_SCRIPTED_STAGE='gameplay',
    CAPTURE_DIR=str(output), INPUT_FILE=str(fixture), SCRIPT_EXIT_AFTER_POLLS='3300',
    USER_DATA_DIR=str(userdata), V82_TEST_NATIVE_NPC_TYPES=args.roster,
    TRACE_VRAM_PACKING='1')
env = {k:v for k,v in os.environ.items() if not k.startswith('RECOMPONE_')}
env.update({'RECOMPONE_'+k:v for k,v in options.items()})
env['SDL_AUDIODRIVER'] = 'dummy'
with (output/'stdout.log').open('w') as stdout, (output/'stderr.log').open('w') as stderr:
    result = subprocess.run([str(exe)], cwd=install, env=env, stdout=stdout,
        stderr=stderr, timeout=600, creationflags=subprocess.CREATE_NO_WINDOW)
for frame in output.glob('*.ppm'):
    Image.open(frame).save(frame.with_suffix('.png'))
    frame.unlink()
log = (output/'stderr.log').read_text()
checks = dict(exit_zero=result.returncode == 0,
    expected_roster=f'types=74,-1,{args.roster}' in log,
    no_vram_failures='Out of VRAM' not in log and '[V82VRAM] failure' not in log,
    sharing_exercised='[V82ImageSharing] reused=' in log,
    captures=len(list(output.glob('*gameplay*.png'))) == 2)
report = dict(executable_sha256=hashlib.sha256(exe.read_bytes()).hexdigest(),
    install=str(install), roster=args.roster, checks=checks,
    visual_review='Required: inspect captured vehicle, HUD, terrain and objects.')
(output/'result.json').write_text(json.dumps(report, indent=2)+'\n')
print(json.dumps(report, indent=2))
if not all(checks.values()):
    raise SystemExit(1)
