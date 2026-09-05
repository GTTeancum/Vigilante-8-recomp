// Follow direct and table-mediated references to selected game asset names.

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileOptions;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceIterator;
import ghidra.program.model.symbol.ReferenceManager;

import java.io.File;
import java.io.PrintWriter;
import java.util.ArrayDeque;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.Map;
import java.util.Set;

public class DreamcastStringXrefRecon extends GhidraScript {
    private static class Node {
        Address address;
        int depth;
        Node(Address address, int depth) { this.address = address; this.depth = depth; }
    }

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 2) throw new IllegalArgumentException("output path and addresses required");
        ReferenceManager references = currentProgram.getReferenceManager();
        Set<Address> visited = new LinkedHashSet<>();
        Map<Address, Function> functions = new LinkedHashMap<>();
        try (PrintWriter out = new PrintWriter(new File(args[0]), "UTF-8")) {
            ArrayDeque<Node> queue = new ArrayDeque<>();
            for (int i = 1; i < args.length; i++) {
                Address address = toAddr(args[i]);
                queue.add(new Node(address, 0));
                out.println("ROOT " + address);
            }
            while (!queue.isEmpty()) {
                Node node = queue.removeFirst();
                if (!visited.add(node.address)) continue;
                ReferenceIterator iterator = references.getReferencesTo(node.address);
                while (iterator.hasNext()) {
                    Reference reference = iterator.next();
                    Address from = reference.getFromAddress();
                    Function function = currentProgram.getFunctionManager().getFunctionContaining(from);
                    out.println("depth=" + node.depth + " target=" + node.address
                        + " <- " + from + " type=" + reference.getReferenceType()
                        + " function=" + (function == null ? "-" : function.getEntryPoint()));
                    if (function != null) functions.put(function.getEntryPoint(), function);
                    else if (node.depth < 4 && from.isMemoryAddress()) queue.add(new Node(from, node.depth + 1));
                }
            }

            DecompInterface decompiler = new DecompInterface();
            decompiler.setOptions(new DecompileOptions());
            decompiler.openProgram(currentProgram);
            for (Function function : functions.values()) {
                out.println();
                out.println("============================================================");
                out.println("FUNCTION " + function.getEntryPoint() + " " + function.getName()
                    + " size=" + function.getBody().getNumAddresses());
                DecompileResults result = decompiler.decompileFunction(function, 120, monitor);
                out.println(result != null && result.decompileCompleted()
                    ? result.getDecompiledFunction().getC() : "<decompile failed>");
            }
            decompiler.dispose();
        }
        println("DreamcastStringXrefRecon functions=" + functions.size());
    }
}
