#!/usr/bin/env python3
"""Capture native carousel audio with process-local input and a silent SDL sink."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import wave


def native_two_player_checks(log, loose, minimum_visits=1):
    from verify_selector_voice_capture import parse_bank
    pitches = {entry.pitch for bank in (loose / 'SHELL').glob('BEGIN*.SND')
               for entry in parse_bank(bank)}
    events = []
    visits = 0
    active = False
    for line in log.splitlines():
        stage = re.search(r"\[Input\] stage '([^']+)'", line)
        if stage:
            active = stage[1] == 'choose_player'
            visits += int(active)
        key = re.search(r'\[SPU\] key-on voice=([23]) .*pitch=0x([0-9A-F]+) vol=0x2000,0x2000', line)
        if active and key:
            events.append((int(key[1]), int(key[2], 16)))
    return {
        'both_native_confirmation_channels': {e[0] for e in events} == {2, 3},
        'native_confirmation_pitches_match_source': bool(events) and bool(pitches)
            and all(pitch in pitches for _, pitch in events),
        'minimum_selector_visits': visits >= minimum_visits,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--input', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--polls', type=int, default=4800)
    parser.add_argument('--two-player', action='store_true')
    parser.add_argument('--minimum-selector-visits', type=int, default=1)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    loose = root / 'V8_2_LOOSE'
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    settings = loose / 'settings.json'
    saved = settings.read_bytes()
    (output / 'settings.before.json').write_bytes(saved)
    env = {k: v for k, v in os.environ.items() if not k.startswith('RECOMPONE_')}
    env.update(SDL_AUDIODRIVER='dummy', RECOMPONE_WINDOW_VISIBLE='0',
               RECOMPONE_DISABLE_LIVE_INPUT='1', RECOMPONE_SUPPRESS_RUMBLE='1',
               RECOMPONE_INPUT_FILE=str(args.input.resolve()),
               RECOMPONE_SCRIPT_EXIT_AFTER_POLLS=str(args.polls),
               RECOMPONE_TRACE_AUDIO='1', RECOMPONE_AUDIO_CAPTURE=str(output / 'audio.wav'),
               RECOMPONE_TRACE_V82_SELECTOR='1', RECOMPONE_LOG_PATH=str(output / 'runtime.log'),
               RECOMPONE_PRESENTATION_CAPTURE='0', RECOMPONE_DISPLAY_PROBE_IMAGES='0',
               RECOMPONE_CAPTURE_NATIVE_GUEST_SELECTOR='0')
    if args.two_player:
        env['RECOMPONE_FORCE_PAD2_CONNECTED'] = '1'
    config = json.loads(saved.decode('utf-8-sig'))
    # Capture the normal mixer, with audible output directed only to SDL's
    # dummy device. Never mute the PCM being tested or touch host sound input.
    game = config.get('Game', config.get('game', config))
    for key in list(game):
        if key.lower() == 'muted': game[key] = False
        if key.lower() == 'mastervolume': game[key] = 1.0
    try:
        settings.write_text(json.dumps(config), encoding='utf-8')
        with (output / 'stdout.log').open('wb') as out, (output / 'stderr.log').open('wb') as err:
            result = subprocess.run([str(loose / 'Vigilante82PC.exe'), '--loose', str(loose)],
                                    cwd=loose, env=env, stdout=out, stderr=err,
                                    creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0),
                                    timeout=max(240, args.polls / 20))
    finally:
        settings.write_bytes(saved)
    log = (output / 'runtime.log').read_text(encoding='utf-8', errors='replace')
    checks = {
        'clean_exit': result.returncode == 0 and f'completed at poll {args.polls}' in log,
        'no_live_bank_reclaim': 'reclaimed front-end SPU bank' not in log,
        'no_selector_allocation_failure': '[V82Audio] selector SPU allocation failed' not in log,
        'no_fatal': not any(x in log.lower() for x in ('[fatal]', '[v82fatal]', 'unmapped call', 'unhandled exception')),
        'player_selector_entered': "stage 'choose_player' at absolute" in log,
        'next_setup_stage_entered': ("stage 'v82_ready_prompt' at absolute" if args.two_player
                                     else "stage 'choose_enemies' at absolute") in log,
    }
    with wave.open(str(output / 'audio.wav'), 'rb') as audio:
        pcm = audio.readframes(audio.getnframes())
        checks['nonzero_native_pcm'] = any(pcm)
    imported = []
    last_key = None
    for line in log.splitlines():
        key = re.search(r'\[SPU\] key-on voice=(\d+) start=0x([0-9A-F]+).*pitch=0x([0-9A-F]+)', line)
        if key:
            last_key = tuple(int(value, base) for value, base in zip(key.groups(), (10, 16, 16)))
        voice = re.search(r'\[V82SelectionVoice\] guest=(\d+).*native_voice=(\d+).*source_pitch=(\d+) spu=0x([0-9A-F]+)', line)
        if voice:
            guest, channel, pitch = map(int, voice.groups()[:3])
            address = int(voice[4], 16)
            imported.append(dict(guest=guest, native_channel=channel, pitch=pitch,
                                 exact_key_on=last_key == (channel - 1, address >> 3, pitch)))
    if imported:
        checks['imported_voice_key_ons_match_source'] = all(v['exact_key_on'] for v in imported)
    if args.two_player:
        checks.update(native_two_player_checks(log, loose, args.minimum_selector_visits))
    hashes = {str(p.relative_to(loose)): hashlib.sha256(p.read_bytes()).hexdigest()
              for p in (loose/'Vigilante82PC.exe', loose/'SHELL/SOUNDS.SND', loose/'SHELL/V8VOICES.SND')}
    report = dict(checks=checks, passed=all(checks.values()), hashes=hashes, imported_voices=imported,
                  input=str(args.input.resolve()), polls=args.polls,
                  note='Native PCM and event checks; audible user acceptance remains separate.')
    (output / 'proof.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
    print(json.dumps(report, indent=2))
    return 0 if report['passed'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
