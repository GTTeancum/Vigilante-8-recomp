#!/usr/bin/env python3
"""Run classic ESRGAN RRDB x4 weights on PNG files using CPU PyTorch."""

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
        self.conv1 = nn.Sequential(nn.Conv2d(features, growth, 3, 1, 1))
        self.conv2 = nn.Sequential(
            nn.Conv2d(features + growth, growth, 3, 1, 1))
        self.conv3 = nn.Sequential(
            nn.Conv2d(features + growth * 2, growth, 3, 1, 1))
        self.conv4 = nn.Sequential(
            nn.Conv2d(features + growth * 3, growth, 3, 1, 1))
        self.conv5 = nn.Sequential(
            nn.Conv2d(features + growth * 4, features, 3, 1, 1))
        self.activation = nn.LeakyReLU(negative_slope=0.2, inplace=True)

    def forward(self, x):
        x1 = self.activation(self.conv1(x))
        x2 = self.activation(self.conv2(torch.cat((x, x1), 1)))
        x3 = self.activation(self.conv3(torch.cat((x, x1, x2), 1)))
        x4 = self.activation(self.conv4(torch.cat((x, x1, x2, x3), 1)))
        x5 = self.conv5(torch.cat((x, x1, x2, x3, x4), 1))
        return x + x5 * 0.2


class RRDB(nn.Module):
    def __init__(self):
        super().__init__()
        self.RDB1 = ResidualDenseBlock()
        self.RDB2 = ResidualDenseBlock()
        self.RDB3 = ResidualDenseBlock()

    def forward(self, x):
        return x + self.RDB3(self.RDB2(self.RDB1(x))) * 0.2


class ShortcutBlock(nn.Module):
    def __init__(self):
        super().__init__()
        self.sub = nn.Sequential(
            *(RRDB() for _ in range(23)),
            nn.Conv2d(64, 64, 3, 1, 1),
        )

    def forward(self, x):
        return x + self.sub(x)


class RRDBNetX4(nn.Module):
    def __init__(self):
        super().__init__()
        self.model = nn.Sequential(
            nn.Conv2d(3, 64, 3, 1, 1),
            ShortcutBlock(),
            nn.Upsample(scale_factor=2, mode="nearest"),
            nn.Conv2d(64, 64, 3, 1, 1),
            nn.LeakyReLU(negative_slope=0.2, inplace=True),
            nn.Upsample(scale_factor=2, mode="nearest"),
            nn.Conv2d(64, 64, 3, 1, 1),
            nn.LeakyReLU(negative_slope=0.2, inplace=True),
            nn.Conv2d(64, 64, 3, 1, 1),
            nn.LeakyReLU(negative_slope=0.2, inplace=True),
            nn.Conv2d(64, 3, 3, 1, 1),
        )

    def forward(self, x):
        return self.model(x)


def image_tensor(image: Image.Image):
    rgb = image.convert("RGB")
    data = bytearray(rgb.tobytes())
    tensor = torch.frombuffer(data, dtype=torch.uint8)
    return tensor.reshape(rgb.height, rgb.width, 3).permute(2, 0, 1).float().div_(255)


def output_image(tensor) -> Image.Image:
    tensor = tensor.detach().clamp_(0, 1).mul_(255).round_().byte()
    tensor = tensor.permute(1, 2, 0).contiguous()
    return Image.frombytes(
        "RGB", (tensor.shape[1], tensor.shape[0]), tensor.numpy().tobytes())


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--model", type=Path, required=True)
    parser.add_argument("--threads", type=int, default=8)
    parser.add_argument("--skip-existing", action="store_true")
    args = parser.parse_args()

    torch.set_num_threads(max(1, args.threads))
    checkpoint = torch.load(args.model, map_location="cpu", weights_only=False)
    model = RRDBNetX4().eval()
    model.load_state_dict(checkpoint)

    paths = [args.input] if args.input.is_file() else sorted(args.input.glob("*.png"))
    args.output.mkdir(parents=True, exist_ok=True)
    with torch.inference_mode():
        for index, path in enumerate(paths, 1):
            destination = args.output / path.name
            if args.skip_existing and destination.exists():
                continue
            image = Image.open(path).convert("RGB")
            tensor = image_tensor(image).unsqueeze(0)
            result = model(tensor)[0]
            output_image(result).save(destination)
            print(f"[{index}/{len(paths)}] {path.name}", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
