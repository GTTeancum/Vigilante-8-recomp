from pathlib import Path
import os, subprocess, json, hashlib, sys, shutil, argparse
from PIL import Image
root=Path(__file__).resolve().parents[2]
parser=argparse.ArgumentParser(description="Process-local GUI setup and installed-launch smoke; no desktop input")
parser.add_argument('mode',choices=['first-run','relaunch','guest-relaunch','guest-baseline','wrong-title','missing-bin'])
parser.add_argument('--base',type=Path,default=root/'artifacts/setup-20260915')
parser.add_argument('--install',type=Path)
parser.add_argument('--source',type=Path,default=root/'Vigilante 8 - 2nd Offense (USA) (Track 01).bin')
args=parser.parse_args()
base=args.base.resolve()
install=(args.install or base/'user-install').resolve()
mode=args.mode
out=base/mode
out.mkdir(parents=True,exist_ok=False)
if mode=='guest-baseline': install=root/'V8_2_LOOSE'
if mode in ('wrong-title','missing-bin'):
 install=out/'install'
 install.mkdir(exist_ok=True)
 shutil.copy2((args.install or base/'user-install')/'Vigilante8PC.exe',install/'Vigilante8PC.exe')
script='''1+2=LEFT
480+2=LEFT
540+3=START
900+3=START
[after:v82_main_menu]
100+1=NONE
340+3=P1:PHYS:A
440+3=P1:PHYS:A
[after:choose_location_1]
100+1=NONE
160+3=P1:PHYS:A
[after:choose_player]
200+1=NONE
300+3=P1:PHYS:A
500+3=P1:PHYS:A
[after:choose_enemies]
300+3=P1:PHYS:A
[after:v82_ready_prompt]
40+120=P1:CROSS
[gameplay]
120+1=NONE
300+1=NONE
'''
if mode in ('guest-relaunch','guest-baseline'):
 script=script.replace('[after:choose_player]\n200+1=NONE\n300+3=P1:PHYS:A\n500+3=P1:PHYS:A', '[after:choose_player]\n80+3=P1:PHYS:LSLEFT\n140+3=P1:PHYS:LSLEFT\n200+3=P1:PHYS:LSLEFT\n260+3=P1:PHYS:LSLEFT\n340+1=NONE\n400+3=P1:PHYS:A\n600+3=P1:PHYS:A')
(out/'input.txt').write_text(script)
env={k:v for k,v in os.environ.items() if not k.startswith('RECOMPONE_')}
opts=dict(DISABLE_LIVE_INPUT='1',WINDOW_VISIBLE='0',GPU_HLE='1',GRAPHICS_PRESET='Enhanced',SUPPRESS_RUMBLE='1',UNTHROTTLED='1',DISABLE_SCRIPT_STAGE_CAPTURES='1',DISPLAY_PROBE_IMAGES='0',LOG_PATH=str(out/'runtime.log'),PRESENTATION_CAPTURE='1',PRESENTATION_RESOLUTION='1280x720',CAPTURE_SCRIPTED_STAGE='*',CAPTURE_DIR=str(out),INPUT_FILE=str(out/'input.txt'),SCRIPT_EXIT_AFTER_POLLS='4000',SETUP_TEST_SOURCE=str(args.source.resolve()),SETUP_CAPTURE_DIR=str(out),AUDIO_CAPTURE=str(out/'audio.wav'))
env.update({'RECOMPONE_'+k:v for k,v in opts.items()});env['SDL_AUDIODRIVER']='dummy'
if mode in ('relaunch','guest-relaunch'): env.pop('RECOMPONE_SETUP_TEST_SOURCE')
if mode=='guest-baseline':
 env.pop('RECOMPONE_SETUP_TEST_SOURCE')
 env['RECOMPONE_USER_DATA_DIR']=str(out/'userdata')
if mode=='wrong-title': env['RECOMPONE_SETUP_TEST_SOURCE']=str(root/'BINCUE/Vigilante 8 (USA).cue')
if mode=='missing-bin':
 source=out/'missing.cue'
 source.write_text('FILE "missing.bin" BINARY\n  TRACK 01 MODE2/2352\n    INDEX 01 00:00:00\n')
 env['RECOMPONE_SETUP_TEST_SOURCE']=str(source)
with (out/'stdout.log').open('w') as so,(out/'stderr.log').open('w') as se:
 p=subprocess.Popen([str(install/'Vigilante8PC.exe')],cwd=install,env=env,stdout=so,stderr=se,creationflags=subprocess.CREATE_NO_WINDOW)
 (out/'pid.txt').write_text(str(p.pid))
 try: code=p.wait(timeout=1200)
 except subprocess.TimeoutExpired: p.terminate();p.wait();code=-1
for f in out.glob('*.ppm'):
 Image.open(f).save(f.with_suffix('.png'));f.unlink()
result=dict(exit_code=code,exe_sha256=hashlib.sha256((install/'Vigilante8PC.exe').read_bytes()).hexdigest(),images=[f.name for f in out.glob('*.png')])
logs=(out/'stderr.log').read_text(errors='replace')+(out/'stdout.log').read_text(errors='replace')
if mode in ('wrong-title','missing-bin'):
 result['checks']={'clean_exit':code==0,'error_shown':'setup-error.png' in result['images'],'no_completed_install':not (install/'game_data/.recompone-import-complete').exists()}
else:
 result['checks']={'clean_exit':code==0,'standalone_installed_data':f'[Host] standalone-loose={install / "game_data"}' in logs,'enhanced_textures_active':'2x texture replacements=On' in logs,'gameplay_capture':any('gameplay_0300' in f for f in result['images']),'no_fatal':not any(x in logs.lower() for x in ('unhandled exception','[fatal]','[v82fatal]'))}
 if mode in ('relaunch','guest-relaunch'): result['checks']['no_setup_prompt']='[Setup]' not in logs and not any(f.startswith('setup-') for f in result['images'])
 else: result['checks']['setup_handoff']='[Setup] complete 144/144' in logs
 if mode=='guest-relaunch': result['checks']['guest_voice_loaded']='[V82SelectionVoice] guest=' in logs
result['automated_checks_passed']=all(result['checks'].values())
(out/'result.json').write_text(json.dumps(result,indent=2));print(json.dumps(result))
sys.exit(0 if result['automated_checks_passed'] else 1)
