using System.Numerics;
using ImGuiNET;

namespace RecompOne.Runtime.Host.Window;

internal sealed class OutputPanel : IPanel
{
    public string Name => "Output";
    public bool IsOpen { get; set; } = true;

    static uint _texId;
    static int _texW, _texH;
    static float _aspect = 4f / 3f;
    static Vector2 _available;
    static Vector2 _framebufferScale = Vector2.One;
    static bool _loggedSet;
    static bool _loggedDraw;
    static float _lastLoggedAspect = -1f;
    static Vector2 _availableMin;
    static Vector2 _availableSize;
    static Vector2 _imageMin;
    static Vector2 _imageSize;

    public static bool TryGetCompositionLayout(
        out Vector2 availableMin,
        out Vector2 availableSize,
        out Vector2 imageMin,
        out Vector2 imageSize)
    {
        availableMin = _availableMin;
        availableSize = _availableSize;
        imageMin = _imageMin;
        imageSize = _imageSize;
        return availableSize.X > 0f && availableSize.Y > 0f &&
            imageSize.X > 0f && imageSize.Y > 0f;
    }

    public static void SetTexture(uint id, int w, int h, float aspect = 0f)
    {
        (_texId, _texW, _texH, _aspect) = (id, w, h, aspect > 0f ? aspect : 4f / 3f);
        if (!_loggedSet)
        {
            _loggedSet = true;
            Console.WriteLine(
                $"[Host] output texture id={id} size={w}x{h} " +
                $"aspect={_aspect:F3}");
        }
        if (MathF.Abs(_aspect - _lastLoggedAspect) >= 0.01f)
        {
            _lastLoggedAspect = _aspect;
            Console.WriteLine(
                $"[OutputAspect] texture={w}x{h} aspect={_aspect:F6} " +
                $"gameplay=" +
                $"{(Hle.GpuHle.GameplayActive ? 1 : 0)}");
        }
    }

    public static void InvalidateWidescreenPresentation()
    {
        _texId = 0;
        _texW = 0;
        _texH = 0;
        _aspect = Hle.GpuHle.BaseAspect;
        _imageMin = Vector2.Zero;
        _imageSize = Vector2.Zero;
    }

    public static (int w, int h) GetPresentationSize(float aspect, int fallbackW, int fallbackH)
    {
        Vector2 available = _available;
        Vector2 scale = _framebufferScale;
        if (available.X <= 0f || available.Y <= 0f)
        {
            available = new Vector2(fallbackW, fallbackH);
            scale = Vector2.One;
        }

        var fitted = FitAspect(new Vector2(aspect > 0f ? aspect : 4f / 3f, 1f), available);
        return (Math.Max(1, (int)MathF.Round(fitted.X * scale.X)),
                Math.Max(1, (int)MathF.Round(fitted.Y * scale.Y)));
    }

    public void Draw()
    {
        ImGui.SetNextWindowSize(new Vector2(640, 480), ImGuiCond.FirstUseEver);

        bool open = IsOpen;
        if (!ImGui.Begin(Name, ref open))
        {
            IsOpen = open;
            ImGui.End();
            return;
        }

        if (_texId != 0 && _texW > 0 && _texH > 0)
        {
            var avail = ImGui.GetContentRegionAvail();
            _available = avail;
            _framebufferScale = ImGui.GetIO().DisplayFramebufferScale;
            var imageSize = FitAspect(new Vector2(_aspect, 1f), avail);
            var offset = (avail - imageSize) * 0.5f;
            var availableMin = ImGui.GetCursorScreenPos();
            var imageMin = availableMin + offset;
            _availableMin = availableMin * _framebufferScale;
            _availableSize = avail * _framebufferScale;
            _imageMin = imageMin * _framebufferScale;
            _imageSize = imageSize * _framebufferScale;
            // A 16:9 gameplay target becomes a 4:3 authored shell image on
            // match exit. Explicitly paint the newly exposed host regions
            // every frame so they cannot retain the previous wide image.
            ImGui.GetWindowDrawList().AddRectFilled(
                availableMin,
                availableMin + avail,
                0xFF000000);
            if (!_loggedDraw)
            {
                _loggedDraw = true;
                Console.WriteLine($"[Host] drawing output texture id={_texId} available={avail.X:F0}x{avail.Y:F0} image={imageSize.X:F0}x{imageSize.Y:F0}");
            }
            ImGui.SetCursorPos(ImGui.GetCursorPos() + offset);
            ImGui.Image((nint)_texId, imageSize);
        }

        IsOpen = open;
        ImGui.End();
    }

    static Vector2 FitAspect(Vector2 src, Vector2 dst)
    {
        float scale = MathF.Min(dst.X / src.X, dst.Y / src.Y);
        return src * scale;
    }
}
