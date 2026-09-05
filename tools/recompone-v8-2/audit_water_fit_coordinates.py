"""Read-only bounds/pointer audit of native attachment diagnostic JSON.

Checks cached render-copy geometry, not visual approval. Does not mutate assets.
"""
import argparse
import json
from pathlib import Path
import numpy as np


def audit(path):
    capture = json.loads(path.read_text())
    nodes = {n['address']: n for n in capture['nodes']}
    root = capture['vehicle']
    transforms = {root: np.eye(4)}
    def pose(address):
        if address not in transforms:
            node = nodes[address]
            local = np.eye(4)
            local[:3, :3] = np.array(node['matrix']).reshape(3, 3) / 4096
            local[:3, 3] = node['position']
            transforms[address] = pose(node['parent']) @ local
        return transforms[address]
    def points(raw, shift, address):
        matrix = pose(address)
        return np.array(raw) * (2. ** (16-shift)) @ matrix[:3, :3].T + matrix[:3, 3]
    if capture['mode'] != 2 or capture['transition'] != 0:
        return {'file': str(path), 'type': capture['type'], 'mode': capture['mode'],
            'transition': capture['transition'], 'replacements': [],
            'note': 'Not settled water mode; cached copies are inactive and may reference superseded native meshes.'}
    replacements = (capture.get('lastRenderFit') or {}).get('replacements', [])
    original_points = [points(n['vertices'], n['scaleShift'], n['address'])
        for n in nodes.values() if n['vertices']]
    # The native alternate LOD can extend beyond the primary mesh. Include
    # its original bounds too, or unchanged outboard vertices become false
    # positives in an otherwise valid fit.
    for replacement in replacements:
        owners = [n for n in nodes.values() if replacement['mesh'] in (n['mesh'], n['alternateMesh'])]
        if len(owners) == 1 and replacement['original'] == replacement['currentSource']:
            original_points.append(points(replacement['originalVertices'], replacement['scaleShift'], owners[0]['address']))
    original = np.concatenate(original_points)
    low, high = original.min(axis=0), original.max(axis=0)
    results = []
    for replacement in replacements:
        owners = [n for n in nodes.values() if replacement['mesh'] in (n['mesh'], n['alternateMesh'])]
        if len(owners) != 1:
            results.append({'mesh': replacement['mesh'], 'owner_count': len(owners)})
            continue
        node = owners[0]
        old = points(replacement['originalVertices'], replacement['scaleShift'], node['address'])
        new = points(replacement['fittedVertices'], replacement['scaleShift'], node['address'])
        tolerance = 2. ** (16-replacement['scaleShift']) * 2
        results.append({
            'mesh': replacement['mesh'], 'node': node['address'], 'slot': node['slot'],
            'primary': replacement['mesh'] == node['mesh'],
            'pointer_restored': replacement['original'] == replacement['currentSource'],
            'primary_source_matches_snapshot': replacement['originalVertices'] == node['vertices']
                if replacement['mesh'] == node['mesh'] else None,
            'maximum_displacement': float(np.linalg.norm(new-old, axis=1).max()),
            'original_bounds': [old.min(axis=0).tolist(), old.max(axis=0).tolist()],
            'fitted_bounds': [new.min(axis=0).tolist(), new.max(axis=0).tolist()],
            'within_original_assembly_bounds': bool(np.all(new >= low-tolerance) and np.all(new <= high+tolerance)),
        })
    return {'file': str(path), 'type': capture['type'], 'vehicle': root,
        'mode': capture['mode'], 'transition': capture['transition'],
        'original_assembly_bounds': [low.tolist(), high.tolist()],
        'replacements': results, 'note': 'Cached-copy audit only; not rendered polygon ownership or visual approval.'}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('directory', type=Path)
    args = parser.parse_args()
    print(json.dumps([audit(p) for p in sorted(args.directory.glob('water_geometry*.json'))], indent=2))
