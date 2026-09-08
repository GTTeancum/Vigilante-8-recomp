// Emulate the retail initializers/header compiler for the terrain and ordinary
// world-object material contexts. Texture-address bits use a deterministic
// synthetic descriptor; all state words and field overrides are retail code.

import ghidra.app.emulator.EmulatorHelper;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.lang.Register;

import java.io.File;
import java.io.PrintWriter;
import java.math.BigInteger;

public class DreamcastSceneMaterialHeaderEmu extends GhidraScript {
    private static final long HEADER_COMPILER = 0x8c041e60L;
    private static final long OBJECT_DEFAULTS = 0x8c07be40L;
    private static final long SPRITE_DEFAULTS = 0x8c07bee0L;
    private static final long TEXTURE_BINDER = 0x8c069160L;
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
            emitTerrainBase(out);
            emitTerrainTextured(out);
            emitOrdinaryObject(out);
            emitSky(out);
            emitReflection(out, "reflection-primary");
            emitReflection(out, "reflection-secondary");
        }
        println("DreamcastSceneMaterialHeaderEmu complete");
    }

    private void emitTerrainBase(PrintWriter out) throws Exception {
        EmulatorHelper emu = createInitializedContext();
        try {
            Address context = toAddr(SCRATCH_CONTEXT);
            write32(emu, context.add(0x00), 0x0000010aL);
            write32(emu, context.add(0x18), 0);
            write32(emu, context.add(0x20), 1);
            write32(emu, context.add(0x3c), 0);
            runFunction(emu, HEADER_COMPILER, context);
            emit(out, "terrain-base", emu, context);
        }
        finally {
            emu.dispose();
        }
    }

    private void emitTerrainTextured(PrintWriter out) throws Exception {
        EmulatorHelper emu = createInitializedContext();
        try {
            Address context = toAddr(SCRATCH_CONTEXT);
            Address descriptor = createSyntheticDescriptor(emu);

            // Initial persistent terrain context configuration from
            // 8c08a442..8c08a450.
            write32(emu, context.add(0x00), 0x00001000L);
            write32(emu, context.add(0x4c), 3);
            runFunction(emu, HEADER_COMPILER, context);

            // Per-level terrain-atlas configuration from
            // 8c095080..8c0950a0.
            write32(emu, context.add(0x00), 0x00090002L);
            write32(emu, context.add(0x18), 2);
            write32(emu, context.add(0x5c), 2);
            write32(emu, context.add(0x68), 0x00100000L);
            write32(emu, context.add(0x6c), descriptor.getOffset());
            runFunction(emu, HEADER_COMPILER, context);
            emit(out, "terrain-textured", emu, context);
        }
        finally {
            emu.dispose();
        }
    }

    private void emitOrdinaryObject(PrintWriter out) throws Exception {
        EmulatorHelper emu = createInitializedContext();
        try {
            Address context = toAddr(SCRATCH_CONTEXT);
            Address descriptor = createSyntheticDescriptor(emu);

            write32(emu, context.add(0x00), 0x00a110c2L);
            write32(emu, context.add(0x08), 2);
            write32(emu, context.add(0x10), 0);
            write32(emu, context.add(0x18), 3);
            write32(emu, context.add(0x2c), 8);
            write32(emu, context.add(0x30), 6);
            write32(emu, context.add(0x4c), 0);
            write32(emu, context.add(0x5c), 1);
            write32(emu, context.add(0x68), 0x00100000L);
            write32(emu, context.add(0x6c), descriptor.getOffset());
            runFunction(emu, HEADER_COMPILER, context);
            emit(out, "ordinary-world-object", emu, context);
        }
        finally {
            emu.dispose();
        }
    }

    private void emitSky(PrintWriter out) throws Exception {
        EmulatorHelper emu = createInitializedContext(SPRITE_DEFAULTS);
        try {
            Address context = toAddr(SCRATCH_CONTEXT);
            Address descriptor = createSyntheticDescriptor(emu);
            write32(emu, context.add(0x00), 0x00080000L);
            write32(emu, context.add(0x68), 0x00100000L);
            write32(emu, context.add(0x6c), descriptor.getOffset());
            runFunction(emu, HEADER_COMPILER, context);
            emit(out, "sky-xbgm", emu, context);
        }
        finally {
            emu.dispose();
        }
    }

    private void emitReflection(PrintWriter out, String name) throws Exception {
        EmulatorHelper emu = createInitializedContext(SPRITE_DEFAULTS);
        try {
            Address context = toAddr(SCRATCH_CONTEXT);
            Address descriptor = createSyntheticDescriptor(emu);

            // The retail REFLECT.PVR loader binds the same texture descriptor
            // to the independently initialized contexts at 8c243300 and
            // 8c2433c0 through 8c069160.  Run that exact binder so its dirty
            // flags, texture-layout handling, and header recompile are part of
            // the recovered material contract rather than reproduced by hand.
            runFunction(emu, TEXTURE_BINDER, context, descriptor);
            emit(out, name, emu, context);
        }
        finally {
            emu.dispose();
        }
    }

    private Address createSyntheticDescriptor(EmulatorHelper emu)
            throws Exception {
        Address descriptor = toAddr(SCRATCH_DESCRIPTOR);
        emu.writeMemory(descriptor, new byte[0x20]);

        // A deterministic 64x64, non-mipmapped descriptor. These fields
        // affect TCW address/format only; material state comes from the
        // retail context writes.
        write32(emu, descriptor.add(0x08), 0);
        write32(emu, descriptor.add(0x18), 0x00000005L);
        write32(emu, descriptor.add(0x1c), 0x00010000L);
        return descriptor;
    }

    private EmulatorHelper createInitializedContext() throws Exception {
        return createInitializedContext(OBJECT_DEFAULTS);
    }

    private EmulatorHelper createInitializedContext(long initializer)
            throws Exception {
        EmulatorHelper emu = new EmulatorHelper(currentProgram);
        Address context = toAddr(SCRATCH_CONTEXT);
        emu.writeMemory(context, new byte[0xc0]);
        runFunction(emu, initializer, context);
        return emu;
    }

    private void emit(PrintWriter out, String name, EmulatorHelper emu,
            Address context) throws Exception {
        out.println(name);
        out.printf("dirty=%08x%n", read32(emu, context));
        out.printf("pcw=%08x%n", read32(emu, context.add(0x90)));
        out.printf("isp=%08x%n", read32(emu, context.add(0x94)));
        out.printf("tsp=%08x%n", read32(emu, context.add(0x98)));
        out.printf("tcw=%08x%n", read32(emu, context.add(0x9c)));
        out.println();
    }

    private void runFunction(EmulatorHelper emu, long entry, Address context)
            throws Exception {
        runFunction(emu, entry, context, null);
    }

    private void runFunction(EmulatorHelper emu, long entry, Address context,
            Address secondArgument) throws Exception {
        Register pc = currentProgram.getLanguage().getProgramCounter();
        emu.writeRegister(pc, BigInteger.valueOf(entry));
        emu.writeRegister("r4", BigInteger.valueOf(context.getOffset()));
        emu.writeRegister("r5", secondArgument == null
            ? BigInteger.ZERO
            : BigInteger.valueOf(secondArgument.getOffset()));
        emu.writeRegister("r15", BigInteger.valueOf(STACK_TOP));
        emu.writeRegister("pr", BigInteger.valueOf(RETURN_SENTINEL));
        emu.writeRegister("FPSCR_PR", BigInteger.ZERO);
        emu.writeRegister("FPSCR_SZ", BigInteger.ZERO);
        for (int register = 8; register <= 14; register++)
            emu.writeRegister("r" + register, BigInteger.ZERO);
        for (int step = 0; step < 40000; step++) {
            if (emu.getExecutionAddress().getOffset() == RETURN_SENTINEL)
                return;
            if (!emu.step(monitor))
                throw new IllegalStateException(
                    "emulation failed at " + emu.getExecutionAddress() +
                    ": " + emu.getLastError());
        }
        throw new IllegalStateException("function exceeded step limit");
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
