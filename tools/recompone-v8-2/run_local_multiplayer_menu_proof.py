from pathlib import Path
import os, subprocess, json, hashlib, sys, re
from PIL import Image

root=Path(__file__).resolve().parents[2]
keyboard = '--keyboard' in sys.argv
two = '--two' in sys.argv
mode = sys.argv[sys.argv.index('--mode')+1] if '--mode' in sys.argv else 'versus'
if mode not in ('versus','cooperative','quest') or (mode != 'versus' and not two):
    raise ValueError('--mode requires --two and versus, cooperative, or quest')
cancel = '--cancel' in sys.argv
three = '--three' in sys.argv
imports = '--imports' in sys.argv
pause = '--pause' in sys.argv
disconnect = '--disconnect' in sys.argv
quit_match = '--quit' in sys.argv
solo_after = '--solo-after' in sys.argv
coop_result = '--coop-result' in sys.argv
if coop_result and mode == 'versus': raise ValueError('--coop-result requires Cooperative or Quest')
if solo_after and not quit_match: raise ValueError('--solo-after requires --quit')
result_winner = int(sys.argv[sys.argv.index('--result')+1]) if '--result' in sys.argv else None
rematch = '--rematch' in sys.argv
pause_player = int(sys.argv[sys.argv.index('--pause-player')+1]) if '--pause-player' in sys.argv else 3
if pause_player not in (3,4): raise ValueError('extra pause player must be 3 or 4')
out=root/'artifacts/multiplayer-menu-20260914'/('select-split-two-surface' if two else 'select-split-cancel' if cancel else 'select-split-three-final' if three else 'select-split-imports-isolated' if imports else 'select-split-four-paint')
diagnostic = '--diagnostic' in sys.argv
selector_sounds = '--selector-sounds' in sys.argv
if pause: out=out.with_name(out.name+'-pause')
if '--tag' in sys.argv:
    tag=sys.argv[sys.argv.index('--tag')+1]
    if not tag.replace('-','').isalnum(): raise ValueError('tag must be a simple artifact suffix')
    out=out.with_name(out.name+'-'+tag)
if diagnostic: out=out.with_name(out.name+'-diagnostic')
out.mkdir(exist_ok=True)
user_data=out/'user-data'
user_data.mkdir(exist_ok=True)
preserved={}
for name in ('settings.json','interface.ini','carda.sav','cardb.sav'):
    source=root/'V8_2_LOOSE'/name
    if source.exists():
        data=source.read_bytes()
        preserved[name]=hashlib.sha256(data).hexdigest()
        (user_data/name).write_bytes(data)
exe=root/'V8_2_LOOSE/Vigilante82PC.exe'
script='''540+3=START
900+3=START
[after:v82_main_menu]
100+3=DOWN
140+3=P1:PHYS:A
[after:multiplayer_join]
40+3=P1:PHYS:A
80+3=P2:PHYS:A
120+3=P3:PHYS:A
160+3=P4:PHYS:A
240+3=P1:PHYS:START
[choose_location_1]
100+1=NONE
[after:choose_location_1]
160+3=P1:PHYS:A
'''
if two:
    script=script.replace('40+3=P1:PHYS:A\n80+3=P2:PHYS:A\n120+3=P3:PHYS:A\n160+3=P4:PHYS:A\n240+3=P1:PHYS:START', '40+3=P3:PHYS:A\n80+3=P1:PHYS:A\n240+3=P3:PHYS:START')
    script=script.replace('160+3=P1:PHYS:A', '160+3=P3:PHYS:A')
    script+='[after:multiplayer_mode]\n'
    for i in range(('versus','cooperative','quest').index(mode)):
        script+=f'{100+i*60}+3=P3:PHYS:LSDOWN\n'
    script+='240+3=P3:PHYS:A\n'
    if mode != 'versus':
        script+='[after:choose_player]\n300+3=P3:PHYS:A,P1:PHYS:A\n500+3=P3:PHYS:A,P1:PHYS:A\n'
        script+='[after:v82_ready_prompt]\n40+120=P1:CROSS,P2:CROSS,P3:CROSS,P4:CROSS\n'
if three: script=script.replace('160+3=P4:PHYS:A\n','')
script+='[multiplayer_select]\n100+1=NONE\n220+1=NONE\n440+1=NONE\n600+1=NONE\n[after:multiplayer_select]\n'
script+=('160+3=P3:PHYS:LSRIGHT\n260+3=P1:PHYS:B\n340+3=P1:PHYS:LSRIGHT\n' if two else '160+3=P1:PHYS:LSRIGHT,P3:PHYS:LSRIGHT\n260+3=P2:PHYS:B,P4:PHYS:B\n340+3=P2:PHYS:LSRIGHT,P4:PHYS:LSRIGHT\n')
if cancel:
    script+='500+3=P1:PHYS:Y\n[choose_location_2]\n220+1=NONE\n'
else:
    script+=('500+3=P1:PHYS:A\n560+3=P3:PHYS:A\n680+3=P1:PHYS:A\n' if two else '500+3=P2:PHYS:A,P4:PHYS:A\n560+3=P1:PHYS:A,P3:PHYS:A'+('' if three else ',P4:PHYS:A')+'\n680+3=P2:PHYS:A\n')
    script+='[after:multiplayer_match]\n40+240=P1:CROSS,P2:CROSS,P3:CROSS,P4:CROSS\n[gameplay]\n120+1=NONE\n300+1=NONE\n'
if three:
    script=script.replace(',P4:PHYS:B',',P3:PHYS:B').replace(',P4:PHYS:LSRIGHT',',P3:PHYS:LSRIGHT')
    script=script.replace('500+3=P2:PHYS:A,P4:PHYS:A','500+3=P2:PHYS:A,P3:PHYS:A')
    script=script.replace('680+3=P2:PHYS:A', '600+3=P3:PHYS:Y\n620+3=P4:PHYS:A,P4:PHYS:LSRIGHT\n640+3=P3:PHYS:A\n680+3=P2:PHYS:A')
    script=script.replace('600+1=NONE', '600+1=NONE\n625+1=NONE')
if imports:
    script=script[:script.index('[multiplayer_select]')]
    script+='[multiplayer_select]\n700+1=NONE\n900+1=NONE\n[after:multiplayer_select]\n'
    for tick in (160,280,400,520):
        script+=str(tick)+'+3=P1:PHYS:LSLEFT,P2:PHYS:LSLEFT,P3:PHYS:LSLEFT,P4:PHYS:LSLEFT\n'
    script+='740+3=P2:PHYS:B,P4:PHYS:B\n800+3=P2:PHYS:LSRIGHT,P4:PHYS:LSRIGHT\n960+3=P2:PHYS:A,P4:PHYS:A\n1040+3=P1:PHYS:A,P2:PHYS:A,P3:PHYS:A,P4:PHYS:A\n[after:multiplayer_match]\n40+240=P1:CROSS,P2:CROSS,P3:CROSS,P4:CROSS\n[gameplay]\n120+1=NONE\n300+1=NONE\n'
if pause:
    script+=f'[after:gameplay]\n400+3=P{pause_player}:PHYS:START\n520+1=NONE\n620+3=P{pause_player}:PHYS:START\n740+1=NONE\n'
    if disconnect:
        script=script.replace(f'400+3=P{pause_player}:PHYS:START',f'400+200=P{pause_player}:PHYS:DISCONNECTED')
if quit_match:
    # Returning SHELL first animates the replay surface away. The stage signal
    # precedes that transition, so wait for the native menu to accept input.
    script=script.replace('[after:v82_main_menu]\n', '[after:v82_main_menu@1]\n')
    script+=f'[after:gameplay@1]\n400+3=P{pause_player}:PHYS:START\n480+3=P{pause_player}:PHYS:LSLEFT\n540+3=P{pause_player}:PHYS:A\n600+3=P{pause_player}:PHYS:LSLEFT\n640+1=NONE\n680+3=P{pause_player}:PHYS:A\n[after:v82_main_menu@2]\n80+1=NONE\n[after:gameplay@2]\n300+1=NONE\n'
    script+=('[after:v82_main_menu@2]\n340+3=P1:PHYS:A\n440+3=P1:PHYS:A\n' if solo_after else '[after:v82_main_menu@2]\n300+3=DOWN\n340+3=P1:PHYS:A\n')
    script+='[after:choose_location_2]\n160+3=P1:PHYS:A\n'
    if solo_after:
        script+='[after:choose_player]\n300+3=P1:PHYS:A\n500+3=P1:PHYS:A\n[after:choose_enemies]\n300+3=P1:PHYS:A\n[after:v82_ready_prompt]\n40+120=P1:CROSS\n'
if result_winner is not None:
    if rematch:
        script+='[after:multiplayer_results@1]\n350+1=NONE\n420+3=P3:PHYS:B\n[after:multiplayer_results@2]\n350+1=NONE\n420+3=P1:PHYS:A\n'
        if two: script+='[after:v82_ready_prompt@2]\n40+120=P1:CROSS,P3:CROSS\n'
    else:
        script+='[after:multiplayer_results]\n350+1=NONE\n420+3=P1:PHYS:A\n'
if mode != 'versus':
    script+='[after:gameplay]\n1000+1=NONE\n'
    if mode == 'quest':
        # A fresh native Cross edge acknowledges the briefing. Start is
        # intercepted by the match loop and opens the pause menu instead.
        script+='[after:gameplay]\n400+3=P3:CROSS\n'
if coop_result:
    if rematch:
        script+='[after:multiplayer_results@1]\n350+1=NONE\n420+3=P3:PHYS:B\n[after:multiplayer_results@2]\n350+1=NONE\n420+3=P3:PHYS:A\n'
    else: script+='[after:multiplayer_results]\n350+1=NONE\n420+3=P3:PHYS:A\n'
if keyboard:
    # Replace one physical gamepad with actual keyboard key pulses through
    # KeyState/ResolveKeys, never a synthetic native controller image.
    device = 3 if two or three else 4
    keys = {'A':'Z', 'B':'X', 'X':'A', 'Y':'S', 'START':'Enter',
            'LSRIGHT':'Right', 'LSLEFT':'Left', 'LSUP':'Up', 'LSDOWN':'Down'}
    script=re.sub(rf'P{device}:PHYS:([A-Z]+)', lambda m: 'KEY:'+keys[m.group(1)], script)
    script=script.replace(f'P{device}:CROSS', 'KEY:Z')
script=script.replace('[after:multiplayer_join]\n', '[after:multiplayer_join]\n220+1=NONE\n')
(out/'input.txt').write_text(script)
opts=dict(DISABLE_LIVE_INPUT='1',WINDOW_VISIBLE='0',GPU_HLE='1',GRAPHICS_PRESET='Enhanced',MUTE='1',SUPPRESS_RUMBLE='1',UNTHROTTLED='1',DISABLE_SCRIPT_STAGE_CAPTURES='1',DISPLAY_PROBE_IMAGES='0',LOG_PATH=str(out/'runtime.log'),MOD_DIR=str(root/'V8_2_LOOSE/mods'),PRESENTATION_CAPTURE='1',PRESENTATION_RESOLUTION='1280x720',CAPTURE_SCRIPTED_STAGE='*',CAPTURE_DIR=str(out),INPUT_FILE=str(out/'input.txt'),SCRIPT_EXIT_AFTER_POLLS='4500' if imports else '3600',TRACE_NATIVE_OPTIONS='1')
env={k:v for k,v in os.environ.items() if not k.startswith('RECOMPONE_')}
env.update({'RECOMPONE_'+k:v for k,v in opts.items()});env['SDL_AUDIODRIVER']='dummy'
if selector_sounds: env['RECOMPONE_TRACE_AUDIO']='1'
if coop_result:
    env['RECOMPONE_V82_COOP_RESULT_TEST']='1'
    env['RECOMPONE_SCRIPT_EXIT_AFTER_POLLS']='7000' if mode == 'quest' else '6000' if rematch else '4500'
if quit_match: env['RECOMPONE_SCRIPT_EXIT_AFTER_POLLS']='6000'
env['RECOMPONE_USER_DATA_DIR']=str(user_data)
if result_winner is not None:
    env['RECOMPONE_V82_SPLIT_RESULT_TEST']=str(result_winner)
    env['RECOMPONE_SCRIPT_EXIT_AFTER_POLLS']='4500'
    if rematch: env['RECOMPONE_SCRIPT_EXIT_AFTER_POLLS']='5500'
env['RECOMPONE_V82_QUEST_SAVE_PATH']=str(user_data/'quest-progress.json')
with (out/'stdout.log').open('w') as so,(out/'stderr.log').open('w') as se:
    proc=subprocess.Popen([str(exe),'--loose',str(root/'V8_2_LOOSE')],cwd=root/'V8_2_LOOSE',env=env,stdout=so,stderr=se,creationflags=subprocess.CREATE_NO_WINDOW)
    try:
        if diagnostic:
            try: code=proc.wait(timeout=65)
            except subprocess.TimeoutExpired:
                with (out/'stacks.txt').open('w') as stacks:
                    subprocess.run([str(Path.home()/'.dotnet/tools/dotnet-stack.exe'),'report','-p',str(proc.pid)],stdout=stacks,stderr=subprocess.STDOUT,timeout=30,creationflags=subprocess.CREATE_NO_WINDOW)
                code=proc.wait(timeout=90)
        else: code=proc.wait(timeout=300 if quit_match or coop_result or keyboard else 180)
    except subprocess.TimeoutExpired: proc.terminate();proc.wait();code=-1
for p in out.glob('*.ppm'):
    Image.open(p).save(p.with_suffix('.png'));p.unlink()
result=dict(exit_code=code,sha256=hashlib.sha256(exe.read_bytes()).hexdigest().upper(),images=[p.name for p in out.glob('*.png')])
result['user_files_unchanged']=all(hashlib.sha256((root/'V8_2_LOOSE'/name).read_bytes()).hexdigest()==digest for name,digest in preserved.items())
logs=(out/'stderr.log').read_text(errors='replace')
if selector_sounds:
    events=re.findall(r'\[LocalSelectionSound\] sample=(\d+)',logs)
    result['selector_sound_samples']=events
    result['selector_sound_events']=all(str(sample) in events for sample in (0,6,11)) and len(events)<40
    keyed=re.findall(r'\[SPU\] key-on voice=1 start=0x([0-9A-F]+).*pitch=0x([0-9A-F]+).*\n\[LocalSelectionSound\] sample=(\d+)',logs)
    result['selector_sound_keyed']=len(keyed)==len(events) and all(int(start,16)>0 and int(pitch,16)>0 for start,pitch,sample in keyed)
result['six_combatants']=('[V82SplitVerified] combatants=6' in logs)
result['expected_stage']=('choose_location_2' if cancel else 'gameplay')
result['passed']=code==0 and result['user_files_unchanged'] and any(result['expected_stage'] in p for p in result['images']) and 'Host window unavailable' not in logs and (cancel or result['six_combatants'])
if keyboard:
    result['keyboard_roster'] = ('devices=5,1' if two else 'devices=1,2,5' if three else 'devices=1,2,3,5') in logs
    result['passed'] = result['passed'] and result['keyboard_roster']
if mode != 'versus':
    result['mode']=mode
    result['native_mode_verified']=f'pre-match callee state mode={6 if mode == "cooperative" else 7} ' in logs
    result['human_spawns']=len(re.findall(r'vehicle request source=.* variant=-[12]\b',logs))
    result['vehicle_spawns']=len(re.findall(r'vehicle request source=',logs))
    result['simulation_advanced']=any(int(frame)>=300 for frame in re.findall(r'\[V82Gameplay\] frame=(\d+)',logs))
    result['passed']=code==0 and result['user_files_unchanged'] and result['native_mode_verified'] and result['human_spawns']==2 and 2<=result['vehicle_spawns']<=6 and result['simulation_advanced'] and any('aftergameplay_1000_' in p for p in result['images'])
if quit_match:
    result['returned_to_menu']=logs.count("[Input] stage 'v82_main_menu'") >= 2
    result['second_match']=logs.count('[V82SplitVerified] combatants=6') >= 2 and any('aftergameplay2_0300_' in p or 'aftergameplay@2_0300_' in p for p in result['images'])
    if solo_after:
        last=logs.rsplit("[Input] stage 'v82_main_menu'",1)[-1]
        result['solo_human_spawns']=len(re.findall(r'vehicle request source=.* variant=-[12]\b',last))
        result['solo_mode_selected']=bool(re.search(r'\[V82NativeMainMenu\] phase=exit .* result=1 ',last))
        result['second_match']=result['solo_human_spawns']==1 and result['solo_mode_selected'] and any('aftergameplay2_0300_' in p for p in result['images']) and '[V82SplitVerified]' not in last
    result['passed']=result['passed'] and result['returned_to_menu'] and result['second_match']
if result_winner is not None:
    result['winner']=f'winner={result_winner}' in logs
    result['round_survived_first_elimination']='destroyed player=1 tick=180' in logs and 'destroyed player=2 tick=240' in logs
    expected_scores=[0]*(2 if two else 3 if three else 4)
    if result_winner: expected_scores[result_winner-1]=1
    result['first_round_score']='scores='+','.join(map(str,expected_scores)) in logs
    result['results_capture']=any(('aftermultiplayer_results1_0350_' if rematch else 'aftermultiplayer_results_0350_') in p for p in result['images'])
    result['returned_to_menu']=logs.count("[Input] stage 'v82_main_menu'") >= 2
    result['passed']=result['passed'] and result['winner'] and result['results_capture'] and result['returned_to_menu'] and result['first_round_score']
    if result_winner not in (1,2): result['passed']=result['passed'] and result['round_survived_first_elimination']
    if result_winner >= 3:
        result['extra_player_awards']=logs.count('[LocalResultFixture] award applied=') == (2 if rematch else 1)
        result['passed']=result['passed'] and result['extra_player_awards']
    if imports and result_winner:
        result['imported_result_voice']='wrote original V8 XA path=' in logs and 'original V8 XA filter channel=0' in logs and '[V82ResultVoice] native file lookup failed' not in logs
        result['passed']=result['passed'] and result['imported_result_voice']
    if rematch:
        result['rematch']=logs.count('[LocalResult] object=') == 2 and '[LocalRematch] restored humans=' in logs
        scores=[0]*(2 if two else 3 if three else 4)
        if result_winner: scores[result_winner-1]=2
        result['two_round_score']='scores='+','.join(map(str,scores)) in logs
        result['passed']=result['passed'] and result['rematch'] and result['two_round_score']
    if two:
        result['rematch']=not rematch or logs.count('[LocalResult] native2 object=')==2
        result['native_rosters']=len(re.findall(r'vehicle request source=',logs))==6*(2 if rematch else 1)
        result['passed']=code==0 and result['user_files_unchanged'] and result['six_combatants'] and result['winner'] and result['first_round_score'] and result['results_capture'] and result['returned_to_menu'] and result['rematch'] and result['native_rosters'] and (not rematch or result['two_round_score'])
if pause:
    result['pause_owner']=pause_player
    result['pause_resume']=f'[LocalPause] state={0x28+pause_player:08X} owner={pause_player}' in logs and '[LocalPause] state=00000000 owner=0' in logs
    result['paused_and_resumed_captures']=all(any(f'aftergameplay_{tick:04d}_' in p for p in result['images']) for tick in (520,740))
    resumed=re.search(r'\[LocalPause\] state=00000000 owner=0 tick=(\d+)',logs)
    result['simulation_resumed']=bool(resumed and any(int(t)>int(resumed.group(1))+30 for t in re.findall(r'\[V82SplitState\] tick=(\d+)',logs[resumed.end():])))
    result['passed'] = result['passed'] and result['pause_resume'] and result['paused_and_resumed_captures'] and result['simulation_resumed']
    if disconnect:
        lost=logs.find(f'[LocalController] player={pause_player} connected=False')
        restored=logs.find(f'[LocalController] player={pause_player} connected=True',lost+1)
        result['device_reconnected']=lost >= 0 and restored > lost
        result['passed']=result['passed'] and result['device_reconnected']
    if three:
        # The unused quadrant must be black after a full-display modal closes.
        # Leave a small border for presentation filtering at viewport edges.
        resumed_image=out/'recompone_present_aftergameplay_0740_1280x720_fxaa.png'
        result['unused_viewport_clear']=resumed_image.exists() and all(hi <= 2 for lo,hi in Image.open(resumed_image).convert('RGB').crop((650,370,1108,708)).getextrema())
        result['passed'] = result['passed'] and result['unused_viewport_clear']
if selector_sounds: result['passed']=result['passed'] and result['selector_sound_events'] and result['selector_sound_keyed']
if coop_result:
    result['native_result']='[LocalCoopResult]' in logs
    result['continued']='[V82ShellReturn]' in logs
    result['passed']=code==0 and result['user_files_unchanged'] and result['native_result'] and result['continued']
    if rematch:
        first=logs.split('[LocalCoopResult]',1)[0]
        result['coop_rematch']=logs.count('[LocalCoopResult]')==2 and len(re.findall(r'vehicle request source=',logs))==2*len(re.findall(r'vehicle request source=',first))
        result['passed']=result['passed'] and result['coop_rematch']
(out/'result.json').write_text(json.dumps(result,indent=2));print(json.dumps(result))



sys.exit(0 if result["passed"] else 1)
