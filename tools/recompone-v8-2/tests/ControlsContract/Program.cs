using System.Reflection;
using System.Text;
using System.Text.Json;
using RecompOne.Runtime;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Hardware;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;

void Check(bool ok, string message)
{
    if (!ok) throw new Exception(message);
    Console.WriteLine("PASS " + message);
}
var game = new GameConfig();
// Retail export order is not uniform: Olympic follows two object exports.
// Validate real staged DLL tables when this contract is run from the repo.
string arenaRoot = Path.Combine(Environment.CurrentDirectory, "V8_2_LOOSE", "LEVELS");
if (Directory.Exists(arenaRoot))
    foreach (string dll in Directory.EnumerateFiles(arenaRoot, "*.DLL", SearchOption.AllDirectories))
    {
        string stem = Path.GetFileNameWithoutExtension(dll);
        string export = V82ArenaRegistry.ArenaExportName(File.ReadAllBytes(dll), stem.ToLowerInvariant());
        Check(string.Equals(export, stem, StringComparison.OrdinalIgnoreCase),
            $"native arena export lookup finds {stem} independently of export order");
    }
Check(InputPromptLabels.Format("PRESS START", game, 0, false, false, false) == "PRESS ENTER",
    "title keyboard prompt uses glyphs supported by the uppercase title font");
Check(InputPromptLabels.Format("PRESS START", game, 0, true, false, false) == "PRESS START",
    "title controller prompt retains the familiar START label");
InputProfiles.Apply(game, InputProfiles.TriggerDrive);
game.Keys.Cross = "Space";
var p1 = game.Pad;
InputProfiles.ApplyPlayer(game, InputProfiles.Southpaw, 1);
Check(ReferenceEquals(p1, game.Pad) && game.Keys.Cross == "Space", "P2 preset preserves P1 and keyboard bindings");
Check(InputProfiles.ForPlayer(game, 0) == InputProfiles.TriggerDrive &&
    InputProfiles.ForPlayer(game, 1) == InputProfiles.Southpaw, "independent preset labels");
string prompt = "Press \x01@@\x80\x82\x01\x80\x80\x80 to start...";
Check(InputPromptLabels.Format(prompt, game, 0, true, false, true) ==
    "Press \x01@@\x80RT\x01\x80\x80\x80 to play", "ready prompt uses RT even before gameplay and preserves RGB commands");
Check(InputPromptLabels.Format(prompt, game, 0, false, true, false).Contains("Space"), "keyboard prompt uses assigned key");
Check(InputPromptLabels.Format("\x82" + "accept", game, 0, false, false, false) == "Z accept",
    "keyboard menus retain navigation independently of driving keys");
Check(InputBindingResolver.ResolveKeys(KeyBindings.Empty(), 0, false, false).Cross == "Z",
    "clearing gameplay keyboard bindings cannot lock out settings");
Check(InputPromptLabels.Format("\x82" + "accept", game, 0, true, false, false) == "A accept", "menu footer uses menu binding");
game.Pad.Cross = [9];
InputProfiles.SetPlayerProfile(game, 0, InputProfiles.Custom);
Check(InputPromptLabels.Format(prompt, game, 0, true, true, false).Contains("LB"), "custom prompt reads binding rather than preset name");
Check(InputBindingResolver.ResolvePad(InputProfiles.Custom, game.Pad, true, true).Cross.SequenceEqual([0]),
    "custom driving layout retains ordinary pause controls");
Check(InputPromptLabels.Button(game, 1, Controller.Cross, true, true, false) == "A", "P2 prompt uses P2 binding");
var restored = JsonSerializer.Deserialize<GameConfig>(JsonSerializer.Serialize(game))!;
Check(restored.Pad.Cross.SequenceEqual([9]) && restored.Pad2.Down.Contains(109) &&
    restored.InputProfile == InputProfiles.Custom && restored.InputProfile2 == InputProfiles.Southpaw,
    "per-player custom bindings and presets serialize independently");

// Drive the real physical-input resolver in-process, without an SDL window,
// OS input, or virtual PS1 button shortcut. RT must leave the one-shot ready
// screen even after its text has not been redrawn for many input polls.
Environment.SetEnvironmentVariable("RECOMPONE_INPUT_SCRIPT", "100+3=PHYS:RT;110+3=PHYS:LT;120+3=PHYS:A");
Environment.SetEnvironmentVariable("RECOMPONE_DISABLE_SCRIPT_STAGE_CAPTURES", "1");
var input = typeof(Runtime).Assembly.GetType("RecompOne.Runtime.Host.InputManager")!;
void Call(string name) => input.GetMethod(name, BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic)!.Invoke(null, null);
Call("InitializeHeadless");
InputProfiles.Apply(ConfigManager.Game, InputProfiles.TriggerDrive);
var memory = new PSMemory();
var cpu = new CpuContext { SP = 0x801F0000, A1 = 0x80020000 };
byte[] bytes = Encoding.Latin1.GetBytes(prompt + "\0");
for (uint i = 0; i < bytes.Length; i++) memory.WriteU8(cpu.A1 + i, bytes[i]);
for (uint i = 0; i < 64; i++) memory.WriteU8(cpu.SP + i, (byte)i);
using (V82ControlPrompts.Rewrite(cpu, memory))
{
    Check(cpu.SP == 0x801EFE00 && memory.ReadU8(cpu.SP + 16) == 16, "prompt replacement preserves native stack arguments");
}
Check(cpu.SP == 0x801F0000 && cpu.A1 == 0x80020000 && memory.ReadU8(cpu.SP + 16) == 16,
    "prompt scope restores SP, pointer, and caller memory");
for (int poll = 1; poll <= 123; poll++)
{
    Call("Poll");
    if (poll == 101)
        Check((Controller.State & Controller.Cross) == 0, "physical RT accepts after 100 ready-screen polls");
    if (poll == 111)
        Check((Controller.State & Controller.Down) == 0, "physical LT resolves to brake/reverse");
    if (poll == 115) V82ControlPrompts.EndReadyPrompt();
    if (poll == 121)
        Check((Controller.State & Controller.Cross) == 0 && (Controller.State & Controller.L2) != 0,
            "physical A returns to menu acceptance after ready context ends");
}
Console.WriteLine("Controls contract complete.");

cpu.A0 = 2;
bytes = Encoding.Latin1.GetBytes("\x82" + "replace\0\x83" + "back\0");
for (uint i = 0; i < bytes.Length; i++) memory.WriteU8(cpu.A1 + i, bytes[i]);
using (V82ControlPrompts.RewriteFooter(cpu, memory))
{
    byte[] expected = Encoding.Latin1.GetBytes("A replace\0Y back\0");
    Check(expected.Select((value, i) => value == memory.ReadU8(cpu.A1 + (uint)i)).All(ok => ok),
        "footer replaces every packed string before native width measurement");
}
Check(cpu.SP == 0x801F0000 && cpu.A1 == 0x80020000, "footer scope restores caller");

// Exercise the same analog latch and disconnect path used by SDL, entirely
// in this process with no controller handle or OS input.
var latch = input.GetMethod("AnalogLatch", BindingFlags.Static | BindingFlags.NonPublic)!;
bool Analog(int pad, int binding, int value, int threshold, bool update = true) =>
    (bool)latch.Invoke(null, new object[] { pad, binding, value, threshold, update })!;
foreach (var (binding, threshold) in new[] { (101, 8000), (103, 16000) })
{
    Check(!Analog(0, binding, threshold, threshold), "analog boundary does not press");
    Check(Analog(0, binding, threshold + 1, threshold), "analog press crosses full threshold");
    Check(Analog(0, binding, threshold * 7 / 8, threshold), "analog hysteresis holds steady near threshold");
    Check(!Analog(1, binding, threshold * 7 / 8, threshold), "analog latch is independent for P2");
    Check(!Analog(0, binding, threshold * 3 / 4, threshold), "analog releases at lower boundary");
    Check(!Analog(0, binding, threshold * 7 / 8, threshold), "released analog does not chatter back on");
}
Check(Analog(0, 101, 9000, 8000) && Analog(1, 101, 9000, 8000), "both trigger latches primed before disconnect");
Controller.LeftX = 0; Controller.LeftY = 255;
Controller.RightX = 0; Controller.RightY = 255;
Controller.LeftX2 = 255; Controller.LeftY2 = 0;
Controller.RightX2 = 255; Controller.RightY2 = 0;
Call("CloseControllers");
Check(!Analog(0, 101, 7000, 8000) && !Analog(1, 101, 7000, 8000),
    "controller reconnect cannot inherit the previous device's trigger latch");
Check(new[] { Controller.LeftX, Controller.LeftY, Controller.RightX, Controller.RightY,
    Controller.LeftX2, Controller.LeftY2, Controller.RightX2, Controller.RightY2 }.All(v => v == 128),
    "disconnect returns all native stick axes to neutral");

// Independent P3/P4 physical pulses traverse the real binding resolver.
Environment.SetEnvironmentVariable("RECOMPONE_INPUT_SCRIPT", "1+2=P3:PHYS:RT,P4:PHYS:A;4+2=P3:PHYS:LSLEFT,P4:PHYS:LSRIGHT");
Call("InitializeHeadless");
InputProfiles.ApplyPlayer(ConfigManager.Game, InputProfiles.TriggerDrive, 2);
InputProfiles.ApplyPlayer(ConfigManager.Game, InputProfiles.Classic, 3);
RecompOne.Runtime.Hle.GpuHle.GameplayActive = true;
Call("Poll"); Call("Poll");
Check((Controller.GetState(2) & Controller.Cross) == 0 &&
    (Controller.GetState(3) & Controller.Cross) == 0 && Controller.State == 0xFFFF && Controller.State2 == 0xFFFF,
    "P3/P4 physical inputs honor independent profiles without touching P1/P2");
Check(Controller.LocalConnected[2] && Controller.LocalConnected[3], "scripted extension ports report connected");
Call("Poll"); Call("Poll"); Call("Poll");
Check((byte)(Controller.GetAxes(2) >> 16) == 0 && (byte)(Controller.GetAxes(3) >> 16) == 255,
    "P3/P4 analog axes remain independent");
Call("CloseControllers");
Check(Enumerable.Range(0, 4).All(p => Controller.GetState(p) == 0xFFFF && Controller.GetAxes(p) == 0x80808080),
    "disconnect clears all four pad images");
RecompOne.Runtime.Hle.GpuHle.GameplayActive = false;

ushort[] held = [Controller.Cross, 0, 0, 0];
bool[] connected = [true, true, true, true];
var join = new LocalJoinState(held);
join.Update(held, connected);
Check(join.Devices.Count == 0, "carried entry A does not join automatically");
Array.Clear(held); join.Update(held, connected);
held[2] = Controller.Cross; join.Update(held, connected);
held[2] = Controller.Start; join.Update(held, connected);
Check(join.Devices.SequenceEqual([2]) && !join.Confirmed, "first join owns P1; one player cannot confirm multiplayer");
held[0] = Controller.Cross; join.Update(held, connected);
held[0] = Controller.Start; held[2] = 0; join.Update(held, connected);
Check(!join.Confirmed, "another player's Start cannot confirm");
held[1] = held[3] = Controller.Cross; join.Update(held, connected);
join.Update(held, connected);
Check(join.Devices.SequenceEqual([2, 0, 1, 3]), "four devices join once in join order");
connected[3] = false; held[2] = Controller.Start; join.Update(held, connected);
Check(!join.Confirmed, "disconnected joined device prevents confirmation");
Array.Clear(held); join.Update(held, connected); connected[3] = true;
held[2] = Controller.Start; join.Update(held, connected);
Check(join.Confirmed, "P1 Start confirms all connected players");
var keyboardJoin = new LocalJoinState(new ushort[4]);
keyboardJoin.Update([Controller.Cross, Controller.Cross, 0, 0], connected);
keyboardJoin.Update(new ushort[4], connected, keyboardConfirm: true);
Check(keyboardJoin.Confirmed, "Enter can confirm the joined roster");

// Join order is applied at the end of the real poll, after physical profiles.
Environment.SetEnvironmentVariable("RECOMPONE_INPUT_SCRIPT", "1+2=P3:PHYS:A,P1:PHYS:B,P4:PHYS:LSLEFT");
Call("InitializeHeadless");
LocalInputSession.Begin([2, 0, 3]);
Call("Poll"); Call("Poll");
Check((Controller.State & Controller.Cross) == 0 && (Controller.State & Controller.Circle) != 0 &&
    (Controller.State2 & Controller.Circle) == 0 && (Controller.State2 & Controller.Cross) != 0,
    "first joined physical pad owns P1 and each other joined pad owns its player");
Check((byte)(Controller.GetAxes(2) >> 16) == 0 && Controller.GetState(3) == 0xFFFF &&
    Controller.GetAxes(3) == 0x80808080 && !Controller.LocalConnected[3],
    "join routing preserves independent sticks and leaves unjoined slots neutral");
Check(LocalInputSession.DeviceForPlayer(0) == 2 && LocalInputSession.DeviceForPlayer(1) == 0 &&
    LocalInputSession.DeviceForPlayer(2) == 3 && LocalInputSession.DeviceForPlayer(3) == -1,
    "feedback routing uses the same device ownership as player input");
for (int p = 0; p < 4; p++)
{
    Controller.SetState(p, unchecked((ushort)~Controller.Cross));
    Controller.LocalConnected[p] = p != 2;
}
LocalInputSession.RoutePolledDevices();
Check(!Controller.LocalConnected[0] && Controller.State == 0xFFFF &&
    Controller.LocalConnected[1] && (Controller.State2 & Controller.Cross) == 0,
    "disconnection neutralizes its owner without transferring another player's device");
LocalInputSession.Reset();
Check(LocalInputSession.PlayerCount == 0 && LocalInputSession.DeviceForPlayer(0) == 0,
    "returning to the main menu restores ordinary input ownership");

LocalInputSession.Begin([2, 0, 3]);
using (LocalInputSession.FocusSelector(2, [1, 19]))
{
    Check(!LocalInputSession.IsTypeAvailable(1) && !LocalInputSession.IsTypeAvailable(19) &&
        LocalInputSession.IsTypeAvailable(2), "claimed retail and imported identities share the same selection rule");
    Check(LocalInputSession.SelectorPlayer == 2 && LocalInputSession.DeviceForPlayer(0) == 3 &&
        LocalInputSession.DeviceForPlayer(1) == -1,
        "character page focuses its owner onto the native menu input port");
    for (int p = 0; p < 4; p++)
    {
        Controller.SetState(p, unchecked((ushort)~Controller.Cross));
        Controller.LocalConnected[p] = true;
    }
    LocalInputSession.RoutePolledDevices();
    Check((Controller.State & Controller.Cross) == 0 &&
        Enumerable.Range(1, 3).All(p => Controller.GetState(p) == 0xFFFF),
        "non-owning players cannot operate another player's character page");
}
Check(LocalInputSession.SelectorPlayer == -1 && LocalInputSession.DeviceForPlayer(0) == 2 &&
    LocalInputSession.DeviceForPlayer(2) == 3 && LocalInputSession.IsTypeAvailable(19),
    "selector exit restores the joined gameplay ownership and releases menu-only exclusions");
LocalInputSession.Reset();

// Keyboard is a fifth physical identity, not a second input for gamepad one.
foreach (int[] roster in new[] { new[] { 4, 0 }, new[] { 0, 4 }, new[] { 4, 0, 1 }, new[] { 0, 1, 2, 4 }, new[] { 4, 2, 0, 1 }, new[] { 0, 1, 2, 3 } })
{
    LocalInputSession.Reset();
    Environment.SetEnvironmentVariable("RECOMPONE_INPUT_SCRIPT", "1+3=KEY:Z,P1:PHYS:B,P2:PHYS:X,P3:PHYS:Y,P4:PHYS:LSRIGHT");
    Call("InitializeHeadless");
    LocalInputSession.Begin(roster);
    Call("Poll"); Call("Poll");
    ushort[] expected = [Controller.Circle, Controller.Square, Controller.Triangle, 0, Controller.Cross];
    for (int player = 0; player < roster.Length; player++)
    {
        int device = roster[player];
        Check(((ushort)~Controller.GetState(player) & (Controller.Cross | Controller.Circle | Controller.Square | Controller.Triangle)) == expected[device],
            $"roster {string.Join(',', roster)} player {player + 1} receives only device {device}");
        Check(RecompOne.Runtime.Host.LocalMultiplayerInput.UsesGamepad(player) == (device != 4),
            "prompts follow physical ownership");
        if (device == 4) Check(Controller.GetAxes(player) == 0x80808080 && Controller.LocalConnected[player], "keyboard has neutral analog axes and its own connection");
    }
    Check(RecompOne.Runtime.Host.LocalMultiplayerInput.DeviceState(0) == unchecked((ushort)~Controller.Circle) &&
        RecompOne.Runtime.Host.LocalMultiplayerInput.DeviceState(4) == unchecked((ushort)~Controller.Cross),
        "join screen sees separate raw keyboard and first gamepad images");
}
LocalInputSession.Reset();
var mixedJoin = new LocalJoinState(new ushort[5]);
mixedJoin.Update([0, 0, 0, 0, Controller.Cross], [true, true, true, true, true]);
mixedJoin.Update([Controller.Cross, Controller.Cross, Controller.Cross, 0, 0], [true, true, true, true, true]);
mixedJoin.Update([0, 0, 0, Controller.Cross, 0], [true, true, true, true, true]);
Check(mixedJoin.Devices.SequenceEqual([4, 0, 1, 2]), "keyboard plus three gamepads join; fifth device cannot add a fifth player");
mixedJoin.Update([0, 0, 0, 0, Controller.Start], [true, true, true, true, true]);
Check(mixedJoin.Confirmed, "keyboard host Enter/Start confirms mixed roster");
Environment.SetEnvironmentVariable("RECOMPONE_INPUT_SCRIPT", "1+2=KEY:Z,P1:PHYS:DISCONNECTED");
Call("InitializeHeadless"); LocalInputSession.Begin([0, 4]); Call("Poll"); Call("Poll");
Check(!Controller.LocalConnected[0] && Controller.LocalConnected[1] && Controller.State == 0xFFFF &&
    (Controller.State2 & Controller.Cross) == 0, "gamepad disconnection leaves keyboard owner connected and independent");
LocalInputSession.Reset();
Environment.SetEnvironmentVariable("RECOMPONE_INPUT_SCRIPT", "1+2=KEY:Z");
Call("InitializeHeadless"); Call("Poll"); Call("Poll");
Check((Controller.State & Controller.Cross) == 0, "return to solo retains ordinary keyboard controls");
LocalInputSession.Begin([0, 4]);
Check(InputPromptLabels.Button(ConfigManager.Game, 1, Controller.Cross, false, false, true) == "Z", "keyboard at P2 resolves keyboard menu prompts");
LocalInputSession.Reset();

Environment.SetEnvironmentVariable("RECOMPONE_INPUT_SCRIPT", "1+20=KEY:Space,KEY:Enter,P3:PHYS:B");
Call("InitializeHeadless");
string savedCross = ConfigManager.Game.Keys.Cross;
ConfigManager.Game.Keys.Cross = "Space";
RecompOne.Runtime.Hle.GpuHle.GameplayActive = true;
LocalInputSession.Begin([2, 4]);
for (int i = 0; i < 10; i++) Call("Poll");
Check((Controller.State2 & Controller.Cross) == 0 && (Controller.State & Controller.Cross) != 0,
    "keyboard driving remap follows its joined player without affecting the gamepad");
Check(RecompOne.Runtime.Host.LocalMultiplayerInput.EnterHeld &&
    InputPromptLabels.Button(ConfigManager.Game, 1, Controller.Cross, false, true, false) == "Space",
    "keyboard Enter and remapped driving prompt use the actual key path");
RecompOne.Runtime.Hle.GpuHle.GameplayActive = false;
using (LocalInputSession.FocusSelector(1))
{
    Call("Poll");
    Check((Controller.State & Controller.Start) == 0 && Controller.State2 == 0xFFFF && !RecompOne.Runtime.Host.LocalMultiplayerInput.UsesGamepad(0),
        "keyboard selector focus routes only its owner and retains keyboard prompts");
}
ConfigManager.Game.Keys.Cross = savedCross;
LocalInputSession.Reset();
