"""Run the real host with a deliberate shader link failure; never change the checkout.

Run from an OpenGL 4.5 desktop or a software-driver/Xvfb environment:
  python3 check_rejected.py
For the additional v1.0 regression on a 16-attribute driver, optionally pass
its released GlShaders.cs. Only its shader text replaces the current text;
the program builder, host, and test remain the candidate version.
"""
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

project = Path(__file__).resolve().parent
root = project.parents[3]
runtime = Path("tools/recompone-reference/RecompOne.Runtime")
probe = project.relative_to(root)
with tempfile.TemporaryDirectory(prefix="v8-gl-reject-") as tmp:
    work = Path(tmp)
    for rel in (runtime, probe):
        shutil.copytree(root / rel, work / rel, ignore=shutil.ignore_patterns("bin", "obj", "__pycache__"))
    shutil.copyfile(root / "NuGet.Config", work / "NuGet.Config")
    shader = work / runtime / "Gpu/Enhanced/GlShaders.cs"
    text = shader.read_text()
    if len(sys.argv) == 2:
        legacy = Path(sys.argv[1]).read_text()
        boundary = "    public static uint Build("
        assert legacy.count(boundary) == text.count(boundary) == 1
        text = legacy.split(boundary)[0] + boundary + text.split(boundary)[1]
    elif len(sys.argv) == 1:
        # Both stages compile. Their used varying has incompatible types,
        # so the driver must reject the linked program on any GPU.
        for name, body in {
            "PrimVs": "out vec3 testValue; void main() { testValue=vec3(1); gl_Position=vec4(1); }",
            "PrimFs": "in vec4 testValue; out vec4 color; void main() { color=testValue; }",
        }.items():
            pattern = 'public const string ' + name + r' = """.*?\n        """;'
            replacement = 'public const string ' + name + ' = """\n        #version 330 core\n        ' + body + '\n        """;'
            text, n = re.subn(pattern, replacement, text, flags=re.DOTALL)
            assert n == 1, f"cannot isolate {name}"
    else:
        raise SystemExit("usage: check_rejected.py [v1.0-GlShaders.cs]")
    shader.write_text(text)
    subprocess.run(["dotnet", "run", "--project", str(work / probe), "-c", "Release",
                    "-p:NuGetAudit=false", "--", "--expect-rejected"], cwd=work, check=True)
