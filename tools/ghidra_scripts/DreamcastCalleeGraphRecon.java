// Export a bounded, callee-only call graph and decompilation for Dreamcast SH-4 entries.

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileOptions;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.symbol.Reference;

import java.io.File;
import java.io.PrintWriter;
import java.util.ArrayDeque;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.Map;
import java.util.Set;

public class DreamcastCalleeGraphRecon extends GhidraScript {
    private static class Node {
        Function function;
        int depth;

        Node(Function function, int depth) {
            this.function = function;
            this.depth = depth;
        }
    }

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 3) {
            throw new IllegalArgumentException("output, depth, entries required");
        }

        int maxDepth = Integer.parseInt(args[1]);
        ArrayDeque<Node> queue = new ArrayDeque<>();
        for (int i = 2; i < args.length; i++) {
            Function function = getFunctionAt(toAddr(args[i]));
            if (function == null) {
                function = getFunctionContaining(toAddr(args[i]));
            }
            if (function != null) {
                queue.add(new Node(function, 0));
            }
        }

        Map<Address, Function> selected = new LinkedHashMap<>();
        Map<Address, Integer> depths = new LinkedHashMap<>();
        Set<String> edges = new LinkedHashSet<>();
        while (!queue.isEmpty()) {
            Node node = queue.removeFirst();
            if (selected.containsKey(node.function.getEntryPoint())) {
                continue;
            }
            selected.put(node.function.getEntryPoint(), node.function);
            depths.put(node.function.getEntryPoint(), node.depth);
            for (Function callee : node.function.getCalledFunctions(monitor)) {
                edges.add(node.function.getEntryPoint() + " -> " + callee.getEntryPoint());
                if (node.depth < maxDepth) {
                    queue.addLast(new Node(callee, node.depth + 1));
                }
            }
        }

        DecompInterface decompiler = new DecompInterface();
        decompiler.setOptions(new DecompileOptions());
        decompiler.toggleCCode(true);
        decompiler.toggleSyntaxTree(true);
        if (!decompiler.openProgram(currentProgram)) {
            throw new IllegalStateException("decompiler could not open current program");
        }

        try (PrintWriter out = new PrintWriter(new File(args[0]), "UTF-8")) {
            out.println("functions=" + selected.size());
            out.println("max_depth=" + maxDepth);
            out.println("-- edges --");
            for (String edge : edges) {
                out.println(edge);
            }

            for (Function function : selected.values()) {
                out.println();
                out.println("============================================================");
                out.println("FUNCTION " + function.getEntryPoint() + " " + function.getName()
                    + " depth=" + depths.get(function.getEntryPoint())
                    + " size=" + function.getBody().getNumAddresses());
                out.println("-- called --");
                for (Function callee : function.getCalledFunctions(monitor)) {
                    out.println(callee.getEntryPoint() + " " + callee.getName()
                        + " size=" + callee.getBody().getNumAddresses());
                }

                out.println("-- external references --");
                Set<String> references = new LinkedHashSet<>();
                Instruction instruction = getInstructionAt(function.getEntryPoint());
                while (instruction != null && function.getBody().contains(instruction.getAddress())) {
                    for (Reference reference : instruction.getReferencesFrom()) {
                        Address target = reference.getToAddress();
                        if (!function.getBody().contains(target)) {
                            references.add(instruction.getAddress() + " "
                                + reference.getReferenceType() + " -> " + target);
                        }
                    }
                    instruction = instruction.getNext();
                }
                for (String reference : references) {
                    out.println(reference);
                }

                out.println("-- decompile --");
                DecompileResults result = decompiler.decompileFunction(function, 180, monitor);
                out.println(result != null && result.decompileCompleted()
                    ? result.getDecompiledFunction().getC()
                    : "<decompile failed: "
                        + (result == null ? "no result" : result.getErrorMessage()) + ">");
            }
        } finally {
            decompiler.dispose();
        }
        println("DreamcastCalleeGraphRecon functions=" + selected.size());
    }
}
