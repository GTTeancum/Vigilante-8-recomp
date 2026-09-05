// Locate functions that encode the paired single-player/split-screen terrain
// distance tuples used by the PlayStation build.  This is intentionally a
// structural search: SH-4 immediates and referenced float literals are both
// counted, then the best candidates are emitted with complete disassembly.

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
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class DreamcastThresholdTupleRecon extends GhidraScript {
    private static final long[] VALUES = {
        12, 20, 24, 40, 48, 80,
        0x41400000L, 0x41a00000L, 0x41c00000L,
        0x42200000L, 0x42400000L, 0x42a00000L
    };

    private static class Candidate {
        Function function;
        Set<Long> values = new HashSet<>();
        List<String> hits = new ArrayList<>();
        int score;
    }

    private boolean wanted(long value) {
        for (long candidate : VALUES) {
            if (value == candidate) return true;
        }
        return false;
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
            InstructionIterator instructions = currentProgram.getListing()
                .getInstructions(function.getBody(), true);
            while (instructions.hasNext()) {
                Instruction instruction = instructions.next();
                for (int operand = 0; operand < instruction.getNumOperands(); operand++) {
                    for (Object object : instruction.getOpObjects(operand)) {
                        if (object instanceof Scalar) {
                            long value = ((Scalar)object).getUnsignedValue();
                            if (wanted(value)) {
                                candidate.values.add(value);
                                candidate.hits.add(instruction.getAddress() + ": " + instruction);
                            }
                        }
                    }
                }
                for (Reference reference : instruction.getReferencesFrom()) {
                    Address target = reference.getToAddress();
                    if (!target.isMemoryAddress()) continue;
                    try {
                        long value = Integer.toUnsignedLong(getInt(target));
                        if (wanted(value)) {
                            candidate.values.add(value);
                            candidate.hits.add(instruction.getAddress() + ": " + instruction
                                + " -> " + target + " = 0x" + Long.toHexString(value));
                        }
                    }
                    catch (Exception ignored) {
                    }
                }
            }
            int small = 0;
            int floats = 0;
            for (long value : candidate.values) {
                if (value <= 80) small++;
                else floats++;
            }
            candidate.score = small * 10 + floats;
            if (candidate.score >= 20) candidates.add(candidate);
        }

        candidates.sort(Comparator.comparingInt((Candidate c) -> c.score).reversed());
        try (PrintWriter out = new PrintWriter(new File(getScriptArgs()[0]), "UTF-8")) {
            out.println("program=" + currentProgram.getName());
            out.println("candidates=" + candidates.size());
            for (Candidate candidate : candidates) {
                out.println();
                out.println("============================================================");
                out.println("FUNCTION " + candidate.function.getEntryPoint() + " "
                    + candidate.function.getName() + " score=" + candidate.score
                    + " values=" + candidate.values);
                out.println("-- tuple hits --");
                for (String hit : candidate.hits) out.println(hit);
                out.println("-- instructions --");
                InstructionIterator instructions = currentProgram.getListing()
                    .getInstructions(candidate.function.getBody(), true);
                while (instructions.hasNext()) {
                    Instruction instruction = instructions.next();
                    out.println(instruction.getAddress() + ": " + instruction);
                }
            }
        }
        println("DreamcastThresholdTupleRecon candidates=" + candidates.size());
    }
}
