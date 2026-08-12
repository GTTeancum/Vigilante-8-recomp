#!/usr/bin/env python3
"""Run the official RealESRGAN_x2plus weights on a PNG file or directory.

This small CPU-only runner implements the RRDBNet inference graph used by the
official Real-ESRGAN project. It is a build tool; PyTorch and the model are not
runtime dependencies and are never copied into V8_2_LOOSE.

One pass produces 2x output. Two passes produce the terrain pack's requested
4x output while retaining the same official model and deterministic CPU path.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import sys

from PIL import Image


ROOT = Path(__file__).resolve().parents[2]
DEFAULT_PYTHON = ROOT / "build" / "realesrgan" / "python"
if DEFAULT_PYTHON.is_dir():
    sys.path.insert(0, str(DEFAULT_PYTHON))

import torch  # noqa: E402
from torch import nn  # noqa: E402
from torch.nn import functional as F  # noqa: E402


class ResidualDenseBlock(nn.Module):
    def __init__(self, features: int = 64, growth: int = 32):
        super().__init__()
        self.conv1 = nn.Conv2d(features, growth, 3, 1, 1)
        self.conv2 = nn.Conv2d(features + growth, growth, 3, 1, 1)
        self.conv3 = nn.Conv2d(features + growth * 2, growth, 3, 1, 1)
        self.conv4 = nn.Conv2d(features + growth * 3, growth, 3, 1, 1)
        self.conv5 = nn.Conv2d(features + growth * 4, features, 3, 1, 1)
        self.activation = nn.LeakyReLU(negative_slope=0.2, inplace=True)

    def forward(self, x):
        x1 = self.activation(self.conv1(x))
        x2 = self.activation(self.conv2(torch.cat((x, x1), 1)))
        x3 = self.activation(self.conv3(torch.cat((x, x1, x2), 1)))
        x4 = self.activation(self.conv4(torch.cat((x, x1, x2, x3), 1)))
        x5 = self.conv5(torch.cat((x, x1, x2, x3, x4), 1))
        return x + x5 * 0.2


class RRDB(nn.Module):
    def __init__(self, features: int = 64, growth: int = 32):
        super().__init__()
        self.rdb1 = ResidualDenseBlock(features, growth)
        self.rdb2 = ResidualDenseBlock(features, growth)
        self.rdb3 = ResidualDenseBlock(features, growth)

    def forward(self, x):
        return x + self.rdb3(self.rdb2(self.rdb1(x))) * 0.2


class RRDBNetX2(nn.Module):
    def __init__(self):
        super().__init__()
        # The official x2 graph pixel-unshuffles the input once, performs two
        # learned x2 upsampling stages, and therefore returns net 2x output.
        self.conv_first = nn.Conv2d(12, 64, 3, 1, 1)
        self.body = nn.Sequential(*(RRDB() for _ in range(23)))
        self.conv_body = nn.Conv2d(64, 64, 3, 1, 1)
        self.conv_up1 = nn.Conv2d(64, 64, 3, 1, 1)
        self.conv_up2 = nn.Conv2d(64, 64, 3, 1, 1)
        self.conv_hr = nn.Conv2d(64, 64, 3, 1, 1)
        self.conv_last = nn.Conv2d(64, 3, 3, 1, 1)
        self.activation = nn.LeakyReLU(negative_slope=0.2, inplace=True)

    def forward(self, x):
        feature = self.conv_first(F.pixel_unshuffle(x, 2))
        feature = feature + self.conv_body(self.body(feature))
        feature = self.activation(self.conv_up1(F.interpolate(
            feature, scale_factor=2, mode="nearest")))
        feature = self.activation(self.conv_up2(F.interpolate(
            feature, scale_factor=2, mode="nearest")))
        return self.conv_last(self.activation(self.conv_hr(feature)))


def image_tensor(image: Image.Image):
    rgb = image.convert("RGB")
    data = bytearray(rgb.tobytes())
    tensor = torch.frombuffer(data, dtype=torch.uint8)
    return tensor.reshape(rgb.height, rgb.width, 3).permute(2, 0, 1).float().div_(255)


def output_image(tensor) -> Image.Image:
    tensor = tensor.detach().clamp_(0, 1).mul_(255).round_().byte()
    tensor = tensor.permute(1, 2, 0).contiguous()
    return Image.frombytes("RGB", (tensor.shape[1], tensor.shape[0]), tensor.numpy().tobytes())


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--model", type=Path, required=True)
    parser.add_argument("--threads", type=int, default=8)
    parser.add_argument(
        "--passes", type=int, choices=(1, 2), default=1,
        help="one learned pass is 2x; two learned passes are 4x",
    )
    parser.add_argument(
        "--skip-existing", action="store_true",
        help="leave already-generated output PNGs untouched",
    )
    parser.add_argument("--shard-index", type=int, default=0)
    parser.add_argument("--shard-count", type=int, default=1)
    parser.add_argument(
        "--only", action="append", default=[],
        help="process only these filename stems (repeatable)",
    )
    parser.add_argument(
        "--only-file", type=Path,
        help="newline-delimited filename stems to process",
    )
    args = parser.parse_args()
    torch.set_num_threads(max(1, args.threads))
    model = RRDBNetX2().eval()
    checkpoint = torch.load(args.model, map_location="cpu", weights_only=True)
    model.load_state_dict(checkpoint.get("params_ema", checkpoint.get("params", checkpoint)))

    paths = [args.input] if args.input.is_file() else sorted(args.input.glob("*.png"))
    if args.shard_count <= 0 or not 0 <= args.shard_index < args.shard_count:
        parser.error("--shard-index must be within --shard-count")
    paths = paths[args.shard_index::args.shard_count]
    selected = set(args.only)
    if args.only_file:
        selected.update(
            line.strip()
            for line in args.only_file.read_text(encoding="utf-8").splitlines()
            if line.strip()
        )
    if selected:
        paths = [path for path in paths if path.stem in selected]
    args.output.mkdir(parents=True, exist_ok=True)
    with torch.inference_mode():
        for index, path in enumerate(paths, 1):
            destination = args.output / path.name
            if args.skip_existing and destination.exists():
                continue
            image = Image.open(path).convert("RGB")
            result_image = image
            for _ in range(args.passes):
                width, height = result_image.size
                tensor = image_tensor(result_image).unsqueeze(0)
                pad_x = width & 1
                pad_y = height & 1
                if pad_x or pad_y:
                    tensor = F.pad(
                        tensor, (0, pad_x, 0, pad_y), mode="replicate"
                    )
                result = model(tensor)[0, :, :height * 2, :width * 2]
                result_image = output_image(result)
            result_image.save(destination)
            print(f"[{index}/{len(paths)}] {path.name}", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
