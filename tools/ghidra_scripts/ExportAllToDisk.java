// Export functions, strings, xrefs, decomp, MIPS for current program.
// Arg: outputRoot

import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileOptions;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Data;
import ghidra.program.model.listing.DataIterator;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.listing.Program;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceManager;
import ghidra.program.model.symbol.SymbolIterator;
import ghidra.program.model.symbol.Symbol;

import java.io.File;
import java.io.FileWriter;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.List;

public class ExportAllToDisk extends GhidraScript {

    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 1) {
            println("ExportAllToDisk: missing output root arg");
            return;
        }
        File root = new File(args[0]);
        root.mkdirs();
        File decompDir = new File(root, "decomp");
        decompDir.mkdirs();
        File mipsDir = new File(root, "mips");
        mipsDir.mkdirs();

        Program program = currentProgram;
        println("ExportAllToDisk: program=" + program.getName() + " out=" + root.getAbsolutePath());

        DecompInterface dec = new DecompInterface();
        dec.setOptions(new DecompileOptions());
        dec.openProgram(program);

        FunctionIterator fit = program.getFunctionManager().getFunctions(true);
        PrintWriter fjson = new PrintWriter(new FileWriter(new File(root, "functions.json")));
        fjson.println("[");
        boolean first = true;
        int n = 0;
        while (fit.hasNext()) {
            if (monitor.isCancelled()) break;
            Function f = fit.next();
            String addr = f.getEntryPoint().toString();
            String name = f.getName();
            long size = f.getBody().getNumAddresses();
            int paramCount = f.getParameterCount();

            if (!first) fjson.println(",");
            first = false;
            fjson.print("  {\"address\":\"0x" + addr + "\",\"name\":\""
                + esc(name) + "\",\"size\":" + size + ",\"params\":" + paramCount + "}");

            try {
                DecompileResults dr = dec.decompileFunction(f, 60, monitor);
                if (dr != null && dr.decompileCompleted()) {
                    PrintWriter pw = new PrintWriter(new FileWriter(new File(decompDir, addr + ".c")));
                    pw.println("// addr: 0x" + addr + "  name: " + name);
                    pw.print(dr.getDecompiledFunction().getC());
                    pw.close();
                }
            } catch (Exception e) {
                println("decomp fail " + addr + " : " + e.getMessage());
            }

            PrintWriter pm = new PrintWriter(new FileWriter(new File(mipsDir, addr + ".s")));
            pm.println("# addr: 0x" + addr + "  name: " + name);
            InstructionIterator iit = program.getListing().getInstructions(f.getBody(), true);
            while (iit.hasNext()) {
                Instruction ins = iit.next();
                pm.println(ins.getAddress() + ":  " + ins.toString());
            }
            pm.close();

            n++;
            if ((n % 200) == 0) println("  decomp progress: " + n);
        }
        fjson.println();
        fjson.println("]");
        fjson.close();
        println("functions exported: " + n);

        // Strings -- iterate defined data, dump string-like values.
        PrintWriter sout = new PrintWriter(new FileWriter(new File(root, "strings.txt")));
        DataIterator dit = program.getListing().getDefinedData(true);
        while (dit.hasNext()) {
            if (monitor.isCancelled()) break;
            Data d = dit.next();
            try {
                if (d.hasStringValue()) {
                    Object v = d.getValue();
                    if (v != null) {
                        sout.println(d.getAddress() + "\t" + v.toString().replace("\n", "\\n").replace("\t", "\\t"));
                    }
                }
            } catch (Exception ignored) {}
        }
        sout.close();

        // Xrefs
        PrintWriter xj = new PrintWriter(new FileWriter(new File(root, "xrefs.json")));
        xj.println("{");
        ReferenceManager rm = program.getReferenceManager();
        FunctionIterator fit2 = program.getFunctionManager().getFunctions(true);
        boolean firstFn = true;
        while (fit2.hasNext()) {
            if (monitor.isCancelled()) break;
            Function f = fit2.next();
            Address ep = f.getEntryPoint();
            List<String> callers = new ArrayList<String>();
            for (Reference r : rm.getReferencesTo(ep)) {
                Function cf = program.getFunctionManager().getFunctionContaining(r.getFromAddress());
                if (cf != null) callers.add(cf.getEntryPoint().toString());
                else callers.add(r.getFromAddress().toString());
            }
            List<String> callees = new ArrayList<String>();
            for (Address a : f.getBody().getAddresses(true)) {
                for (Reference r : rm.getReferencesFrom(a)) {
                    if (r.getReferenceType().isCall()) {
                        Function cf = program.getFunctionManager().getFunctionAt(r.getToAddress());
                        if (cf != null) callees.add(cf.getEntryPoint().toString());
                    }
                }
            }
            if (!firstFn) xj.println(",");
            firstFn = false;
            xj.print("  \"0x" + ep + "\": {\"callers\":[");
            for (int i = 0; i < callers.size(); i++) {
                if (i > 0) xj.print(",");
                xj.print("\"0x" + callers.get(i) + "\"");
            }
            xj.print("],\"callees\":[");
            for (int i = 0; i < callees.size(); i++) {
                if (i > 0) xj.print(",");
                xj.print("\"0x" + callees.get(i) + "\"");
            }
            xj.print("]}");
        }
        xj.println();
        xj.println("}");
        xj.close();

        PrintWriter sy = new PrintWriter(new FileWriter(new File(root, "symbols.txt")));
        SymbolIterator si = program.getSymbolTable().getAllSymbols(false);
        while (si.hasNext()) {
            Symbol s = si.next();
            sy.println(s.getAddress() + "\t" + s.getSymbolType() + "\t" + s.getName());
        }
        sy.close();

        println("ExportAllToDisk: done");
        dec.dispose();
    }

    private static String esc(String s) {
        return s.replace("\\", "\\\\").replace("\"", "\\\"");
    }
}
