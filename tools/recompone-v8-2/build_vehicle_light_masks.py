"""Author light-only palette entries without changing any stock texel color.

Regions are source asset metadata. Runtime recoloring consumes only palette
content and protected indices, independent of roster identity or BIN addresses.
"""
from pathlib import Path
from dataclasses import replace
import argparse,hashlib,json,struct,sys

ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'tools/blender_addons/tests'))
from build_v8_to_v82_guest_roster import decode_bank,build_projects
from vigilante8_vehicle_tools import compiler,iff,xobf

def texture_key(t):
    return (t.width,t.height,t.palette_bgr555,t.indices)

def isolate(t,rects,red_ratio=2.2,full_rects=()):
    """Split masked texels even when their source color is also body paint."""
    mask=set()
    for x0,y0,x1,y1 in rects:
        assert 0<=x0<x1<=t.width and 0<=y0<y1<=t.height,(t.name,t.width,t.height,rects)
        for y in range(y0,y1):
            for x in range(x0,x1):
                n=y*t.width+x;c=t.palette_bgr555[t.indices[n]]
                r,g,b=c&31,(c>>5)&31,(c>>10)&31
                # Keep the colored lens; retain transparent holes and neutral
                # chrome in the ordinary material. Region authoring, not this
                # saturation test, supplies the semantic light classification.
                if r>=3 and r>g*red_ratio and r>b*2.2:mask.add(n)
    # Explicit lens regions preserve amber, pale red and neutral highlights too.
    # Unlike broad rear-panel regions, these bounds exclude painted bodywork.
    for x0,y0,x1,y1 in full_rects:
        assert 0<=x0<x1<=t.width and 0<=y0<y1<=t.height,(t.name,t.width,t.height,full_rects)
        mask.update(y*t.width+x for y in range(y0,y1) for x in range(x0,x1)
                    if t.palette_bgr555[t.indices[y*t.width+x]] != 0)
    if not mask:return t,[]
    used=set(t.indices);free=[i for i in range(256) if i not in used]
    colors=sorted({t.palette_bgr555[t.indices[n]] for n in mask})
    assert len(colors)<=len(free),f'{t.name}: light mask needs a larger texture format'
    assigned=dict(zip(colors,free));pal=list(t.palette_bgr555)
    for color,index in assigned.items():
        pal.extend([0]*max(0,index+1-len(pal)));pal[index]=color
    indices=bytes(assigned[t.palette_bgr555[index]] if n in mask else index for n,index in enumerate(t.indices))
    depth=0 if max(indices)<16 else 1
    # A palette slot is not necessarily a distinct color. Retail palettes can
    # contain duplicate BGR555 entries; retaining all their indices needlessly
    # promotes small light masks to 8bpp and doubles their image VRAM footprint.
    # Preserve the light/body distinction even when both use the same color.
    if depth == 1:
        protected=set(assigned.values())
        classes=sorted({(pal[i],i in protected) for i in indices})
        if len(classes)<=16:
            lookup={value:i for i,value in enumerate(classes)}
            indices=bytes(lookup[(pal[i],i in protected)] for i in indices)
            pal=[color for color,_ in classes]+[0]*(16-len(classes))
            assigned={color:i for i,(color,is_light) in enumerate(classes) if is_light}
            depth=0
    result=replace(t,depth=depth,palette_bgr555=tuple(pal),indices=indices)
    assert [t.palette_bgr555[i] for i in t.indices]==[result.palette_bgr555[i] for i in result.indices]
    return result,sorted(assigned.values())

def palette_key(t):
    return hashlib.sha256(struct.pack('<'+'H'*len(t.palette_bgr555),*t.palette_bgr555)).hexdigest().upper()

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,default=ROOT/'artifacts/vehicle-light-masks')
    out=parser.parse_args().output.resolve();out.mkdir(parents=True,exist_ok=True)
    regions=json.loads(Path(__file__).with_name('vehicle_light_regions.json').read_text())
    native={name:ROOT/'V8_2_WORK/disc'/name for name in ('SHELL/VEHICLES.EXP','SHARED/COMMON.EXP')}
    catalog={};protected={};report=[]
    for section,game,path in [('V8','V8',ROOT/'PS1 game/SHELL/VEHICLES.EXP'),('V8_2','V8_2',native['SHELL/VEHICLES.EXP']),('V8_2_COMMON','V8_2',native['SHARED/COMMON.EXP'])]:
        fixed=regions.get('fixed_lens_regions',{}).get(section,{})
        for vehicle in sorted(set(regions[section])|set(fixed),key=int):
            items=regions[section].get(vehicle,{})
            bank=decode_bank(path,game,int(vehicle))
            for index in sorted(set(items)|set(fixed.get(vehicle,{})),key=int):
                rects=items.get(index,[])
                t=bank.textures[int(index)]
                amber=int(index) in regions.get('amber_lenses',{}).get(section,{}).get(vehicle,[])
                new,indices=isolate(t,rects,1.4 if amber else 2.2,fixed.get(vehicle,{}).get(index,[]))
                catalog[texture_key(t)]=new
                key=palette_key(new)
                assert key not in protected or protected[key]==indices
                protected[key]=indices
                report.append(dict(game=section,vehicle=int(vehicle),texture=int(index),pixels=sum(a!=b for a,b in zip(t.indices,new.indices)),protected=indices,depth=new.depth))
    def convert(t):return catalog.get(texture_key(t),t)
    def bank(b):return replace(b,textures=tuple(map(convert,b.textures))) if b else b
    from vigilante8_vehicle_tools import registry
    guests=[]
    for v in build_projects():
        v=replace(v,textures=tuple(map(convert,v.textures)),selector_preview_bank=bank(v.selector_preview_bank),transformation_bank=bank(v.transformation_bank))
        guests.append(v)
    package=registry.compile_package(tuple(guests))
    (out/'CUSTOM.EXP').write_bytes(package.archive);(out/'VEHICLES.V8R').write_bytes(package.registry)
    for name,path in native.items():
        doc=iff.parse(path.read_bytes());changed=0
        for form in doc.forms(b'XOBF'):
            chunk=next((x for x in form.children if x.tag==b'BIN '),None)
            if chunk is None:continue
            model=xobf.Model(chunk.payload,dialect='V8_2')
            # Decode only the native bank, retaining all unrelated leaf data.
            b=registry._decode_bank(iff.IffChunk(tag=b'FORM',form_type=b'XOBF',children=[chunk]),'V8_2')
            data=bytearray(chunk.payload);table=struct.unpack_from('<I',data,0x14)[0]
            for i,t in enumerate(b.textures):
                new=convert(t)
                if new is t:continue
                while len(data)%4:data.append(0)
                struct.pack_into('<I',data,table+i*4,len(data)-table)
                data+=compiler._compile_texture(new);changed+=1
            chunk.payload=bytes(data)
        (out/name.replace('/','-')).write_bytes(doc.to_bytes())
        print(name,'masked textures',changed)
    resource=ROOT/'tools/recompone-reference/RecompOne.Runtime/sdk/VehicleLightPalettes.json'
    resource.write_text(json.dumps(protected,indent=2)+'\n')
    (out/'report.json').write_text(json.dumps(report,indent=2))
    print('source regions',len(report),'palettes',len(protected))

if __name__=='__main__':main()
