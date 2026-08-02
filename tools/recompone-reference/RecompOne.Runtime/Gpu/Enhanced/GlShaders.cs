using Silk.NET.OpenGL;

namespace RecompOne.Runtime.Enhanced;

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
        layout(location = 10) in vec3 inViewPosition;
        layout(location = 11) in vec3 inProjection;
        layout(location = 12) in float inHasViewSpace;
        layout(location = 13) in int inMaterial;

        out vec4 vColorPerspective;
        noperspective out vec4 vColorAffine;
        out vec2 vUVPerspective;
        noperspective out vec2 vUVAffine;
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
        flat out int   vVehicle;
        flat out int   vMaterial;
        flat out ivec4 vUvBounds;
        noperspective out vec3 vBary;
        out float vDepth;

        uniform vec2 uVertexOffset;
        uniform vec2 uPosBias;
        uniform vec2 uFbInv;

        void main() {
            bool modernGeometry = inHasViewSpace > 0.5;
            float viewZ = inViewPosition.z;
            float safeViewZ =
                abs(viewZ) < 0.0001
                    ? (viewZ < 0.0 ? -0.0001 : 0.0001)
                    : viewZ;
            vec2 projectedPosition = modernGeometry
                ? inProjection.xy +
                    inViewPosition.xy *
                    (inProjection.z / safeViewZ)
                : inPos;
            vec2 p =
                (projectedPosition + uVertexOffset + uPosBias) *
                uFbInv - 1.0;
            float w = modernGeometry
                ? viewZ
                : max(inPerspectiveW, 1.0);
            // Window depth must use the same reciprocal projection as XY.
            // Feeding linear camera Z directly as NDC depth makes OpenGL
            // interpolate large terrain triangles affinely in screen space;
            // their interiors then move in front of nearby vehicles even
            // though the triangle vertices are farther away.  Build a normal
            // perspective depth projection from the camera-space value
            // carried by the renderer seam instead.
            const float depthNear = 1.0;
            const float depthFar = 65535.0;
            float cameraDepth =
                clamp(inRasterDepth, depthNear / depthFar, 1.0) * depthFar;
            float depthA =
                (depthFar + depthNear) / (depthFar - depthNear);
            float depthB =
                (-2.0 * depthFar * depthNear) /
                (depthFar - depthNear);
            float ndcDepth = depthA + depthB / cameraDepth;
            float clipZ = ndcDepth * w;
            gl_Position = vec4(p * w, clipZ, w);

            vec4 unpackedColor = vec4(
                float(inColor & 0xFFu),
                float((inColor >> 8) & 0xFFu),
                float((inColor >> 16) & 0xFFu), 0.0) / 255.0;
            vColorPerspective = unpackedColor;
            vColorAffine = unpackedColor;
            vDither = (inTexpage >> 10) & 1;
            vSmooth = (inTexpage >> 11) & 1;
            vUiTexture = (inTexpage >> 12) & 1;
            vParticle = (inTexpage >> 13) & 1;
            vShadow = (inTexpage >> 14) & 1;
            vRadar = (inTexpage >> 16) & 1;
            vHudPlate = (inTexpage >> 17) & 1;
            vHealthPlate = (inTexpage >> 18) & 1;
            vVehicle = (inTexpage >> 19) & 1;
            vMaterial = inMaterial;
            vLongestEdge = inClut;
            vUvBounds = ivec4(round(inUvBounds));
            vBary = inBary;
            vDepth = inDepth;

            if ((inTexpage & 0x8000) != 0) {
                texMode = 4;
            } else {
                texMode = (inTexpage >> 7) & 3;
                vUVPerspective = inUV;
                vUVAffine = inUV;
                pageBase = ivec2((inTexpage & 0xf) * 64, ((inTexpage >> 4) & 1) * 256);
                clutBase = ivec2((inClut & 0x3f) * 16, (inClut >> 6) & 0x1ff);
            }
        }
        """;

    public const string PrimFs = """
        #version 330 core
        in vec4 vColorPerspective;
        noperspective in vec4 vColorAffine;
        in vec2 vUVPerspective;
        noperspective in vec2 vUVAffine;
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
        flat in int   vVehicle;
        flat in int   vMaterial;
        flat in ivec4 vUvBounds;
        noperspective in vec3 vBary;
        in float vDepth;

        layout(location = 0, index = 0) out vec4 FragColor;
        layout(location = 0, index = 1) out vec4 BlendColor;

        uniform sampler2D uVram;
        uniform sampler2D uDest;
        uniform sampler2D uHudSvg;
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
        uniform int   uPerspectiveCorrectTextures;
        uniform int   uPerspectiveCorrectColors;
        uniform int   uTrueColor;
        uniform int   uVectorFonts;
        uniform int   uVectorIcons;
        uniform int   uStockPaintCorrection;
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
            // Resolve the indexed PS1 page before filtering.  Enhanced keeps
            // continuous sub-texel coordinates and clamps each primitive to
            // its authored UV bounds, producing a clean virtual 512-class
            // source without palette-index filtering or atlas bleed.
            vec2 p = uvf - vec2(0.5);
            ivec2 uv0 = ivec2(floor(p));
            vec2 f = fract(p);
            ivec2 boundMin = vUvBounds.xy;
            ivec2 boundMax = max(vUvBounds.zw, boundMin);
            vec4 s00 = textureTexel(clamp(uv0, boundMin, boundMax));
            vec4 s10 = textureTexel(clamp(
                uv0 + ivec2(1, 0), boundMin, boundMax));
            vec4 s01 = textureTexel(clamp(
                uv0 + ivec2(0, 1), boundMin, boundMax));
            vec4 s11 = textureTexel(clamp(
                uv0 + ivec2(1, 1), boundMin, boundMax));
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
        vec4 svgHudTexture(vec2 p) {
            // The three source documents are rasterized at 8x into a 1024
            // square atlas. Geometry and rings originate in the SVGs; this
            // shader only maps the current HUD rectangle into its atlas tile.
            vec2 origin = vRadar != 0
                ? vec2(0.0, 0.0)
                : (vHealthPlate != 0
                    ? vec2(672.0, 440.0)
                    : vec2(0.0, 440.0));
            vec2 atlasPixel = origin + p * 8.0;
            return texture(uHudSvg, (atlasPixel + vec2(0.5)) / 1024.0);
        }
        vec3 stockPaintCorrection(vec3 rgb) {
            if (vVehicle == 0 || uStockPaintCorrection == 0) return rgb;
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
        vec3 distanceFog(vec3 rgb) {
            if (uEnhancedFog == 0 ||
                vUiTexture != 0 ||
                vShadow != 0 ||
                vDepth <= 1.0) {
                return rgb;
            }

            // Fade every world material using camera/OT depth after texture
            // modulation. The previous textured-only pre-modulation haze was
            // undone by bright vertex colours and skipped meshes without an
            // exact GTE SZ, leaving distant buildings fully saturated.
            float amount = smoothstep(2600.0, 8500.0, vDepth) * 0.76;
            float lum = dot(rgb, vec3(0.299, 0.587, 0.114));
            // Converge bright props and dark terrain toward the same bounded
            // atmospheric range. Keeping the source luminance unbounded made
            // white/red distant buildings remain conspicuous against already
            // fogged terrain even though their saturation had been reduced.
            float atmosphericLum = clamp(mix(lum, 0.58, 0.70), 0.48, 0.68);
            float warmth = clamp((rgb.r - rgb.b) * 0.25 + 0.10, 0.0, 0.22);
            vec3 cool = vec3(0.98, 1.00, 1.04) * atmosphericLum;
            vec3 warm = vec3(1.06, 1.00, 0.90) * atmosphericLum;
            vec3 atmosphere = mix(cool, warm, warmth);
            return clamp(mix(rgb, atmosphere, amount), 0.0, 1.0);
        }
        vec3 stockPaintCorrection8(ivec3 c8) {
            return stockPaintCorrection(vec3(c8) / 255.0);
        }
        vec3 quant5(ivec3 c8) {
            if (uTrueColor != 0)
                return vec3(clamp(c8, 0, 255)) / 255.0;
            if (vDither != 0) {
                ivec2 vp = ivec2(floor(gl_FragCoord.xy / float(uScale) - uPosBias));
                c8 = clamp(c8 + ditherTbl[(vp.y & 3) * 4 + (vp.x & 3)], 0, 255);
            }
            return vec3(min(c8 >> 3, 31)) / 31.0;
        }
        void main() {
            if (uCheckMask != 0 && texelFetch(uDest, ivec2(gl_FragCoord.xy), 0).a >= 0.5) discard;

            if (texMode == 4) {
                vec4 vertexColor = uPerspectiveCorrectColors != 0
                    ? vColorPerspective
                    : vColorAffine;
                vec3 corrected = distanceFog(
                    stockPaintCorrection(vertexColor.rgb));
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

            vec2 sampleUV = uPerspectiveCorrectTextures != 0
                ? vUVPerspective
                : vUVAffine;
            vec4 vertexColor = uPerspectiveCorrectColors != 0
                ? vColorPerspective
                : vColorAffine;
            int rawU = dFdx(sampleUV.x) < 0.0 ? int(ceil(sampleUV.x - 0.0001)) : int(floor(sampleUV.x + 0.0001));
            int rawV = dFdy(sampleUV.y) < 0.0 ? int(ceil(sampleUV.y - 0.0001)) : int(floor(sampleUV.y + 0.0001));
            ivec2 nearestUv = ivec2(rawU, rawV);
            if (vUiTexture == 0) {
                ivec2 boundMin = vUvBounds.xy;
                ivec2 boundMax = max(vUvBounds.zw, boundMin);
                nearestUv = clamp(nearestUv, boundMin, boundMax);
            }
            vec4 nearestTexel = textureTexel(nearestUv);
            vec4 texel = uTextureSmoothing != 0 && vSmooth != 0
                ? filteredTexture(sampleUV, nearestTexel)
                : nearestTexel;
            bool vehicleGlass = vMaterial == 3;
            bool vectorFont =
                vUiTexture != 0 && vParticle != 0 && uVectorFonts != 0;
            bool vectorIcon =
                vUiTexture != 0 && vShadow != 0 && uVectorIcons != 0;
            bool enhancedParticle =
                vUiTexture == 0 && vParticle != 0 && uEnhancedParticles != 0;
            bool svgHud = vHudPlate != 0 && uVectorIcons != 0;
            vec2 hudLocal = sampleUV - vec2(vUvBounds.xy);
            float hudCoverage = 1.0;
            float contourCoverage = 1.0;
            float contourStp = nearestTexel.a;

            if ((vectorFont || vectorIcon || enhancedParticle) && !svgHud) {
                texel = contourTexture(sampleUV, contourCoverage, contourStp);
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

            if (transparentBlack(nearestTexel) && !vehicleGlass) {
                // UI transparency is binary in the original packets. Do not
                // synthesize coverage outside glyph/icon silhouettes: opaque
                // UI draws do not have a usable alpha blend and doing so
                // creates dark halos. Particle sprites are semitransparent and
                // can safely reconstruct their edge coverage.
                bool filteredEdge = vectorFont || vectorIcon || enhancedParticle;
                if (!svgHud && !filteredEdge) discard;
            }
            if (svgHud) {
                vec4 vectorTexel = svgHudTexture(hudLocal);
                hudCoverage = vectorTexel.a;
                if (hudCoverage <= 0.001) discard;
                vectorTexel.rgb /= max(hudCoverage, 0.001);
                // Every nontransparent backing texel in the retail CLUT has
                // STP set and uses blend mode 0. Preserve that material blend;
                // only the authored SVG supplies color and edge coverage.
                texel = vec4(vectorTexel.rgb, 1.0);
            }
            texel.rgb = stockPaintCorrection(texel.rgb);
            if (vehicleGlass) {
                // Imported windows use PS1 zero/STP texels as an instruction
                // to blend the body polygon underneath.  In a modern material
                // pass those texels are actual glass: retain the authored
                // tint when present and provide a neutral blue-grey tint for
                // fully clear samples instead of cutting a hole through the
                // vehicle.
                float authored = max(
                    texel.r,
                    max(texel.g, texel.b));
                vec3 glassTint = authored > 0.02
                    ? texel.rgb
                    : vec3(0.18, 0.25, 0.29);
                FragColor = vec4(
                    distanceFog(glassTint),
                    authored > 0.02 ? 0.46 : 0.34);
                BlendColor = vec4(1.0);
                return;
            }
            ivec3 t8 = ivec3(texel.rgb * 31.0 + 0.5) << 3;
            ivec3 c8 = (t8 * ivec3(vertexColor.rgb * 255.0 + 0.5)) >> 7;
            vec3 corrected = distanceFog(stockPaintCorrection8(c8));
            FragColor = vec4(
                quant5(ivec3(corrected * 255.0 + 0.5)),
                max(texel.a, uSetMask));
            if (svgHud) {
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
