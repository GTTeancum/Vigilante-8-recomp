// Fix the retail game's invariant SH-4 single-precision/vector-disabled
// FPSCR state over selected functions so Ghidra does not interleave dead
// double-precision interpretations into the decompiler output.

import ghidra.app.script.GhidraScript;
import ghidra.program.model.lang.Register;
import ghidra.program.model.listing.Function;

import java.math.BigInteger;

public class DreamcastSinglePrecisionContext extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length == 0)
            throw new IllegalArgumentException("function entries required");

        Register precision = currentProgram.getRegister("FPSCR_PR");
        Register vectorSize = currentProgram.getRegister("FPSCR_SZ");
        if (precision == null || vectorSize == null)
            throw new IllegalStateException("SH-4 FPSCR context registers unavailable");

        for (String arg : args) {
            Function function = getFunctionContaining(toAddr(arg));
            if (function == null)
                throw new IllegalArgumentException("function not found: " + arg);
            currentProgram.getProgramContext().setValue(
                precision, function.getBody().getMinAddress(),
                function.getBody().getMaxAddress(), BigInteger.ZERO);
            currentProgram.getProgramContext().setValue(
                vectorSize, function.getBody().getMinAddress(),
                function.getBody().getMaxAddress(), BigInteger.ZERO);
            println("single-precision context: " + function.getEntryPoint());
        }
    }
}
