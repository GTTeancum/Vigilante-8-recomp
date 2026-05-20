/* psyq_stubs.c -- Phase 0 placeholder PSY-Q stub bodies.
 *
 * This file exists so the psyq_stubs static library has at least one
 * compilation unit and CMake stops complaining. Phase 1 splits this
 * into libgs / libgte / libcd / libapi and adds real bodies for the
 * in-scope subset (libgte math, libcd file I/O, libapi VSync/RCnt).
 *
 * Right now no symbols are defined here -- the v8core .c files contain
 * the `extern` decls, but Phase 0 does not yet attempt to link them.
 */

/* Compiler keep-alive: prevents "empty translation unit" warnings on
 * MSVC and "no symbols" errors on GNU ar. */
int v8_psyq_stubs_phase0_placeholder = 0;
