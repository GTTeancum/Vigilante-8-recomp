"""Check native two-player confirmation channels against their source SND PCM."""
import argparse,json,re
from pathlib import Path
from verify_selector_voice_capture import parse_bank,at_output_rate,load_capture,correlation

def verify(output):
    root=Path(__file__).resolve().parents[2]
    log=(output/'runtime.log').read_text(errors='replace')
    rate,pcm,_=load_capture(output/'audio.wav')
    references=[(bank.name,i,entry,at_output_rate(entry))
                for bank in (root/'V8_2_LOOSE/SHELL').glob('BEGIN*.SND')
                for i,entry in enumerate(parse_bank(bank))]
    events=[]; frame=0; in_selector=False
    names={}; visit=0
    for line in log.splitlines():
        a=re.search(r'\[AUDIO\] frame=(\d+)',line)
        if a: frame=int(a[1])
        stage=re.search(r"\[Input\] stage '([^']+)'",line)
        if stage:
            in_selector=stage[1]=='choose_player'
            if in_selector: visit+=1
        label=re.search(r'ra=0x80108020 .*?"([^"]+)" a2=0x80115E(50|58)',line)
        if label: names[0 if label[2]=='50' else 1]=label[1]
        k=re.search(r'\[SPU\] key-on voice=([23]) start=0x([0-9A-F]+).*pitch=0x([0-9A-F]+) vol=0x2000,0x2000',line)
        if not(in_selector and k):continue
        channel=int(k[1]);pitch=int(k[3],16)
        ranked=[]
        for bank,index,entry,reference in references:
            score,start=correlation(pcm,reference,frame+rate//2,rate)
            ranked.append((score,bank,index,entry.pitch,start))
        ranked.sort(reverse=True)
        score,bank,index,source_pitch,start=ranked[0]
        events.append(dict(visit=visit,player=channel-1,vehicle=names.get(channel-2),
            native_channel=channel+1,start_address=int(k[2],16)*8,pitch=pitch,
            source_bank=bank,source_sample=index,source_pitch=source_pitch,
            correlation=round(score,5),audio_start=start/rate,
            passed=score>.65 and pitch==source_pitch))
    checks=dict(both_players_recorded={e['player'] for e in events}=={1,2},
                source_audio_and_pitch=bool(events) and all(e['passed'] for e in events),
                no_resource_errors=not any(x in log.lower() for x in
                    ('[v82fatal]','out of spu','selector spu allocation failed','reclaimed front-end spu')))
    report=dict(passed=all(checks.values()),checks=checks,events=events,selector_visits=visit)
    (output/'two-player-audio.json').write_text(json.dumps(report,indent=2))
    print(json.dumps(report,indent=2))
    return report

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('output',type=Path)
    raise SystemExit(0 if verify(p.parse_args().output)['passed'] else 1)
