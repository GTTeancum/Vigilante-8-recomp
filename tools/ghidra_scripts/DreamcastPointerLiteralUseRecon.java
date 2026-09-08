// Resolve SH-4 PC-relative pointer-literal uses when Ghidra records only the
// literal-to-target data reference and omits the instruction-to-literal xref.

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;

import java.io.File;
import java.io.PrintWriter;
import java.util.HashSet;
import java.util.Set;

public class DreamcastPointerLiteralUseRecon extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 2)
            throw new IllegalArgumentException("output and literal addresses required");

        Set<String> targets = new HashSet<>();
        for (int index = 1; index < args.length; index++)
            targets.add(toAddr(args[index]).toString());

        try (PrintWriter out = new PrintWriter(new File(args[0]), "UTF-8")) {
            for (Instruction instruction : currentProgram.getListing().getInstructions(true)) {
                for (int operand = 0; operand < instruction.getNumOperands(); operand++) {
                    String representation = instruction
                        .getDefaultOperandRepresentation(operand).toLowerCase();
                    for (String target : targets) {
                        if (!representation.contains(target))
                            continue;
                        Function function = getFunctionContaining(instruction.getAddress());
                        out.println(target + " <- " + instruction.getAddress() + " "
                            + instruction + " function="
                            + (function == null ? "<no-function>" :
                                function.getEntryPoint() + " " + function.getName()));
                    }
                }
            }
        }
        println("DreamcastPointerLiteralUseRecon complete");
    }
}
