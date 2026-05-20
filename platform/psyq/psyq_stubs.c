/* psyq_stubs.c -- Phase 1 PSY-Q stub bodies.
 *
 * Empty/return-0 bodies for every PSY-Q function the decompiled
 * v8core references. Phase 1 goal is *link clean*, not behave; later
 * phases fill in real bodies for the in-scope subset (libgte math,
 * libcd file I/O, libapi VSync/RCnt).
 *
 * Renderer/audio/controls remain trivial stubs per CLAUDE.md -- they
 * are being rewritten in a separate effort.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "psyq_stubs.h"

/* ---- libcd ---- */
int   CdInit(void)                                                          { return 1; }
int   CdRead       (int count, uint32_t *buf, int mode)                     { (void)count; (void)buf; (void)mode; return 1; }
int   CdReadSync   (int mode, uint8_t *result)                              { (void)mode; (void)result; return 0; }
int   CdControl    (uint8_t com, const uint8_t *param, uint8_t *result)     { (void)com; (void)param; (void)result; return 1; }
int   CdControlF   (uint8_t com, const uint8_t *param)                      { (void)com; (void)param; return 1; }
int   CdControlB   (uint8_t com, const uint8_t *param, uint8_t *result)     { (void)com; (void)param; (void)result; return 1; }
int   CdRead2      (int mode)                                               { (void)mode; return 0; }
void *CdReadyCallback(void (*cb)(int, uint8_t *))                           { (void)cb; return NULL; }
void  CdIntToPos   (int i, CdlLOC *p)                                       { (void)i; if (p) memset(p, 0, sizeof(*p)); }
int   CdPosToInt   (const CdlLOC *p)                                        { (void)p; return 0; }

/* ---- libgs / libgpu (renderer-adjacent) ---- */
struct DRAWENV { uint8_t opaque[0x5c]; };
struct DISPENV { uint8_t opaque[0x14]; };
struct DR_ENV  { uint8_t opaque[0x5c]; };

void  SetDefDrawEnv (DRAWENV *env, int x, int y, int w, int h)              { (void)env; (void)x; (void)y; (void)w; (void)h; }
void  SetDefDispEnv (DISPENV *env, int x, int y, int w, int h)              { (void)env; (void)x; (void)y; (void)w; (void)h; }
void  PutDrawEnv    (DRAWENV *env)                                          { (void)env; }
void  PutDispEnv    (DISPENV *env)                                          { (void)env; }
DRAWENV *GetDrawEnv (DRAWENV *env)                                          { return env; }
void  SetDrawEnv    (DR_ENV *de, DRAWENV *env)                              { (void)de; (void)env; }
void  ClearOTagR    (u_long *ot, int n)                                     { if (ot && n > 0) memset(ot, 0, n * sizeof(u_long)); }
void  ClearOTag     (u_long *ot, int n)                                     { if (ot && n > 0) memset(ot, 0, n * sizeof(u_long)); }
void  DrawOTag      (const u_long *ot)                                      { (void)ot; }
void  DrawSync      (int mode)                                              { (void)mode; }
void  ClearImage    (const RECT *r, uint8_t r8, uint8_t g8, uint8_t b8)     { (void)r; (void)r8; (void)g8; (void)b8; }
void  MargePrim     (void *a, void *b)                                      { (void)a; (void)b; }

/* ---- libgte: all real implementations live in libgte.c.
 * Symbols formerly stubbed here:
 *   InitGeom, SetGeomOffset, SetGeomScreen, RotMatrix, TransMatrix,
 *   ScaleMatrix, CompMatrix, CompMatrixLV, MulMatrix, MulMatrix0,
 *   ReadRotMatrix, SetRotMatrix, SetTransMatrix, ApplyMatrix,
 *   ApplyMatrixLV, MatrixNormal, VectorNormalSS, rsin, rcos, ratan2.
 * GTE primitive ops (gte_ldR11R12, gte_rtir_b, gte_stIR1, ...) also
 * live in libgte.c. */

/* ---- libapi ---- */
void  ResetGraph    (int mode)                                              { (void)mode; }
void  SetGraphDebug (int level)                                             { (void)level; }
void  VSync         (int mode)                                              { (void)mode; }
void  VSyncCallback (void (*cb)(void))                                      { (void)cb; }
long  GetRCnt       (int spec)                                              { (void)spec; return 0; }
long  SetRCnt       (int spec, uint16_t target, long mode)                  { (void)spec; (void)target; (void)mode; return 0; }
void  StartRCnt     (int spec)                                              { (void)spec; }
void  StopRCnt      (int spec)                                              { (void)spec; }
int   OpenEvent     (uint32_t klass, long spec, long mode, void (*func)(void)) { (void)klass; (void)spec; (void)mode; (void)func; return 0; }
int   EnableEvent   (int event)                                             { (void)event; return 0; }
int   CloseEvent    (int event)                                             { (void)event; return 0; }

/* ---- libspu ---- */
void  SpuInit(void)                                                         { }
void  SpuFree       (uint32_t spuAddr)                                      { (void)spuAddr; }
uint32_t SpuMalloc  (uint32_t size)                                         { (void)size; return 0x1000; }
void  SpuSetTransferMode     (int mode)                                     { (void)mode; }
void  SpuSetTransferStartAddr(uint32_t addr)                                { (void)addr; }
int   SpuRead       (uint8_t *buf, uint32_t size)                           { (void)buf; (void)size; return 0; }
int   SpuWrite      (const uint8_t *buf, uint32_t size)                     { (void)buf; (void)size; return (int)size; }
int   SpuIsTransferCompleted(int mode)                                      { (void)mode; return 1; }
void  SpuSetVoiceVolume(int v, int16_t l, int16_t r)                        { (void)v; (void)l; (void)r; }

/* ---- libpad ---- */
void  PadInit       (int mode)                                              { (void)mode; }
void  PadInitDirect (uint8_t *port1, uint8_t *port2)                        { (void)port1; (void)port2; }
uint32_t PadRead    (int id)                                                { (void)id; return 0; }
