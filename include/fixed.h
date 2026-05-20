#ifndef V8_FIXED_H
#define V8_FIXED_H

#include <stdint.h>

/* PSX fixed-point types used throughout Vigilante 8.
 * 16.16 signed for world coordinates, velocities, most physics quantities.
 * 4.12 signed (4096 = 360 degrees) for angles, matching the PSY-Q convention
 * (rsin/rcos take values in [0, 4096)).
 *
 * Convention: never convert to float in 1:1 code. The renderer may convert at
 * the seam, but the physics tick must stay in fixed.
 */

typedef int32_t fixed16_t;   /* 16.16 signed -- 1.0 == 0x00010000 */
typedef int32_t fixed20_t;   /* 12.20 signed (occasional GTE intermediate) */
typedef int16_t angle12_t;   /* 4.12 signed, 4096 == 360 deg */
typedef int16_t short_q12_t; /* 4.12 signed intermediate */

#define FIXED16_ONE   0x00010000
#define FIXED16_HALF  0x00008000
#define ANGLE_360     4096
#define ANGLE_180     2048
#define ANGLE_90      1024

/* 16.16 * 16.16 -> 16.16 with truncation toward zero (PSY-Q-style). */
static inline fixed16_t fmul16(fixed16_t a, fixed16_t b) {
    return (fixed16_t)(((int64_t)a * (int64_t)b) >> 16);
}

/* 16.16 / 16.16 -> 16.16. */
static inline fixed16_t fdiv16(fixed16_t a, fixed16_t b) {
    return (fixed16_t)(((int64_t)a << 16) / b);
}

#define FIXED16_FROM_INT(x)  ((fixed16_t)((x) << 16))
#define FIXED16_TO_INT(x)    ((int32_t)((x) >> 16))

#endif /* V8_FIXED_H */
