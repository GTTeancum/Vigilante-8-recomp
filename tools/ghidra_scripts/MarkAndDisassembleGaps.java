// MarkAndDisassembleGaps.java -- create-function at known cross-DLL call targets.
//
// Reads a list of u32 addresses from the script args (hex strings,
// space-separated) and calls createFunction() at each in the current program.
// Pass via -postScript MarkAndDisassembleGaps.java 0x80019e20 0x80019e7c ...

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;

public class MarkAndDisassembleGaps extends GhidraScript {

    public void run() throws Exception {
        String[] args = getScriptArgs();
        for (String a : args) {
            long va = Long.decode(a);
            Address addr = currentProgram.getAddressFactory()
                                         .getDefaultAddressSpace()
                                         .getAddress(va);
            try {
                disassemble(addr);
                createFunction(addr, null);
                println("created function at " + addr);
            } catch (Exception e) {
                println("FAIL " + a + ": " + e.getMessage());
            }
        }
    }
}
