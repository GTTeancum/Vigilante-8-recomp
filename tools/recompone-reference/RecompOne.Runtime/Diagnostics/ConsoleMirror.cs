using System.Text;
using System.Security.Cryptography;

namespace RecompOne.Runtime.Diagnostics;

public static class ConsoleMirror
{
    const int MaxLines = 4000;

    static readonly object _gate = new();
    static readonly List<string> _lines = new();
    static readonly StringBuilder _pending = new();
    static StreamWriter? _file;
    static string? _filePath;
    static int _version;
    static bool _installed;

    public static int Version { get { lock (_gate) return _version; } }

    public static void Install()
    {
        if (_installed) return;
        _installed = true;
        string executable = Environment.ProcessPath ?? "";
        string executableDirectory =
            Path.GetDirectoryName(executable) ?? AppContext.BaseDirectory;
        try
        {
            string configured =
                Environment.GetEnvironmentVariable("RECOMPONE_LOG_PATH") ?? "";
            _filePath = Path.GetFullPath(
                string.IsNullOrWhiteSpace(configured)
                    ? Path.Combine(executableDirectory, "v8_latest.log")
                    : configured);
            _file = new StreamWriter(
                new FileStream(
                    _filePath, FileMode.Create, FileAccess.Write,
                    FileShare.ReadWrite),
                new UTF8Encoding(encoderShouldEmitUTF8Identifier: false))
            {
                AutoFlush = true,
            };
        }
        catch (Exception exception)
        {
            _file = null;
            _filePath = null;
            Console.Error.WriteLine(
                $"[Diagnostics] could not create persistent log: " +
                $"{exception.Message}");
        }
        Console.SetOut(new Tee(Console.Out));
        Console.SetError(new Tee(Console.Error));
        AppDomain.CurrentDomain.UnhandledException += (_, eventArgs) =>
        {
            Console.Error.WriteLine(
                $"[Fatal] unhandled terminating=" +
                $"{eventArgs.IsTerminating} object={eventArgs.ExceptionObject}");
            FlushFile();
        };
        TaskScheduler.UnobservedTaskException += (_, eventArgs) =>
        {
            Console.Error.WriteLine(
                $"[Fatal] unobserved task exception: {eventArgs.Exception}");
            FlushFile();
        };
        AppDomain.CurrentDomain.ProcessExit += (_, _) => FlushFile();
        if (string.IsNullOrEmpty(executable))
            executable = "(unknown)";
        string executableHash = "(unavailable)";
        try
        {
            if (File.Exists(executable))
                executableHash = Convert.ToHexString(
                    SHA256.HashData(File.ReadAllBytes(executable)));
        }
        catch (Exception exception) when (
            exception is IOException or UnauthorizedAccessException)
        {
            executableHash = $"(error: {exception.Message})";
        }
        Console.WriteLine(
            $"[Diagnostics] session={DateTimeOffset.Now:O} " +
            $"pid={Environment.ProcessId} base={AppContext.BaseDirectory} " +
            $"exe={executable} sha256={executableHash} " +
            $"log={_filePath ?? "(unavailable)"}");
    }

    public static void Clear()
    {
        lock (_gate)
        {
            _lines.Clear();
            _pending.Clear();
            _version++;
        }
    }

    public static int SnapshotInto(List<string> dst)
    {
        lock (_gate)
        {
            dst.Clear();
            dst.AddRange(_lines);
            if (_pending.Length > 0) dst.Add(_pending.ToString());
            return _version;
        }
    }

    static void Append(string? text)
    {
        if (string.IsNullOrEmpty(text)) return;
        lock (_gate)
        {
            foreach (char c in text)
            {
                if (c == '\n') FlushPendingLocked();
                else if (c != '\r') _pending.Append(c);
            }
            _version++;
        }
    }

    static void AppendChar(char c)
    {
        lock (_gate)
        {
            if (c == '\n') FlushPendingLocked();
            else if (c != '\r') _pending.Append(c);
            _version++;
        }
    }

    static void FlushPendingLocked()
    {
        _lines.Add(_pending.ToString());
        _pending.Clear();
        if (_lines.Count > MaxLines) _lines.RemoveRange(0, _lines.Count - MaxLines);
    }

    static void WriteFile(string? text)
    {
        if (string.IsNullOrEmpty(text)) return;
        lock (_gate)
        {
            try
            {
                _file?.Write(text);
            }
            catch (Exception exception) when (
                exception is IOException or ObjectDisposedException)
            {
                _file = null;
            }
        }
    }

    static void FlushFile()
    {
        lock (_gate)
        {
            try
            {
                _file?.Flush();
            }
            catch (Exception exception) when (
                exception is IOException or ObjectDisposedException)
            {
                _file = null;
            }
        }
    }

    sealed class Tee : TextWriter
    {
        readonly TextWriter _inner;
        public Tee(TextWriter inner) => _inner = inner;

        public override Encoding Encoding => _inner.Encoding;

        public override void Write(char value)
        {
            _inner.Write(value);
            WriteFile(value.ToString());
            AppendChar(value);
        }

        public override void Write(string? value)
        {
            _inner.Write(value);
            WriteFile(value);
            Append(value);
        }

        public override void WriteLine(string? value)
        {
            _inner.WriteLine(value);
            WriteFile(value);
            WriteFile(Environment.NewLine);
            Append(value);
            AppendChar('\n');
        }

        public override void Flush()
        {
            _inner.Flush();
            FlushFile();
        }
    }
}
