// MarkDllEntry.java -- mark image[1] as a function entry point.
//
// V8 .DLL overlays store their entry function pointer at offset 4
// (the second u32 of the image), already relocated to an absolute VA
// by tools/dll_preproc.py. This script reads that u32, disassembles
// at the target, and creates a function. Used to seed Ghidra's
// recursive code-flow analysis for DLLs where auto-analysis found
// zero functions.
//
// Pass the base VA via the script arg.

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.address.AddressFactory;
import ghidra.program.model.listing.Program;
import ghidra.program.model.mem.Memory;

public class MarkDllEntry extends GhidraScript {

    public void run() throws Exception {
        String[] args = getScriptArgs();
        long base = (args.length > 0) ? Long.decode(args[0]) : 0x80100000L;

        Program p = currentProgram;
        AddressFactory af = p.getAddressFactory();
        Memory mem = p.getMemory();

        Address hdr = af.getDefaultAddressSpace().getAddress(base);
        long imageEnd = Integer.toUnsignedLong(mem.getInt(hdr));     // image[0]
        long entryVA  = Integer.toUnsignedLong(mem.getInt(hdr.add(4))); // image[1]

        println(String.format("DLL: base=0x%08x imageEnd=0x%08x entry=0x%08x",
                              base, imageEnd, entryVA));

        if (entryVA == 0 || entryVA < base || entryVA >= base + imageEnd) {
            println("entry pointer out of image; trying offset-style entry");
            // Some overlays store entry as an *offset* in image[1] rather than
            // an absolute VA (depends on whether relocation was applied to that
            // word). Try base + entryVA.
            entryVA = base + entryVA;
            if (entryVA >= base + imageEnd) {
                println("still out of range; aborting");
                return;
            }
        }

        Address entry = af.getDefaultAddressSpace().getAddress(entryVA);
        disassemble(entry);
        createFunction(entry, "DllEntry");
        println("created function at " + entry);
    }
}
