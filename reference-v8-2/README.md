# Vigilante 8: 2nd Offense PC reference

Target: a smooth, playable PC build of the USA release (`SLUS-00868`) through
RecompOne, with vehicle physics and collision stability treated as early
acceptance gates.

The implementation reuses the first game's generic PS1 runtime and presentation
work. Game-specific patches, callback addresses, object layouts, automation,
and loose-media manifests are not inherited without sequel-binary evidence.

Generated recompilation output and retail assets remain local and are ignored
by Git.

The host also supports a complete standalone loose-file tree for all 128 data
files, Mode 2 movie/XA streams, and 16 CD-audio tracks. See
`reference-v8-2/LOOSE_FILES.md`.

## Initial acceptance order

1. Recompile the main executable and all loadable DLLs.
2. Reach boot movies and the front end with stable audio/video pacing.
3. Enter one arena with responsive controls and a live vehicle.
4. Stabilize spawn, suspension, steering, braking, collision, and recovery.
5. Exercise weapons, damage, pause, result, teardown, and replay.
6. Expand bounded soaks across every arena and both single/split-screen modes.
