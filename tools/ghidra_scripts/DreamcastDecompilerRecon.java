// Export decompiler output for selected Dreamcast SH-4 functions.

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

import java.io.File;
import java.io.PrintWriter;

public class DreamcastDecompilerRecon extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 2)
            throw new IllegalArgumentException(
                "output and function entries required");

        DecompInterface decompiler = new DecompInterface();
        decompiler.toggleCCode(true);
        decompiler.toggleSyntaxTree(true);
        if (!decompiler.openProgram(currentProgram))
            throw new IllegalStateException(
                "decompiler could not open current program");

        try (PrintWriter out = new PrintWriter(new File(args[0]), "UTF-8")) {
            for (int index = 1; index < args.length; index++) {
                Address requested = toAddr(args[index]);
                Function function = getFunctionAt(requested);
                if (function == null)
                    function = getFunctionContaining(requested);
                if (function == null) {
                    out.println("MISSING " + requested);
                    continue;
                }

                out.println("============================================================");
                out.println("FUNCTION " + function.getEntryPoint() + " " +
                    function.getName());
                DecompileResults result = decompiler.decompileFunction(
                    function, 120, monitor);
                if (!result.decompileCompleted()) {
                    out.println("DECOMPILE_FAILED: " + result.getErrorMessage());
                    continue;
                }
                out.println(result.getDecompiledFunction().getC());
            }
        } finally {
            decompiler.dispose();
        }
        println("DreamcastDecompilerRecon complete");
    }
}
