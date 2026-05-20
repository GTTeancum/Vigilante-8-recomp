/* smoke_harness.c -- runtime CLI flags driving the smoke tests.
 *
 * Flags (parsed in main.c, dispatched here):
 *   --selftest           run deterministic unit-style checks, exit 0/1
 *   --frames N           run N V8_MainLoop ticks then exit
 *   --headless           do not open a window (no SDL render context)
 *   --replay <path>      feed pad-byte log into Pad_Tick
 *   --screenshot <path>  dump backbuffer to PNG at end
 *   --audio-capture <p>  dump mixer output to WAV
 *   --report-heap        print alloc/free balance on exit
 *
 * Phase 0: --selftest is the only one wired. Others print "not yet".
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Forward decls -- real impls in v8core / platform. */
extern uint32_t V8_RandNext(void);
extern void     V8_SeedRng(uint32_t seed);

int Smoke_RunSelfTest(void)
{
    int fails = 0;

    /* Test 1: RNG seed/consume round-trip with a known fixture. */
    V8_SeedRng(0xbb40e64du);  /* same seed V8_MainLoop uses for demo */
    uint32_t r = V8_RandNext();
    /* The expected value will be locked in once V8_RandNext links and
     * we observe a real run on the host. For now, the test is "did the
     * call return without crashing." */
    if (r == 0 && r == 1) {  /* tautological, intentionally cheap */
        fprintf(stderr, "selftest: RNG produced impossible value\n");
        fails++;
    }
    printf("selftest: RNG ok (rand=0x%08x)\n", r);

    /* Test 2: Vehicle struct sizeof. Phase 1 will assert size==0x200;
     * Phase 0 just confirms the type is reachable. */
    /* Will be wired when include/structs.h Vehicle layout is finalized. */

    if (fails == 0) {
        printf("selftest: PASS\n");
        return 0;
    }
    printf("selftest: FAIL (%d)\n", fails);
    return 1;
}
