// Emulate the retail Dreamcast material-header compiler for the two water
// contexts after applying the exact default and per-pass field writes.

import ghidra.app.emulator.EmulatorHelper;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.lang.Register;

import java.io.File;
import java.io.PrintWriter;
import java.math.BigInteger;

public class DreamcastMaterialHeaderEmu extends GhidraScript {
    private static final long HEADER_COMPILER = 0x8c041e60L;
    private static final long RETURN_SENTINEL = 0x8c010000L;
    private static final long STACK_TOP = 0x8c35f000L;
    private static final long SCRATCH_CONTEXT = 0x8c350000L;
    private static final long SCRATCH_DESCRIPTOR = 0x8c351000L;

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length != 1)
            throw new IllegalArgumentException("output path required");

        try (PrintWriter out = new PrintWriter(new File(args[0]), "UTF-8")) {
            emitWaterContext(out, "untextured", 0x002004caL, false);
            emitWaterContext(out, "textured-xwat", 0x00aa1ec2L, true);
        }
        println("DreamcastMaterialHeaderEmu complete");
    }

    private void emitWaterContext(
            PrintWriter out, String name, long dirtyMask, boolean textured)
            throws Exception {
        EmulatorHelper emu = new EmulatorHelper(currentProgram);
        try {
            Address context = toAddr(SCRATCH_CONTEXT);
            byte[] zeroes = new byte[0xc0];
            emu.writeMemory(context, zeroes);
            writeDefaults(emu, context);
            runCompiler(emu, context);

            write32(emu, context.add(0x00), dirtyMask);
            write32(emu, context.add(0x08), 2);
            write32(emu, context.add(0x18), 0);
            write32(emu, context.add(0x2c), 8);
            write32(emu, context.add(0x30), 6);
            write32(emu, context.add(0x44), 1);
            if (!textured) {
                write32(emu, context.add(0x20), 1);
            }
            else {
                write32(emu, context.add(0x40), 0);
                write32(emu, context.add(0x48), 0);
                write32(emu, context.add(0x4c), 0);
                write32(emu, context.add(0x60), 3);

                // Retail XWAT's descriptor fields consumed by the compiler.
                // The texture pointer itself only affects mode3's address;
                // keep it deterministic in scratch memory.
                Address descriptor = toAddr(SCRATCH_DESCRIPTOR);
                emu.writeMemory(descriptor, new byte[0x20]);
                write32(emu, descriptor.add(0x08), 0);
                write32(emu, descriptor.add(0x18), 0x00000005L);
                write32(emu, descriptor.add(0x1c), 0x00010000L);
                write32(emu, context.add(0x68), 0x00100000L);
                write32(emu, context.add(0x6c), descriptor.getOffset());
            }
            runCompiler(emu, context);

            out.println(name);
            out.printf("dirty=%08x%n", read32(emu, context));
            out.printf("pcw=%08x%n", read32(emu, context.add(0x90)));
            out.printf("isp=%08x%n", read32(emu, context.add(0x94)));
            out.printf("tsp=%08x%n", read32(emu, context.add(0x98)));
            out.printf("tcw=%08x%n", read32(emu, context.add(0x9c)));
            out.println();
        }
        finally {
            emu.dispose();
        }
    }

    private void writeDefaults(EmulatorHelper emu, Address c) throws Exception {
        long[][] values = {
            {0x00, 0x13f3ffffL}, {0x04, 0}, {0x08, 0}, {0x0c, 0},
            {0x10, 1}, {0x14, 4}, {0x18, 2}, {0x1c, 0},
            {0x20, 3}, {0x24, 0}, {0x28, 0}, {0x2c, 10},
            {0x30, 11}, {0x34, 0}, {0x38, 0}, {0x3c, 0},
            {0x40, 0}, {0x44, 1}, {0x48, 0}, {0x4c, 3},
            {0x50, 0}, {0x54, 1}, {0x58, 0}, {0x5c, 4},
            {0x60, 1}, {0x64, 0}, {0x68, 0}, {0x6c, 0},
            {0x70, 0x3f800000L}, {0x74, 0x3f800000L},
            {0x78, 0x3f800000L}, {0x7c, 0x3f800000L},
            {0x80, 0}, {0x84, 0}, {0x88, 0}, {0x8c, 0},
            {0xbc, 2},
        };
        for (long[] value : values)
            write32(emu, c.add(value[0]), value[1]);
    }

    private void runCompiler(EmulatorHelper emu, Address context) throws Exception {
        Register pc = currentProgram.getLanguage().getProgramCounter();
        emu.writeRegister(pc, BigInteger.valueOf(HEADER_COMPILER));
        emu.writeRegister("r4", BigInteger.valueOf(context.getOffset()));
        emu.writeRegister("r15", BigInteger.valueOf(STACK_TOP));
        emu.writeRegister("pr", BigInteger.valueOf(RETURN_SENTINEL));
        for (int register = 8; register <= 14; register++)
            emu.writeRegister("r" + register, BigInteger.ZERO);
        for (int step = 0; step < 20000; step++) {
            if (emu.getExecutionAddress().getOffset() == RETURN_SENTINEL)
                return;
            if (!emu.step(monitor))
                throw new IllegalStateException(
                    "emulation failed at " + emu.getExecutionAddress() +
                    ": " + emu.getLastError());
        }
        throw new IllegalStateException("header compiler exceeded step limit");
    }

    private void write32(EmulatorHelper emu, Address address, long value)
            throws Exception {
        byte[] bytes = {
            (byte)value,
            (byte)(value >>> 8),
            (byte)(value >>> 16),
            (byte)(value >>> 24),
        };
        emu.writeMemory(address, bytes);
    }

    private long read32(EmulatorHelper emu, Address address) throws Exception {
        byte[] bytes = emu.readMemory(address, 4);
        return (bytes[0] & 0xffL) |
            ((bytes[1] & 0xffL) << 8) |
            ((bytes[2] & 0xffL) << 16) |
            ((bytes[3] & 0xffL) << 24);
    }
}
