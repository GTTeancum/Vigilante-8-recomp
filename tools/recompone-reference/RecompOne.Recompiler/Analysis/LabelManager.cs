using RecompOne.Recompiler.Disasm;

namespace RecompOne.Recompiler.Analysis;

public static class LabelManager
{
    public static HashSet<uint> Collect(MipsFunction func)
    {
        var labels = new HashSet<uint>();

        foreach (var instr in func.Instructions)
        {
            if (!instr.HasDelaySlot) continue;

            uint op = instr.Word >> 26;

            if (op is 1 or 4 or 5 or 6 or 7 or 20 or 21 or 22 or 23)
                // REGIMM and ordinary/likely BEQ, BNE, BLEZ and BGTZ.
            {
                uint t = instr.BranchTarget;
                if (t >= func.Start && t < func.End) labels.Add(t);
            }
            else if (op == 18 && ((instr.Word >> 21) & 0x1F) == 8)
            {
                uint t = instr.BranchTarget;
                if (t >= func.Start && t < func.End) labels.Add(t);
            }
            else if (op == 2) //internal function goto
            {
                uint t = instr.JumpTarget;
                if (t >= func.Start && t < func.End) labels.Add(t);
            }
        }

        foreach (var jtbl in func.JumpTables)
            foreach (uint entry in jtbl.Entries)
                if (entry >= func.Start && entry < func.End)
                    labels.Add(entry);

        return labels;
    }
}
