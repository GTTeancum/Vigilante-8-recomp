using System.Globalization;
using System.Numerics;
using System.Reflection;
using System.Xml.Linq;
using Silk.NET.OpenGL;

namespace RecompOne.Runtime.Enhanced;

/// <summary>
/// Loads the authored V8:2 HUD SVGs and rasterizes them into a supersampled
/// atlas. This deliberately supports only the small SVG subset used by those
/// assets, keeping the single-file build free of an external SVG dependency.
/// </summary>
internal sealed class HudSvgAtlas : IDisposable
{
    public const int RasterScale = 8;
    public const int AtlasSize = 1024;

    readonly GL _gl;
    public uint Texture { get; }

    readonly record struct Tile(string File, int X, int Y, int Width, int Height);
    static readonly Tile[] Tiles =
    [
        new("v82_radar.svg", 0, 0, 64, 55),
        new("v82_status.svg", 0, 440, 90, 34),
        // Status occupies atlas x=0..719 after its measured six-pixel
        // left extension. Keep health in a disjoint aligned tile; the former
        // x=672 origin overwrote the status panel's final six native pixels.
        new("v82_health.svg", 736, 440, 16, 49),
    ];

    interface IPaint
    {
        Vector4 At(Vector2 point);
    }

    sealed record SolidPaint(Vector4 Color) : IPaint
    {
        public Vector4 At(Vector2 point) => Color;
    }

    readonly record struct Stop(float Offset, Vector4 Color);

    sealed class LinearPaint(Vector2 from, Vector2 to, Stop[] stops) : IPaint
    {
        public Vector4 At(Vector2 point)
        {
            Vector2 axis = to - from;
            float lengthSquared = Vector2.Dot(axis, axis);
            float t = lengthSquared > 1e-8f
                ? Vector2.Dot(point - from, axis) / lengthSquared
                : 0f;
            return SampleStops(stops, t);
        }
    }

    sealed class RadialPaint(Vector2 center, float radius, Stop[] stops) : IPaint
    {
        public Vector4 At(Vector2 point) =>
            SampleStops(stops, Vector2.Distance(point, center) / MathF.Max(radius, 1e-6f));
    }

    interface IGeometry
    {
        float SignedDistance(Vector2 point);
    }

    sealed record CircleGeometry(Vector2 Center, float Radius) : IGeometry
    {
        public float SignedDistance(Vector2 point) =>
            Vector2.Distance(point, Center) - Radius;
    }

    sealed record RectGeometry(Vector2 Center, Vector2 HalfSize) : IGeometry
    {
        public float SignedDistance(Vector2 point)
        {
            Vector2 q = Vector2.Abs(point - Center) - HalfSize;
            return Vector2.Distance(Vector2.Zero, Vector2.Max(q, Vector2.Zero)) +
                MathF.Min(MathF.Max(q.X, q.Y), 0f);
        }
    }

    sealed record LineGeometry(Vector2 A, Vector2 B) : IGeometry
    {
        public float SignedDistance(Vector2 point)
        {
            Vector2 edge = B - A;
            float denominator = MathF.Max(Vector2.Dot(edge, edge), 1e-8f);
            float t = Math.Clamp(Vector2.Dot(point - A, edge) / denominator, 0f, 1f);
            return Vector2.Distance(point, A + edge * t);
        }
    }

    sealed class PolygonGeometry(Vector2[] points) : IGeometry
    {
        public float SignedDistance(Vector2 point)
        {
            float distanceSquared = float.MaxValue;
            bool inside = false;
            int previous = points.Length - 1;
            for (int i = 0; i < points.Length; i++)
            {
                Vector2 a = points[i];
                Vector2 b = points[previous];
                Vector2 edge = b - a;
                Vector2 relative = point - a;
                float denominator = MathF.Max(Vector2.Dot(edge, edge), 1e-8f);
                Vector2 nearest = relative - edge *
                    Math.Clamp(Vector2.Dot(relative, edge) / denominator, 0f, 1f);
                distanceSquared = MathF.Min(distanceSquared, Vector2.Dot(nearest, nearest));
                if ((a.Y > point.Y) != (b.Y > point.Y) &&
                    point.X < (b.X - a.X) * (point.Y - a.Y) / (b.Y - a.Y) + a.X)
                    inside = !inside;
                previous = i;
            }
            return (inside ? -1f : 1f) * MathF.Sqrt(distanceSquared);
        }
    }

    sealed record Shape(
        IGeometry Geometry,
        IPaint? Fill,
        IPaint? Stroke,
        float StrokeWidth,
        bool StrokeOnly);

    public HudSvgAtlas(GL gl)
    {
        _gl = gl;
        ValidateTiles();
        byte[] rgba = new byte[AtlasSize * AtlasSize * 4];
        foreach (Tile tile in Tiles)
            Rasterize(LoadSvg(tile.File), tile, rgba);

        Texture = gl.GenTexture();
        gl.BindTexture(TextureTarget.Texture2D, Texture);
        gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureMinFilter, (int)GLEnum.LinearMipmapLinear);
        gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureMagFilter, (int)GLEnum.Linear);
        gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureWrapS, (int)GLEnum.ClampToEdge);
        gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureWrapT, (int)GLEnum.ClampToEdge);
        gl.PixelStore(PixelStoreParameter.UnpackAlignment, 1);
        gl.TexImage2D<byte>(
            TextureTarget.Texture2D, 0, InternalFormat.Rgba8,
            AtlasSize, AtlasSize, 0, PixelFormat.Rgba,
            PixelType.UnsignedByte, rgba);
        gl.GenerateMipmap(TextureTarget.Texture2D);
    }

    static void ValidateTiles()
    {
        for (int i = 0; i < Tiles.Length; i++)
        {
            Tile a = Tiles[i];
            int ax1 = a.X + a.Width * RasterScale;
            int ay1 = a.Y + a.Height * RasterScale;
            if (a.X < 0 || a.Y < 0 ||
                ax1 > AtlasSize || ay1 > AtlasSize)
                throw new InvalidDataException(
                    $"HUD SVG atlas tile is out of bounds: {a.File}");

            for (int j = i + 1; j < Tiles.Length; j++)
            {
                Tile b = Tiles[j];
                int bx1 = b.X + b.Width * RasterScale;
                int by1 = b.Y + b.Height * RasterScale;
                bool overlaps =
                    a.X < bx1 && ax1 > b.X &&
                    a.Y < by1 && ay1 > b.Y;
                if (overlaps)
                    throw new InvalidDataException(
                        $"HUD SVG atlas tiles overlap: {a.File}, {b.File}");
            }
        }
    }

    public void Dispose()
    {
        if (Texture != 0)
            _gl.DeleteTexture(Texture);
    }

    static XDocument LoadSvg(string file)
    {
        string[] candidates =
        [
            Path.Combine(Runtime.ExecutableDirectory, "SHARED", "HUD", file),
            Path.Combine(Environment.CurrentDirectory, "SHARED", "HUD", file),
        ];
        foreach (string candidate in candidates.Distinct(StringComparer.OrdinalIgnoreCase))
            if (File.Exists(candidate))
                return XDocument.Parse(File.ReadAllText(candidate), LoadOptions.None);

        Assembly assembly = typeof(HudSvgAtlas).Assembly;
        string suffix = "." + file;
        string? resource = assembly.GetManifestResourceNames()
            .FirstOrDefault(n => n.EndsWith(suffix, StringComparison.OrdinalIgnoreCase));
        if (resource == null)
            throw new FileNotFoundException($"HUD vector asset not found: {file}");
        using Stream stream = assembly.GetManifestResourceStream(resource)!;
        return XDocument.Load(stream, LoadOptions.None);
    }

    static void Rasterize(XDocument document, Tile tile, byte[] atlas)
    {
        XElement root = document.Root ?? throw new InvalidDataException($"{tile.File}: missing SVG root");
        var gradients = ParseGradients(root);
        List<Shape> shapes = ParseShapes(root, gradients);
        int pixelWidth = tile.Width * RasterScale;
        int pixelHeight = tile.Height * RasterScale;

        for (int y = 0; y < pixelHeight; y++)
        {
            for (int x = 0; x < pixelWidth; x++)
            {
                Vector2 point = new(
                    (x + 0.5f) / RasterScale,
                    (y + 0.5f) / RasterScale);
                Vector4 color = Vector4.Zero;
                foreach (Shape shape in shapes)
                {
                    float distance = shape.Geometry.SignedDistance(point);
                    if (shape.Fill != null && !shape.StrokeOnly)
                    {
                        float coverage = Math.Clamp(0.5f - distance * RasterScale, 0f, 1f);
                        Composite(ref color, shape.Fill.At(point), coverage);
                    }
                    if (shape.Stroke != null && shape.StrokeWidth > 0f)
                    {
                        float strokeDistance = shape.StrokeOnly
                            ? distance - shape.StrokeWidth * 0.5f
                            : MathF.Abs(distance) - shape.StrokeWidth * 0.5f;
                        float coverage = Math.Clamp(0.5f - strokeDistance * RasterScale, 0f, 1f);
                        Composite(ref color, shape.Stroke.At(point), coverage);
                    }
                }

                int atlasX = tile.X + x;
                int atlasY = tile.Y + y;
                int offset = (atlasY * AtlasSize + atlasX) * 4;
                // Premultiply before mip generation so transparent atlas
                // padding cannot produce a dark fringe around vector edges.
                atlas[offset + 0] = ToByte(color.X * color.W);
                atlas[offset + 1] = ToByte(color.Y * color.W);
                atlas[offset + 2] = ToByte(color.Z * color.W);
                atlas[offset + 3] = ToByte(color.W);
            }
        }
    }

    static void Composite(ref Vector4 destination, Vector4 source, float coverage)
    {
        float alpha = Math.Clamp(source.W * coverage, 0f, 1f);
        float outAlpha = alpha + destination.W * (1f - alpha);
        if (outAlpha <= 1e-8f)
        {
            destination = Vector4.Zero;
            return;
        }
        Vector3 rgb = (new Vector3(source.X, source.Y, source.Z) * alpha +
            new Vector3(destination.X, destination.Y, destination.Z) *
            destination.W * (1f - alpha)) / outAlpha;
        destination = new Vector4(rgb, outAlpha);
    }

    static Dictionary<string, IPaint> ParseGradients(XElement root)
    {
        var result = new Dictionary<string, IPaint>(StringComparer.Ordinal);
        foreach (XElement element in root.Descendants())
        {
            string name = element.Name.LocalName;
            if (name is not ("linearGradient" or "radialGradient"))
                continue;
            string? id = Attr(element, "id");
            if (string.IsNullOrEmpty(id))
                continue;
            Stop[] stops = element.Elements()
                .Where(e => e.Name.LocalName == "stop")
                .Select(ParseStop)
                .OrderBy(s => s.Offset)
                .ToArray();
            if (stops.Length == 0)
                continue;

            if (name == "linearGradient")
            {
                Vector2 from = new(Float(element, "x1"), Float(element, "y1"));
                Vector2 to = new(Float(element, "x2"), Float(element, "y2"));
                result[id] = new LinearPaint(from, to, stops);
            }
            else
            {
                Vector2 center = new(Float(element, "cx"), Float(element, "cy"));
                result[id] = new RadialPaint(center, Float(element, "r"), stops);
            }
        }
        return result;
    }

    static Stop ParseStop(XElement element)
    {
        float offset = Scalar(Attr(element, "offset") ?? "0", percent: true);
        Vector4 color = ParseColor(Attr(element, "stop-color") ?? "#000000");
        color.W *= Scalar(Attr(element, "stop-opacity") ?? "1");
        return new Stop(offset, color);
    }

    static List<Shape> ParseShapes(XElement root, Dictionary<string, IPaint> gradients)
    {
        var shapes = new List<Shape>();
        foreach (XElement element in root.Descendants())
        {
            string name = element.Name.LocalName;
            IGeometry? geometry = name switch
            {
                "circle" => new CircleGeometry(
                    new Vector2(Float(element, "cx"), Float(element, "cy")),
                    Float(element, "r")),
                "rect" => Rect(element),
                "line" => new LineGeometry(
                    new Vector2(Float(element, "x1"), Float(element, "y1")),
                    new Vector2(Float(element, "x2"), Float(element, "y2"))),
                "polygon" => Polygon(element),
                _ => null,
            };
            if (geometry == null)
                continue;

            IPaint? fill = Paint(Attr(element, "fill"), gradients);
            IPaint? stroke = Paint(Attr(element, "stroke"), gradients);
            float opacity = Scalar(Attr(element, "opacity") ?? "1");
            fill = WithOpacity(fill, Scalar(Attr(element, "fill-opacity") ?? "1") * opacity);
            stroke = WithOpacity(stroke, Scalar(Attr(element, "stroke-opacity") ?? "1") * opacity);
            bool strokeOnly = name == "line";
            shapes.Add(new Shape(
                geometry, fill, stroke,
                Float(element, "stroke-width", stroke == null ? 0f : 1f),
                strokeOnly));
        }
        return shapes;
    }

    static IGeometry Rect(XElement element)
    {
        float x = Float(element, "x");
        float y = Float(element, "y");
        float width = Float(element, "width");
        float height = Float(element, "height");
        return new RectGeometry(
            new Vector2(x + width * 0.5f, y + height * 0.5f),
            new Vector2(width * 0.5f, height * 0.5f));
    }

    static IGeometry Polygon(XElement element)
    {
        string points = Attr(element, "points") ??
            throw new InvalidDataException("SVG polygon is missing points");
        float[] values = points
            .Split([' ', ','], StringSplitOptions.RemoveEmptyEntries)
            .Select(v => Scalar(v))
            .ToArray();
        if (values.Length < 6 || (values.Length & 1) != 0)
            throw new InvalidDataException("SVG polygon has invalid points");
        var result = new Vector2[values.Length / 2];
        for (int i = 0; i < result.Length; i++)
            result[i] = new Vector2(values[i * 2], values[i * 2 + 1]);
        return new PolygonGeometry(result);
    }

    static IPaint? Paint(string? value, Dictionary<string, IPaint> gradients)
    {
        if (string.IsNullOrWhiteSpace(value) ||
            value.Equals("none", StringComparison.OrdinalIgnoreCase))
            return null;
        if (value.StartsWith("url(#", StringComparison.Ordinal) && value.EndsWith(')'))
        {
            string id = value[5..^1];
            return gradients.TryGetValue(id, out IPaint? paint)
                ? paint
                : throw new InvalidDataException($"Unknown SVG paint: {value}");
        }
        return new SolidPaint(ParseColor(value));
    }

    static IPaint? WithOpacity(IPaint? paint, float opacity)
    {
        if (paint == null || opacity >= 0.9999f)
            return paint;
        return new OpacityPaint(paint, opacity);
    }

    sealed record OpacityPaint(IPaint Inner, float Opacity) : IPaint
    {
        public Vector4 At(Vector2 point)
        {
            Vector4 color = Inner.At(point);
            color.W *= Opacity;
            return color;
        }
    }

    static Vector4 ParseColor(string value)
    {
        if (value.Length == 7 && value[0] == '#')
        {
            byte r = byte.Parse(value.AsSpan(1, 2), NumberStyles.HexNumber);
            byte g = byte.Parse(value.AsSpan(3, 2), NumberStyles.HexNumber);
            byte b = byte.Parse(value.AsSpan(5, 2), NumberStyles.HexNumber);
            return new Vector4(r / 255f, g / 255f, b / 255f, 1f);
        }
        throw new InvalidDataException($"Unsupported SVG color: {value}");
    }

    static Vector4 SampleStops(Stop[] stops, float value)
    {
        float t = Math.Clamp(value, 0f, 1f);
        if (t <= stops[0].Offset)
            return stops[0].Color;
        for (int i = 1; i < stops.Length; i++)
        {
            if (t > stops[i].Offset)
                continue;
            float width = MathF.Max(stops[i].Offset - stops[i - 1].Offset, 1e-8f);
            return Vector4.Lerp(stops[i - 1].Color, stops[i].Color,
                (t - stops[i - 1].Offset) / width);
        }
        return stops[^1].Color;
    }

    static float Float(XElement element, string name, float fallback = 0f)
    {
        string? value = Attr(element, name);
        return value == null ? fallback : Scalar(value);
    }

    static string? Attr(XElement element, string name) =>
        element.Attribute(name)?.Value;

    static float Scalar(string value, bool percent = false)
    {
        string text = value.Trim();
        bool isPercent = text.EndsWith('%');
        if (isPercent)
            text = text[..^1];
        float result = float.Parse(text, NumberStyles.Float, CultureInfo.InvariantCulture);
        return (percent && isPercent) ? result / 100f : result;
    }

    static byte ToByte(float value) =>
        (byte)Math.Clamp((int)MathF.Round(value * 255f), 0, 255);
}
