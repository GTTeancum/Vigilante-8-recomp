// Identify Dreamcast functions that operate on the 64x64 terrain grid used by
// the original V8:2 renderer.  The PS1 routine combines a 6-bit cell index,
// 0x3f wrapping, 0x200 row strides, 0x7ff height masks, and 0x2000/0x2040
// material-map offsets.  Ranking that whole fingerprint is substantially less
// ambiguous than searching for distance constants in isolation.

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileOptions;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.scalar.Scalar;
import ghidra.program.model.symbol.Reference;

import java.io.File;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

public class DreamcastTerrainGridRecon extends GhidraScript {
    private static final long[] VALUES = {
        0x3f, 0x40, 0x80, 0x100, 0x200, 0x400, 0x7ff, 0x800,
        0x1e00, 0x2000, 0x2040, 0x3fff, 0xc000
    };

    private static class Candidate {
        Function function;
        Set<Long> values = new LinkedHashSet<>();
        Map<Long, List<String>> hits = new LinkedHashMap<>();
        int score;
    }

    private int weight(long value) {
        if (value == 0x7ff || value == 0x1e00 || value == 0x2040) return 8;
        if (value == 0x3f || value == 0x2000 || value == 0x3fff || value == 0xc000) return 5;
        if (value == 0x200 || value == 0x400 || value == 0x800) return 2;
        return 1;
    }

    private boolean wanted(long value) {
        for (long candidate : VALUES) if (value == candidate) return true;
        return false;
    }

    private void record(Candidate candidate, long value, String text) {
        candidate.values.add(value);
        candidate.hits.computeIfAbsent(value, ignored -> new ArrayList<>()).add(text);
    }

    @Override
    public void run() throws Exception {
        if (getScriptArgs().length != 1) {
            throw new IllegalArgumentException("output path required");
        }

        List<Candidate> candidates = new ArrayList<>();
        FunctionIterator functions = currentProgram.getFunctionManager().getFunctions(true);
        while (functions.hasNext() && !monitor.isCancelled()) {
            Function function = functions.next();
            Candidate candidate = new Candidate();
            candidate.function = function;
            InstructionIterator instructions = currentProgram.getListing().getInstructions(function.getBody(), true);
            while (instructions.hasNext()) {
                Instruction instruction = instructions.next();
                for (int operand = 0; operand < instruction.getNumOperands(); operand++) {
                    for (Object object : instruction.getOpObjects(operand)) {
                        if (object instanceof Scalar) {
                            long value = ((Scalar)object).getUnsignedValue();
                            if (wanted(value)) record(candidate, value,
                                instruction.getAddress() + ": " + instruction);
                        }
                    }
                }
                for (Reference reference : instruction.getReferencesFrom()) {
                    Address target = reference.getToAddress();
                    if (!target.isMemoryAddress()) continue;
                    try {
                        long value = Integer.toUnsignedLong(getInt(target));
                        if (wanted(value)) record(candidate, value,
                            instruction.getAddress() + ": " + instruction + " -> " + target);
                    }
                    catch (Exception ignored) {
                    }
                }
            }
            for (long value : candidate.values) candidate.score += weight(value);
            if (candidate.score >= 8) candidates.add(candidate);
        }

        candidates.sort(Comparator.comparingInt((Candidate c) -> c.score).reversed());
        DecompInterface decompiler = new DecompInterface();
        decompiler.setOptions(new DecompileOptions());
        decompiler.openProgram(currentProgram);
        try (PrintWriter out = new PrintWriter(new File(getScriptArgs()[0]), "UTF-8")) {
            out.println("program=" + currentProgram.getName());
            out.println("candidates=" + candidates.size());
            int emitted = 0;
            for (Candidate candidate : candidates) {
                if (emitted++ >= 40) break;
                out.println();
                out.println("============================================================");
                out.println("FUNCTION " + candidate.function.getEntryPoint() + " "
                    + candidate.function.getName() + " size="
                    + candidate.function.getBody().getNumAddresses() + " score="
                    + candidate.score + " values=" + candidate.values);
                out.println("-- hits --");
                for (Map.Entry<Long, List<String>> entry : candidate.hits.entrySet()) {
                    out.println("value=0x" + Long.toHexString(entry.getKey()));
                    int count = 0;
                    for (String hit : entry.getValue()) {
                        if (count++ == 8) break;
                        out.println("  " + hit);
                    }
                }
                out.println("-- decompile --");
                DecompileResults result = decompiler.decompileFunction(candidate.function, 120, monitor);
                if (result != null && result.decompileCompleted()) {
                    out.println(result.getDecompiledFunction().getC());
                }
                else {
                    out.println("<decompile failed>");
                }
            }
        }
        decompiler.dispose();
        println("DreamcastTerrainGridRecon candidates=" + candidates.size());
    }
}
