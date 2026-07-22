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
    static bool _loggedSet;
    static bool _loggedDraw;

    public static void SetTexture(uint id, int w, int h, float aspect = 0f)
    {
        (_texId, _texW, _texH, _aspect) = (id, w, h, aspect > 0f ? aspect : 4f / 3f);
        if (_loggedSet) return;
        _loggedSet = true;
        Console.WriteLine($"[Host] output texture id={id} size={w}x{h} aspect={_aspect:F3}");
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
            var imageSize = FitAspect(new Vector2(_aspect, 1f), avail);
            var offset = (avail - imageSize) * 0.5f;
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
