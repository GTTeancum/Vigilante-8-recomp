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

    public static void SetTexture(uint id, int w, int h, float aspect = 0f)
        => (_texId, _texW, _texH, _aspect) = (id, w, h, aspect > 0f ? aspect : 4f / 3f);

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
