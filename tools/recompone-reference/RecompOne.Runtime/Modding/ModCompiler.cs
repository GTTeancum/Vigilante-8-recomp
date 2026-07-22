using System.Text;
using System.Reflection;
using System.Reflection.Metadata;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis.Emit;
using Microsoft.CodeAnalysis.Text;

namespace RecompOne.Runtime.Modding;

public static class ModCompiler
{
    static List<MetadataReference>? _references;
    static readonly List<AssemblyMetadata> _metadata = [];

    public static byte[]? Compile(string modId, IReadOnlyList<(string Path, string Text)> sources)
    {
        var parseOptions = CSharpParseOptions.Default.WithLanguageVersion(LanguageVersion.Latest);
        var trees = sources.Select(s => CSharpSyntaxTree.ParseText(SourceText.From(s.Text, Encoding.UTF8), parseOptions, s.Path)).ToList();
        var options = new CSharpCompilationOptions(OutputKind.DynamicallyLinkedLibrary).WithAllowUnsafe(true).WithOptimizationLevel(OptimizationLevel.Release);

        var compilation = CSharpCompilation.Create($"mod-{modId}", trees, References(), options);
        using var ms = new MemoryStream();
        var result = compilation.Emit(ms, options: new EmitOptions(debugInformationFormat: DebugInformationFormat.Embedded));
        if (!result.Success)
        {
            foreach (var diag in result.Diagnostics.Where(d => d.Severity == DiagnosticSeverity.Error))
                Console.Error.WriteLine($"[Mods] {modId}: {diag}");
            return null;
        }

        return ms.ToArray();
    }

    static unsafe List<MetadataReference> References()
    {
        if (_references != null) return _references;

        // Assembly.Location is empty for assemblies bundled into a single-file
        // executable. Read their loaded metadata instead so source mods compile
        // identically in framework-dependent and published single-file builds.
        _references = [];
        foreach (Assembly assembly in AppDomain.CurrentDomain.GetAssemblies())
        {
            if (assembly.IsDynamic ||
                !assembly.TryGetRawMetadata(out byte* metadata, out int length))
                continue;

            ModuleMetadata module = ModuleMetadata.CreateFromMetadata(
                (IntPtr)metadata, length);
            AssemblyMetadata image = AssemblyMetadata.Create(module);
            _metadata.Add(image);
            _references.Add(image.GetReference(
                filePath: $"{assembly.GetName().Name}.dll"));
        }

        if (_references.Count == 0)
            throw new InvalidOperationException(
                "No runtime metadata was available for C# mod compilation.");
        return _references;
    }
}
