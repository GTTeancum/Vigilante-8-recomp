// Export exact instruction listings and scalar/data references for selected functions.

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.symbol.Reference;

import java.io.File;
import java.io.PrintWriter;

public class DreamcastInstructionRecon extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 2)
            throw new IllegalArgumentException("output and function entries required");

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
                out.println("FUNCTION " + function.getEntryPoint() + " " + function.getName()
                    + " size=" + function.getBody().getNumAddresses());
                Instruction instruction = getInstructionAt(function.getEntryPoint());
                while (instruction != null && function.getBody().contains(instruction.getAddress())) {
                    StringBuilder line = new StringBuilder();
                    line.append(instruction.getAddress()).append(": ");
                    for (byte value : instruction.getBytes())
                        line.append(String.format("%02x", value & 0xff));
                    while (line.length() < 30)
                        line.append(' ');
                    line.append(instruction.getMnemonicString()).append(' ');
                    for (int operand = 0; operand < instruction.getNumOperands(); operand++) {
                        if (operand != 0)
                            line.append(", ");
                        line.append(instruction.getDefaultOperandRepresentation(operand));
                    }
                    Reference[] references = instruction.getReferencesFrom();
                    if (references.length != 0) {
                        line.append(" ; refs=");
                        for (int ref = 0; ref < references.length; ref++) {
                            if (ref != 0)
                                line.append('|');
                            line.append(references[ref].getReferenceType())
                                .append(':').append(references[ref].getToAddress());
                        }
                    }
                    out.println(line);
                    instruction = instruction.getNext();
                }
                out.println();
            }
        }
        println("DreamcastInstructionRecon complete");
    }
}
