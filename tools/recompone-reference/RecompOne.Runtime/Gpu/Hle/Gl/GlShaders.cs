using Silk.NET.OpenGL;

namespace RecompOne.Runtime.Hle;

internal static class GlShaders
{
    public const string FullscreenVs = """
        #version 330 core
        layout(location = 0) in vec2 aPos;
        out vec2 vUv;
        void main() {
            vUv = aPos * 0.5 + 0.5;
            gl_Position = vec4(aPos, 0.0, 1.0);
        }
        """;

    public const string PresentFs = """
        #version 330 core
        in vec2 vUv;
        uniform sampler2D uVram;
        uniform vec2 uOrigin;
        uniform vec2 uSize;
        uniform vec2 uTexSize;
        out vec4 oColor;
        void main() {
            vec2 t = (uOrigin + vUv * uSize) / uTexSize;
            oColor = vec4(texture(uVram, t).rgb, 1.0);
        }
        """;

    public const string Present24Fs = """
        #version 330 core
        in vec2 vUv;
        uniform sampler2D uVram;
        uniform vec2 uOrigin;
        uniform vec2 uSize;
        uniform int uScale;
        out vec4 oColor;

        int u5(float f) { return int(floor(f * 31.0 + 0.5)); }
        int texel16(int lin) {
            vec4 p = texelFetch(uVram, ivec2((lin & 1023) * uScale, ((lin >> 10) & 511) * uScale), 0);
            return u5(p.r) | (u5(p.g) << 5) | (u5(p.b) << 10) | (int(ceil(p.a)) << 15);
        }
        int byteAt(int b) {
            int t = texel16(b >> 1);
            return (b & 1) == 0 ? (t & 0xff) : ((t >> 8) & 0xff);
        }
        void main() {
            int px = int(floor(vUv.x * uSize.x));
            int py = int(floor(vUv.y * uSize.y));
            int ty = int(uOrigin.y) + py;
            int base = (ty * 1024 + int(uOrigin.x)) * 2 + px * 3;
            oColor = vec4(float(byteAt(base)) / 255.0, float(byteAt(base + 1)) / 255.0,
                          float(byteAt(base + 2)) / 255.0, 1.0);
        }
        """;

    public const string PrimVs = """
        #version 330 core
        layout(location = 0) in vec2  inPos;
        layout(location = 1) in uint  inColor;
        layout(location = 2) in int   inClut;
        layout(location = 3) in int   inTexpage;
        layout(location = 4) in vec2  inUV;
        layout(location = 5) in float inPerspectiveW;
        layout(location = 6) in vec3  inBary;
        layout(location = 7) in vec4  inUvBounds;
        layout(location = 8) in float inDepth;
        layout(location = 9) in float inRasterDepth;

        out vec4 vColor;
        out vec2 vUV;
        flat out ivec2 clutBase;
        flat out ivec2 pageBase;
        flat out int   texMode;
        flat out int   vDither;
        flat out int   vSmooth;
        flat out int   vUiTexture;
        flat out int   vParticle;
        flat out int   vShadow;
        flat out int   vLongestEdge;
        flat out int   vRadar;
        flat out int   vHudPlate;
        flat out int   vHealthPlate;
        flat out ivec4 vUvBounds;
        noperspective out vec3 vBary;
        out float vDepth;

        uniform vec2 uVertexOffset;
        uniform vec2 uPosBias;
        uniform vec2 uFbInv;

        void main() {
            vec2 p = (inPos + uVertexOffset + uPosBias) * uFbInv - 1.0;
            float w = max(inPerspectiveW, 1.0);
            float clipZ = (clamp(inRasterDepth, 0.0, 1.0) * 2.0 - 1.0) * w;
            gl_Position = vec4(p * w, clipZ, w);

            vColor = vec4(float(inColor & 0xFFu), float((inColor >> 8) & 0xFFu), float((inColor >> 16) & 0xFFu), 0.0) / 255.0;
            vDither = (inTexpage >> 10) & 1;
            vSmooth = (inTexpage >> 11) & 1;
            vUiTexture = (inTexpage >> 12) & 1;
            vParticle = (inTexpage >> 13) & 1;
            vShadow = (inTexpage >> 14) & 1;
            vRadar = (inTexpage >> 16) & 1;
            vHudPlate = (inTexpage >> 17) & 1;
            vHealthPlate = (inTexpage >> 18) & 1;
            vLongestEdge = inClut;
            vUvBounds = ivec4(round(inUvBounds));
            vBary = inBary;
            vDepth = inDepth;

            if ((inTexpage & 0x8000) != 0) {
                texMode = 4;
            } else {
                texMode = (inTexpage >> 7) & 3;
                vUV = inUV;
                pageBase = ivec2((inTexpage & 0xf) * 64, ((inTexpage >> 4) & 1) * 256);
                clutBase = ivec2((inClut & 0x3f) * 16, (inClut >> 6) & 0x1ff);
            }
        }
        """;

    public const string PrimFs = """
        #version 330 core
        in vec4 vColor;
        in vec2 vUV;
        flat in ivec2 clutBase;
        flat in ivec2 pageBase;
        flat in int   texMode;
        flat in int   vDither;
        flat in int   vSmooth;
        flat in int   vUiTexture;
        flat in int   vParticle;
        flat in int   vShadow;
        flat in int   vLongestEdge;
        flat in int   vRadar;
        flat in int   vHudPlate;
        flat in int   vHealthPlate;
        flat in ivec4 vUvBounds;
        noperspective in vec3 vBary;
        in float vDepth;

        layout(location = 0, index = 0) out vec4 FragColor;
        layout(location = 0, index = 1) out vec4 BlendColor;

        uniform sampler2D uVram;
        uniform sampler2D uDest;
        uniform ivec4 uTexWindow;
        uniform vec4  uBlend;
        uniform vec4  uBlendOpaque = vec4(1.0, 1.0, 1.0, 0.0);
        uniform float uSetMask;
        uniform int   uCheckMask;
        uniform int   uTextureSmoothing;
        uniform int   uTextureMipmaps;
        uniform int   uAnisotropy;
        uniform int   uEnhancedShadows;
        uniform int   uEnhancedParticles;
        uniform int   uEnhancedFog;
        uniform int   uVectorFonts;
        uniform int   uVectorIcons;
        uniform int   uScale;
        uniform vec2  uPosBias;

        const int ditherTbl[16] = int[16](
            -4,  0, -3,  1,
             2, -2,  3, -1,
            -3,  1, -4,  0,
             3, -1,  2, -2 );

        int u5(float f) { return int(floor(f * 31.0 + 0.5)); }
        vec4 fetch(ivec2 c) { return texelFetch(uVram, (c & ivec2(1023, 511)) * uScale, 0); }
        int fetch16(ivec2 c) {
            vec4 p = fetch(c);
            return u5(p.r) | (u5(p.g) << 5) | (u5(p.b) << 10) | (int(ceil(p.a)) << 15);
        }
        ivec2 textureWindow(ivec2 uv) {
            uv = (uv & uTexWindow.xy) | uTexWindow.zw;
            return uv & ivec2(0xff);
        }
        vec4 textureTexel(ivec2 uv) {
            uv = textureWindow(uv);

            if (texMode == 0) {
                int s = fetch16(ivec2(pageBase.x + (uv.x >> 2), pageBase.y + uv.y));
                int idx = (s >> ((uv.x & 3) << 2)) & 0xf;
                return fetch(ivec2(clutBase.x + idx, clutBase.y));
            } else if (texMode == 1) {
                int s = fetch16(ivec2(pageBase.x + (uv.x >> 1), pageBase.y + uv.y));
                int idx = (s >> ((uv.x & 1) << 3)) & 0xff;
                return fetch(ivec2(clutBase.x + idx, clutBase.y));
            }

            return fetch(ivec2(pageBase.x + uv.x, pageBase.y + uv.y));
        }
        bool transparentBlack(vec4 texel) {
            return all(equal(texel.rgb, vec3(0.0))) && texel.a < 0.5;
        }

        vec4 smoothedTexture(vec2 uvf, vec4 nearestTexel) {
            // Reconstruct the native PS1 texture page in shader rather than
            // allocating replacement assets. Pages are at most 256x256, so the
            // virtual reconstruction stays inside the requested 512x512 class.
            // Flat UI may use the full 4x/1024 class; 3D is quantized to 2x.
            // Four palette-resolved taps provide the desired continuous image
            // without the previous 16-tap bicubic cost per reconstruction.
            if (vUiTexture == 0)
                uvf = floor(uvf * 2.0) * 0.5 + 0.25;
            vec2 p = uvf - vec2(0.5);
            ivec2 uv0 = ivec2(floor(p));
            vec2 f = fract(p);
            vec4 s00 = textureTexel(uv0);
            vec4 s10 = textureTexel(uv0 + ivec2(1, 0));
            vec4 s01 = textureTexel(uv0 + ivec2(0, 1));
            vec4 s11 = textureTexel(uv0 + ivec2(1, 1));
            if (transparentBlack(s00)) s00.rgb = nearestTexel.rgb;
            if (transparentBlack(s10)) s10.rgb = nearestTexel.rgb;
            if (transparentBlack(s01)) s01.rgb = nearestTexel.rgb;
            if (transparentBlack(s11)) s11.rgb = nearestTexel.rgb;
            vec3 rgb = mix(
                mix(s00.rgb, s10.rgb, f.x),
                mix(s01.rgb, s11.rgb, f.x),
                f.y);
            return vec4(rgb, nearestTexel.a);
        }
        vec4 filteredTexture(vec2 uvf, vec4 nearestTexel) {
            vec4 base = smoothedTexture(uvf, nearestTexel);
            bool vectorUi = vUiTexture != 0 &&
                ((vParticle != 0 && uVectorFonts != 0) ||
                 (vShadow != 0 && uVectorIcons != 0));
            if ((vUiTexture != 0 && !vectorUi) ||
                (uTextureMipmaps == 0 && uAnisotropy <= 1))
                return base;

            // The source is an indexed PS1 VRAM page, so conventional hardware
            // mipmaps would blend palette indices. Reconstruct the footprint
            // after palette lookup instead. This is a bounded, shader-side
            // mip/anisotropic filter and leaves UI texels and transparency exact.
            vec2 dx = dFdx(uvf), dy = dFdy(uvf);
            float lx = length(dx), ly = length(dy);
            vec2 major = lx >= ly ? dx : dy;
            float footprint = max(lx, ly);
            float minor = max(min(lx, ly), 1.0);
            float ratio = clamp(footprint / minor, 1.0, float(max(uAnisotropy, 1)));
            float mipBlend = uTextureMipmaps != 0 ? smoothstep(1.0, 3.0, footprint) : 0.0;
            float span = 0.35 * max(ratio - 1.0, mipBlend);
            if (span <= 0.001) return base;
            vec2 axis = normalize(major + vec2(1e-6)) * span;
            vec3 rgb =
                smoothedTexture(uvf - axis, nearestTexel).rgb * 0.5 +
                smoothedTexture(uvf + axis, nearestTexel).rgb * 0.5;
            return vec4(rgb, nearestTexel.a);
        }
        vec4 contourTexture(vec2 uvf, out float coverage, out float stp) {
            // Reconstruct a continuous silhouette from the four surrounding
            // indexed texels. This supplies sub-texel contours for small font
            // sprites and genuine fractional coverage for translucent world
            // effects without filtering palette indices or black transparency
            // into the visible colour.
            vec2 p = uvf - vec2(0.5);
            ivec2 uv0 = ivec2(floor(p));
            vec2 f = fract(p);
            ivec2 boundMin = vUvBounds.xy;
            ivec2 boundMax = max(vUvBounds.zw, boundMin);
            vec4 s00 = textureTexel(clamp(uv0, boundMin, boundMax));
            vec4 s10 = textureTexel(clamp(uv0 + ivec2(1, 0), boundMin, boundMax));
            vec4 s01 = textureTexel(clamp(uv0 + ivec2(0, 1), boundMin, boundMax));
            vec4 s11 = textureTexel(clamp(uv0 + ivec2(1, 1), boundMin, boundMax));
            vec4 w = vec4(
                (1.0 - f.x) * (1.0 - f.y),
                f.x * (1.0 - f.y),
                (1.0 - f.x) * f.y,
                f.x * f.y);
            vec4 o = vec4(
                transparentBlack(s00) ? 0.0 : 1.0,
                transparentBlack(s10) ? 0.0 : 1.0,
                transparentBlack(s01) ? 0.0 : 1.0,
                transparentBlack(s11) ? 0.0 : 1.0);
            vec4 ow = o * w;
            coverage = dot(ow, vec4(1.0));
            if (coverage <= 0.0001) {
                stp = 0.0;
                return vec4(0.0);
            }
            vec3 rgb = (s00.rgb * ow.x + s10.rgb * ow.y +
                        s01.rgb * ow.z + s11.rgb * ow.w) / coverage;
            stp = (s00.a * ow.x + s10.a * ow.y +
                   s01.a * ow.z + s11.a * ow.w) / coverage;
            return vec4(rgb, stp);
        }
        bool vividHudMarker(vec3 rgb) {
            bool red = rgb.r > 0.16 &&
                rgb.r > rgb.g * 1.20 &&
                rgb.r > rgb.b * 1.20;
            bool green = rgb.g > 0.16 &&
                rgb.g > rgb.r * 1.20 &&
                rgb.g > rgb.b * 1.20;
            return red || green;
        }
        float sdBox(vec2 p, vec2 halfSize) {
            vec2 q = abs(p) - halfSize;
            return length(max(q, vec2(0.0))) +
                min(max(q.x, q.y), 0.0);
        }
        float sdPolygon24(vec2 p, vec2 vertices[24], int count) {
            float distanceSquared = 1e20;
            bool inside = false;
            int previous = count - 1;
            for (int i = 0; i < 24; i++) {
                if (i >= count) break;
                vec2 a = vertices[i];
                vec2 b = vertices[previous];
                vec2 edge = b - a;
                vec2 relative = p - a;
                vec2 nearest = relative - edge * clamp(
                    dot(relative, edge) / max(dot(edge, edge), 1e-8),
                    0.0, 1.0);
                distanceSquared = min(distanceSquared, dot(nearest, nearest));
                bool crosses = (a.y > p.y) != (b.y > p.y);
                if (crosses &&
                    p.x < (b.x - a.x) * (p.y - a.y) /
                        (b.y - a.y) + a.x)
                    inside = !inside;
                previous = i;
            }
            return (inside ? -1.0 : 1.0) * sqrt(distanceSquared);
        }
        float radarPlateDistance(vec2 p) {
            float d = length(p - vec2(27.5)) - 27.5;
            vec2 polygon[24];

            // Exact upper connector outline, measured from atlas rows 4-13.
            polygon[0] = vec2(61.0, 4.0);
            polygon[1] = vec2(64.0, 4.0);
            polygon[2] = vec2(64.0, 14.0);
            polygon[3] = vec2(49.0, 14.0);
            polygon[4] = vec2(49.0, 10.0);
            polygon[5] = vec2(57.0, 10.0);
            d = min(d, sdPolygon24(p, polygon, 6));

            // Middle connector: separated rows 14-17, joined rows 18-21.
            polygon[0] = vec2(57.0, 14.0);
            polygon[1] = vec2(64.0, 14.0);
            polygon[2] = vec2(64.0, 22.0);
            polygon[3] = vec2(53.0, 22.0);
            polygon[4] = vec2(53.0, 18.0);
            polygon[5] = vec2(57.0, 18.0);
            d = min(d, sdPolygon24(p, polygon, 6));

            // Lower connector and the exact four-pixel health stem.
            polygon[0] = vec2(57.0, 22.0);
            polygon[1] = vec2(64.0, 22.0);
            polygon[2] = vec2(64.0, 28.0);
            polygon[3] = vec2(61.0, 28.0);
            polygon[4] = vec2(57.0, 24.0);
            d = min(d, sdPolygon24(p, polygon, 5));
            d = min(d, sdBox(p - vec2(12.0, 52.0), vec2(2.0, 3.0)));
            return d;
        }
        float mainHudPlateDistance(vec2 p) {
            vec2 polygon[24];
            polygon[0] = vec2(0.0, 0.0);
            polygon[1] = vec2(33.0, 0.0);
            polygon[2] = vec2(37.0, 4.0);
            polygon[3] = vec2(37.0, 20.0);
            polygon[4] = vec2(33.0, 24.0);
            polygon[5] = vec2(0.0, 24.0);
            float d = sdPolygon24(p, polygon, 6);

            polygon[0] = vec2(44.0, 0.0);
            polygon[1] = vec2(80.0, 0.0);
            polygon[2] = vec2(84.0, 4.0);
            polygon[3] = vec2(84.0, 20.0);
            polygon[4] = vec2(80.0, 24.0);
            polygon[5] = vec2(44.0, 24.0);
            polygon[6] = vec2(40.0, 20.0);
            polygon[7] = vec2(40.0, 4.0);
            d = min(d, sdPolygon24(p, polygon, 8));
            d = min(d, sdBox(p - vec2(38.5, 8.0), vec2(1.5, 2.0)));

            // Ammo tail. The one-pixel steps are the measured authored
            // boundary, represented as a continuous high-resolution path.
            polygon[0]  = vec2(62.0, 22.0);
            polygon[1]  = vec2(83.0, 22.0);
            polygon[2]  = vec2(83.0, 23.0);
            polygon[3]  = vec2(84.0, 23.0);
            polygon[4]  = vec2(84.0, 31.0);
            polygon[5]  = vec2(83.0, 31.0);
            polygon[6]  = vec2(83.0, 32.0);
            polygon[7]  = vec2(82.0, 32.0);
            polygon[8]  = vec2(82.0, 33.0);
            polygon[9]  = vec2(81.0, 33.0);
            polygon[10] = vec2(81.0, 34.0);
            polygon[11] = vec2(65.0, 34.0);
            polygon[12] = vec2(65.0, 33.0);
            polygon[13] = vec2(64.0, 33.0);
            polygon[14] = vec2(64.0, 32.0);
            polygon[15] = vec2(63.0, 32.0);
            polygon[16] = vec2(63.0, 31.0);
            polygon[17] = vec2(62.0, 31.0);
            d = min(d, sdPolygon24(p, polygon, 18));
            return d;
        }
        float healthHudPlateDistance(vec2 p) {
            vec2 polygon[24];
            polygon[0]  = vec2(2.0, 1.0);
            polygon[1]  = vec2(14.0, 1.0);
            polygon[2]  = vec2(14.0, 3.0);
            polygon[3]  = vec2(15.0, 3.0);
            polygon[4]  = vec2(15.0, 4.0);
            polygon[5]  = vec2(16.0, 4.0);
            polygon[6]  = vec2(16.0, 46.0);
            polygon[7]  = vec2(15.0, 46.0);
            polygon[8]  = vec2(15.0, 47.0);
            polygon[9]  = vec2(14.0, 47.0);
            polygon[10] = vec2(14.0, 48.0);
            polygon[11] = vec2(13.0, 48.0);
            polygon[12] = vec2(13.0, 49.0);
            polygon[13] = vec2(3.0, 49.0);
            polygon[14] = vec2(3.0, 48.0);
            polygon[15] = vec2(2.0, 48.0);
            polygon[16] = vec2(2.0, 47.0);
            polygon[17] = vec2(1.0, 47.0);
            polygon[18] = vec2(1.0, 46.0);
            polygon[19] = vec2(0.0, 46.0);
            polygon[20] = vec2(0.0, 4.0);
            polygon[21] = vec2(1.0, 4.0);
            polygon[22] = vec2(1.0, 3.0);
            polygon[23] = vec2(2.0, 3.0);
            float d = sdPolygon24(p, polygon, 24);
            d = min(d, sdBox(p - vec2(4.0, 0.5), vec2(2.0, 0.5)));
            d = min(d, sdBox(p - vec2(12.0, 0.5), vec2(2.0, 0.5)));
            return d;
        }
        float hudPlateDistance(vec2 p) {
            return vRadar != 0
                ? radarPlateDistance(p)
                : (vHealthPlate != 0
                    ? healthHudPlateDistance(p)
                    : mainHudPlateDistance(p));
        }
        float analyticHudCoverage(vec2 p) {
            float distance = hudPlateDistance(p);
            float aa = max(fwidth(p.x), fwidth(p.y)) * 0.75;
            return 1.0 - smoothstep(-aa, aa, distance);
        }
        vec4 filledHudSource(ivec2 uv, vec4 center) {
            if (!transparentBlack(center))
                return center;

            ivec2 boundMin = vUvBounds.xy;
            ivec2 boundMax = max(vUvBounds.zw, boundMin);
            for (int radius = 1; radius <= 3; radius++) {
                vec4 sum = vec4(0.0);
                float count = 0.0;
                for (int y = -3; y <= 3; y++) {
                    for (int x = -3; x <= 3; x++) {
                        if (max(abs(x), abs(y)) != radius)
                            continue;
                        ivec2 sampleUv = clamp(
                            uv + ivec2(x, y), boundMin, boundMax);
                        vec4 sampleTexel = textureTexel(sampleUv);
                        if (!transparentBlack(sampleTexel)) {
                            sum += sampleTexel;
                            count += 1.0;
                        }
                    }
                }
                if (count > 0.0)
                    return sum / count;
            }
            return center;
        }
        vec4 analyticHudTexture(vec2 p, vec4 nearestTexel) {
            ivec2 sourceUv = ivec2(
                floor(vUV + vec2(0.0001)));
            vec4 source = filledHudSource(
                sourceUv, nearestTexel);
            vec3 background = source.rgb;
            float plateDistance = hudPlateDistance(p);

            if (vRadar != 0 && length(p - vec2(27.5)) < 27.5) {
                vec2 q = p - vec2(27.5);
                float radialDistance = length(q);
                float aa = max(fwidth(q.x), fwidth(q.y)) * 0.75;
                float halfLine = 0.5;
                float axis = min(abs(q.x), abs(q.y));
                float diagonal =
                    min(abs(q.x - q.y), abs(q.x + q.y)) * 0.7071;
                float spokes = 1.0 - smoothstep(
                    halfLine - aa, halfLine + aa, min(axis, diagonal));
                float ring = 1.0 - smoothstep(
                    halfLine - aa, halfLine + aa,
                    abs(radialDistance - 13.5));
                float hub =
                    1.0 - smoothstep(
                        1.5 - aa, 1.5 + aa, radialDistance);
                // Native source measurements excluding the one-pixel grid:
                // upper face RGB5 mean (2.355,2.855,2.120), lower face
                // (5.830,6.740,4.985). A continuous interpolation retains
                // those authored material endpoints and removes only the
                // unwanted concentric/radial color noise between them.
                vec3 upperFace =
                    vec3(2.355, 2.855, 2.120) / 31.0;
                vec3 lowerFace =
                    vec3(5.830, 6.740, 4.985) / 31.0;
                float faceY = smoothstep(3.0, 52.0, p.y);
                background = mix(upperFace, lowerFace, faceY);
                // The line geometry is analytic, but its color remains the
                // exact palette-resolved source texel at that location. This
                // retains the original quadrant-dependent material while
                // removing the circular/radial noise from the backing.
                vec3 authoredLine = source.rgb;
                background = mix(
                    background, authoredLine, max(spokes, ring));
                background = mix(background, authoredLine, hub);
            }

            // Preserve exact authored border coloration at the measured
            // silhouette. Interior samples are bounded to the same shape.
            if (plateDistance > -1.25)
                background = source.rgb;

            // Every non-transparent source texel in all three retail backing
            // CLUTs has STP set and the packet uses blend mode 0. Retaining
            // that bit gives the exact half-source/half-destination color
            // response while the analytic path removes jagged mask edges.
            return vec4(background, 1.0);
        }
        vec3 stockPaintCorrection(vec3 rgb) {
            if (vUiTexture != 0) return rgb;
            float dominantGreen = rgb.g - max(rgb.r, rgb.b);
            if (dominantGreen <= 0.08 || rgb.g <= 0.16 || rgb.r >= 0.48) return rgb;
            float body = smoothstep(0.08, 0.28, dominantGreen) *
                (1.0 - smoothstep(0.46, 0.60, rgb.r));
            vec3 blue = vec3(
                rgb.r * 0.24 + rgb.b * 0.05,
                rgb.g * 0.24 + rgb.r * 0.08,
                clamp(rgb.g * 1.12 + rgb.b * 0.45, 0.0, 1.0));
            return mix(rgb, blue, body);
        }
        vec3 stockPaintCorrection8(ivec3 c8) {
            return stockPaintCorrection(vec3(c8) / 255.0);
        }
        vec3 quant5(ivec3 c8) {
            if (vDither != 0) {
                ivec2 vp = ivec2(floor(gl_FragCoord.xy / float(uScale) - uPosBias));
                c8 = clamp(c8 + ditherTbl[(vp.y & 3) * 4 + (vp.x & 3)], 0, 255);
            }
            return vec3(min(c8 >> 3, 31)) / 31.0;
        }

        void main() {
            if (uCheckMask != 0 && texelFetch(uDest, ivec2(gl_FragCoord.xy), 0).a >= 0.5) discard;

            if (texMode == 4) {
                vec3 corrected = stockPaintCorrection(vColor.rgb);
                FragColor = vec4(quant5(ivec3(corrected * 255.0 + 0.5)), uSetMask);
                float coverage = 1.0;
                if (uEnhancedShadows != 0 && vShadow != 0) {
                    // Shadow quads arrive as two triangles. Ignore each
                    // triangle's longest edge, normally the shared diagonal,
                    // so softening does not draw a seam through the shadow.
                    float edge = vLongestEdge == 0 ? min(vBary.y, vBary.z) :
                                 vLongestEdge == 1 ? min(vBary.x, vBary.z) :
                                                     min(vBary.x, vBary.y);
                    coverage = smoothstep(0.0, max(fwidth(edge) * 2.5, 0.001), edge) * 0.72;
                }
                BlendColor = vec4(uBlend.rgb * coverage, uBlend.a);
                return;
            }

            int rawU = dFdx(vUV.x) < 0.0 ? int(ceil(vUV.x - 0.0001)) : int(floor(vUV.x + 0.0001));
            int rawV = dFdy(vUV.y) < 0.0 ? int(ceil(vUV.y - 0.0001)) : int(floor(vUV.y + 0.0001));
            vec4 nearestTexel = textureTexel(ivec2(rawU, rawV));
            vec4 texel = uTextureSmoothing != 0 && vSmooth != 0
                ? filteredTexture(vUV, nearestTexel)
                : nearestTexel;
            bool vectorFont =
                vUiTexture != 0 && vParticle != 0 && uVectorFonts != 0;
            bool vectorIcon =
                vUiTexture != 0 && vShadow != 0 && uVectorIcons != 0;
            bool enhancedParticle =
                vUiTexture == 0 && vParticle != 0 && uEnhancedParticles != 0;
            bool analyticHud = vHudPlate != 0 && uVectorIcons != 0;
            vec2 hudLocal = vUV - vec2(vUvBounds.xy);
            float hudCoverage = 1.0;
            float contourCoverage = 1.0;
            float contourStp = nearestTexel.a;

            if ((vectorFont || vectorIcon || enhancedParticle) && !analyticHud) {
                texel = contourTexture(vUV, contourCoverage, contourStp);
                if (vectorIcon && vHudPlate == 0)
                    texel.rgb = nearestTexel.rgb;
                if (vectorFont || vectorIcon) {
                    // A half-coverage contour gives the original bitmap glyph
                    // or UI plate a stable, resolution-independent high-
                    // resolution edge. Final presentation AA handles the
                    // fractional screen edge.
                    if (contourCoverage < 0.5) {
                        discard;
                    }
                } else if (contourCoverage <= 0.01) {
                    discard;
                }
            }

            if (transparentBlack(nearestTexel)) {
                // UI transparency is binary in the original packets. Do not
                // synthesize coverage outside glyph/icon silhouettes: opaque
                // UI draws do not have a usable alpha blend and doing so
                // creates dark halos. Particle sprites are semitransparent and
                // can safely reconstruct their edge coverage.
                bool filteredEdge = vectorFont || vectorIcon || enhancedParticle;
                if (!analyticHud && !filteredEdge) discard;
            }
            if (analyticHud) {
                hudCoverage = analyticHudCoverage(hudLocal);
                // Resolve the measured analytic silhouette while retaining
                // the retail semitransparent material blend. The replacement
                // fill is uniform, so it preserves authored coloration and
                // world interaction without the source stipple/grain.
                if (hudCoverage < 0.5) discard;
                texel = analyticHudTexture(hudLocal, nearestTexel);
            }
            texel.rgb = stockPaintCorrection(texel.rgb);
            if (uEnhancedFog != 0 && vUiTexture == 0 && vDepth > 1.0) {
                float haze = smoothstep(3500.0, 7500.0, vDepth) * 0.22;
                texel.rgb = mix(texel.rgb, vec3(0.48, 0.52, 0.56), haze);
            }
            ivec3 t8 = ivec3(texel.rgb * 31.0 + 0.5) << 3;
            ivec3 c8 = (t8 * ivec3(vColor.rgb * 255.0 + 0.5)) >> 7;
            FragColor = vec4(quant5(ivec3(stockPaintCorrection8(c8) * 255.0 + 0.5)), max(texel.a, uSetMask));
            if (analyticHud) {
                vec4 analyticBlend =
                    texel.a >= 0.5 ? uBlend : uBlendOpaque;
                BlendColor = vec4(
                    analyticBlend.rgb * hudCoverage,
                    mix(1.0, analyticBlend.a, hudCoverage));
            } else if (enhancedParticle && contourCoverage < 0.999) {
                if (contourStp >= 0.5) {
                    BlendColor = vec4(
                        uBlend.rgb * contourCoverage,
                        mix(1.0, uBlend.a, contourCoverage));
                } else {
                    BlendColor = vec4(
                        vec3(contourCoverage),
                        1.0 - contourCoverage);
                }
            } else {
                BlendColor = nearestTexel.a >= 0.5 ? uBlend : uBlendOpaque;
            }
        }
        """;

    public static uint Build(GL gl, string vsSrc, string fsSrc, string name)
    {
        uint vs = CompileStage(gl, ShaderType.VertexShader, vsSrc, name);
        uint fs = CompileStage(gl, ShaderType.FragmentShader, fsSrc, name);
        if (vs == 0 || fs == 0) return 0;

        uint prog = gl.CreateProgram();
        gl.AttachShader(prog, vs);
        gl.AttachShader(prog, fs);
        gl.LinkProgram(prog);
        gl.GetProgram(prog, ProgramPropertyARB.LinkStatus, out int ok);
        if (ok == 0)
        {
            Console.WriteLine($"[GlBackend] link failed ({name}): {gl.GetProgramInfoLog(prog)}");
            gl.DeleteProgram(prog);
            prog = 0;
        }
        gl.DeleteShader(vs);
        gl.DeleteShader(fs);
        return prog;
    }

    static string Ascii(string s)
    {
        var a = s.ToCharArray();
        for (int i = 0; i < a.Length; i++) if (a[i] > 0x7F) a[i] = ' ';
        return new string(a);
    }

    static uint CompileStage(GL gl, ShaderType type, string src, string name)
    {
        uint sh = gl.CreateShader(type);
        gl.ShaderSource(sh, Ascii(src));
        gl.CompileShader(sh);
        gl.GetShader(sh, ShaderParameterName.CompileStatus, out int ok);
        if (ok == 0)
        {
            Console.WriteLine($"[GlBackend] compile failed ({name} {type}) {gl.GetShaderInfoLog(sh)}");
            gl.DeleteShader(sh);
            return 0;
        }
        return sh;
    }
}
