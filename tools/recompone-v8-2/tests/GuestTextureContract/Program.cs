using RecompOne.Runtime.Context;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;
using RecompOne.Runtime.Hle;
using System.Reflection;

void Check(bool value, string description)
{
    if (!value) throw new Exception(description);
    Console.WriteLine("PASS " + description);
}
var dimensions = new NativeVramAllocation(16, 32, 64, 256, 64, 256);
byte[] pixels = Enumerable.Range(0, 1024).Select(i => (byte)i).ToArray();
string Key(NativeVramAllocation r, bool compressed, byte[] data) => NativeVehicleBankSource.TextureContentKey(r, compressed, data);
string original = Key(dimensions, false, pixels);
Check(Key(dimensions, false, pixels.ToArray()) == original, "identical bytes share a key independently of their source address");
pixels[512] ^= 1;
Check(Key(dimensions, false, pixels) != original, "single changed pixel cannot alias");
pixels[512] ^= 1;
Check(Key(dimensions with { Height = 16, Width = 32 }, false, pixels) != original, "different shape cannot alias");
Check(Key(dimensions with { LimitWidth = 128 }, false, pixels) != original, "different bit depth cannot alias");
Check(Key(dimensions, true, pixels) != original, "compressed and raw payloads cannot alias");
var banks = NativeVehicleBankSource.Open(args[0]);
var requests = banks.SelectMany(bank => bank.ReadVramAllocations(true)).ToArray();
Check(requests.All(r => r.ContentKey != null), "every imported image and palette has content identity");
Check(requests.Distinct().Count() < requests.Length, "actual package has byte-identical shareable resources");
var coords = (HashSet<(uint,uint)>)typeof(V82Compat).GetField("SharedGuestCoordinates",BindingFlags.Static|BindingFlags.NonPublic)!.GetValue(null)!;
var releasing = typeof(V82Compat).GetField("_releasingGuestTextures",BindingFlags.Static|BindingFlags.NonPublic)!;
var c = new CpuContext { A0 = 320, A1 = 0 };
var m = new PSMemory();
coords.Add((320,0));
Check(!V82Compat.RetainSharedGuestTexture(c,m) && c.V0 == 1, "one bank cannot free shared match data");
Check(!V82Compat.RetainSharedGuestTexture(c,m), "subsequent bank teardown still retains match ownership");
c.A0 = 384;
Check(V82Compat.RetainSharedGuestTexture(c,m), "native and unrelated rectangles retain normal cleanup");
c.A0 = 320; releasing.SetValue(null,true);
Check(V82Compat.RetainSharedGuestTexture(c,m), "match teardown releases shared backing through native allocator");
releasing.SetValue(null,false);
var descriptors = (HashSet<uint>)typeof(V82Compat).GetField("SyntheticVramDescriptors",BindingFlags.Static|BindingFlags.NonPublic)!.GetValue(null)!;
var backing = (HashSet<uint>)typeof(V82Compat).GetField("SyntheticVramBackingLive",BindingFlags.Static|BindingFlags.NonPublic)!.GetValue(null)!;
uint descriptor = 0x80180000;
descriptors.Add(descriptor); backing.Add(descriptor);
m.WriteU16(descriptor,320); m.WriteU16(descriptor+2,0); c.A0=descriptor;
Check(!V82Compat.IgnoreSyntheticVramFree(c,m) && descriptors.Contains(descriptor) && backing.Contains(descriptor),
    "direct descriptor destruction cannot retire a texture still shared by the match");
V82Compat.RetireVramBackingTree(c,m);
c.A0=320; c.A1=0;
Check(V82Compat.RetainSharedGuestTexture(c,m) && backing.Count==0 && descriptors.Contains(descriptor),
    "native whole-tree retirement invalidates backing ownership but preserves CPU descriptors");
descriptors.Clear(); coords.Clear();
GpuHle.GameplayActive = true;
const uint pathAddress=0x80180100;
void LoadPath(string path)
{
    var bytes=System.Text.Encoding.ASCII.GetBytes(path+"\0");
    for(uint i=0;i<bytes.Length;i++)m.WriteU8(pathAddress+i,bytes[i]);
    c.A0=pathAddress;
    V82ArenaRegistry.CaptureLevelLoad(c,m);
}
LoadPath("LEVELS\\V8\\VALLYFRM");
LoadPath("LEVELS\\V8\\OILFIELD");
Check(V82ArenaRegistry.LoadingOverlayName=="LEVELS_V8_OILFIELD",
    "next native load request replaces stale arena presentation identity");
GpuHle.WideAspect = 16f / 9f;
GpuHle.DebugGameplayTick = 123;
GpuHle.RegisterNativeModalPanel(0x80123450);
V82Compat.CompleteMatchTeardown();
Check(V82ArenaRegistry.LoadingOverlayName==null, "arena presentation identity ends with the match");
Check(!GpuHle.GameplayActive && GpuHle.DebugGameplayTick == 0 && !GpuHle.IsNativeModalPanel(0x80123450),
    "end-of-level boundary removes scene ownership and gameplay state");
Check(GpuHle.WideAspect == 16f / 9f, "persistent display configuration survives scene cleanup");

var nativeImages = new NativeImageSharing();
Check(!nativeImages.TryAcquire("image-a", out _), "unknown native image requires an allocation");
nativeImages.Record("image-a", 0x80190000, 400, 300, 1200);
Check(nativeImages.TryAcquire("image-a", out uint sharedImage) && sharedImage == 0x80190000,
    "identical native image shares its occupied backing descriptor");
nativeImages.Record("image-a", sharedImage, 400, 300, 1200);
Check(!nativeImages.Release(400,300), "first native image owner cannot free a surviving owner's pixels");
Check(nativeImages.Release(400,300), "last native image owner releases the backing rectangle");
Check(!nativeImages.TryAcquire("image-a", out _), "freed native image cannot return a stale descriptor");
nativeImages.Record("image-b", 0x80190040, 400, 300, 800);
Check(nativeImages.TryAcquire("image-b", out sharedImage) && sharedImage == 0x80190040,
    "freed coordinates can be reused by unrelated image content");
Check(nativeImages.Hits == 2 && nativeImages.SavedWords == 2000, "native sharing accounts for reused payload words");
nativeImages.Clear();
Check(nativeImages.Release(400,300) && !nativeImages.TryAcquire("image-b", out _) && nativeImages.Hits == 0,
    "whole-tree retirement invalidates all native image ownership before another match");
