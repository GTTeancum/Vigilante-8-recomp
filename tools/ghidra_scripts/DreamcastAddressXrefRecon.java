// Export all references to selected Dreamcast addresses and their containing functions.

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.symbol.Reference;

import java.io.File;
import java.io.PrintWriter;

public class DreamcastAddressXrefRecon extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 2)
            throw new IllegalArgumentException("output and target addresses required");

        try (PrintWriter out = new PrintWriter(new File(args[0]), "UTF-8")) {
            for (int index = 1; index < args.length; index++) {
                Address target = toAddr(args[index]);
                out.println("============================================================");
                out.println("TARGET " + target);
                Reference[] references = getReferencesTo(target);
                int count = 0;
                for (Reference reference : references) {
                    Function function = getFunctionContaining(reference.getFromAddress());
                    out.println(reference.getFromAddress() + " "
                        + reference.getReferenceType() + " from="
                        + (function == null ? "<no-function>" :
                            function.getEntryPoint() + " " + function.getName()));
                    if (reference.getReferenceType().isCall()) {
                        Instruction instruction = getInstructionAt(reference.getFromAddress());
                        Instruction start = instruction;
                        for (int back = 0; back < 8 && start != null; back++)
                            start = start.getPrevious();
                        for (int line = 0; line < 11 && start != null; line++) {
                            out.println("    " + start.getAddress() + " " + start);
                            start = start.getNext();
                        }
                    }
                    count++;
                }
                out.println("count=" + count);
            }
        }
        println("DreamcastAddressXrefRecon complete");
    }
}
