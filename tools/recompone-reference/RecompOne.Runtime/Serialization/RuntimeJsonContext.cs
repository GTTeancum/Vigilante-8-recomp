using System.Text.Json.Serialization;
using RecompOne.Runtime.Cdrom;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Modding;

namespace RecompOne.Runtime.Serialization;

[JsonSourceGenerationOptions(
    WriteIndented = true,
    DefaultIgnoreCondition = JsonIgnoreCondition.Never,
    PropertyNameCaseInsensitive = true,
    ReadCommentHandling = System.Text.Json.JsonCommentHandling.Skip,
    AllowTrailingCommas = true)]
[JsonSerializable(typeof(GameConfig))]
[JsonSerializable(typeof(V8LooseManifest))]
[JsonSerializable(typeof(ModInfo))]
internal partial class RuntimeJsonContext : JsonSerializerContext
{
}
