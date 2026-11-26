"""Build/qualify the public, source-only Actions component. Never read private inputs."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]


def run(argv, **kwargs):
    return subprocess.run(list(map(str, argv)), cwd=ROOT, check=True, **kwargs)


def git(*args):
    return subprocess.check_output(["git", *args], cwd=ROOT, text=True).strip()


def digest(path):
    with path.open("rb") as f:
        return hashlib.file_digest(f, "sha256").hexdigest()


def build(out, pair, requested_tree=None, local_contract=False):
    tree, commit = git("rev-parse", "HEAD^{tree}"), git("rev-parse", "HEAD")
    if requested_tree and tree != requested_tree.lower():
        raise ValueError("The checked out tree does not match the requesting Linux build.")
    if not local_contract and (os.name != "nt" or os.environ.get("GITHUB_ACTIONS") != "true"):
        raise ValueError("The Windows platform artifact must be produced on a Windows Actions runner.")
    if not local_contract and git("status", "--porcelain"):
        raise ValueError("A clean checkout is required for provenance.")
    props = ["-p:IncludeSourceRevisionInInformationalVersion=true", "-p:PublishReadyToRun=false",
             "-p:PublishTrimmed=false", "-p:DebugType=None", "-p:DebugSymbols=false"]
    def stamp(sha):
        return ["-p:SourceRevisionId=" + sha, *props]
    with tempfile.TemporaryDirectory() as tmp:
        temp = Path(tmp)
        published = temp / "publish"
        args = ["dotnet", "publish", HERE/"host/WindowsHost.csproj", "-c", "Release", "-o", published, *stamp(tree)]
        if local_contract:
            args += ["--self-contained", "false", "-p:PublishSingleFile=false"]
        else:
            args += ["-r", "win-x64", "--self-contained", "true", "-p:PublishSingleFile=true"]
        run(args)
        app = published/("Vigilante8PC.exe" if os.name == "nt" else "Vigilante8PC")
        src = published/"Vigilante82PC.dll"
        if src.exists(): raise ValueError("Source-only publish unexpectedly contains a game module.")
        if not local_contract:
            extra = {p.name for p in published.iterdir()} - {app.name}
            if extra: raise ValueError("Unexpected external runtime inputs; check single-file publish: " + repr(extra))
        env = os.environ.copy()
        env.pop("V8_TEST_SYNTHETIC_MODULE", None)
        env["RECOMPONE_LOG_PATH"] = str(temp/"host.log")
        def invoke(args, code, phrase):
            if Path(env["RECOMPONE_LOG_PATH"]).exists():
                Path(env["RECOMPONE_LOG_PATH"]).unlink()
            p = subprocess.run(list(map(str, [app, *args])), env=env, cwd=temp, timeout=90)
            log = Path(env["RECOMPONE_LOG_PATH"]).read_text(encoding="utf-8", errors="replace")
            if p.returncode != code or phrase not in log:
                raise RuntimeError(f"Host {args} expected {code}/{phrase}, got {p.returncode}: {log[-2400:]}")
        invoke(["--verify-paired-game"], 2, "missing")
        def fixture(revision):
            dst = temp/("q-" + revision)
            run(["dotnet", "build", HERE/"fixture/Fixture.csproj", "-c", "Release", "-o", dst, *stamp(revision)])
            shutil.copy2(dst/"Vigilante82PC.dll", src)
        wrong = "0"*40 if tree != "0"*40 else "1"*40
        fixture(wrong)
        invoke(["--verify-paired-game"], 2, "different builds")
        fixture(tree)
        invoke(["--verify-paired-game"], 2, "fixture is not")
        env["V8_TEST_SYNTHETIC_MODULE"] = "1"
        invoke(["--probe-native-controls"], 0, "[NativeControlsProbe] PASS")
        loose = temp/"test loose"
        loose.mkdir()
        (loose/"qualification-sentinel").touch()
        invoke(["--loose", loose], 0, "[Host] emulation entry returned")
        if (loose/"qualification-entered.txt").read_text() != "entered\n":
            raise RuntimeError("Did not enter the synthetic guest via the normal game entry.")

        # Allowlist a single EXE plus provenance. Never upload any fixture, logs,
        # published directory, intermediate/private module, media or mod directory.
        if not local_contract:
            out.mkdir(exist_ok=False, parents=True)
            shutil.copyfile(app, out/app.name)
            meta = dict(format=1, kind="source-only-windows-host", source_tree=tree, source_commit=commit,
                pair_id=pair, run_id=os.environ["GITHUB_RUN_ID"], run_attempt=os.environ["GITHUB_RUN_ATTEMPT"],
                host_sha256=digest(out/app.name), host_bytes=(out/app.name).stat().st_size)
            (out/"windows-host.json").write_text(json.dumps(meta, indent=2)+"\n", encoding="utf-8")
    print("PASS: missing/stale/fixture rejection; native control probe; normal entry into synthetic guest. No game data used.")


if __name__ == "__main__":
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--out", type=Path, default=ROOT/"artifacts"/"windows-platform-only")
    p.add_argument("--pair", required=True)
    p.add_argument("--tree")
    p.add_argument("--local-contract", action="store_true", help="exercise the ABI on this OS; never emit a Windows artifact")
    a = p.parse_args()
    build(a.out, a.pair, a.tree, a.local_contract)
