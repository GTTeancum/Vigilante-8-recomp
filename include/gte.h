#ifndef V8_GTE_H
#define V8_GTE_H

#include <stdint.h>
#include "fixed.h"

/* PSX Geometry Transformation Engine -- declarations only.
 * Implementations must be bit-exact (matching saturation, rounding, overflow).
 * Reference: PCSX-Redux GTE C implementation. The port will either link that
 * code or vendor a verified copy under tools/gte/.
 *
 * NEVER substitute naive C math for these operations.
 */

typedef struct { int16_t vx, vy, vz, pad; } SVECTOR;     /* short vector, q12 */
typedef struct { int32_t vx, vy, vz;       } VECTOR;     /* long vector, q16  */
typedef struct {
    int16_t m[3][3];   /* q12 rotation */
    int16_t pad;
    int32_t t[3];      /* q16 translation */
} MATRIX;

typedef struct { uint8_t r, g, b, cd; } CVECTOR;          /* color + code */

/* GTE control register accessors */
void   gte_set_rot_matrix(const MATRIX *m);
void   gte_set_translation(const VECTOR *t);

/* RTPS / RTPT -- perspective transform, single / triple vertex */
void   gte_RTPS(const SVECTOR *v0);
void   gte_RTPT(const SVECTOR *v0, const SVECTOR *v1, const SVECTOR *v2);

/* MVMVA -- matrix * vector + vector. Many modes; preserve cop2 control bits. */
void   gte_MVMVA(int mx, int v, int cv, int sf, int lm);

/* NCLIP -- backface cull via cross product of three screen points */
int32_t gte_NCLIP(void);

/* AVSZ3 / AVSZ4 -- average screen Z, fixed weights */
int32_t gte_AVSZ3(void);
int32_t gte_AVSZ4(void);

/* OP -- outer product (cross). SQR -- square of IR vector. */
void   gte_OP(int sf);
void   gte_SQR(int sf);

/* DPCS / DPCT -- depth cueing. GPF / GPL -- general purpose interpolation. */
void   gte_DPCS(void);
void   gte_GPF(int sf);
void   gte_GPL(int sf);

/* IR / SXY / SZ / RGB register read/write accessors as needed.
 * PSY-Q macros gte_stsxy(), gte_ldxy(), etc., map to these. */

/* LZCS/LZCR: leading-bit count helper (used by sqrt normalisation).
 * Writing LZCS updates LZCR to the count of leading bits that match
 * the sign bit. */
void    gte_ldLZCS(int32_t v);
int32_t gte_stLZCR(void);

/* PSY-Q BIOS integer sqrt: floor(sqrt(n)) for unsigned n. */
long SquareRoot0(long n);

#endif /* V8_GTE_H */
