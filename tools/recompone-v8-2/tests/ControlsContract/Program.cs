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
