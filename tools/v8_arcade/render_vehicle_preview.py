#!/usr/bin/env python3
"""Build an interactive perspective viewer for extracted vehicle Model3 OBJs."""

from __future__ import annotations

import argparse
import base64
import json
import struct
from pathlib import Path


def read_obj(path: Path) -> tuple[list[float], list[int]]:
    vertices: list[float] = []
    triangles: list[int] = []
    for line in path.read_text(encoding="ascii").splitlines():
        if line.startswith("v "):
            vertices.extend(round(float(value), 6) for value in line.split()[1:4])
        elif line.startswith("f "):
            triangles.extend(int(value.split("/", 1)[0]) - 1 for value in line.split()[1:4])
    return vertices, triangles


def encode_geometry(vertices: list[float], triangles: list[int]) -> tuple[str, str]:
    axes = [vertices[index::3] for index in range(3)]
    centers = [(min(axis) + max(axis)) * 0.5 for axis in axes]
    span = max(max(axis) - min(axis) for axis in axes) or 1.0
    quantized = [
        max(-32767, min(32767, round((value - centers[index % 3]) / span * 30000)))
        for index, value in enumerate(vertices)
    ]
    vertex_data = base64.b64encode(
        struct.pack(f"<{len(quantized)}h", *quantized)
    ).decode("ascii")
    triangle_data = base64.b64encode(
        struct.pack(f"<{len(triangles)}H", *triangles)
    ).decode("ascii")
    return vertex_data, triangle_data


def load_models(
    extraction_root: Path, model_set: str, first_only: bool, dedupe: bool
) -> list[dict[str, object]]:
    models: list[dict[str, object]] = []
    manifest_field = {
        "native": "probable_native_vehicle_models",
        "ps1": "probable_ps1_bonus_models",
    }[model_set]
    for manifest_path in sorted(extraction_root.glob("*/manifest.json")):
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
        seen_geometry: set[tuple[str, str]] = set()
        for candidate_index, candidate in enumerate(
            manifest[manifest_field]
        ):
            if first_only and candidate_index > 0:
                continue
            model = manifest["models"][candidate["model_index"]]
            vertices, triangles = read_obj(manifest_path.parent / model["obj"])
            vertex_data, triangle_data = encode_geometry(vertices, triangles)
            geometry_key = (vertex_data, triangle_data)
            if dedupe and geometry_key in seen_geometry:
                continue
            seen_geometry.add(geometry_key)
            models.append(
                {
                    "vehicle": manifest_path.parent.name,
                    "candidate": candidate_index + 1,
                    "offset": f"0x{model['offset']:08X}",
                    "vertexCount": len(vertices) // 3,
                    "triangleCount": len(triangles) // 3,
                    "vertexData": vertex_data,
                    "triangleData": triangle_data,
                }
            )
    return models


def build_fragment(models: list[dict[str, object]], model_set: str) -> str:
    model_json = json.dumps(models, separators=(",", ":"))
    candidate_label = "Native model candidate" if model_set == "native" else "PS1 bonus model"
    return f"""<div id="v8-perspective-viewer">
  <div class="viz-controls">
    <label class="form-label" for="v8-vehicle">Vehicle
      <select class="form-select" id="v8-vehicle"></select>
    </label>
    <label class="form-label" for="v8-candidate">{candidate_label}
      <select class="form-select" id="v8-candidate"></select>
    </label>
    <button class="btn" type="button" id="v8-reset">Reset view</button>
  </div>
  <canvas id="v8-canvas" width="900" height="540" role="img" aria-label="Interactive perspective wireframe of an extracted vehicle"></canvas>
  <div class="text-small text-muted" id="v8-detail" aria-live="polite"></div>
</div>
<style>
  #v8-perspective-viewer {{ width: 100%; color: var(--foreground); }}
  #v8-perspective-viewer .viz-controls {{ margin-bottom: 0.75rem; }}
  #v8-perspective-viewer canvas {{
    display: block;
    width: 100%;
    height: auto;
    max-height: 560px;
    background: color-mix(in srgb, var(--card) 45%, transparent);
    border: 1px solid var(--border);
    border-radius: 0.5rem;
    cursor: grab;
    touch-action: none;
  }}
  #v8-perspective-viewer canvas:active {{ cursor: grabbing; }}
  #v8-perspective-viewer #v8-detail {{ margin-top: 0.5rem; text-align: center; }}
</style>
<script>
(() => {{
  const root = document.getElementById("v8-perspective-viewer");
  const canvas = document.getElementById("v8-canvas");
  const ctx = canvas.getContext("2d");
  const vehicleSelect = document.getElementById("v8-vehicle");
  const candidateSelect = document.getElementById("v8-candidate");
  const detail = document.getElementById("v8-detail");
  const reset = document.getElementById("v8-reset");
  const models = {model_json};
  const vehicles = [...new Set(models.map(model => model.vehicle))];
  let yaw = -0.62;
  let pitch = 0.42;
  let zoom = 1;
  let dragging = false;
  let lastX = 0;
  let lastY = 0;

  function options(select, values) {{
    select.replaceChildren(...values.map(value => {{
      const option = document.createElement("option");
      option.value = String(value.value);
      option.textContent = value.label;
      return option;
    }}));
  }}

  options(vehicleSelect, vehicles.map(vehicle => ({{ value: vehicle, label: vehicle }})));

  function availableModels() {{
    return models.filter(model => model.vehicle === vehicleSelect.value);
  }}

  function updateCandidates() {{
    const available = availableModels();
    options(candidateSelect, available.map((model, index) => ({{
      value: index,
      label: `Candidate ${{model.candidate}} — ${{model.triangleCount}} triangles`
    }})));
    draw();
  }}

  function selectedModel() {{
    const model = availableModels()[Number(candidateSelect.value) || 0];
    if (model && !model.vertices) {{
      const vertexBytes = Uint8Array.from(atob(model.vertexData), char => char.charCodeAt(0));
      const vertexView = new DataView(vertexBytes.buffer);
      model.vertices = new Array(vertexBytes.length / 2);
      for (let index = 0; index < model.vertices.length; index++) {{
        model.vertices[index] = vertexView.getInt16(index * 2, true) / 30000;
      }}
      const triangleBytes = Uint8Array.from(atob(model.triangleData), char => char.charCodeAt(0));
      const triangleView = new DataView(triangleBytes.buffer);
      model.triangles = new Array(triangleBytes.length / 2);
      for (let index = 0; index < model.triangles.length; index++) {{
        model.triangles[index] = triangleView.getUint16(index * 2, true);
      }}
    }}
    return model;
  }}

  function edgesFor(model) {{
    if (model.edges) return model.edges;
    const seen = new Set();
    const edges = [];
    for (let index = 0; index < model.triangles.length; index += 3) {{
      const triangle = model.triangles.slice(index, index + 3);
      for (let edge = 0; edge < 3; edge++) {{
        let a = triangle[edge];
        let b = triangle[(edge + 1) % 3];
        if (a > b) [a, b] = [b, a];
        const key = `${{a}}:${{b}}`;
        if (!seen.has(key)) {{
          seen.add(key);
          edges.push(a, b);
        }}
      }}
    }}
    model.edges = edges;
    return edges;
  }}

  function projectedVertices(model, width, height) {{
    const source = model.vertices;
    const cosY = Math.cos(yaw), sinY = Math.sin(yaw);
    const cosP = Math.cos(pitch), sinP = Math.sin(pitch);
    const scale = Math.min(width, height) * 0.68 * zoom;
    const projected = [];
    for (let index = 0; index < source.length; index += 3) {{
      const x = source[index];
      const y = source[index + 1];
      const z = source[index + 2];
      const x1 = x * cosY - y * sinY;
      const y1 = x * sinY + y * cosY;
      const y2 = y1 * cosP - z * sinP;
      const z2 = y1 * sinP + z * cosP;
      const perspective = 1 / Math.max(1.25 - z2 * 0.55, 0.45);
      projected.push(
        width / 2 + x1 * scale * perspective,
        height / 2 - y2 * scale * perspective,
        z2
      );
    }}
    return projected;
  }}

  function draw() {{
    const model = selectedModel();
    if (!model) return;
    const rect = canvas.getBoundingClientRect();
    const dpr = Math.min(window.devicePixelRatio || 1, 2);
    const width = Math.max(320, Math.round(rect.width || 900));
    const height = Math.round(width * 0.6);
    if (canvas.width !== Math.round(width * dpr) || canvas.height !== Math.round(height * dpr)) {{
      canvas.width = Math.round(width * dpr);
      canvas.height = Math.round(height * dpr);
    }}
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    ctx.clearRect(0, 0, width, height);
    const styles = getComputedStyle(root);
    const lineColor = styles.getPropertyValue("--viz-series-1").trim() || styles.color;
    const mutedColor = styles.getPropertyValue("--border").trim() || styles.color;
    const points = projectedVertices(model, width, height);
    const edges = edgesFor(model);

    ctx.lineWidth = 1;
    ctx.strokeStyle = mutedColor;
    ctx.globalAlpha = 0.35;
    for (let index = 0; index < edges.length; index += 2) {{
      const a = edges[index] * 3;
      const b = edges[index + 1] * 3;
      if ((points[a + 2] + points[b + 2]) / 2 > 0) continue;
      ctx.beginPath();
      ctx.moveTo(points[a], points[a + 1]);
      ctx.lineTo(points[b], points[b + 1]);
      ctx.stroke();
    }}
    ctx.strokeStyle = lineColor;
    ctx.globalAlpha = 0.95;
    ctx.lineWidth = 1.25;
    for (let index = 0; index < edges.length; index += 2) {{
      const a = edges[index] * 3;
      const b = edges[index + 1] * 3;
      if ((points[a + 2] + points[b + 2]) / 2 <= 0) continue;
      ctx.beginPath();
      ctx.moveTo(points[a], points[a + 1]);
      ctx.lineTo(points[b], points[b + 1]);
      ctx.stroke();
    }}
    ctx.globalAlpha = 1;
    detail.textContent = `${{model.vehicle}} · Candidate ${{model.candidate}} · ${{model.vertexCount}} vertices · ${{model.triangleCount}} triangles · ${{model.offset}}`;
    canvas.setAttribute("aria-label", `${{model.vehicle}} candidate ${{model.candidate}} perspective wireframe, ${{model.vertexCount}} vertices and ${{model.triangleCount}} triangles`);
  }}

  vehicleSelect.addEventListener("change", updateCandidates);
  candidateSelect.addEventListener("change", draw);
  reset.addEventListener("click", () => {{
    yaw = -0.62; pitch = 0.42; zoom = 1; draw();
  }});
  canvas.addEventListener("pointerdown", event => {{
    dragging = true; lastX = event.clientX; lastY = event.clientY;
    canvas.setPointerCapture(event.pointerId);
  }});
  canvas.addEventListener("pointermove", event => {{
    if (!dragging) return;
    yaw += (event.clientX - lastX) * 0.009;
    pitch = Math.max(-1.35, Math.min(1.35, pitch + (event.clientY - lastY) * 0.009));
    lastX = event.clientX; lastY = event.clientY; draw();
  }});
  canvas.addEventListener("pointerup", () => {{ dragging = false; }});
  canvas.addEventListener("pointercancel", () => {{ dragging = false; }});
  canvas.addEventListener("wheel", event => {{
    event.preventDefault();
    zoom = Math.max(0.55, Math.min(2.2, zoom * Math.exp(-event.deltaY * 0.001)));
    draw();
  }}, {{ passive: false }});
  new ResizeObserver(draw).observe(root);
  updateCandidates();
}})();
</script>
"""


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("extraction_root", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument(
        "--set",
        choices=("native", "ps1"),
        default="native",
        dest="model_set",
        help="model classification to display (default: native)",
    )
    parser.add_argument(
        "--first-only",
        action="store_true",
        help="include only the first classified model for each vehicle",
    )
    parser.add_argument(
        "--dedupe",
        action="store_true",
        help="omit byte-identical geometry candidates within each vehicle",
    )
    args = parser.parse_args()
    models = load_models(
        args.extraction_root.resolve(), args.model_set, args.first_only, args.dedupe
    )
    if not models:
        raise RuntimeError(f"no {args.model_set} models found")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(build_fragment(models, args.model_set), encoding="utf-8")
    print(f"Wrote {len(models)} models to {args.output.resolve()}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
