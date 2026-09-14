"""Convert decoded original PS1 campaigns to the shared native quest schema."""
from pathlib import Path
import argparse
import json
import sys
from audit_quest_sources import ROOT, V8_NAMES, original_quest, original_file, decode_original, u32, cstring

def convert(data):
    campaigns = []
    for character in decode_original(data):
        if not character["missions"]:
            continue
        missions = []
        for source in character["missions"]:
            text = source["briefing"]
            # Read the authored directive, rather than assigning factions by name.
            directive = text.split("\n")[-1]
            if "PROTECT: " in directive:
                destroy, target = False, directive.split("PROTECT: ", 1)[1]
            elif "DESTROY: " in directive:
                destroy, target = True, directive.split("DESTROY: ", 1)[1]
            else:
                raise ValueError("Quest has no decoded objective directive")
            actors = []
            for actor in source["actors"]:
                value = actor["type"]
                sentinel = value if value in (254, 255) else 0
                source_type = value & 127
                actors.append({"vehicle": "" if sentinel else "guest.v8." + V8_NAMES[source_type],
                    "sentinel": sentinel, "deferred": bool(value & 128) and not sentinel,
                    "flags": actor["flags"], "spawn": actor["spawn"], "aux": actor["aux"]})
            missions.append({"arena": source["arena"] + 8,
                "loadingCardVariant": source["loading_card_variant"],
                # Resolved against installed arena XLSC records below.
                "nativeLoadingCardVariant": 0,
                "targetFirst": source["target_first"], "targetLast": source["target_last"],
                "destroy": destroy, "actors": actors, "briefing": text,
                "objectives": ("Destroy " if destroy else "Protect ") + target + "\nDestroy All Enemies\n"})
        campaigns.append({"vehicle": character["stable_id"], "missions": missions})
    return {"version": 1, "campaigns": campaigns}

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--install-endings", type=Path)
    args = parser.parse_args()
    data = original_quest(ROOT / "BINCUE/Vigilante 8 (USA) (Track 01).bin")
    result = convert(data)
    sys.path.insert(0, str(ROOT / "tools/blender_addons"))
    from vigilante8_vehicle_tools import iff
    from build_loading_cards import ARENAS
    cards = {}
    for campaign in result["campaigns"]:
        for mission in campaign["missions"]:
            arena = mission["arena"]
            if arena not in cards:
                path = ROOT / "V8_2_LOOSE" / (ARENAS[arena].replace("_", "/") + ".EXP")
                cards[arena] = [node.payload[3] for form in iff.parse(path.read_bytes()).forms(b"TERR")
                                for node in form.children if node.tag == b"XLSC"]
            requested = mission["loadingCardVariant"]
            available = cards[arena]
            if not available: raise ValueError(f"Arena {arena} has no loading cards")
            mission["nativeLoadingCardVariant"] = requested if requested in available else available[0]
    disc = ROOT / "BINCUE/Vigilante 8 (USA) (Track 01).bin"
    shell = original_file(disc, 'SHELL/SHELL.DLL')
    for index, campaign in enumerate(result['campaigns']):
        source = cstring(shell, u32(shell, 0x11c68 + index * 20 + 4))
        target = 'Movies/V8_' + source.replace('\\', '/').split('/')[-1]
        campaign['endingMovie'] = target
        if args.install_endings:
            output = args.install_endings / target
            output.parent.mkdir(parents=True, exist_ok=True)
            output.write_bytes(original_file(disc, source, raw_stream=True))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2))
    print(f"Built {len(result['campaigns'])} campaigns: {args.output}")

if __name__ == "__main__":
    main()
