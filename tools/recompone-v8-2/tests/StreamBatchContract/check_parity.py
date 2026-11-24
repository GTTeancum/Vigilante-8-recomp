"""Run the standalone fixture inside an already configured GL 4.5 display."""

import argparse
import os
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("candidate", type=Path, help="standalone candidate fixture DLL")
    parser.add_argument("--baseline", type=Path, help="same fixture built with unmodified runtime")
    parser.add_argument("--dotnet", default="dotnet")
    args = parser.parse_args()
    environment = {k: v for k, v in os.environ.items() if not k.startswith("RECOMPONE_")}
    environment["RECOMPONE_TRACE_ENHANCED_PERFORMANCE"] = "0"

    def replay(assembly, case, samples, road_triangles, **flags):
        print(f"RUN {assembly} mode={case} msaa={samples} road-triangles={road_triangles} flags={flags}",
              flush=True)
        with TemporaryDirectory(prefix="v8-stream-contract-") as profile:
            result = subprocess.run(
                [args.dotnet, str(assembly.resolve()), case, str(samples)],
                cwd=profile, env={**environment, **flags,
                                  "RECOMPONE_MOD_DIR": profile,
                                  "RECOMPONE_USER_DATA_DIR": profile,
                                  "RECOMPONE_DIAGNOSTIC_ROAD_TRIANGLE_BATCHES": str(road_triangles)},
                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, timeout=90,
            )
        print(result.stdout, end="", flush=True)
        if result.returncode:
            raise SystemExit(result.returncode)
        lines = result.stdout.splitlines()
        outputs = [line for line in lines if line.startswith("OUTPUT ")]
        if len(outputs) != 8 or sum(line.startswith("PASS ") for line in lines) != 1:
            raise SystemExit("Missing replay checkpoints")
        return outputs

    trial = {"RECOMPONE_V82_STREAM_GAMEPLAY_BATCHES": "1"}
    for samples in (0, 4):
        for road_triangles in (0, 1):
            # Same viewport/options and driver; changing road batching or
            # MSAA controls is deliberately not a cross-case image assertion.
            legacy = replay(args.candidate, "legacy", samples, road_triangles)
            controls = [
                ("isolated opt-in", replay(args.candidate, "trial", samples, road_triangles, **trial)),
                ("stream master disabled", replay(args.candidate, "legacy", samples, road_triangles,
                                                  **trial, RECOMPONE_STREAM_BATCHES="0")),
            ]
            if args.baseline:
                controls.append(("unchanged runtime", replay(args.baseline, "legacy", samples, road_triangles, **trial)))
                original_direct = replay(args.baseline, "direct", samples, road_triangles, RECOMPONE_DIRECT_TERRAIN="1")
                current_direct = replay(args.candidate, "direct", samples, road_triangles, RECOMPONE_DIRECT_TERRAIN="1")
                if original_direct != current_direct:
                    raise SystemExit("Existing direct mode changed")
            for label, observed in controls:
                if observed != legacy:
                    raise SystemExit(f"Replay mismatch in {label}, {samples}x, road-triangles={road_triangles}")
            print(f"PARITY msaa={samples} road-triangles={road_triangles}: 22 attachment/native image digests matched per control",
                  flush=True)


if __name__ == "__main__":
    main()
