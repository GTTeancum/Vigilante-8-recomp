using System.Text;
using RecompOne.Runtime;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;

namespace Recompiled;

public static partial class Vigilante82PC
{
    // Audio is a rewritten subsystem. Keep the game's selection/count globals,
    // but resolve songs against the host playlist rather than expanding the
    // retail TOC buffer or indexing beyond its fifteen song-name pointers.
    static class V82NativeAudio
    {
        public static bool Refresh(CpuContext c, IMemory m)
        {
            if (Runtime.Cd?.HasSoundtrackSelection != true) return false;
            int count = Runtime.Cd.LastTrackNumber - 1;
            m.WriteU8(c.GP + 0x839u, (byte)count);
            if (m.ReadU8(c.GP + 0x83Au) >= count)
                m.WriteU8(c.GP + 0x83Au, 0);
            return true;
        }

        public static bool Play(CpuContext c, IMemory m)
        {
            if (!Refresh(c, m)) return false;
            int count = Runtime.Cd!.LastTrackNumber - 1;
            int index = ((int)c.A0 % count + count) % count;
            bool repeat = c.A1 != 0;
            var snapshot = c.Snapshot();
            try
            {
                m.WriteU8(c.GP + 0x83Au, (byte)index);
                // The host loops on the exact PCM-sector boundary. The native
                // callback still updates the current song, but must not also
                // restart it using a rounded, one-second CD position report.
                m.WriteU32(c.GP + 0xE10u, int.MaxValue);
                c.A0 = 0x8001D78Cu;
                c.RA = 0x8001D980u;
                func_80055E14(c, m);
                LibCd.PlaySoundtrack(index, repeat);
            }
            finally { c.Restore(snapshot); }
            return true;
        }

        public static void ChangeSoundtrack(CpuContext c, IMemory m, int direction)
        {
            var mode = SoundtrackSettings.Cycle(ConfigManager.Game.Soundtrack, direction);
            LibCd.ChangeSoundtrack(mode);
            ConfigManager.Game.Soundtrack = mode;
            ConfigManager.SaveGame();
            // A pinned track from the old album cannot retain its old index.
            m.WriteU8(0x8006A822u, 0);
            var snapshot = c.Snapshot();
            try { c.A0 = 0; c.A1 = 0; Play(c, m); }
            finally { c.Restore(snapshot); }
        }

        public static bool TrackTitle(CpuContext c, IMemory m)
        {
            if (Runtime.Cd?.HasSoundtrackSelection != true) return false;
            int index = m.ReadU8(c.GP + 0xC14u);
            string source = Runtime.Cd.SoundtrackSource(index) ?? "";
            string stem = Path.GetFileNameWithoutExtension(source);
            if (stem.StartsWith("track", StringComparison.OrdinalIgnoreCase) &&
                int.TryParse(stem.AsSpan(5), out int number) && number is >= 2 and <= 16)
            {
                c.A1 = m.ReadU32(0x800639B0u + (uint)(number - 2) * 4u);
                return true;
            }
            string title = stem.Equals("track17", StringComparison.OrdinalIgnoreCase)
                ? "Second Offense 16" : "V8: " + stem;
            byte[] bytes = Encoding.ASCII.GetBytes(title);
            uint address = c.SP + 0x28u; // The retail Track-number text buffer.
            int length = Math.Min(bytes.Length, 31);
            for (int i = 0; i < length; i++) m.WriteU8(address + (uint)i, bytes[i]);
            m.WriteU8(address + (uint)length, 0);
            c.A1 = address;
            return true;
        }

        public static void Volume(CpuContext c, IMemory m, uint address, int direction)
        {
            int value = Math.Clamp(m.ReadU16(address) + Math.Sign(direction) * 512, 0, 0x3FFF);
            m.WriteU16(address, (ushort)value);
            ApplyMix(c, m);
        }

        public static void ApplyMix(CpuContext c, IMemory m)
        {
            var snapshot = c.Snapshot();
            try
            {
                c.A0 = m.ReadU8(0x8006B02Bu) == 0 ? 1u : 0u;
                c.A1 = m.ReadU16(0x8006B60Eu);
                c.A2 = m.ReadU16(0x8006B60Cu);
                func_8001DD0C(c, m);
            }
            finally { c.Restore(snapshot); }
        }

        public static void Track(CpuContext c, IMemory m, int direction)
        {
            var snapshot = c.Snapshot();
            try
            {
                int count = Runtime.Cd!.LastTrackNumber - 1;
                int index = (m.ReadU8(0x8006B02Au) + Math.Sign(direction) + count) % count;
                int mode = (sbyte)m.ReadU8(0x8006A822u);
                if (mode != 0)
                    m.WriteU8(0x8006A822u, unchecked((byte)(Math.Sign(mode) * (index + 1))));
                c.A0 = (uint)index;
                c.A1 = mode > 0 ? 1u : 0u;
                Play(c, m);
            }
            finally { c.Restore(snapshot); }
        }

        public static void Playback(CpuContext c, IMemory m, int direction)
        {
            int value = (sbyte)m.ReadU8(0x8006A822u);
            int mode = value == 0 ? 0 : value < 0 ? 1 : 2;
            mode = (mode + Math.Sign(direction) + 3) % 3;
            int track = m.ReadU8(0x8006B02Au) + 1;
            m.WriteU8(0x8006A822u, unchecked((byte)(mode == 0 ? 0 : mode == 1 ? -track : track)));
            Track(c, m, 0);
        }
    }
}
