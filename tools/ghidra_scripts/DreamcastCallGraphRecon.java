// Export a bounded call graph and decompilation for named entry addresses.

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileOptions;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Reference;

import java.io.File;
import java.io.PrintWriter;
import java.util.ArrayDeque;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.Map;
import java.util.Set;

public class DreamcastCallGraphRecon extends GhidraScript {
    private static class Node {
        Function function;
        int depth;
        Node(Function function, int depth) { this.function = function; this.depth = depth; }
    }

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 3) throw new IllegalArgumentException("output, depth, entries required");
        int maxDepth = Integer.parseInt(args[1]);
        ArrayDeque<Node> queue = new ArrayDeque<>();
        for (int i = 2; i < args.length; i++) {
            Function function = getFunctionAt(toAddr(args[i]));
            if (function == null) function = getFunctionContaining(toAddr(args[i]));
            if (function != null) queue.add(new Node(function, 0));
        }
        Map<Address, Function> selected = new LinkedHashMap<>();
        Set<String> edges = new LinkedHashSet<>();
        while (!queue.isEmpty()) {
            Node node = queue.removeFirst();
            if (selected.putIfAbsent(node.function.getEntryPoint(), node.function) != null) continue;
            for (Function callee : node.function.getCalledFunctions(monitor)) {
                edges.add(node.function.getEntryPoint() + " -> " + callee.getEntryPoint());
                if (node.depth < maxDepth) queue.add(new Node(callee, node.depth + 1));
            }
            for (Reference reference : getReferencesTo(node.function.getEntryPoint())) {
                Function caller = getFunctionContaining(reference.getFromAddress());
                if (caller == null) continue;
                edges.add(caller.getEntryPoint() + " -> " + node.function.getEntryPoint());
                if (node.depth < maxDepth) queue.add(new Node(caller, node.depth + 1));
            }
        }

        DecompInterface decompiler = new DecompInterface();
        decompiler.setOptions(new DecompileOptions());
        decompiler.openProgram(currentProgram);
        try (PrintWriter out = new PrintWriter(new File(args[0]), "UTF-8")) {
            out.println("functions=" + selected.size());
            out.println("-- edges --");
            for (String edge : edges) out.println(edge);
            for (Function function : selected.values()) {
                out.println();
                out.println("============================================================");
                out.println("FUNCTION " + function.getEntryPoint() + " " + function.getName()
                    + " size=" + function.getBody().getNumAddresses());
                out.println("-- called --");
                for (Function callee : function.getCalledFunctions(monitor)) {
                    out.println(callee.getEntryPoint() + " " + callee.getName()
                        + " size=" + callee.getBody().getNumAddresses());
                }
                out.println("-- decompile --");
                DecompileResults result = decompiler.decompileFunction(function, 180, monitor);
                out.println(result != null && result.decompileCompleted()
                    ? result.getDecompiledFunction().getC() : "<decompile failed>");
            }
        }
        decompiler.dispose();
        println("DreamcastCallGraphRecon functions=" + selected.size());
    }
}
