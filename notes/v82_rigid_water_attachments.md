# Rigid water attachment anchoring

The user explicitly approved correcting the original PS1 attachment placement programmatically while keeping the complete pontoons rigid. This supersedes the earlier requirement to leave the original spacing untouched; the rejected vertex-stretching experiment remains disconnected.

`V82WaterAttachmentPlacement` finds the body-facing end of each shared water support and locates a chassis socket on the visible body mesh. It moves the complete support subtree to that socket during native packet generation. Vertices, dimensions, local rotations, animated children, source assets and native physics contacts are unchanged. A render scope restores the three modified matrix-translation words afterward.

The solver casts from outside the chassis to avoid moving a support that is already embedded through the opposite wall. If the end does not face a body surface horizontally, it uses the nearest body surface. Hidden body subtrees are excluded. Vehicle-authored transformation overrides are preserved; stock and imported shared assemblies use the same bank/slot qualification, without vehicle-name exceptions.

Native folding changes the mode before swapping models at transition tick 16. The correction follows a reversible smoothstep over the water-visible half of that transition, including the old water mesh during retraction. The resting socket is solved without the temporary parent's fold rotation; the actual parent's inverse converts the visual offset back into local coordinates. The native fold and outboard animation remain intact.

Socket solutions are cached by vehicle, body geometry/pose and support rest pose. World movement and the outboard rotor animation do not trigger a new solve. No runtime vertex copies or PS1 heap allocations are needed.

## Validation

- 384 transformation contract assertions pass, including 35 new anchoring checks: entry/exit continuity, mirrored transition trajectories, inverse compensation under a rotated parent, embedded-support preservation, authored-assembly exclusion, unchanged source vertices and child state, and byte-exact restoration of the entire synthetic vehicle/mesh memory region after drawing.
- Initial native Sheila test (`artifacts/rigid-attachments-sheila`) completed repeated water/dry transitions through tick 1050 with clean exit. Individually inspected frame 150 shows the visible support roots joining the chassis, with unmodified pontoon shapes. Water partly obscures the joints.
- Final staged build: `V8_2_LOOSE/Vigilante82PC.exe`, SHA256 `C598A42408DD7DA52D155CCEE13711B374AA0EA652BDF2D5075F42ACD5790475`.
- Final native inspection covered Sheila, Torque, Garbage (six supports), imported Chassey, and authored Obake. All five processes exited cleanly. Shared assemblies visibly join at rest; Obake remains excluded. Evidence is in `artifacts/rigid-attachments-{sheila-final,torque,garbage,chassey,obake}/visual-review.json`.
- Chassey retained one false aggregate land-sample expectation: after returning to wheels at samples 480-570, it coasted back into water at tick 573. The final repeated land interval passed. This is documented in its visual review; the aggregate is not represented as fully passing.
- Individually inspected all 37 requested sampled animation captures across `artifacts/rigid-attachments-animation` and `artifacts/rigid-attachments-fold-boundaries`. Deployed supports are joined and retain their shapes. Native folding still briefly separates roots before/after deployment (notably boundary captures 102-104 and 376-378); the correction intentionally preserves that fold and mesh swap. Both runs passed repeated water/dry checks and exited cleanly. Capture presentation may lag the diagnostic tick.
- Coverage is five representative vehicles and sampled entry/exit views, not all vehicles, all frames, or all angles. This is an intentional visual placement correction, not a claim of original PS1 equivalence.

Native screenshots and input use only the game's process-local fixture. No desktop input/capture or performance benchmark was used. Timing fields recorded automatically by the fixture are not an efficiency result.

The 526 mesh clipping/provenance assertions also pass. `artifacts/rigid-attachments-sheila-final/source-preservation.json` records 30 exact comparisons against the independently executed original PS1 game: all ten rendered parts retain their original vertices, scale, and native local position outside the render scope.
