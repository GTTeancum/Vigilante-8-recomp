using System.Numerics;

namespace RecompOne.Runtime.Enhanced;

/// <summary>Continuous projective UV weights for a convex planar face.</summary>
public static class PlanarQuadMapping
{
    // A planar face can split one straight texture edge at an intermediate
    // mesh vertex. Its UV must lie at the same fraction along that edge;
    // otherwise adjoining triangles bend a straight texture feature.
    public static bool TryStraightEdgeUv(Vector3 a, Vector3 b, Vector3 middle,
        Vector2 uvA, Vector2 uvB, Vector2 uvMiddle, out Vector2 corrected)
    {
        corrected = uvMiddle;
        Vector3 edge = b - a;
        float length = edge.LengthSquared();
        if (!float.IsFinite(length) || length < 0.0001f) return false;
        float t = Vector3.Dot(middle - a, edge) / length;
        // Integer-authored model vertices can differ by one source unit along
        // an otherwise straight edge. Accommodate that quantization across
        // model scales, while retaining visibly bent outlines.
        float tolerance = MathF.Max(0.1f, MathF.Min(1.01f, MathF.Sqrt(length) * 0.002f));
        if (t <= 0.01f || t >= 0.99f ||
            Vector3.Distance(a + t * edge, middle) > tolerance) return false;
        bool horizontal = uvA.Y == uvB.Y && uvMiddle.Y == uvA.Y && uvA.X != uvB.X;
        bool vertical = uvA.X == uvB.X && uvMiddle.X == uvA.X && uvA.Y != uvB.Y;
        if (!horizontal && !vertical) return false;
        float uvT = horizontal ? (uvMiddle.X - uvA.X) / (uvB.X - uvA.X)
            : (uvMiddle.Y - uvA.Y) / (uvB.Y - uvA.Y);
        if (uvT <= 0f || uvT >= 1f) return false;
        corrected = Vector2.Lerp(uvA, uvB, t);
        return Vector2.DistanceSquared(corrected, uvMiddle) > 0.0001f;
    }

    public static bool TryWeights(ReadOnlySpan<Vector3> p, Span<float> weights)
    {
        if (p.Length != 4 || weights.Length < 4) return false;
        Vector3 a = p[2] - p[0], b = p[3] - p[1], d = p[1] - p[0];
        Vector3 n = Vector3.Cross(a, b);
        float nn = n.LengthSquared();
        if (!float.IsFinite(nn) || nn < 0.0001f) return false;
        float s = Vector3.Dot(Vector3.Cross(d, b), n) / nn;
        float t = Vector3.Dot(Vector3.Cross(d, a), n) / nn;
        if (s <= 0.05f || s >= 0.95f || t <= 0.05f || t >= 0.95f)
            return false;
        // Reject nonplanar faces: changing their UV mapping would hide a
        // real crease. The small tolerance only accommodates GTE rounding.
        if (Vector3.Distance(p[0] + s * a, p[1] + t * b) > 0.1f)
            return false;
        // A true parallelogram already has one affine UV plane. Preserve
        // exactly unit weights instead of amplifying GTE float roundoff.
        if (MathF.Abs(s - 0.5f) < 0.0001f && MathF.Abs(t - 0.5f) < 0.0001f)
        {
            weights[..4].Fill(1f);
            return true;
        }
        weights[0] = 0.5f / (1f - s);
        weights[2] = 0.5f / s;
        weights[1] = 0.5f / (1f - t);
        weights[3] = 0.5f / t;
        return true;
    }
}
