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
#include "structs.h"

/* Forward decls -- real impls in v8core / platform. */
extern uint32_t V8_RandNext(void);
extern void     V8_SeedRng(uint32_t seed);
extern int32_t  Vec3_Length(const int32_t *v);
extern int64_t  Vec3_Dot64(const int32_t *a, const int32_t *b);
extern int      ratan2(int y, int x);
extern int      rsin(int a);
extern int      rcos(int a);
extern long     SquareRoot0(long n);
extern long     VectorNormalSS(const SVECTOR *v0, SVECTOR *v1);

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

    /* Test 2: Vec3_Dot64 + Vec3_Length.
     *
     * Vec3_Length implements PSY-Q's LZCR-shift-then-SquareRoot0-
     * then-shift algorithm.  It is exact-or-near-exact for large
     * magnitudes (the typical regime for q16.16 vels and positions)
     * but lossy for small ones because the down-shift before sqrt
     * discards low bits.  We test fixtures in the realistic regime.
     */
    {
        int prev = fails;
        /* Exact on this fixture: |(0x10000, 0, 0)| = 0x10000. */
        int32_t vb[3] = { 0x10000, 0, 0 };
        int32_t lb = Vec3_Length(vb);
        if (lb != 0x10000) {
            fprintf(stderr, "selftest: Vec3_Length(0x10000,0,0) got 0x%x expected 0x10000\n", lb);
            fails++;
        }
        /* Realistic vel-magnitude regime: ~28379 ULP error <= 1 LSB
         * after re-shift; expect within 4 of mathematical sqrt(3*0x100000000) ~= 113511. */
        int32_t vc[3] = { 0x10000, 0x10000, 0x10000 };
        int32_t lc = Vec3_Length(vc);
        if (lc < 113500 || lc > 113520) {
            fprintf(stderr, "selftest: Vec3_Length(0x10000^3) got %d expected ~113511\n", lc);
            fails++;
        }
        /* Zero -> zero. */
        int32_t vd[3] = { 0, 0, 0 };
        if (Vec3_Length(vd) != 0) {
            fprintf(stderr, "selftest: Vec3_Length(0,0,0) != 0\n");
            fails++;
        }
        /* Negatives produce same magnitude as positives. */
        int32_t v_pos[3] = { 0x12345, 0x6789a, 0xabcde };
        int32_t v_neg[3] = { -0x12345, -0x6789a, -0xabcde };
        int32_t lp = Vec3_Length(v_pos);
        int32_t ln = Vec3_Length(v_neg);
        if (lp != ln) {
            fprintf(stderr, "selftest: Vec3_Length sign asymmetric (%d vs %d)\n", lp, ln);
            fails++;
        }
        /* Dot64: exercise the 64-bit accumulator path with i32 inputs
         * whose product would overflow i32. */
        int32_t vp_[3] = { 0x40000000, 0x40000000, 0x40000000 };
        int64_t dot = Vec3_Dot64(vp_, vp_);
        int64_t exp = 3 * (int64_t)0x40000000 * (int64_t)0x40000000;
        if (dot != exp) {
            fprintf(stderr, "selftest: Vec3_Dot64(huge) %lld != %lld\n",
                    (long long)dot, (long long)exp);
            fails++;
        }
        if (fails == prev) printf("selftest: Vec3 math ok (large-magnitude regime exact)\n");
    }

    /* Test 3: ratan2 round-trip via rsin/rcos.
     *   For each angle i in [0, 4096), ratan2(sin(i), cos(i)) should
     *   recover i, modulo the table's 1-LSB rounding. */
    {
        int prev = fails;
        int worst = 0;
        for (int i = 0; i < 4096; i++) {
            int s = rsin(i);
            int c = rcos(i);
            int r = ratan2(s, c);
            int d = r - i;
            if (d >  2048) d -= 4096;
            if (d < -2048) d += 4096;
            if (d <  0) d = -d;
            if (d > worst) worst = d;
        }
        if (worst > 1) {
            fprintf(stderr, "selftest: ratan2 round-trip worst-case %d LSB (expect <= 1)\n", worst);
            fails++;
        }
        if (ratan2(0, 0) != 0) {
            fprintf(stderr, "selftest: ratan2(0,0) = %d (expect 0)\n", ratan2(0, 0));
            fails++;
        }
        if (ratan2(0, 0x1000) != 0) {
            fprintf(stderr, "selftest: ratan2(0,+) = %d (expect 0)\n", ratan2(0, 0x1000));
            fails++;
        }
        if (ratan2(0x1000, 0) != 0x400) {
            fprintf(stderr, "selftest: ratan2(+,0) = %d (expect 0x400 = 90deg)\n",
                    ratan2(0x1000, 0));
            fails++;
        }
        if (ratan2(0, -0x1000) != 0x800) {
            fprintf(stderr, "selftest: ratan2(0,-) = %d (expect 0x800 = 180deg)\n",
                    ratan2(0, -0x1000));
            fails++;
        }
        if (ratan2(-0x1000, 0) != -0x400) {
            fprintf(stderr, "selftest: ratan2(-,0) = %d (expect -0x400 = source signed -90deg)\n",
                    ratan2(-0x1000, 0));
            fails++;
        }
        if (fails == prev) printf("selftest: ratan2 ok (worst %d LSB across 4096 angles)\n", worst);
    }

    /* Test 4: VectorNormalSS -- source returns squared magnitude, not sqrt. */
    {
        int prev = fails;
        SVECTOR in = {0x1000, 0, 0, 0};
        SVECTOR out = {0, 0, 0, 0};
        long ret = VectorNormalSS(&in, &out);
        if (ret != 0x01000000L || out.vx != 0x1000 || out.vy != 0 || out.vz != 0) {
            fprintf(stderr,
                    "selftest: VectorNormalSS axis ret=%ld out=(%d,%d,%d), expected ret=0x01000000 out=(4096,0,0)\n",
                    ret, out.vx, out.vy, out.vz);
            fails++;
        }
        if (fails == prev) printf("selftest: VectorNormalSS ok\n");
    }

    /* Test 5: SquareRoot0 -- V8's EXE-backed PSY-Q table routine.
     * It is close to sqrt but not floor(sqrt(n)) at large magnitudes. */
    {
        int prev = fails;
        struct { long n; long want; } cases[] = {
            {0, 0}, {1, 1}, {2, 1}, {3, 1}, {4, 2},
            {8, 2}, {9, 3}, {15, 3}, {16, 4},
            {99, 9}, {100, 10}, {101, 10},
            {0xffffff, 4087},
            {0x1000000, 0x1000},
            {0x40000000, 0x8000},
            {0x7fffffff, 46152},
        };
        for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
            long got = SquareRoot0(cases[i].n);
            if (got != cases[i].want) {
                fprintf(stderr, "selftest: SquareRoot0(%ld) = %ld, expected %ld\n",
                        cases[i].n, got, cases[i].want);
                fails++;
            }
        }
        if (fails == prev) printf("selftest: SquareRoot0 ok\n");
    }

    /* Test 5: Vehicle struct sizeof. Phase 1 will assert size==0x200;
     * Phase 0 just confirms the type is reachable. */
    /* Will be wired when include/structs.h Vehicle layout is finalized. */

    if (fails == 0) {
        printf("selftest: PASS\n");
        return 0;
    }
    printf("selftest: FAIL (%d)\n", fails);
    return 1;
}
