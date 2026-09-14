"""Read-only inventory of byte-identical guest texture payloads per Quest."""
import hashlib
import json
from pathlib import Path
import struct

ROOT = Path(__file__).resolve().parents[2]
PACKAGE = ROOT / 'V8_2_LOOSE/mods/v8_to_v82_guest_roster'

def u16(b, p): return struct.unpack_from('<H', b, p)[0]
def u32(b, p): return struct.unpack_from('<I', b, p)[0]

def audit():
    archive = (PACKAGE / 'CUSTOM.EXP').read_bytes()
    banks = []; cursor = 0
    while cursor < len(archive):
        size = struct.unpack_from('>I', archive, cursor + 4)[0]
        end = cursor + 8 + size; part = cursor + 12; payload = None
        while part < end:
            count = struct.unpack_from('>I', archive, part + 4)[0]
            if archive[part:part+4] == b'BIN ': payload = archive[part+8:part+8+count]
            part += 8 + count + (count & 1)
        assert payload is not None
        banks.append(payload); cursor = end + (size & 1)
    registry = (PACKAGE / 'VEHICLES.V8R').read_bytes()
    count, stride = u16(registry, 8), u16(registry, 10)
    table, strings = u32(registry, 12), u32(registry, 16)
    entries = {}
    for index in range(count):
        at = table + index * stride; name = u32(registry, at)
        name = registry[name:registry.index(0, name)].decode()
        entries[name] = [n for n in (u16(registry, at+8),u16(registry, at+10)) if n != 65535]
    def textures(bank):
        b = banks[bank]; table = u32(b,20)
        for index in range(u32(b,16)):
            at = table + u32(b,table + index*4); flags = u32(b,at+4)
            if flags & 3 > 1: continue
            w,h = u16(b,at+16),u16(b,at+18)
            data = b[at+20:at+20+w*h*2]
            yield ('palette',w,h,0,hashlib.sha256(data).hexdigest())
            image = at + u32(b,at+8); size = u32(b,image+8)-12
            data = b[image+20:image+20+size]; assert len(data) == size
            yield ('image',u16(b,image+16),u16(b,image+18),flags & 19,hashlib.sha256(data).hexdigest())
    result=[]
    for campaign in json.loads((PACKAGE / 'quests.json').read_text())['campaigns']:
        for n, mission in enumerate(campaign['missions']):
            names = {campaign['vehicle']} | {a['vehicle'] for a in mission['actors'] if a['sentinel'] == 0}
            requests = [r for name in names for bank in entries[name] for r in textures(bank)]
            unique = set(requests)
            result.append(dict(vehicle=campaign['vehicle'],mission=n+1,requests=len(requests),unique=len(unique),
                               words=sum(r[1]*r[2] for r in requests),unique_words=sum(r[1]*r[2] for r in unique)))
    return result

if __name__ == '__main__':
    print(json.dumps(audit(),indent=2))
