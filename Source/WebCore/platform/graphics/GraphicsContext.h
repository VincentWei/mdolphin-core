/*
 * Copyright (C) 2003, 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
 * Copyright (C) 2008-2009 Torch Mobile, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef GraphicsContext_h
#define GraphicsContext_h

#include "ColorSpace.h"
#include "DashArray.h"
#include "FloatRect.h"
#include "Gradient.h"
#include "Image.h"
#include "Path.h"
#include "Pattern.h"
#include <wtf/Noncopyable.h>
#include <wtf/PassOwnPtr.h>

#if PLATFORM(CG)
typedef struct CGContext PlatformGraphicsContext;
#elif PLATFORM(CAIRO)
namespace WebCore {
class ContextShadow;
class PlatformContextCairo;
}
typedef WebCore::PlatformContextCairo PlatformGraphicsContext;
#elif PLATFORM(OPENVG)
namespace WebCore {
class SurfaceOpenVG;
}
typedef class WebCore::SurfaceOpenVG PlatformGraphicsContext;
#elif PLATFORM(QT)
#include <QPainter>
namespace WebCore {
class ContextShadow;
}
typedef QPainter PlatformGraphicsContext;
#elif PLATFORM(WX)
class wxGCDC;
class wxWindowDC;

// wxGraphicsContext allows us to support Path, etc.
// but on some platforms, e.g. Linux, it requires fairly
// new software.
#if USE(WXGC)
// On OS X, wxGCDC is just a typedef for wxDC, so use wxDC explicitly to make
// the linker happy.
#ifdef __APPLE__
    class wxDC;
    typedef wxDC PlatformGraphicsContext;
#else
    typedef wxGCDC PlatformGraphicsContext;
#endif
#else
    typedef wxWindowDC PlatformGraphicsContext;
#endif
#elif USE(SKIA)
namespace WebCore {
class PlatformContextSkia;
}
typedef WebCore::PlatformContextSkia PlatformGraphicsContext;
#elif PLATFORM(HAIKU)
class BView;
typedef BView PlatformGraphicsContext;
struct pattern;
#elif OS(WINCE)
typedef struct HDC__ PlatformGraphicsContext;
#elif PLATFORM(MG) 
//typedef unsigned int HDC;
typedef GHANDLE HDC;            // gengyue
typedef HDC PlatformGraphicsContext;

#if ENABLE(CAIRO_MG)
namespace WebCore {
class ContextShadow;
class GraphicsContextCairo;
}
#endif
#else
typedef void PlatformGraphicsContext;
#endif

#if PLATFORM(MG) && ENABLE(CAIRO_MG)
#define VIRTUAL virtual
#else
#define VIRTUAL
#endif

#if PLATFORM(WIN)
#include "DIBPixelData.h"
typedef struct HDC__* HDC;
#if !PLATFORM(CG)
// UInt8 is defined in CoreFoundation/CFBase.h
typedef unsigned char UInt8;
#endif
#endif

#if PLATFORM(QT) && defined(Q_WS_WIN)
#include <windows.h>
#endif

namespace WebCore {

#if OS(WINCE) && !PLATFORM(QT)
    class SharedBitmap;
    class SimpleFontData;
    class GlyphBuffer;
#endif

    const int cMisspellingLineThickness = 3;
    const int cMisspellingLinePatternWidth = 4;
    const int cMisspellingLinePatternGapWidth = 1;

    class AffineTransform;
    class DrawingBuffer;
    class Font;
    class Generator;
    class GraphicsContextPlatformPrivate;
    class ImageBuffer;
    class IntRect;
    class RoundedIntRect;
    class KURL;
    class SharedGraphicsContext3D;
    class TextRun;

    enum TextDrawingMode {
        TextModeInvisible = 0,
        TextModeFill      = 1 << 0,
        TextModeStroke    = 1 << 1,
        TextModeClip      = 1 << 2
    };
    typedef unsigned TextDrawingModeFlags;

    enum StrokeStyle {
        NoStroke,
        SolidStroke,
        DottedStroke,
        DashedStroke
    };

    enum InterpolationQuality {
        InterpolationDefault,
        InterpolationNone,
        InterpolationLow,
        InterpolationMedium,
        InterpolationHigh
    };

    struct GraphicsContextState {
        GraphicsContextState()
            : strokeThickness(0)
            , shadowBlur(0)
#if PLATFORM(CAIRO) || (PLATFORM(MG) && ENABLE(CAIRO_MG))
            , globalAlpha(1)
#endif
            , textDrawingMode(TextModeFill)
            , strokeColor(Color::black)
            , fillColor(Color::black)
            , strokeStyle(SolidStroke)
            , fillRule(RULE_NONZERO)
            , strokeColorSpace(ColorSpaceDeviceRGB)
            , fillColorSpace(ColorSpaceDeviceRGB)
            , shadowColorSpace(ColorSpaceDeviceRGB)
            , compositeOperator(CompositeSourceOver)
            , shouldAntialias(true)
            , shouldSmoothFonts(true)
            , paintingDisabled(false)
            , shadowsIgnoreTransforms(false)
#if PLATFORM(CG)
            // Core Graphics incorrectly renders shadows with radius > 8px (<rdar://problem/8103442>),
            // but we need to preserve this buggy behavior for canvas and -webkit-box-shadow.
            , shadowsUseLegacyRadius(false)
#endif
        {
        }

        RefPtr<Gradient> strokeGradient;
        RefPtr<Pattern> strokePattern;
        
        RefPtr<Gradient> fillGradient;
        RefPtr<Pattern> fillPattern;

        FloatSize shadowOffset;

        float strokeThickness;
        float shadowBlur;

#if PLATFORM(CAIRO) || (PLATFORM(MG) && ENABLE(CAIRO_MG))
        float globalAlpha;
#endif
        TextDrawingModeFlags textDrawingMode;

        Color strokeColor;
        Color fillColor;
        Color shadowColor;

        StrokeStyle strokeStyle;
        WindRule fillRule;

        ColorSpace strokeColorSpace;
        ColorSpace fillColorSpace;
        ColorSpace shadowColorSpace;

        CompositeOperator compositeOperator;

        bool shouldAntialias : 1;
        bool shouldSmoothFonts : 1;
        bool paintingDisabled : 1;
        bool shadowsIgnoreTransforms : 1;
#if PLATFORM(CG)
        bool shadowsUseLegacyRadius : 1;
#endif
    };

    class GraphicsContext {
        WTF_MAKE_NONCOPYABLE(GraphicsContext); WTF_MAKE_FAST_ALLOCATED;
    public:
        GraphicsContext(PlatformGraphicsContext*);
        VIRTUAL ~GraphicsContext();

#if !OS(WINCE) || PLATFORM(QT)
#if PLATFORM(MG)
        VIRTUAL PlatformGraphicsContext* platformContext() const;
#else
        PlatformGraphicsContext* platformContext() const;
#endif
#endif

#if PLATFORM(MG)
        VIRTUAL bool isCairoCanvas() {return false;}
#endif

        float strokeThickness() const;
        void setStrokeThickness(float);
        StrokeStyle strokeStyle() const;
        void setStrokeStyle(StrokeStyle);
        Color strokeColor() const;
        ColorSpace strokeColorSpace() const;
        void setStrokeColor(const Color&, ColorSpace);

        void setStrokePattern(PassRefPtr<Pattern>);
        Pattern* strokePattern() const;

        void setStrokeGradient(PassRefPtr<Gradient>);
        Gradient* strokeGradient() const;

        WindRule fillRule() const;
        void setFillRule(WindRule);
        Color fillColor() const;
        ColorSpace fillColorSpace() const;
        void setFillColor(const Color&, ColorSpace);

        void setFillPattern(PassRefPtr<Pattern>);
        Pattern* fillPattern() const;

        void setFillGradient(PassRefPtr<Gradient>);
        Gradient* fillGradient() const;

        void setShadowsIgnoreTransforms(bool);
        bool shadowsIgnoreTransforms() const;

        void setShouldAntialias(bool);
        bool shouldAntialias() const;

        void setShouldSmoothFonts(bool);
        bool shouldSmoothFonts() const;

        const GraphicsContextState& state() const;

#if PLATFORM(CG)
        void applyStrokePattern();
        void applyFillPattern();
        void drawPath(const Path&);
        
        // Allow font smoothing (LCD antialiasing). Not part of the graphics state.
        void setAllowsFontSmoothing(bool);
        
        void setIsCALayerContext(bool);
        bool isCALayerContext() const;

        void setIsAcceleratedContext(bool);
        bool isAcceleratedContext() const;
#endif

        void save();
        void restore();

        // These draw methods will do both stroking and filling.
        // FIXME: ...except drawRect(), which fills properly but always strokes
        // using a 1-pixel stroke inset from the rect borders (of the correct
        // stroke color).
        VIRTUAL void drawRect(const IntRect&);
        VIRTUAL void drawLine(const IntPoint&, const IntPoint&);
        VIRTUAL void drawEllipse(const IntRect&);
        VIRTUAL void drawConvexPolygon(size_t numPoints, const FloatPoint*, bool shouldAntialias = false);

        VIRTUAL void fillPath(const Path&);
        VIRTUAL void strokePath(const Path&);

        // Arc drawing (used by border-radius in CSS) just supports stroking at the moment.
        VIRTUAL void strokeArc(const IntRect&, int startAngle, int angleSpan);

        VIRTUAL void fillRect(const FloatRect&);
        VIRTUAL void fillRect(const FloatRect&, const Color&, ColorSpace);
        void fillRect(const FloatRect&, Generator&);
        VIRTUAL void fillRoundedRect(const IntRect&, const IntSize& topLeft, const IntSize& topRight, const IntSize& bottomLeft, const IntSize& bottomRight, const Color&, ColorSpace);
        void fillRoundedRect(const RoundedIntRect&, const Color&, ColorSpace);
        void fillRectWithRoundedHole(const IntRect&, const RoundedIntRect& roundedHoleRect, const Color&, ColorSpace);

        VIRTUAL void clearRect(const FloatRect&);

        VIRTUAL void strokeRect(const FloatRect&, float lineWidth);

        void drawImage(Image*, ColorSpace styleColorSpace, const IntPoint&, CompositeOperator = CompositeSourceOver);
        void drawImage(Image*, ColorSpace styleColorSpace, const IntRect&, CompositeOperator = CompositeSourceOver, bool useLowQualityScale = false);
        void drawImage(Image*, ColorSpace styleColorSpace, const IntPoint& destPoint, const IntRect& srcRect, CompositeOperator = CompositeSourceOver);
        void drawImage(Image*, ColorSpace styleColorSpace, const IntRect& destRect, const IntRect& srcRect, CompositeOperator = CompositeSourceOver, bool useLowQualityScale = false);
        void drawImage(Image*, ColorSpace styleColorSpace, const FloatRect& destRect, const FloatRect& srcRect = FloatRect(0, 0, -1, -1),
                       CompositeOperator = CompositeSourceOver, bool useLowQualityScale = false);
        void drawTiledImage(Image*, ColorSpace styleColorSpace, const IntRect& destRect, const IntPoint& srcPoint, const IntSize& tileSize,
                       CompositeOperator = CompositeSourceOver, bool useLowQualityScale = false);
        void drawTiledImage(Image*, ColorSpace styleColorSpace, const IntRect& destRect, const IntRect& srcRect,
                            Image::TileRule hRule = Image::StretchTile, Image::TileRule vRule = Image::StretchTile,
                            CompositeOperator = CompositeSourceOver, bool useLowQualityScale = false);

        void drawImageBuffer(ImageBuffer*, ColorSpace styleColorSpace, const IntPoint&, CompositeOperator = CompositeSourceOver);
        void drawImageBuffer(ImageBuffer*, ColorSpace styleColorSpace, const IntRect&, CompositeOperator = CompositeSourceOver, bool useLowQualityScale = false);
        void drawImageBuffer(ImageBuffer*, ColorSpace styleColorSpace, const IntPoint& destPoint, const IntRect& srcRect, CompositeOperator = CompositeSourceOver);
        void drawImageBuffer(ImageBuffer*, ColorSpace styleColorSpace, const IntRect& destRect, const IntRect& srcRect, CompositeOperator = CompositeSourceOver, bool useLowQualityScale = false);
        void drawImageBuffer(ImageBuffer*, ColorSpace styleColorSpace, const FloatRect& destRect, const FloatRect& srcRect = FloatRect(0, 0, -1, -1),
                             CompositeOperator = CompositeSourceOver, bool useLowQualityScale = false);

        VIRTUAL void setImageInterpolationQuality(InterpolationQuality);
        VIRTUAL InterpolationQuality imageInterpolationQuality() const;

        void clip(const IntRect&);
        VIRTUAL void clip(const FloatRect&);
        void addRoundedRectClip(const RoundedIntRect&);
        VIRTUAL void addInnerRoundedRectClip(const IntRect&, int thickness);
        VIRTUAL void clipOut(const IntRect&);
        void clipOutRoundedRect(const RoundedIntRect&);
        VIRTUAL void clipPath(const Path&, WindRule);
        VIRTUAL void clipConvexPolygon(size_t numPoints, const FloatPoint*, bool antialias = true);
        VIRTUAL void clipToImageBuffer(ImageBuffer*, const FloatRect&);
        
        IntRect clipBounds() const;

        TextDrawingModeFlags textDrawingMode() const;
        void setTextDrawingMode(TextDrawingModeFlags);

        void drawText(const Font&, const TextRun&, const FloatPoint&, int from = 0, int to = -1);
        void drawEmphasisMarks(const Font&, const TextRun& , const AtomicString& mark, const FloatPoint&, int from = 0, int to = -1);
        void drawBidiText(const Font&, const TextRun&, const FloatPoint&);
        void drawHighlightForText(const Font&, const TextRun&, const FloatPoint&, int h, const Color& backgroundColor, ColorSpace, int from = 0, int to = -1);

        enum RoundingMode {
            RoundAllSides,
            RoundOriginAndDimensions
        };
        VIRTUAL FloatRect roundToDevicePixels(const FloatRect&, RoundingMode = RoundAllSides);

        VIRTUAL void drawLineForText(const FloatPoint&, float width, bool printing);
        enum TextCheckingLineStyle {
            TextCheckingSpellingLineStyle,
            TextCheckingGrammarLineStyle,
            TextCheckingReplacementLineStyle
        };
        VIRTUAL void drawLineForTextChecking(const FloatPoint&, float width, TextCheckingLineStyle);

        bool paintingDisabled() const;
        void setPaintingDisabled(bool);

        bool updatingControlTints() const;
        void setUpdatingControlTints(bool);

        VIRTUAL void beginTransparencyLayer(float opacity);
        VIRTUAL void endTransparencyLayer();

        bool hasShadow() const;
        void setShadow(const FloatSize&, float blur, const Color&, ColorSpace);
        // Legacy shadow blur radius is used for canvas, and -webkit-box-shadow.
        // It has different treatment of radii > 8px.
        void setLegacyShadow(const FloatSize&, float blur, const Color&, ColorSpace);

        bool getShadow(FloatSize&, float&, Color&, ColorSpace&) const;
        void clearShadow();

        VIRTUAL void drawFocusRing(const Vector<IntRect>&, int width, int offset, const Color&);
        VIRTUAL void drawFocusRing(const Path&, int width, int offset, const Color&);

        VIRTUAL void setLineCap(LineCap);
        VIRTUAL void setLineDash(const DashArray&, float dashOffset);
        VIRTUAL void setLineJoin(LineJoin);
        VIRTUAL void setMiterLimit(float);

        VIRTUAL void setAlpha(float);
#if PLATFORM(MG)
        VIRTUAL float getAlpha(){return 0.0f;}
#endif
#if PLATFORM(CAIRO)
        float getAlpha();
#endif

        void setCompositeOperation(CompositeOperator);
        CompositeOperator compositeOperation() const;

        VIRTUAL void clip(const Path&);

        // This clip function is used only by <canvas> code. It allows
        // implementations to handle clipping on the canvas differently since
        // the discipline is different.
        VIRTUAL void canvasClip(const Path&);
        VIRTUAL void clipOut(const Path&);

        VIRTUAL void scale(const FloatSize&);
        VIRTUAL void rotate(float angleInRadians);
        void translate(const FloatSize& size) { translate(size.width(), size.height()); }
        VIRTUAL void translate(float x, float y);

        VIRTUAL void setURLForRect(const KURL&, const IntRect&);

        VIRTUAL void concatCTM(const AffineTransform&);
        VIRTUAL void setCTM(const AffineTransform&);
        VIRTUAL AffineTransform getCTM() const;

#if OS(WINCE) && !PLATFORM(QT)
        void setBitmap(PassRefPtr<SharedBitmap>);
        const AffineTransform& affineTransform() const;
        AffineTransform& affineTransform();
        void resetAffineTransform();
        void fillRect(const FloatRect&, const Gradient*);
        void drawText(const SimpleFontData* fontData, const GlyphBuffer& glyphBuffer, int from, int numGlyphs, const FloatPoint& point);
        void drawFrameControl(const IntRect& rect, unsigned type, unsigned state);
        void drawFocusRect(const IntRect& rect);
        void paintTextField(const IntRect& rect, unsigned state);
        void drawBitmap(SharedBitmap*, const IntRect& dstRect, const IntRect& srcRect, ColorSpace styleColorSpace, CompositeOperator compositeOp);
        void drawBitmapPattern(SharedBitmap*, const FloatRect& tileRectIn, const AffineTransform& patternTransform, const FloatPoint& phase, ColorSpace styleColorSpace, CompositeOperator op, const FloatRect& destRect, const IntSize& origSourceSize);
        void drawIcon(HICON icon, const IntRect& dstRect, UINT flags);
        HDC getWindowsContext(const IntRect&, bool supportAlphaBlend = false, bool mayCreateBitmap = true); // The passed in rect is used to create a bitmap for compositing inside transparency layers.
        void releaseWindowsContext(HDC, const IntRect&, bool supportAlphaBlend = false, bool mayCreateBitmap = true);    // The passed in HDC should be the one handed back by getWindowsContext.
        void drawRoundCorner(bool newClip, RECT clipRect, RECT rectWin, HDC dc, int width, int height);
#elif PLATFORM(WIN)
        GraphicsContext(HDC, bool hasAlpha = false); // FIXME: To be removed.
        bool inTransparencyLayer() const;
        HDC getWindowsContext(const IntRect&, bool supportAlphaBlend = true, bool mayCreateBitmap = true); // The passed in rect is used to create a bitmap for compositing inside transparency layers.
        void releaseWindowsContext(HDC, const IntRect&, bool supportAlphaBlend = true, bool mayCreateBitmap = true);    // The passed in HDC should be the one handed back by getWindowsContext.

        // When set to true, child windows should be rendered into this context
        // rather than allowing them just to render to the screen. Defaults to
        // false.
        // FIXME: This is a layering violation. GraphicsContext shouldn't know
        // what a "window" is. It would be much more appropriate for this flag
        // to be passed as a parameter alongside the GraphicsContext, but doing
        // that would require lots of changes in cross-platform code that we
        // aren't sure we want to make.
        void setShouldIncludeChildWindows(bool);
        bool shouldIncludeChildWindows() const;

        class WindowsBitmap {
            WTF_MAKE_NONCOPYABLE(WindowsBitmap);
        public:
            WindowsBitmap(HDC, IntSize);
            ~WindowsBitmap();

            HDC hdc() const { return m_hdc; }
            UInt8* buffer() const { return m_pixelData.buffer(); }
            unsigned bufferLength() const { return m_pixelData.bufferLength(); }
            const IntSize& size() const { return m_pixelData.size(); }
            unsigned bytesPerRow() const { return m_pixelData.bytesPerRow(); }
            unsigned short bitsPerPixel() const { return m_pixelData.bitsPerPixel(); }
            const DIBPixelData& windowsDIB() const { return m_pixelData; }

        private:
            HDC m_hdc;
            HBITMAP m_bitmap;
            DIBPixelData m_pixelData;
        };

        WindowsBitmap* createWindowsBitmap(IntSize);
        // The bitmap should be non-premultiplied.
        void drawWindowsBitmap(WindowsBitmap*, const IntPoint&);
#endif

#if (PLATFORM(QT) && defined(Q_WS_WIN)) || (PLATFORM(WX) && OS(WINDOWS))
        HDC getWindowsContext(const IntRect&, bool supportAlphaBlend = true, bool mayCreateBitmap = true);
        void releaseWindowsContext(HDC, const IntRect&, bool supportAlphaBlend = true, bool mayCreateBitmap = true);
        bool shouldIncludeChildWindows() const { return false; }
#endif

#if PLATFORM(WX)
        bool inTransparencyLayer() const { return false; }
#endif

#if PLATFORM(QT)
        bool inTransparencyLayer() const;
        void pushTransparencyLayerInternal(const QRect &rect, qreal opacity, QPixmap& alphaMask);
        void takeOwnershipOfPlatformContext();
#endif

#if PLATFORM(QT) || PLATFORM(CAIRO) || (PLATFORM(MG) && ENABLE(CAIRO_MG))
        ContextShadow* contextShadow();
        GraphicsContextCairo* newCairoContext(GraphicsContext *context);
        void deleteCairoContext(GraphicsContextCairo *cairoContext);
#endif

#if PLATFORM(CAIRO)
        GraphicsContext(cairo_t*);
#endif

#if PLATFORM(GTK)
        void setGdkExposeEvent(GdkEventExpose*);
        GdkWindow* gdkWindow() const;
        GdkEventExpose* gdkExposeEvent() const;
#endif

#if PLATFORM(HAIKU)
        pattern getHaikuStrokeStyle();
#endif

        void setSharedGraphicsContext3D(SharedGraphicsContext3D*, DrawingBuffer*, const IntSize&);
        void syncSoftwareCanvas();
        void markDirtyRect(const IntRect&); // Hints that a portion of the backing store is dirty.

#if PLATFORM(MG) && ENABLE(CAIRO_MG)
    public:
#else
    private:
#endif
        VIRTUAL void platformInit(PlatformGraphicsContext*);
        VIRTUAL void platformDestroy();

#if PLATFORM(WIN) && !OS(WINCE)
        void platformInit(HDC, bool hasAlpha = false);
#endif

        VIRTUAL void savePlatformState();
        VIRTUAL void restorePlatformState();

        void setPlatformTextDrawingMode(TextDrawingModeFlags);
        void setPlatformFont(const Font& font);

        VIRTUAL void setPlatformStrokeColor(const Color&, ColorSpace);
        VIRTUAL void setPlatformStrokeStyle(StrokeStyle);
        VIRTUAL void setPlatformStrokeThickness(float);
        void setPlatformStrokeGradient(Gradient*);
        void setPlatformStrokePattern(Pattern*);

        VIRTUAL void setPlatformFillColor(const Color&, ColorSpace);
        void setPlatformFillGradient(Gradient*);
        void setPlatformFillPattern(Pattern*);

        VIRTUAL void setPlatformShouldAntialias(bool);
        void setPlatformShouldSmoothFonts(bool);

        VIRTUAL void setPlatformShadow(const FloatSize&, float blur, const Color&, ColorSpace);
        VIRTUAL void clearPlatformShadow();

        VIRTUAL void setPlatformCompositeOperation(CompositeOperator);
        
        static void adjustLineToPixelBoundaries(FloatPoint& p1, FloatPoint& p2, float strokeWidth, StrokeStyle);

        GraphicsContextPlatformPrivate* m_data;

        GraphicsContextState m_state;
        Vector<GraphicsContextState> m_stack;
        bool m_updatingControlTints;
    };

} // namespace WebCore

#endif // GraphicsContext_h

