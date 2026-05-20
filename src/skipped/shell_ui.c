/* shell_ui.c -- SHELL.DLL UI/menu state machine (OUT OF SCOPE).
 *
 * Per CLAUDE.md, controls AND renderer/UI are to be rewritten, not
 * decompiled. SHELL.DLL ships the entire pre-match shell -- splash
 * screens, profile select, vehicle select, level select, options
 * menus, demo player, and the inter-match shell. None of these
 * affect physics, asset format parsing, or in-match gameplay
 * (apart from setting the inputs that V8_MainLoop reads: chosen
 * vehicle, level, mode, demo seed).
 *
 * The seam contract:
 *   - On entry, SHELL exports a function-table entry that returns
 *     the level path string (e.g. "Track\OilField.TER") to be loaded.
 *     V8_MainLoop calls this via Overlay_Open("Shell\\Shell.dll").
 *   - On exit, SHELL writes the selected vehicle/mode into:
 *       bRam00000015      g_matchMode (0=quest, 1/2=versus, 3=split,
 *                                     4=team, 5=demo)
 *       DAT_80065674[2]   per-player vehicle IDs
 *       UNK_8006567a[8]   per-bank enable flags
 *       cRam00000600      current quest sub-stage
 *       iRam00000018/19   display offset X/Y
 *
 *   - SHELL also draws its own UI -- all 72 stub functions below are
 *     internal to that UI dispatch:
 *
 *     state-machine roots (the per-screen tick handlers):
 *       0x801012f8  Shell_TickSplash
 *       0x801014d0  Shell_TickProfile
 *       0x80101704  Shell_TickVehicleSelect
 *       0x8010a704..0x8010ae40  Shell_TickQuestProgress + variants
 *
 *     menu sprite/label helpers (build menu rows):
 *       0x8010c784, 0x8010d034, 0x8010d1c8, 0x8010d2c0,
 *       0x8010d654, 0x8010d754
 *
 *     transition / fade / slide animators:
 *       0x8010dca4, 0x8010dcc4, 0x8010dcf0, 0x8010df1c,
 *       0x8010e150, 0x8010e210, 0x8010e2b4
 *
 *     button/dpad/icon emitters:
 *       0x8010e350..0x8010e75c  (small per-glyph helpers)
 *       0x8010ec60, 0x8010edf0, 0x8010ee60, 0x8010eeac,
 *       0x8010eefc, 0x8010ef3c, 0x8010efa4
 *
 *     credit roll / option-list scrollers:
 *       0x8010f178, 0x8010f1e0, 0x8010f414, 0x8010f59c,
 *       0x8010f5e0, 0x8010f698, 0x8010f814, 0x8010f8cc,
 *       0x8010fa48, 0x8010fb60, 0x8010fc68, 0x8010fd80,
 *       0x8010fe88
 *
 *     vehicle-stat radar/chart drawers:
 *       0x80110210, 0x80110418, 0x801105d4, 0x80110660,
 *       0x80110768, 0x80110adc, 0x80110c00, 0x80110c7c,
 *       0x80110dd0
 *
 *     final layer (level-thumbnail + "press start" pulser):
 *       0x80110fcc, 0x80111080, 0x80111188, 0x80111260,
 *       0x8011142c, 0x8011173c, 0x801117cc, 0x8011185c,
 *       0x8011188c, 0x801118c4, 0x8011193c, 0x80111974
 *
 * Total: 72 functions, all renderer/UI bound. The rewritten shell
 * can reproduce the same input->output contract (chosen vehicle,
 * mode, level, seed) with any modern UI framework; none of the bit
 * patterns matter for the bit-exact physics goal.
 *
 * SHELL.DLL fixed entry stubs (10 empty function-table slots
 * shipped by the original) remain in src/gameplay/shell_stubs.c
 * since those slots ARE part of the export contract -- their
 * indices into the SHELL function table must line up.
 */
