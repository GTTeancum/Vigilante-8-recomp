using System.Text;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Host;
using RecompOne.Runtime.Hle;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Sdk;

public static class V82ControlPrompts
{
    public static void EndReadyPrompt() => InputManager.EndReadyPrompt();

    public static IDisposable? RewriteFooter(CpuContext c, IMemory m)
    {
        int count = Math.Abs((long)(int)c.A0) <= 16 ? Math.Abs((int)c.A0) : 0;
        if (count == 0 || c.A1 < 0x80010000 || c.A1 >= 0x80800000) return null;
        var result = new StringBuilder();
        uint pointer = c.A1;
        bool changed = false;
        for (int entry = 0; entry < count; entry++)
        {
            var original = new StringBuilder();
            for (int i = 0; i < 256; i++)
            {
                byte value = m.ReadU8(pointer++);
                if (value == 0) break;
                original.Append((char)value);
            }
            string text = original.ToString();
            string replacement = InputPromptLabels.Format(text, ConfigManager.Game, 0,
                InputManager.PromptUsesGamepad(0), false, true);
            changed |= replacement != text;
            result.Append(replacement).Append('\0');
        }
        // Rewrite before the native compositor measures and spaces the labels.
        return changed && result.Length <= 384 ? new TextScope(c, m, result.ToString()) : null;
    }

    public static IDisposable? Rewrite(CpuContext c, IMemory m, bool gameplayPrompt = false)
    {
        if (c.A1 < 0x80010000 || c.A1 >= 0x80800000) return null;
        Span<byte> bytes = stackalloc byte[256];
        int length = 0;
        bool candidate = m.ReadU8(c.A1) is (byte)'P' or (byte)'p';
        for (uint i = 0; i < 256; i++)
        {
            byte value = m.ReadU8(c.A1 + i);
            if (value == 0) break;
            bytes[length++] = value;
            if (value is >= 0x80 and <= 0x83) candidate = true;
        }
        if (!candidate) return null;
        string text = Encoding.Latin1.GetString(bytes[..length]);
        if (InputPromptLabels.IsReadyPrompt(text))
        {
            InputManager.SignalReadyPrompt();
            InputManager.SignalScriptStage("v82_ready_prompt", captureDelayPolls: 12);
        }
        string replacement = InputPromptLabels.Format(text, ConfigManager.Game, 0,
            InputManager.PromptUsesGamepad(0), GpuHle.GameplayActive || gameplayPrompt,
            InputManager.NativeMenuActive && !gameplayPrompt);
        if (replacement == text || replacement.Length > 384) return null;
        return new TextScope(c, m, replacement);
    }

    // Borrow stack storage for the duration of the native draw, not a global
    // scratch address that can collide with overlays or nested font calls.
    // Copy the caller's stack arguments before shifting SP; all other pointer
    // arguments still refer to their original, untouched memory.
    sealed class TextScope : IDisposable
    {
        readonly CpuContext cpu;
        readonly uint stack, text;
        public TextScope(CpuContext c, IMemory m, string replacement)
        {
            cpu = c; stack = c.SP; text = c.A1;
            c.SP -= 512;
            for (uint i = 0; i < 64; i++) m.WriteU8(c.SP + i, m.ReadU8(stack + i));
            c.A1 = c.SP + 64;
            byte[] data = Encoding.Latin1.GetBytes(replacement);
            for (uint i = 0; i < data.Length; i++) m.WriteU8(c.A1 + i, data[i]);
            m.WriteU8(c.A1 + (uint)data.Length, 0);
        }
        public void Dispose() { cpu.SP = stack; cpu.A1 = text; }
    }
}
