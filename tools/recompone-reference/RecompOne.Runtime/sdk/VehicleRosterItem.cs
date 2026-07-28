namespace RecompOne.Runtime.Sdk;

/// <summary>
/// Decoded, engine-facing identity for an independently authored vehicle.
/// This is deliberately metadata-only: no retail slot, archive donor, or
/// opaque front-end record is part of the selection contract.
/// </summary>
public readonly record struct VehicleRosterItem(
    int Type,
    string StableId,
    string DisplayName,
    int SelectionOrder);
