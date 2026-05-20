/* xobf_parse.c -- parse an XOBF object container.
 *
 * Source: SLUS_005.10  FUN_8002263c.
 *
 * XOBF is the inner format inside an EXP / FORM file that bundles
 * one model with its animation and optional sound effects. Walks
 * sub-chunks via Iff_ReadChunkHeader and dispatches on their 4cc:
 *
 *   "BIN " -- raw model bytes; alloc + Stream_Read into `model`.
 *   "ANM " -- animation track; alloc + Stream_Read into `anim`.
 *   "SND " -- sound bank; consume by SND parser, store handle in `snd`.
 *
 * When the parent's byte counter reaches zero, finalises by calling
 * `Renderer_BuildObject(model, anim)` -- FUN_8001a640 -- which returns
 * the in-memory object handle. If `chainRegister` is non-zero, the
 * object is appended to the global draw chain via FUN_8001a91c. If a
 * sound was parsed, its handle is patched into obj[+8].
 *
 * HIGH confidence: 4cc dispatch is unambiguous; per-chunk action
 * matches the SND streamer's contract.
 *
 * Used by:
 *   - VehicleExp_Load (src/assets/vehicle_exp.c) to load every
 *     character's vehicle template.
 *   - Likely also by terrain EXP loaders for level-specific objects.
 *
 * Pass 2: rename FUN_8001a640 -> Renderer_BuildObject (it's on the
 * renderer seam; the *call* stays here, but the implementation moves
 * to src/skipped/ as the new renderer's contract).
 */
#include <stdint.h>

extern uint32_t Iff_ReadChunkHeader(uint32_t *hdrOut, uint32_t *parentRemaining);
extern void    *Heap_AllocOrRetry(uint32_t n);
extern void     Stream_Read(void *dst, uint32_t n);
extern uint32_t Stream_Tell(void);
extern void     Stream_Seek(uint32_t pos, int relative);
extern void    *Audio_ParseSND(void);
extern void    *Renderer_BuildObject(void *model, void *anim);   /* FUN_8001a640 */
extern void     DrawChain_Register(void *obj);                    /* FUN_8001a91c */

#define BE_BIN   0x42494e20u   /* "BIN " */
#define BE_ANM   0x414e4d20u   /* "ANM " */
#define BE_SND   0x534e4420u   /* "SND " */

static inline uint32_t bswap32_le_to_be(uint32_t v)
{
    return (v >> 24) | ((v >> 8) & 0xff00u) | ((v & 0xff00u) << 8) | (v << 24);
}

void *XOBF_Parse(uint32_t remaining, int chainRegister)
{
    void *model = NULL;
    void *anim  = NULL;
    void *snd   = NULL;

    while (remaining != 0) {
        uint32_t hdr[2];
        uint32_t payloadSize = Iff_ReadChunkHeader(hdr, &remaining);
        uint32_t fcc = bswap32_le_to_be(hdr[0]);

        if (fcc == BE_BIN) {
            model = Heap_AllocOrRetry(payloadSize);
            Stream_Read(model, payloadSize);
        } else if (fcc == BE_ANM) {
            anim = Heap_AllocOrRetry(payloadSize);
            Stream_Read(anim, payloadSize);
        } else if (fcc == BE_SND) {
            uint32_t start = Stream_Tell();
            snd = Audio_ParseSND();
            Stream_Seek(start + payloadSize, 0);
        }
        /* unknown 4cc: implicit skip -- Iff_ReadChunkHeader has already
         * accounted for the bytes; we just don't consume the payload.
         * Strictly speaking we should Stream_Seek past, but the
         * original relies on the chunk-payload being absent for unknowns. */
    }

    void *obj = Renderer_BuildObject(model, anim);
    if (chainRegister != 0) DrawChain_Register(obj);
    if (snd != NULL) {
        *(void **)((uint8_t *)obj + 8) = snd;
    }
    return obj;
}
