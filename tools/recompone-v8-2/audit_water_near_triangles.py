"""Rank native water-proof triangle projections; diagnostic, not visual approval.

Clips valid view-space geometry at the renderer's default z=1 near plane,
then the native 427x240 viewport. Does not simulate depth/material/occlusion.
"""
import argparse
from collections import Counter
import json
from pathlib import Path
import re


def clip(poly, axis, edge, sign):
    result = []
    for a, b in zip(poly, poly[1:] + poly[:1]):
        inside_a = (a[axis] - edge) * sign >= 0
        inside_b = (b[axis] - edge) * sign >= 0
        if inside_a:
            result.append(a)
        if inside_a != inside_b:
            t = (edge - a[axis]) / (b[axis] - a[axis])
            result.append([x + (y-x)*t for x, y in zip(a, b)])
    return result


def audit(path):
    triangles = []
    for line in path.read_text(errors='replace').splitlines():
        if not line.startswith('[WaterNearTriangle] '):
            continue
        fields = dict(re.findall(r'(\w+)=([^ ]+)', line))
        def vectors(key):
            return [[float(v) for v in group.split(',')]
                for group in re.findall(r'\(([^)]+)\)', fields[key])]
        view = vectors('view')
        projection = vectors('projection')
        if fields['valid'] != '(True,True,True)' or not all(p == projection[0] for p in projection):
            triangles.append({'packet': fields['packet'], 'owner': fields['owner'],
                'coherent_projection': False, 'area': 0, 'record': fields})
            continue
        near = clip(view, 2, 1, 1)
        cx, cy, scale = projection[0]
        poly = [[cx + x*scale/z, cy + y*scale/z] for x, y, z in near]
        for axis, edge, sign in ((0,0,1),(0,427,-1),(1,0,1),(1,240,-1)):
            poly = clip(poly, axis, edge, sign)
        area = abs(sum(a[0]*b[1] - b[0]*a[1]
            for a, b in zip(poly, poly[1:]+poly[:1]))) / 2
        triangles.append({'packet': fields['packet'], 'owner': fields['owner'],
            'coherent_projection': True, 'area': area,
            'near_crossing': min(p[2] for p in view) < 1,
            'record': fields})
    return {'count': len(triangles), 'trace_limit_reached': len(triangles) >= 8192,
        'owners': dict(Counter(t['owner'] for t in triangles)),
        'invalid_or_mixed_projection': sum(not t['coherent_projection'] for t in triangles),
        'largest_projected_triangles': sorted(triangles, key=lambda t:t['area'], reverse=True)[:30],
        'note': 'Projected area only; not a visibility, correctness or visual-pass assertion.'}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('log', type=Path)
    args = parser.parse_args()
    print(json.dumps(audit(args.log), indent=2))
