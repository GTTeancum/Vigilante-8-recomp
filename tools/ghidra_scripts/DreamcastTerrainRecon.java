// Export SH-4 code that references plausible terrain-distance scalars.
// Arg: output file

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileOptions;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.address.AddressIterator;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.mem.Memory;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceManager;

import java.io.File;
import java.io.FileWriter;
import java.io.PrintWriter;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.Map;
import java.util.Set;

public class DreamcastTerrainRecon extends GhidraScript {
    private static final Map<Integer, String> VALUES = new LinkedHashMap<>();
    static {
        VALUES.put(0x00001400, "int_5120");
        VALUES.put(0x00002800, "int_10240");
        VALUES.put(Float.floatToIntBits(2560.0f), "float_2560");
        VALUES.put(Float.floatToIntBits(5120.0f), "float_5120");
        VALUES.put(Float.floatToIntBits(10240.0f), "float_10240");
        VALUES.put(Float.floatToIntBits(20.0f), "float_20");
        VALUES.put(Float.floatToIntBits(40.0f), "float_40");
        VALUES.put(Float.floatToIntBits(80.0f), "float_80");
        VALUES.put(Float.floatToIntBits(160.0f), "float_160");
        VALUES.put(Float.floatToIntBits(320.0f), "float_320");
    }

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length != 1) {
            throw new IllegalArgumentException("output file argument required");
        }
        File output = new File(args[0]);
        output.getParentFile().mkdirs();
        Memory memory = currentProgram.getMemory();
        ReferenceManager references = currentProgram.getReferenceManager();
        Set<Function> functions = new LinkedHashSet<>();

        try (PrintWriter writer = new PrintWriter(new FileWriter(output))) {
            writer.println("program=" + currentProgram.getName());
            writer.println("language=" + currentProgram.getLanguageID());
            writer.println();

            for (Map.Entry<Integer, String> wanted : VALUES.entrySet()) {
                writer.println("## " + wanted.getValue() +
                    " bits=0x" + String.format("%08X", wanted.getKey()));
                AddressIterator addresses = memory.getAllInitializedAddressSet()
                    .getAddresses(true);
                while (addresses.hasNext()) {
                    Address address = addresses.next();
                    if ((address.getOffset() & 3) != 0 ||
                        !memory.contains(address.add(3))) {
                        continue;
                    }
                    int value;
                    try {
                        value = memory.getInt(address);
                    }
                    catch (Exception ignored) {
                        continue;
                    }
                    if (value != wanted.getKey()) {
                        continue;
                    }
                    writer.println("literal=" + address);
                    for (Reference reference : references.getReferencesTo(address)) {
                        Function function = currentProgram.getFunctionManager()
                            .getFunctionContaining(reference.getFromAddress());
                        writer.println("  xref=" + reference.getFromAddress() +
                            " type=" + reference.getReferenceType() +
                            " function=" +
                            (function == null ? "-" : function.getEntryPoint()));
                        if (function != null) {
                            functions.add(function);
                        }
                    }
                }
                writer.println();
            }

            DecompInterface decompiler = new DecompInterface();
            decompiler.setOptions(new DecompileOptions());
            decompiler.openProgram(currentProgram);
            for (Function function : functions) {
                writer.println("============================================================");
                writer.println("FUNCTION " + function.getEntryPoint() +
                    " " + function.getName() +
                    " size=" + function.getBody().getNumAddresses());
                writer.println("-- instructions --");
                InstructionIterator instructions = currentProgram.getListing()
                    .getInstructions(function.getBody(), true);
                while (instructions.hasNext()) {
                    Instruction instruction = instructions.next();
                    writer.println(instruction.getAddress() + ": " + instruction);
                }
                writer.println("-- decompile --");
                DecompileResults result = decompiler.decompileFunction(
                    function, 60, monitor);
                if (result != null && result.decompileCompleted()) {
                    writer.println(result.getDecompiledFunction().getC());
                }
                else {
                    writer.println("<decompile failed>");
                }
            }
            decompiler.dispose();
        }
        println("DreamcastTerrainRecon wrote " + output.getAbsolutePath() +
            " functions=" + functions.size());
    }
}
