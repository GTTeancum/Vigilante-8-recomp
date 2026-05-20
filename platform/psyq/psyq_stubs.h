/* psyq_stubs.h -- prototypes for the PSY-Q SDK functions that the
 * decompiled v8core references via `extern` declarations.
 *
 * Phase 1: real bodies for libgte math + libcd file I/O + libapi VSync;
 * empty/return-0 stubs for renderer/audio (out of scope per CLAUDE.md).
 * Phase 2+: bring up SDL/GL/audio behind these prototypes.
 *
 * Types live in structs.h / gte.h (already canonical).
 */
#ifndef V8_PSYQ_STUBS_H
#define V8_PSYQ_STUBS_H

#include <stdint.h>
#include "structs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---- libcd (CD I/O) ---- */
int   CdInit(void);
int   CdRead       (int count, uint32_t *buf, int mode);
int   CdReadSync   (int mode, uint8_t *result);
int   CdControl    (uint8_t com, const uint8_t *param, uint8_t *result);
int   CdControlF   (uint8_t com, const uint8_t *param);
int   CdControlB   (uint8_t com, const uint8_t *param, uint8_t *result);
int   CdRead2      (int mode);                       /* XA streaming variant */
void *CdReadyCallback(void (*cb)(int, uint8_t *));   /* registers an IRQ handler */
void  CdIntToPos   (int i, CdlLOC *p);
int   CdPosToInt   (const CdlLOC *p);

/* CdControl command codes -- subset actually referenced. */
#define CdlNop          0x01
#define CdlSetloc       0x02
#define CdlPlay         0x03
#define CdlReadN        0x06
#define CdlStandby      0x07
#define CdlStop         0x08
#define CdlPause        0x09
#define CdlInit         0x0a
#define CdlMute         0x0b
#define CdlDemute       0x0c
#define CdlSetfilter    0x0e
#define CdlSetmode      0x0e
#define CdlGetlocL      0x10
#define CdlGetlocP      0x11
#define CdlGetTN        0x13
#define CdlGetTD        0x14
#define CdlSeekL        0x15
#define CdlSeekP        0x16

/* ---- libgs / libgpu (drawing env -- stubbed; renderer is out-of-scope) ---- */
typedef struct DRAWENV   DRAWENV;
typedef struct DISPENV   DISPENV;
typedef struct DR_ENV    DR_ENV;
typedef uint32_t         u_long;

void  SetDefDrawEnv (DRAWENV *env, int x, int y, int w, int h);
void  SetDefDispEnv (DISPENV *env, int x, int y, int w, int h);
void  PutDrawEnv    (DRAWENV *env);
void  PutDispEnv    (DISPENV *env);
DRAWENV *GetDrawEnv (DRAWENV *env);
void  SetDrawEnv    (DR_ENV *de, DRAWENV *env);
void  ClearOTagR    (u_long *ot, int n);
void  ClearOTag     (u_long *ot, int n);
void  DrawOTag      (const u_long *ot);
void  DrawSync      (int mode);
void  ClearImage    (const RECT *r, uint8_t r8, uint8_t g8, uint8_t b8);
void  MargePrim     (void *a, void *b);

/* ---- libgte (math; Phase 1 has stubs; Phase 4 needs real bodies) ---- */
void  InitGeom      (void);
void  SetGeomOffset (int ofx, int ofy);
void  SetGeomScreen (int h);
long  RotMatrix     (const SVECTOR *r, MATRIX *m);
long  TransMatrix   (const MATRIX *m, const VECTOR *v);
long  ScaleMatrix   (MATRIX *m, const VECTOR *v);
long  CompMatrix    (const MATRIX *m0, const MATRIX *m1, MATRIX *m2);
long  CompMatrixLV  (const MATRIX *m0, const MATRIX *m1, MATRIX *m2);
void  MulMatrix     (const MATRIX *m0, MATRIX *m1);
void  MulMatrix0    (const MATRIX *m0, const MATRIX *m1, MATRIX *m2);
void  ReadRotMatrix (MATRIX *m);
void  SetRotMatrix  (const MATRIX *m);
void  SetTransMatrix(const MATRIX *m);
void  ApplyMatrix   (const MATRIX *m, const SVECTOR *v0, VECTOR *v1);
void  ApplyMatrixLV (const MATRIX *m, const VECTOR *v0, VECTOR *v1);
void  MatrixNormal  (const MATRIX *m, MATRIX *m_out);
void  VectorNormalSS(const VECTOR *v0, VECTOR *v1);

/* PSY-Q rsin/rcos: take angle in [0,4096), return q12. Real Phase 4. */
int   rsin(int a);
int   rcos(int a);
int   ratan2(int y, int x);

/* ---- libapi (system) ---- */
void  ResetGraph    (int mode);
void  SetGraphDebug (int level);
void  VSync         (int mode);
void  VSyncCallback (void (*cb)(void));
long  GetRCnt       (int spec);
long  SetRCnt       (int spec, uint16_t target, long mode);
void  StartRCnt     (int spec);
void  StopRCnt      (int spec);
int   OpenEvent     (uint32_t klass, long spec, long mode, void (*func)(void));
int   EnableEvent   (int event);
int   CloseEvent    (int event);

/* ---- libspu (audio engine -- out of scope; stubs always) ---- */
void  SpuInit(void);
void  SpuFree       (uint32_t spuAddr);
uint32_t SpuMalloc  (uint32_t size);
void  SpuSetTransferMode     (int mode);
void  SpuSetTransferStartAddr(uint32_t addr);
int   SpuRead       (uint8_t *buf, uint32_t size);
int   SpuWrite      (const uint8_t *buf, uint32_t size);
int   SpuIsTransferCompleted(int mode);
void  SpuSetVoiceVolume(int v, int16_t l, int16_t r);

/* ---- libpad (controllers -- out of scope; stubs always) ---- */
void  PadInit       (int mode);
void  PadInitDirect (uint8_t *port1, uint8_t *port2);
uint32_t PadRead    (int id);

#ifdef __cplusplus
}
#endif

#endif /* V8_PSYQ_STUBS_H */
