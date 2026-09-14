from pathlib import Path
import os,sys,unittest

ROOT=Path(__file__).resolve().parents[3]
sys.path.insert(0,str(ROOT/'tools/recompone-v8-2'))
from build_vehicle_light_masks import isolate,compiler,iff,xobf
from vigilante8_vehicle_tools import project

class LightMaskTests(unittest.TestCase):
    def test_explicit_lenses_preserve_amber_and_pale_red_without_body_bleed(self):
        amber=24|(19<<5)|(4<<10)
        pale_red=20|(12<<5)|(13<<10)
        t=project.Texture('lenses',5,1,0,(0,amber,pale_red),bytes([1,2,1,2,0]))
        new,protected=isolate(t,[],full_rects=[[0,0,2,1]])
        self.assertTrue(all(new.indices[i] in protected for i in (0,1)))
        self.assertTrue(all(new.indices[i] not in protected for i in (2,3,4)))
        self.assertEqual([t.palette_bgr555[i] for i in t.indices],
                         [new.palette_bgr555[i] for i in new.indices])

    def test_same_red_in_body_and_light_is_independent(self):
        t=project.Texture('shared',4,1,0,(0,31),bytes([1,1,1,0]))
        new,protected=isolate(t,[[0,0,1,1]])
        self.assertIn(new.indices[0],protected)
        self.assertNotIn(new.indices[1],protected)
        self.assertEqual(new.palette_bgr555[new.indices[0]],t.palette_bgr555[t.indices[0]])
        self.assertEqual(new.palette_bgr555[new.indices[1]],31)
        self.assertEqual(new.indices[3],0)

    def test_full_four_bit_palette_promotes_without_color_loss(self):
        pal=tuple(range(16))
        t=project.Texture('full',16,1,0,pal,bytes(range(16)))
        new,protected=isolate(t,[[8,0,16,1]])
        self.assertEqual(new.depth,1)
        self.assertEqual([pal[i] for i in t.indices],[new.palette_bgr555[i] for i in new.indices])
        self.assertTrue(all(i>=16 for i in protected))

    def test_generated_native_assets_preserve_every_stock_texel(self):
        output=Path(os.environ.get('V82_LIGHT_MASK_OUTPUT',str(ROOT/'artifacts/vehicle-light-masks')))
        if not (output/'SHELL-VEHICLES.EXP').is_file():
            self.skipTest('Run build_vehicle_light_masks.py first to verify generated assets')
        for name in ('SHELL/VEHICLES.EXP','SHARED/COMMON.EXP'):
            before=iff.parse((ROOT/'V8_2_WORK/disc'/name).read_bytes())
            after=iff.parse((output/name.replace('/','-')).read_bytes())
            a=list(before.forms(b'XOBF'));b=list(after.forms(b'XOBF'))
            self.assertEqual(len(a),len(b))
            for old,new in zip(a,b):
                self.assertEqual([c.tag for c in old.children],[c.tag for c in new.children])
                for x,y in zip(old.children,new.children):
                    if x.tag!=b'BIN ':self.assertEqual(x.payload,y.payload);continue
                    m=xobf.Model(x.payload,dialect='V8_2');n=xobf.Model(y.payload,dialect='V8_2')
                    self.assertEqual(m.texture_count,n.texture_count)
                    for t,u in zip(m.textures(),n.textures()):
                        self.assertEqual((t.width,t.height),(u.width,u.height))
                        self.assertEqual([t.palette[i] for i in t.indices],[u.palette[i] for i in u.indices])

if __name__=='__main__':unittest.main()
