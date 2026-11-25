using Silk.NET.Maths;
using Silk.NET.OpenGL;
using Silk.NET.Windowing;
using Silk.NET.Windowing.Glfw;

GlfwWindowing.Use();
using var window = Window.Create(WindowOptions.Default with
{
    Title = "VehicleSurfaceMaskContract", Size = new Vector2D<int>(64, 64),
    IsVisible = false, API = new GraphicsAPI(ContextAPI.OpenGL, ContextProfile.Core,
        ContextFlags.Default, new APIVersion(4, 5))
});
window.Initialize();
using var gl = GL.GetApi(window);
return SurfaceMaskContract.Run(gl, int.Parse(args.FirstOrDefault() ?? "0"), args.Contains("--hd"));
