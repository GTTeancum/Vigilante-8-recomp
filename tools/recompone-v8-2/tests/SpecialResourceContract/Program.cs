using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;

V82VehicleRegistry.ValidatePackage(args[0]);
var memory = new PSMemory();
int checks = 0;
void Check(bool value, string name)
{
    if (!value) throw new Exception(name);
    checks++;
}
const uint native = (1u << 0) | (1u << 17);
Check(V82VehicleRegistry.SpecialResourceMask(memory, native) == native,
    "native resource mask unchanged");
memory.WriteU8(0x8006B8F4, 71);
Check(V82VehicleRegistry.SpecialResourceMask(memory, native) == native,
    "original special does not borrow a retail DLL");
memory.WriteU8(0x8006B8F4, 76);
Check(V82VehicleRegistry.SpecialResourceMask(memory, native) == native,
    "generic guest has no retail DLL dependency");
V82VehicleRegistry.SelectType(71);
Check(V82VehicleRegistry.SpecialResourceMask(memory, native) == native,
    "original selection remains free of retail DLL dependencies");
V82VehicleRegistry.SelectType(-1);
Check(V82VehicleRegistry.SpecialResourceMask(memory, native) == native,
    "leaving guest selection removes dependency");
checks += OriginalSpecialChecks.Run();
checks += ProjectileOracleChecks.Run();
checks += FlightOracleChecks.Run();
Console.WriteLine($"Special resource contract: {checks} checks passed");
