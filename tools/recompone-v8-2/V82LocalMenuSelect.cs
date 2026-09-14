using RecompOne.Runtime.Config;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Hardware;
using RecompOne.Runtime.Host;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;

namespace Recompiled;

public static partial class Vigilante82PC
{
    static partial class V82LocalMenu
    {
        public static bool SelectPlayers(CpuContext c, IMemory m)
        {
            int count = LocalInputSession.PlayerCount;
            if (count < 2 || m.ReadU8(c.GP + 0x31) != 5) return false;
            var saved = c.Snapshot();
            uint firstMask = c.A0, secondMask = c.A1, scratch = 0, font = 0, title = 0, previewWindow = 0;
            var roster = Enumerable.Range(0, 18).Where(t => (firstMask & (1u << t)) == 0)
                .Concat(V82VehicleRegistry.Roster().Select(v => v.Type)).ToArray();
            int[] types = roster.Take(count).ToArray();
            if (types.Length != count) throw new InvalidOperationException("Not enough selectable vehicles");
            if (types[1] < 18 && (secondMask & (1u << types[1])) != 0)
                types[1] = roster.First(t => !types.Contains(t) && (t >= 18 || (secondMask & (1u << t)) == 0));
            var previews = new V82VehicleRegistry.LocalMenuPreview[count];
            var acceptedColors = new Dictionary<(int Player, int Type), uint>();
            uint Color(int player, int type) => acceptedColors.TryGetValue((player, type), out uint color)
                ? color : V82VehicleRegistry.LocalMenuColor(m, type, player);
            uint[] colors = types.Select((t, p) => Color(p, t)).ToArray();
            bool[] ready = new bool[count], editing = new bool[count];
            ushort[] previous = Enumerable.Range(0, count).Select(p => (ushort)~Controller.GetState(p)).ToArray();
            bool cancelled = false;
            string Button(int player, ushort action) => InputPromptLabels.Button(ConfigManager.Game,
                player, action, LocalMultiplayerInput.UsesGamepad(player), false, true).ToUpperInvariant();
            uint Call(Action setup, Action<CpuContext, IMemory> function)
            {
                var registers = c.Snapshot();
                setup(); c.RA = 0x801081B4;
                function(c, m); uint result = c.V0;
                c.Restore(registers); return result;
            }
            uint Rect(int x, int y, int width, int height)
            {
                uint address = scratch + 640;
                m.WriteU16(address, (ushort)x); m.WriteU16(address + 2, (ushort)y);
                m.WriteU16(address + 4, (ushort)width); m.WriteU16(address + 6, (ushort)height);
                return address;
            }
            void Text(string value, int x, int y, int width, bool heading = false)
            {
                Write(m, scratch, value);
                uint layout = Rect(x, y, width, 32);
                for (uint i = 0; i < 8; i++) m.WriteU8(layout + 8 + i, m.ReadU8(layout + i));
                uint selectedFont = heading ? title : font;
                m.WriteU32(selectedFont + 4, (m.ReadU32(selectedFont + 4) & 0xFF000000) | (heading ? 0x006078u : 0x808080u));
                Call(() => { c.A0 = selectedFont; c.A1 = scratch; c.A2 = layout; c.A3 = heading ? 0x444Au : 2u; }, func_8001A3B0);
            }
            void Fill(int x, int y, int width, int height, uint color)
            {
                uint rect = Rect(x, y, width, height);
                Call(() => { c.A0 = rect; c.A1 = color; }, func_8001ADF8);
            }
            string Name(int type)
            {
                string? custom = V82VehicleRegistry.NameForType(type);
                if (custom != null) return custom;
                uint address = m.ReadU32(0x8006383Cu + (uint)type * 16 + 12);
                var chars = new List<char>();
                for (uint i = 0; i < 80 && m.ReadU8(address + i) != 0; i++) chars.Add((char)m.ReadU8(address + i));
                return new string(chars.ToArray());
            }
            try
            {
                c.SP -= 0x200;
                scratch = Call(() => c.A0 = 1024, V82Compat.PcMalloc);
                Call(() => { }, func_80014894);
                V82VehicleRegistry.LocalMenuScene(true, c, m);
                Call(() => c.A0 = firstMask & secondMask, func_80103AA4);
                Call(() => { }, func_8010289C);
                Call(() => c.A0 = 0, func_80021C24);
                Call(() => c.A0 = 1, func_8001AB64);
                Call(() => { }, func_8001AAFC);
                // The retail 2P preview initializes both environment materials
                // and the light matrix before constructing the vehicle models.
                m.WriteU16(0x8006B63A, 128);
                m.WriteU16(0x8006B63C, 128);
                m.WriteU16(0x8006B640, 0x110);
                Call(() => { c.A0 = 0x801015B0; c.A1 = 0x8006B648; }, func_800212E4);
                Call(() => { }, func_8002DE5C);
                uint root = m.ReadU32(0x80116730);
                font = Call(() => { c.A0 = root + m.ReadU32(root + 0x10); c.A1 = 1; }, func_80019294);
                title = Call(() => { c.A0 = root + m.ReadU32(root + 4); c.A1 = 1; }, func_80019294);
                // All panels render sequentially at the same preview size.
                // Reserve one native render surface before model uploads and
                // reuse it; allocating a fresh surface after every model swap
                // can hit the original fatal VRAM-allocation path.
                uint previewRect = Rect(0, 0, 280, 104);
                previewWindow = Call(() => { c.A0 = previewRect; c.A1 = 0; c.A2 = 0; c.A3 = 0x758080; }, func_8001B1F4);
                for (int p = 0; p < count; p++) previews[p] = V82VehicleRegistry.CreateLocalMenuPreview(c, m, types[p], colors[p]);
                LocalMultiplayerInput.SignalStage("multiplayer_select");
                Console.Error.WriteLine($"[LocalSelection] simultaneous panels={count}");
                int frame = 0;
                while (!cancelled && !ready.All(r => r))
                {
                    LocalMultiplayerInput.MenuContext();
                    Call(() => c.A0 = 0, func_80054C4C);
                    Call(() => { }, func_80015540);
                    Call(() => { }, func_8001AAFC);
                    // Preview construction can pump loading frames. Snapshot every
                    // device first so one player's load cannot consume another's tap.
                    ushort[] heldImages = Enumerable.Range(0, count)
                        .Select(p => (ushort)~Controller.GetState(p)).ToArray();
                    int feedback = -1;
                    void Feedback(int sample) => feedback = Math.Max(feedback, sample);
                    Fill(0, 0, 640, 480, 0x161E1E);
                    for (int p = 0; p < count; p++)
                    {
                        ushort held = heldImages[p];
                        ushort edge = (ushort)(held & ~previous[p]); previous[p] = held;
                        if (!Controller.LocalConnected[p]) { ready[p] = false; edge = 0; }
                        if ((edge & Controller.Triangle) != 0)
                        {
                            if (ready[p]) { ready[p] = false; Feedback(11); }
                            else if (editing[p])
                            {
                                editing[p] = false;
                                Feedback(11);
                                colors[p] = Color(p, types[p]);
                                Call(() => { c.A0 = previews[p].Bank; c.A1 = colors[p]; }, func_8001EF34);
                            }
                            else if (p == 0) { cancelled = true; Feedback(11); }
                        }
                        if (!ready[p])
                        {
                            if ((edge & Controller.Circle) != 0) { editing[p] = !editing[p]; Feedback(0); }
                            int direction = (edge & Controller.Right) != 0 ? 1 : (edge & Controller.Left) != 0 ? -1 : 0;
                            if (!editing[p] && direction != 0)
                            {
                                int at = Array.IndexOf(roster, types[p]);
                                for (int tries = 0; tries < roster.Length; tries++)
                                {
                                    at = (at + direction + roster.Length) % roster.Length;
                                    if (!types.Where((_, i) => i != p).Contains(roster[at]) &&
                                        (p != 1 || roster[at] >= 18 || (secondMask & (1u << roster[at])) == 0)) break;
                                }
                                V82VehicleRegistry.ReleaseLocalMenuPreview(c, m, previews[p]);
                                types[p] = roster[at]; colors[p] = Color(p, types[p]);
                                previews[p] = V82VehicleRegistry.CreateLocalMenuPreview(c, m, types[p], colors[p]);
                                Feedback(0);
                            }
                            else if (editing[p])
                            {
                                uint color = colors[p];
                                int hue = (int)(color & 255), brightness = (sbyte)(color >> 16);
                                if (direction != 0) hue = (hue + direction * 8 + 193) % 193;
                                if ((edge & Controller.Up) != 0) brightness = Math.Min(127, brightness + 8);
                                if ((edge & Controller.Down) != 0) brightness = Math.Max(-128, brightness - 8);
                                // Retail has two authored saturation offsets; imported
                                // paint uses the neutral adjustment, without a retail proxy.
                                int saturation = hue == 0 || types[p] >= 18 ? 0 : (sbyte)m.ReadU8(0x80115BB4u + (uint)types[p]);
                                color = (uint)hue | (uint)(byte)saturation << 8 | (uint)(byte)brightness << 16;
                                if ((edge & Controller.Square) != 0) color = 0;
                                if (color != colors[p])
                                {
                                    colors[p] = color;
                                    Feedback(0);
                                    Call(() => { c.A0 = previews[p].Bank; c.A1 = color; }, func_8001EF34);
                                }
                            }
                            if ((edge & Controller.Cross) != 0)
                            {
                                Feedback(6);
                                acceptedColors[(p, types[p])] = colors[p];
                                V82VehicleRegistry.SaveLocalMenuColor(m, types[p], colors[p], p);
                                if (editing[p]) editing[p] = false;
                                else { ready[p] = true; Console.Error.WriteLine($"[LocalSelection] ready player={p+1} type={types[p]}"); }
                            }
                        }
                        int width = count == 2 ? 640 : 320;
                        int x = count == 2 ? 0 : p % 2 * 320, y = p / (count == 2 ? 1 : 2) * 220;
                        Fill(x + 4, y + 4, width - 8, 212, 0x293535);
                        Text($"PLAYER {p+1}", x + 8, y + 10, width - 16, true);
                        Text("< " + Name(types[p]) + " >", x + 8, y + 42, width - 16);
                        uint window = previewWindow;
                        // Native B4F0 reads these destination coordinates; the
                        // projection center and offscreen surface stay fixed.
                        m.WriteU32(window, (uint)(x + (width - 280) / 2));
                        m.WriteU32(window + 4, (uint)(y + 76));
                        Call(() => { c.A0 = window; c.A1 = 0; }, func_8001B458);
                        Call(() => { }, func_800149AC);
                        Call(() => { c.A0 = 0; c.A1 = 0x801006DC; c.A2 = 0xFFFFFF; }, func_8002DE84);
                        uint vehicle = previews[p].Vehicle;
                        Call(() => { c.A0 = vehicle; c.A1 = 280; }, func_8010536C);
                        Call(() => { }, func_8002DE18);
                        Call(() => { c.A0 = 16; c.A1 = vehicle + 0x20; }, func_8005A5EC);
                        Call(() => c.A0 = vehicle, func_80041AE8);
                        Call(() => { c.A0 = vehicle; c.A1 = m.ReadU32(0x80065480); }, func_8002FEE8);
                        m.WriteU32(vehicle + 0x34, 0x04000000); m.WriteU32(vehicle + 0x3C, 0x04000000);
                        Call(() => c.A0 = vehicle, func_8002D9E0);
                        Call(() => c.A0 = m.ReadU32(0x8006B4D0) + 0x3FFC, func_8005C57C);
                        Call(() => c.A0 = 0, func_8005BF88);
                        Call(() => c.A0 = window, func_8001B4F0);
                        Call(() => { }, func_8001AAFC);
                        Text(!Controller.LocalConnected[p] ? "RECONNECT CONTROLLER" : ready[p] ? "READY" : editing[p] ? "COLOR: LEFT / RIGHT" : $"{Button(p, Controller.Cross)} READY   {Button(p, Controller.Circle)} COLOR", x + 8, y + 181, width - 16);
                    }
                    if (feedback >= 0)
                    {
                        // Native SHELL uses bank 80116738, samples 0/6/11 for
                        // navigation/accept/back and channel 2 in color setup.
                        // Coalesce simultaneous edges so navigation cannot cut
                        // off an accept or back sound in the same frame.
                        Call(() => { c.A0 = 2; c.A1 = m.ReadU32(0x80116738); c.A2 = (uint)feedback; }, func_8001E14C);
                        Console.Error.WriteLine($"[LocalSelectionSound] sample={feedback}");
                    }
                    Text(editing.Any(e => e) ? "UP / DOWN BRIGHTNESS   " + string.Join(" / ", Enumerable.Range(0, count).Where(p => editing[p]).Select(p => Button(p, Controller.Square)).Distinct()) + " DEFAULT" : Button(0, Controller.Triangle) + " BACK", 8, 446, 624);
                    Call(() => c.A0 = 0, func_8005BF88);
                    if (++frame == 120) Console.Error.WriteLine("[LocalSelection] simultaneous frame=120");
                }
                if (!cancelled)
                    for (int p = 0; p < count; p++)
                    {
                        m.WriteU8(c.GP + 0x1104u + (uint)p, (byte)types[p]);
                        V82VehicleRegistry.SelectTypeForPlayer(p, types[p] >= 18 ? types[p] : -1);
                    }
            }
            finally
            {
                foreach (var preview in previews) V82VehicleRegistry.ReleaseLocalMenuPreview(c, m, preview);
                V82VehicleRegistry.LocalMenuScene(false, c, m);
                if (previewWindow != 0) Call(() => c.A0 = previewWindow, func_8001B40C);
                Call(() => c.A0 = 0x8006B648, func_800214F8);
                if (font != 0) Call(() => c.A0 = font, func_80019320);
                if (title != 0) Call(() => c.A0 = title, func_80019320);
                Call(() => { }, func_80103ADC);
                if (scratch != 0) Call(() => c.A0 = scratch, V82Compat.PcFree);
                c.Restore(saved);
                c.V0 = (uint)types[0];
            }
            // The retail caller examines the combined Triangle flag to return
            // to arena choice. Publish the result after draining held buttons.
            do
            {
                LocalMultiplayerInput.MenuContext();
                Call(() => c.A0 = 0, func_80054C4C);
                Call(() => { }, func_80015540);
            } while (Enumerable.Range(0, count).Any(p => (ushort)~Controller.GetState(p) != 0));
            m.WriteU8(c.GP + 0xD08, m.ReadU8(c.GP + 0xCF8));
            Call(() => { }, func_80015540);
            m.WriteU32(0x8006B4EC, cancelled ? 0x00100000u : 0u);
            Console.Error.WriteLine($"[LocalSelection] exit cancelled={cancelled}");
            if (!cancelled)
            {
                SplitMenuArm(c, m, count, firstMask);
                for (int p = 0; p < count; p++)
                {
                    if (types[p] >= 18) continue;
                    // Native func_800365E0 reads player*180 + type*10.
                    // Extra player rows live in the extended session allocation.
                    uint row = SplitUpgradeAddress(0x8006B9E8u + (uint)p * 180 + (uint)types[p] * 10);
                    m.WriteU8(row + 2, (byte)colors[p]);
                    m.WriteU8(row + 3, (byte)(colors[p] >> 16));
                    m.WriteU8(row + 4, (byte)(colors[p] >> 8));
                    Console.Error.WriteLine($"[LocalSelection] match color player={p+1} type={types[p]} value={colors[p]:X6}");
                }
            }
            return true;
        }
    }
}
