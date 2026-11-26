#if V8_WINDOWS_SETUP
using System.Drawing;
using System.Windows.Forms;
using RecompOne.Runtime.Host;
using RecompOne.Runtime.Cdrom;

internal sealed class WindowsDiscSetup : Form
{
    static readonly Color Ink = Color.FromArgb(237, 242, 249);
    static readonly Color Muted = Color.FromArgb(169, 184, 203);
    static readonly Color Gold = Color.FromArgb(255, 202, 94);
    static readonly Color Green = Color.FromArgb(108, 224, 176);
    static readonly Color PanelColor = Color.FromArgb(29, 40, 55);
    readonly string root;
    readonly string? initialSource;
    readonly Label sequelStatus = new();
    readonly Label originalStatus = new();
    readonly Label message;
    readonly ProgressBar progress = new();
    readonly Button launch;
    readonly Button browse;
    readonly Button exit;
    readonly Font normal = new("Segoe UI", 10f);
    readonly Font title = new("Segoe UI Semibold", 27f);
    readonly Font card = new("Segoe UI Semibold", 14f);
    readonly Font small = new("Segoe UI", 9f);
    readonly Bitmap? emblem;
    CancellationTokenSource? cancel;
    bool closeAfterImport;

    WindowsDiscSetup(string root, string? initialSource)
    {
        this.root = root;
        this.initialSource = initialSource;
        Text = "Vigilante 8 Classic Setup";
        Icon = System.Drawing.Icon.ExtractAssociatedIcon(Application.ExecutablePath);
        BackColor = Color.FromArgb(15, 23, 35);
        ForeColor = Ink;
        Font = normal;
        AutoScaleMode = AutoScaleMode.Dpi;
        StartPosition = FormStartPosition.CenterScreen;
        ClientSize = new Size(720, 570);
        FormBorderStyle = FormBorderStyle.FixedDialog;
        MaximizeBox = false;
        var banner = new Panel { Bounds = new Rectangle(0, 0, 720, 153), BackColor = Color.FromArgb(23, 34, 49) };
        Controls.Add(banner);
        banner.Controls.Add(new Panel { Bounds = new Rectangle(0, 0, 720, 5), BackColor = Gold });
        using (var stream = typeof(WindowsDiscSetup).Assembly.GetManifestResourceStream("Vigilante8.SetupEmblem.png"))
        {
            if (stream != null)
            {
                using var source = Image.FromStream(stream);
                emblem = new Bitmap(source);
                banner.Controls.Add(new PictureBox { Bounds = new Rectangle(554, 11, 135, 135),
                    Image = emblem, SizeMode = PictureBoxSizeMode.Zoom, TabStop = false });
            }
        }
        banner.Controls.Add(Label("PC EDITION   /   DISC SETUP", 30, 21, 508, 26, small, Gold));
        banner.Controls.Add(Label("Vigilante 8 Classic", 29, 49, 508, 54, title, Ink));
        banner.Controls.Add(Label("Add both USA PlayStation discs to unlock Launch game.", 32, 111, 508, 25, normal, Muted));

        AddDisc(173, "01", "Vigilante 8: 2nd Offense", "Base game and Second Offense soundtrack", "USA  /  SLUS-00868", sequelStatus);
        AddDisc(268, "02", "Vigilante 8", "Original soundtrack and guest-car engine audio", "USA REV 1  /  SLUS-00510", originalStatus);

        message = Label("Choose either disc first. Keep each CUE with all its BIN files.", 32, 368, 656, 57, normal, Muted);
        Controls.Add(message);
        progress.Bounds = new Rectangle(32, 431, 656, 13);
        progress.Maximum = 1000;
        progress.Visible = false;
        Controls.Add(progress);
        Controls.Add(new Panel { Bounds = new Rectangle(32, 472, 656, 1), BackColor = Color.FromArgb(54, 67, 85) });

        exit = MakeButton("Exit setup", 32, 493, 108, false);
        browse = MakeButton("Browse for disc…", 383, 493, 153, false);
        launch = MakeButton("Launch game", 548, 493, 140, true);
        Controls.AddRange(new Control[] { exit, browse, launch });
        exit.Click += (_, _) =>
        {
            if (cancel == null) Close();
            else
            {
                cancel.Cancel();
                exit.Enabled = false;
                message.Text = "Cancelling safely…";
            }
        };
        browse.Click += async (_, _) =>
        {
            using var files = new OpenFileDialog
            {
                Title = "Select a required Vigilante 8 disc",
                Filter = "PlayStation disc image (*.cue;*.bin)|*.cue;*.bin",
                CheckFileExists = true,
                DereferenceLinks = true
            };
            if (files.ShowDialog(this) == DialogResult.OK) await Import(files.FileName);
        };
        launch.Click += (_, _) =>
        {
            if (FirstRunSetup.Inspect(root).Ready) { DialogResult = DialogResult.OK; Close(); }
            else RefreshState("Setup data changed. Re-import the missing disc.", true);
        };
        Shown += async (_, _) =>
        {
            if (!string.IsNullOrEmpty(initialSource)) await Import(initialSource);
        };
        FormClosing += (_, e) =>
        {
            if (cancel == null) return;
            e.Cancel = true;
            closeAfterImport = true;
            cancel.Cancel();
            message.Text = "Cancelling safely…";
        };
        RefreshState();
    }

    void AddDisc(int y, string number, string name, string description, string id, Label status)
    {
        var panel = new Panel { Bounds = new Rectangle(32, y, 656, 84), BackColor = PanelColor };
        Controls.Add(panel);
        panel.Controls.Add(new Panel { Bounds = new Rectangle(0, 0, 3, 84), BackColor = Color.FromArgb(110, 130, 155) });
        panel.Controls.Add(Label(number, 16, 16, 37, 25, card, Gold));
        panel.Controls.Add(Label(name, 64, 9, 435, 30, card, Ink));
        panel.Controls.Add(Label(description, 66, 40, 430, 23, normal, Muted));
        panel.Controls.Add(Label(id, 473, 49, 167, 22, small, Muted, ContentAlignment.MiddleRight));
        status.SetBounds(503, 14, 136, 25);
        status.TextAlign = ContentAlignment.MiddleRight;
        status.Font = small;
        panel.Controls.Add(status);
    }

    static Label Label(string text, int x, int y, int width, int height, Font font, Color color,
        ContentAlignment align = ContentAlignment.MiddleLeft) => new()
        { Bounds = new Rectangle(x, y, width, height), Text = text, Font = font, ForeColor = color, TextAlign = align };

    Button MakeButton(string text, int x, int y, int width, bool primary)
    {
        var button = new Button { Text = text, Bounds = new Rectangle(x, y, width, 42), FlatStyle = FlatStyle.Flat,
            BackColor = primary ? Gold : PanelColor, ForeColor = primary ? Color.FromArgb(32, 25, 12) : Ink,
            UseVisualStyleBackColor = false };
        button.FlatAppearance.BorderSize = primary ? 0 : 1;
        button.FlatAppearance.BorderColor = Color.FromArgb(72, 91, 114);
        return button;
    }

    void RefreshState(string? detail = null, bool failed = false)
    {
        var status = FirstRunSetup.Inspect(root);
        sequelStatus.Text = status.SecondOffense ? "VERIFIED" : "REQUIRED";
        sequelStatus.ForeColor = status.SecondOffense ? Green : Gold;
        originalStatus.Text = status.Original ? "VERIFIED" : "REQUIRED";
        originalStatus.ForeColor = status.Original ? Green : Gold;
        launch.Enabled = status.Ready;
        browse.Enabled = !status.Ready;
        AcceptButton = status.Ready ? launch : browse;
        message.ForeColor = failed ? Color.FromArgb(255, 161, 139) : Muted;
        message.Text = detail ?? (status.Ready ? "Both discs are verified. You’re ready to play."
            : status.SecondOffense ? "Second Offense is ready. Choose the original Vigilante 8 USA Rev 1 CUE."
            : status.Original ? "Original audio is ready. Choose the Second Offense USA CUE."
            : "Choose either disc first. Keep each CUE with all its BIN files.");
    }

    async Task Import(string source)
    {
        if (cancel != null) return;
        using var sourceCancel = new CancellationTokenSource();
        cancel = sourceCancel;
        launch.Enabled = browse.Enabled = false;
        AcceptButton = null;
        exit.Text = "Cancel import";
        progress.Visible = true;
        progress.Value = 0;
        message.ForeColor = Muted;
        message.Text = "Checking the selected disc…";
        string? error = null;
        var reports = new Progress<LooseImportProgress>(p =>
        {
            if (cancel != sourceCancel || sourceCancel.IsCancellationRequested) return;
            progress.Value = (int)Math.Clamp(1000L * p.Current / Math.Max(1, p.Total), 0L, 1000L);
            string phase = p.Phase switch {
                "music" => "Preparing Second Offense music",
                "soundtrack" => "Preparing original Vigilante 8 music",
                "original-engines" => "Verifying original engine audio",
                "files" => "Importing game data",
                _ => "Checking disc" };
            message.Text = phase + "\n" + p.Item;
        });
        try
        {
            await Task.Run(() => FirstRunSetup.ImportDisc(source, root, p =>
            {
                Console.Error.WriteLine($"[Setup] {p.Phase} {p.Current}/{p.Total} {p.Item}");
                ((IProgress<LooseImportProgress>)reports).Report(p);
            }, sourceCancel.Token), sourceCancel.Token);
        }
        catch (OperationCanceledException) { error = "Import cancelled. Any previously verified disc was kept."; }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"[Setup] {ex}");
            error = ex.Message;
        }
        finally
        {
            cancel = null;
            progress.Visible = false;
            exit.Text = "Exit setup";
            exit.Enabled = true;
            RefreshState(error, error != null);
        }
        if (closeAfterImport) Close();
    }

    protected override void Dispose(bool disposing)
    {
        base.Dispose(disposing);
        if (!disposing) return;
        emblem?.Dispose();
        normal.Dispose(); title.Dispose(); card.Dispose(); small.Dispose();
    }

    static T OnSta<T>(Func<T> action)
    {
        T result = default!;
        System.Runtime.ExceptionServices.ExceptionDispatchInfo? failure = null;
        var thread = new Thread(() => {
            try { result = action(); }
            catch (Exception ex) { failure = System.Runtime.ExceptionServices.ExceptionDispatchInfo.Capture(ex); }
        });
        thread.SetApartmentState(ApartmentState.STA);
        thread.Start();
        thread.Join();
        failure?.Throw();
        return result;
    }

    public static bool Run(string dataRoot, string? initialSource) => OnSta(() =>
    {
        Application.SetHighDpiMode(HighDpiMode.PerMonitorV2);
        Application.EnableVisualStyles();
        using var form = new WindowsDiscSetup(dataRoot, initialSource);
        return form.ShowDialog() == DialogResult.OK;
    });

    sealed class ProbeDirectory : IDisposable
    {
        public string Path { get; } = Directory.CreateTempSubdirectory("v8-two-disc-probe-").FullName;
        public void Dispose() => Directory.Delete(Path, true);
    }

    public static bool Probe(string output) => OnSta(() =>
    {
        Application.SetHighDpiMode(HighDpiMode.PerMonitorV2);
        Application.EnableVisualStyles();
        Directory.CreateDirectory(output);
        using var scratch = new ProbeDirectory();
        string root = scratch.Path;
        bool Check(string name, string sequel, string original)
        {
            using var form = new WindowsDiscSetup(root, null);
            form.ShowInTaskbar = false;
            form.Show();
            Application.DoEvents();
            form.Refresh();
            bool valid = form.emblem != null && !form.launch.Enabled && form.browse.Enabled &&
                form.sequelStatus.Text == sequel && form.originalStatus.Text == original;
            if (!valid) return false;
            using var screenshot = new Bitmap(form.Width, form.Height);
            form.DrawToBitmap(screenshot, new Rectangle(0, 0, form.Width, form.Height));
            // The first banner stripe starts at the client origin. Reject
            // blank captures rather than mistaking file existence for paint.
            var stripe = form.PointToScreen(new Point(150, 2));
            var origin = form.Bounds.Location;
            var pixel = screenshot.GetPixel(stripe.X - origin.X, stripe.Y - origin.Y);
            if (pixel.R < 200 || pixel.G < 145 || pixel.B > 150)
                throw new InvalidOperationException("The native installer banner did not render.");
            screenshot.Save(Path.Combine(output, name + ".png"));
            form.Close();
            return true;
        }
        if (!Check("setup-fresh", "REQUIRED", "REQUIRED")) return false;
        foreach (var name in new[] { ".recompone-import-complete", "SYSTEM.CNF", "SLUS_008.68" })
            File.WriteAllText(Path.Combine(root, name), "Probe-only fixture");
        if (!Check("setup-sequel-only", "VERIFIED", "REQUIRED")) return false;
        Console.Error.WriteLine("[TwoDiscSetupProbe] PASS: native window; missing either disc blocks launch");
        return true;
    });
}
#endif
