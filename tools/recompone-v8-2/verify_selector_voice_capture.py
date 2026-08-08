#!/usr/bin/env python3
"""Verify an imported V8 selector voice in a real V8:2 mixed-audio capture.

The runtime records the exact host-audio frame at which it asks V8:2's native
SND player to start the sample.  This verifier checks the append-only bank
payload against the original V8 bank byte-for-byte, decodes every candidate
selection line, and ranks their normalized correlation in the captured mixer
window.  The requested character must be the best match; a merely non-silent
WAV is not sufficient.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import struct
import wave
from dataclasses import dataclass
from pathlib import Path

import numpy as np
from scipy import signal


SOURCE_IDS = (29, 25, 27, 17, 21, 20, 22, 28, 19, 18, 23, 24)
NAMES = (
    "Chassey Blue",
    "Slick Clyde",
    "Sheila",
    "John Torque",
    "Dave",
    "Convoy",
    "Loki",
    "Houston 3",
    "Boogie",
    "Beezwax",
    "Molo",
    "Sid Burn",
)
TARGET_BASE = 14
VOICE_RE = re.compile(
    r"\[V82SelectionVoice\] guest=(\d+).*?sample=(\d+).*?"
    r"audio_frame=(\d+)"
)
FILTERS = ((0, 0), (60, 0), (115, -52), (98, -55), (122, -60))


@dataclass(frozen=True)
class Entry:
    pitch: int
    encoded: bytes


def parse_bank(path: Path) -> list[Entry]:
    data = path.read_bytes()
    if len(data) < 4:
        raise ValueError(f"{path}: truncated SND header")
    count, size_in_8b = struct.unpack_from("<HH", data)
    table_end = 4 + count * 4
    if table_end > len(data):
        raise ValueError(f"{path}: truncated SND table")
    records = [
        struct.unpack_from("<HH", data, 4 + index * 4)
        for index in range(count)
    ]
    result: list[Entry] = []
    for index, (offset, pitch) in enumerate(records):
        end = records[index + 1][0] if index + 1 < count else size_in_8b
        start_byte = table_end + offset * 8
        end_byte = table_end + end * 8
        if offset > end or end_byte > len(data):
            raise ValueError(f"{path}: invalid entry {index}")
        result.append(Entry(pitch, data[start_byte:end_byte]))
    return result


def signed_nibble(value: int) -> int:
    return value - 16 if value > 7 else value


def decode_spu(entry: Entry) -> np.ndarray:
    previous_1 = 0
    previous_2 = 0
    decoded: list[int] = []
    for offset in range(0, len(entry.encoded) - 15, 16):
        frame = entry.encoded[offset:offset + 16]
        shift = frame[0] & 0x0F
        filter_index = frame[0] >> 4
        if filter_index >= len(FILTERS):
            raise ValueError(f"invalid SPU predictor {filter_index}")
        filter_1, filter_2 = FILTERS[filter_index]
        for byte in frame[2:]:
            for nibble in (byte & 0x0F, byte >> 4):
                sample = (signed_nibble(nibble) << 12) >> shift
                sample += (
                    previous_1 * filter_1 +
                    previous_2 * filter_2 + 32
                ) >> 6
                sample = max(-32768, min(32767, sample))
                decoded.append(sample)
                previous_2, previous_1 = previous_1, sample
        if frame[1] & 1:
            break
    return np.asarray(decoded, dtype=np.float64)


def at_output_rate(entry: Entry) -> np.ndarray:
    if entry.pitch <= 0:
        raise ValueError("SND pitch must be positive")
    # SPU pitch 0x1000 advances the ADPCM source by one sample per output
    # frame. Polyphase resampling is close enough to the hardware Gaussian
    # interpolator for an identity rank without pretending to be bit-exact.
    return signal.resample_poly(decode_spu(entry), 0x1000, entry.pitch)


def load_capture(path: Path) -> tuple[int, np.ndarray, str]:
    with wave.open(str(path), "rb") as stream:
        if stream.getnchannels() != 2 or stream.getsampwidth() != 2:
            raise ValueError(f"{path}: expected stereo S16 WAV")
        rate = stream.getframerate()
        pcm = stream.readframes(stream.getnframes())
    stereo = np.frombuffer(pcm, dtype="<i2").reshape(-1, 2)
    mono = stereo.astype(np.float64).mean(axis=1)
    return rate, mono, hashlib.sha256(pcm).hexdigest().upper()


def correlation(
    capture: np.ndarray,
    reference: np.ndarray,
    nominal_start: int,
    search_frames: int,
) -> tuple[float, int]:
    if len(reference) < 2:
        return 0.0, nominal_start
    begin = max(0, nominal_start - search_frames)
    end = min(
        len(capture),
        nominal_start + search_frames + len(reference),
    )
    window = capture[begin:end]
    if len(window) < len(reference):
        return 0.0, begin
    reference = reference - np.mean(reference)
    norm = np.linalg.norm(reference)
    if norm == 0:
        return 0.0, begin
    reference /= norm
    matched = signal.fftconvolve(window, reference[::-1], mode="valid")
    energy = np.sqrt(
        signal.fftconvolve(
            window * window,
            np.ones(len(reference)),
            mode="valid",
        )
    )
    scores = np.divide(
        matched,
        energy,
        out=np.zeros_like(matched),
        where=energy > 0,
    )
    index = int(np.argmax(np.abs(scores)))
    return float(abs(scores[index])), begin + index


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("log", type=Path)
    parser.add_argument("source_bank", type=Path)
    parser.add_argument("target_bank", type=Path)
    parser.add_argument("--guest", type=int)
    parser.add_argument("--json", type=Path)
    parser.add_argument("--search-ms", type=float, default=100.0)
    args = parser.parse_args()

    text = args.log.read_text(encoding="utf-8", errors="replace")
    matches = list(VOICE_RE.finditer(text))
    if not matches:
        raise ValueError(f"{args.log}: no timed selector-voice event")
    match = matches[-1]
    logged_guest = int(match.group(1))
    guest = logged_guest if args.guest is None else args.guest
    if guest != logged_guest:
        raise ValueError(
            f"requested guest {guest}, log contains guest {logged_guest}"
        )
    if not 0 <= guest < len(SOURCE_IDS):
        raise ValueError(f"guest {guest} is outside imported roster")
    logged_target = int(match.group(2))
    expected_target = TARGET_BASE + guest
    if logged_target != expected_target:
        raise ValueError(
            f"log played sample {logged_target}, expected {expected_target}"
        )
    audio_frame = int(match.group(3))

    source = parse_bank(args.source_bank)
    target = parse_bank(args.target_bank)
    byte_exact: list[bool] = []
    for index, source_id in enumerate(SOURCE_IDS):
        target_id = TARGET_BASE + index
        byte_exact.append(
            source[source_id].pitch == target[target_id].pitch and
            source[source_id].encoded == target[target_id].encoded
        )

    rate, capture, pcm_sha = load_capture(args.capture)
    if rate != 44100:
        raise ValueError(f"{args.capture}: expected 44100 Hz, got {rate}")
    highpass = signal.butter(3, 150.0 / (rate / 2.0), "highpass")
    filtered_capture = signal.sosfiltfilt(
        signal.tf2sos(*highpass), capture
    )
    search_frames = max(1, round(rate * args.search_ms / 1000.0))
    ranking: list[dict[str, object]] = []
    for index, source_id in enumerate(SOURCE_IDS):
        reference = signal.sosfiltfilt(
            signal.tf2sos(*highpass), at_output_rate(source[source_id])
        )
        score, start = correlation(
            filtered_capture,
            reference,
            audio_frame,
            search_frames,
        )
        ranking.append({
            "guest": index,
            "name": NAMES[index],
            "source_sample": source_id,
            "target_sample": TARGET_BASE + index,
            "score": round(score, 9),
            "start_frame": start,
            "start_seconds": round(start / rate, 6),
            "duration_seconds": round(len(reference) / rate, 6),
        })
    ranking.sort(key=lambda item: float(item["score"]), reverse=True)
    winner = int(ranking[0]["guest"])
    requested = next(item for item in ranking if item["guest"] == guest)
    checks = {
        "all_appended_samples_byte_exact": all(byte_exact),
        "logged_target_sample_exact": logged_target == expected_target,
        "captured_identity_rank_1": winner == guest,
        "captured_identity_score": float(requested["score"]) >= 0.05,
    }
    report = {
        "passed": all(checks.values()),
        "checks": checks,
        "guest": guest,
        "name": NAMES[guest],
        "audio_frame": audio_frame,
        "audio_seconds": round(audio_frame / rate, 6),
        "pcm_sha256": pcm_sha,
        "ranking": ranking,
    }
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(
            json.dumps(report, indent=2) + "\n",
            encoding="utf-8",
        )
    print(
        f"[SelectorVoiceCapture] {'PASS' if report['passed'] else 'FAIL'} "
        f"guest={guest} name={NAMES[guest]!r} winner={winner} "
        f"score={float(requested['score']):.6f} "
        f"audio_frame={audio_frame}"
    )
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
