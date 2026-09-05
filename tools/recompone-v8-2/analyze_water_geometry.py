"""Compare captured native arm endpoints to actual active body triangles.

Read-only geometry analysis, not a fit override or an automated visual pass.
The body bank index is a test selection, never a shipping vehicle exception.
"""
import argparse
import json
from pathlib import Path
import runpy
import numpy as np

ROOT = Path(__file__).resolve().parents[2]


def closest_triangle(p, a, b, c):
    # Ericson closest-point regions; degenerates reduce to their edges.
    ab, ac, ap = b-a, c-a, p-a
    d1, d2 = ab@ap, ac@ap
    if d1 <= 0 and d2 <= 0: return a
    bp = p-b
    d3, d4 = ab@bp, ac@bp
    if d3 >= 0 and d4 <= d3: return b
    vc = d1*d4-d3*d2
    if vc <= 0 and d1 >= 0 and d3 <= 0: return a+ab*d1/(d1-d3)
    cp = p-c
    d5, d6 = ab@cp, ac@cp
    if d6 >= 0 and d5 <= d6: return c
    vb = d5*d2-d1*d6
    if vb <= 0 and d2 >= 0 and d6 <= 0: return a+ac*d2/(d2-d6)
    va = d3*d6-d5*d4
    if va <= 0 and d4-d3 >= 0 and d5-d6 >= 0:
        return b+(c-b)*(d4-d3)/((d4-d3)+(d5-d6))
    denom = va+vb+vc
    if abs(denom) < 1e-12: return min((a,b,c), key=lambda v: np.linalg.norm(v-p))
    return a+ab*vb/denom+ac*vc/denom


def analyze(path, body_index):
    capture=json.loads(path.read_text())
    decode=runpy.run_path(str(ROOT/'tools/blender_addons/tests/build_v8_to_v82_guest_roster.py'))['decode_bank']
    bank=decode(ROOT/'V8_2_LOOSE/SHARED/COMMON.EXP', 'V8_2', body_index)
    nodes={n['address']:n for n in capture['nodes']}
    root=capture['vehicle']
    body_bank=nodes[root]['bank']
    matrices={root:np.eye(4)}
    def matrix(address):
        if address not in matrices:
            node=nodes[address]
            local=np.eye(4)
            local[:3,:3]=np.array(node['matrix']).reshape(3,3)/4096.
            local[:3,3]=node['position']
            matrices[address]=matrix(node['parent'])@local
        return matrices[address]
    def points(node):
        v=np.array(node['vertices'],dtype=float)*(2.**(16-node['scaleShift']))
        t=matrix(node['address'])
        return v@t[:3,:3].T+t[:3,3]
    triangles=[]
    rejected=[]
    for node in nodes.values():
        if node['bank'] != body_bank or not node['vertices']: continue
        slot=bank.slots[node['slot']]
        if slot.render_group is None:
            rejected.append(node['slot']); continue
        group=bank.groups[slot.render_group]
        if list(map(list,group.vertices)) != node['vertices'] or group.scale_shift != node['scaleShift']:
            raise ValueError(f"Native/source geometry mismatch at body slot {node['slot']}")
        v=points(node)
        for face in group.faces:
            if len(face.vertices)==3:
                triangles.append((node['slot'],v[list(face.vertices)]))
    if not triangles: raise ValueError('No verified active body triangles')
    results=[]
    for mount,address in enumerate(capture['mounts']):
        if not address: continue
        node=nodes[address]
        p=points(node)
        inward=1 if node['position'][0] < 0 else -1
        inner=max(p[:,0]*inward)
        # Root arm's body-facing edge, within one raw vertex quantization step.
        edge=p[p[:,0]*inward >= inner-2.**(16-node['scaleShift'])]
        endpoints=[]
        for endpoint in edge:
            candidates=[(slot,closest_triangle(endpoint,*tri)) for slot,tri in triangles]
            slot,nearest=min(candidates,key=lambda pair:np.linalg.norm(pair[1]-endpoint))
            endpoints.append({'endpoint':endpoint.tolist(),'nearest_body':nearest.tolist(),
                'body_slot':slot,'gap_fixed_units':float(np.linalg.norm(nearest-endpoint))})
        results.append({'mount':mount,'root_slot':node['slot'],'endpoints':endpoints})
    return {'type':capture['type'],'mode':capture['mode'],'transition':capture['transition'],
        'body_triangles':len(triangles),'unresolved_body_slots':rejected,'mounts':results,
        'note':'Geometric distances only. Not a visual pass; no model or physics mutation.'}


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('capture',type=Path)
    parser.add_argument('--body-bank-index',type=int,required=True)
    args=parser.parse_args()
    print(json.dumps(analyze(args.capture,args.body_bank_index),indent=2))
