using RecompOne.Runtime.Cdrom;
using RecompOne.Runtime.Host;
using System.Text.Json;
var root=Path.GetFullPath(args[0]);
var output=Path.GetFullPath(args[1]);
Directory.CreateDirectory(output);
string cue=Path.Combine(root,"Vigilante 8 - 2nd Offense (USA).cue");
string original=Path.Combine(root,"Vigilante 8 - 2nd Offense (USA) (Track 01).bin");
var results=new Dictionary<string,bool>();
if(args.Length>2) {
 string install=Path.GetFullPath(args[2]);
 Environment.SetEnvironmentVariable("RECOMPONE_MOD_DIR",Path.Combine(install,"mods"));
 using(var fs=CueFs.OpenLoose(Path.Combine(install,"game_data"))) {
  string mod=Path.Combine(install,"mods/v8_to_v82_guest_roster/files");
  results["all_guest_dependencies_resolve"]=Directory.GetFiles(mod,"*",SearchOption.AllDirectories).All(p=>fs.Locate(Path.GetRelativePath(mod,p),out _,out uint size)&&size>0);
  results["selector_voice_mod_override"]=fs.ReadFile("SHELL/V8VOICES.SND").SequenceEqual(File.ReadAllBytes(Path.Combine(mod,"SHELL/V8VOICES.SND")));
  results["authored_light_palettes_loaded"]=new[]{"SHELL/VEHICLES.EXP","SHARED/COMMON.EXP"}.All(p=>fs.ReadFile(p).SequenceEqual(File.ReadAllBytes(Path.Combine(install,"mods/enhanced_textures_2x/files",p))));
 }
 Environment.SetEnvironmentVariable("RECOMPONE_MOD_DIR",null);
}
results["bin_resolves_matching_cue"]=FirstRunSetup.ResolveCue(original)==cue;
try { V82LooseImporter.Import(Path.Combine(root,"BINCUE/Vigilante 8 (USA).cue"),Path.Combine(output,"wrong-title")); results["wrong_title_rejected"]=false; }
catch(InvalidDataException ex) { results["wrong_title_rejected"]=ex.Message.Contains("SLUS-00868"); }
string changed=Path.Combine(output,"changed-track01.bin");
File.Copy(original,changed,true);
int lba; using(var fs=CueFs.Open(cue)) { if(!fs.Locate("SLUS_008.68",out lba,out _)) throw new Exception("No executable"); }
// Change a reserved PS-X EXE header byte, preserving the title and executable code.
using(var f=new FileStream(changed,FileMode.Open,FileAccess.ReadWrite)) { f.Position=(long)lba*2352+24+0x100; int b=f.ReadByte(); f.Position--; f.WriteByte((byte)(b^1)); }
string changedCue=Path.Combine(output,"changed.cue");
string text=File.ReadAllText(cue);
foreach(var line in File.ReadLines(cue)) {
 var match=System.Text.RegularExpressions.Regex.Match(line,"FILE \\\"(.*?)\\\"");
 if(!match.Success) continue;
 string name=match.Groups[1].Value;
 string path=name.EndsWith("(Track 01).bin")?changed:Path.Combine(root,name);
 text=text.Replace("\""+name+"\"","\""+path+"\"");
}
File.WriteAllText(changedCue,text);
using(var cancel=new CancellationTokenSource()) {
 bool reached=false;
 try { V82LooseImporter.Import(changedCue,Path.Combine(output,"changed-import"),p=>{if(p.Phase=="files"){reached=true;cancel.Cancel();}},cancel.Token); }
 catch(OperationCanceledException) {}
 results["same_title_changed_exe_accepted"]=reached;
 results["cancellation_does_not_commit"]=!V82LooseImporter.IsComplete(Path.Combine(output,"changed-import"));
}
File.WriteAllText(Path.Combine(output,"result.json"),JsonSerializer.Serialize(results,new JsonSerializerOptions{WriteIndented=true}));
Console.WriteLine(JsonSerializer.Serialize(results));
return results.Values.All(v=>v)?0:1;
