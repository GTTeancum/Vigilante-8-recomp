"""make_steer_replay.py -- generate a steering test replay file.

Pad layout:
  0x10000000 = forward (accel)
  0x80000000 = left steer
  0x20000000 = right steer

Sequence:
  frames   0-59:  forward only          (warm up, let vehicle settle on terrain)
  frames  60-179: forward + steer right (120 frames = 2 log events at 60,120 tick intervals)
  frames 180-299: forward + steer left  (120 frames = 2 log events)
  frames 300-359: forward only          (coast)
"""
import struct, sys

frames = []
FWD   = 0x10000000
LEFT  = 0x80000000
RIGHT = 0x20000000

# 60 frames: forward only (warm-up)
for _ in range(60):
    frames.append(FWD)

# 120 frames: forward + steer right
for _ in range(120):
    frames.append(FWD | RIGHT)

# 120 frames: forward + steer left
for _ in range(120):
    frames.append(FWD | LEFT)

# 60 frames: coast
for _ in range(60):
    frames.append(FWD)

out_path = 'tools/steer_test.replay'
with open(out_path, 'wb') as f:
    for v in frames:
        f.write(struct.pack('<I', v))

print(f"Written {len(frames)} frames ({len(frames)*4} bytes) to {out_path}")
print(f"Total: {len(frames)} frames")
