using System.Numerics;
using System.Reflection;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;

namespace Recompiled;

public static partial class Vigilante82PC
{
    static void CheckWaterFit()
    {
        var probe=typeof(V82AutoWaterski).Assembly.GetType("RecompOne.Runtime.Sdk.V82TransformationProbe",true)!;
        var angle=probe.GetMethod("InspectionAngle",BindingFlags.Static|BindingFlags.NonPublic)!;
        var separation=probe.GetMethod("InspectionWaterSeparation",BindingFlags.Static|BindingFlags.NonPublic)!;
        const string separationVariable="RECOMPONE_V82_WATER_SITE_SEPARATION_UNITS";
        string? priorSeparation=Environment.GetEnvironmentVariable(separationVariable);
        try
        {
            foreach (var test in new[]{("0",0),("32",32<<16),("128",128<<16),("129",0),("-1",0),("invalid",0)})
            {
                Environment.SetEnvironmentVariable(separationVariable,test.Item1);
                Check((int)separation.Invoke(null,null)! == test.Item2,"fixture water-site separation is bounded native fixed-point");
            }
        }
        finally { Environment.SetEnvironmentVariable(separationVariable,priorSeparation); }
        const string angleVariable="RECOMPONE_V82_WATER_INSPECTION_TEST_DEGREES";
        string? priorAngle=Environment.GetEnvironmentVariable(angleVariable);
        try
        {
            foreach (var test in new[]{("45",512),("-45",-512),("60",683),("180",2048),("181",0),("invalid",0)})
            {
                Environment.SetEnvironmentVariable(angleVariable,test.Item1);
                Check((int)angle.Invoke(null,["TEST",0,-180,180])! == test.Item2,
                    "inspection angle conversion/range fallback is bounded and native-angle accurate");
            }
        }
        finally { Environment.SetEnvironmentVariable(angleVariable,priorAngle); }
        var fit=typeof(V82AutoWaterski).Assembly.GetType("RecompOne.Runtime.Sdk.V82WaterAttachmentFit",true)!;
        var shared=fit.GetMethod("IsSharedWaterSupport",BindingFlags.Static|BindingFlags.NonPublic)!;
        {
            var m=new PSMemory();
            const uint node=0x80028000,commonBank=0x80029000,authoredBank=0x8002A000;
            m.WriteU32(node+0x5C,commonBank); m.WriteU16(node+0x1A,63);
            Check((bool)shared.Invoke(null,[m,node,commonBank,(ushort)63])!,"common water support selected by bank AND native mode-table slot");
            Check(!(bool)shared.Invoke(null,[m,node,commonBank,(ushort)70])!,"same bank different asset role is not fitted");
            m.WriteU32(node+0x5C,authoredBank);
            Check(!(bool)shared.Invoke(null,[m,node,commonBank,(ushort)63])!,"vehicle-authored transformation must not be fitted as generic ski arm");
            Check((bool)shared.Invoke(null,[m,node,authoredBank,(ushort)63])!,"registry-resolved replacement bank follows the same role contract");
            Check(!(bool)shared.Invoke(null,[m,0u,commonBank,(ushort)63])!,"missing support is not fitted");
            Check(!(bool)shared.Invoke(null,[m,node,0u,(ushort)63])!,"missing transformation bank fails closed");
        }
        var poseType=fit.GetNestedType("Pose",BindingFlags.NonPublic)!;
        var triangleType=fit.GetNestedType("Triangle",BindingFlags.NonPublic)!;
        var triangles=(System.Collections.IList)Activator.CreateInstance(typeof(List<>).MakeGenericType(triangleType))!;
        triangles.Add(Activator.CreateInstance(triangleType,new Vector3(0,-100,-100),new Vector3(0,100,-100),new Vector3(0,100,100))!);
        triangles.Add(Activator.CreateInstance(triangleType,new Vector3(0,-100,-100),new Vector3(0,100,100),new Vector3(0,-100,100))!);
        var method=fit.GetMethod("Fit",BindingFlags.Static|BindingFlags.NonPublic)!;
        const uint mesh=0x80026000,source=0x80026100;
        {
            var m=new PSMemory();
            const uint packet=0x80026200;
            m.WriteU8(mesh+1,16); m.WriteU16(mesh+4,3); m.WriteU32(mesh+8,source);
            m.WriteU16(source+8,10); m.WriteU16(source+18,10);
            m.WriteU16(mesh+6,2); m.WriteU32(mesh+0x10,packet);
            // Runtime kind 1, material flags set, byte-offset indices.
            foreach (uint p in new[]{packet,packet+32})
            {
                m.WriteU8(p+3,0xC6); m.WriteU16(p+4,0);
                m.WriteU16(p+6,8); m.WriteU16(p+8,16);
            }
            var pose=Activator.CreateInstance(poseType,Vector3.UnitX,Vector3.UnitY,Vector3.UnitZ,Vector3.Zero)!;
            var read=fit.GetMethod("ReadTriangles",BindingFlags.Static|BindingFlags.NonPublic)!;
            var parsed=(System.Collections.IList)Activator.CreateInstance(typeof(List<>).MakeGenericType(triangleType))!;
            Check((bool)read.Invoke(null,[m,mesh,pose,parsed])!,"runtime packet kind and byte offsets parse");
            Check(parsed.Count==2,"runtime packet stride preserves both triangles");
            parsed.Clear(); m.WriteU16(packet+32+8,17);
            Check(!(bool)read.Invoke(null,[m,mesh,pose,parsed])!,"misaligned runtime vertex offset rejected");
            Check(parsed.Count==0,"invalid trailing packet does not retain partial body");
            m.WriteU16(packet+32+8,24);
            Check(!(bool)read.Invoke(null,[m,mesh,pose,parsed])!,"out of bounds runtime vertex offset rejected");
            Check(parsed.Count==0,"out of bounds mesh remains atomic");
        }
        foreach (int side in new[]{-1,1})
        foreach (int yawSign in new[]{1,-1})
        {
            var m=new PSMemory();
            m.WriteU8(mesh+1,16); m.WriteU16(mesh+4,3); m.WriteU32(mesh+8,source);
            short[] coordinates=[0,0,0,123,0,10,0,456,(short)(side*20*yawSign),0,0,789];
            for (uint i=0;i<coordinates.Length;i++) m.WriteU16(source+i*2,(ushort)coordinates[i]);
            var pose=Activator.CreateInstance(poseType,Vector3.UnitX*yawSign,Vector3.UnitY,Vector3.UnitZ*yawSign,new Vector3(side*1000,0,0))!;
            var result=method.Invoke(null,[m,mesh,pose,triangles])!;
            Check(result!=null,"shared water fit creates candidate for either side");
            var bytes=(byte[])result.GetType().GetProperty("Vertices")!.GetValue(result)!;
            Check(BitConverter.ToInt16(bytes,0)==-side*1001*yawSign,"inner arm endpoint reaches body with one vertex quantum overlap, including rotated native roots");
            Check(BitConverter.ToInt16(bytes,8)==-side*1001*yawSign,"whole body-facing edge is fitted");
            Check(BitConverter.ToInt16(bytes,16)==side*20*yawSign,"outboard float/arm geometry remains fixed");
            Check(BitConverter.ToInt16(bytes,6)==123 && BitConverter.ToInt16(bytes,14)==456 && BitConverter.ToInt16(bytes,22)==789,
                "native vertex padding preserved");
            for (uint i=0;i<coordinates.Length;i++) Check((short)m.ReadU16(source+i*2)==coordinates[i],"plan leaves shared source vertices unchanged");
            Check(m.ReadU32(mesh+8)==source,"plan leaves native source pointer unchanged");
            m.WriteU8(mesh+1,17);
            Check(method.Invoke(null,[m,mesh,pose,triangles])==null,"invalid native scale fails closed");
        }
        {
            // A nearby outboard bumper is closer than the narrow chassis,
            // but it is not in the arm's inward extension direction.
            var assembly=(System.Collections.IList)Activator.CreateInstance(typeof(List<>).MakeGenericType(triangleType))!;
            foreach (var triangle in triangles) assembly.Add(triangle);
            assembly.Add(Activator.CreateInstance(triangleType,new Vector3(-2000,-100,-20),new Vector3(0,-100,-20),new Vector3(0,100,-20))!);
            assembly.Add(Activator.CreateInstance(triangleType,new Vector3(-2000,-100,-20),new Vector3(0,100,-20),new Vector3(-2000,100,-20))!);
            var m=new PSMemory();
            m.WriteU8(mesh+1,16); m.WriteU16(mesh+4,5); m.WriteU32(mesh+8,source);
            short[] coordinates=[0,-5,-5,101,0,5,-5,102,0,5,5,103,0,-5,5,104,-20,0,0,105];
            for (uint i=0;i<coordinates.Length;i++) m.WriteU16(source+i*2,(ushort)coordinates[i]);
            var pose=Activator.CreateInstance(poseType,Vector3.UnitX,Vector3.UnitY,Vector3.UnitZ,new Vector3(-1000,0,0))!;
            var result=method.Invoke(null,[m,mesh,pose,assembly])!;
            var bytes=(byte[])result.GetType().GetProperty("Vertices")!.GetValue(result)!;
            for (int i=0;i<4;i++)
            {
                Check(BitConverter.ToInt16(bytes,i*8)==1001,"inward chassis hit preferred over closer rear bumper");
                Check(BitConverter.ToInt16(bytes,i*8+2)==coordinates[i*4+1] && BitConverter.ToInt16(bytes,i*8+4)==coordinates[i*4+2],
                    "body-facing cross-section is translated coherently, not sheared between body parts");
            }
            Check(BitConverter.ToInt16(bytes,32)==-20,"outboard point preserved on narrow chassis");
            var ray=fit.GetMethod("RayHit",BindingFlags.Static|BindingFlags.NonPublic)!;
            object?[] args=[new Vector3(-1000,0,0),Vector3.UnitX,triangles[0],0f];
            Check((bool)ray.Invoke(null,args)! && Math.Abs((float)args[3]!-1000)<0.01f,"inward ray reaches body");
            args[1]=-Vector3.UnitX;
            Check(!(bool)ray.Invoke(null,args)!,"backward ray hit rejected");
            args[1]=Vector3.UnitY;
            Check(!(bool)ray.Invoke(null,args)!,"parallel ray rejected");
        }
    }
}
