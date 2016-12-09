/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRenderTargetContext_DEFINED
#define GrRenderTargetContext_DEFINED

#include "GrColor.h"
#include "GrContext.h"
#include "GrPaint.h"
#include "GrSurfaceContext.h"
#include "SkRefCnt.h"
#include "SkSurfaceProps.h"
#include "../private/GrInstancedPipelineInfo.h"
#include "../private/GrRenderTargetProxy.h"

class GrClip;
class GrDrawingManager;
class GrDrawOp;
class GrFixedClip;
class GrPipelineBuilder;
class GrRenderTarget;
class GrRenderTargetContextPriv;
class GrRenderTargetOpList;
class GrStyle;
class GrSurface;
class GrTextureProxy;
struct GrUserStencilSettings;
class SkDrawFilter;
struct SkIPoint;
struct SkIRect;
class SkLatticeIter;
class SkMatrix;
class SkPaint;
class SkPath;
struct SkPoint;
struct SkRect;
class SkRegion;
class SkRRect;
struct SkRSXform;
class SkTextBlob;

/**
 * A helper object to orchestrate commands (draws, etc...) for GrSurfaces that are GrRenderTargets.
 */
class SK_API GrRenderTargetContext : public GrSurfaceContext {
public:
    ~GrRenderTargetContext() override;

    bool copySurface(GrSurface* src, const SkIRect& srcRect, const SkIPoint& dstPoint) override;

    // TODO: it is odd that we need both the SkPaint in the following 3 methods.
    // We should extract the text parameters from SkPaint and pass them separately
    // akin to GrStyle (GrTextInfo?)
    virtual void drawText(const GrClip&,  const GrPaint&, const SkPaint&,
                          const SkMatrix& viewMatrix, const char text[], size_t byteLength,
                          SkScalar x, SkScalar y, const SkIRect& clipBounds);
    virtual void drawPosText(const GrClip&, const GrPaint&, const SkPaint&,
                             const SkMatrix& viewMatrix, const char text[], size_t byteLength,
                             const SkScalar pos[], int scalarsPerPosition,
                             const SkPoint& offset, const SkIRect& clipBounds);
    virtual void drawTextBlob(const GrClip&, const SkPaint&,
                              const SkMatrix& viewMatrix, const SkTextBlob*,
                              SkScalar x, SkScalar y,
                              SkDrawFilter*, const SkIRect& clipBounds);

    /**
     * Provides a perfomance hint that the render target's contents are allowed
     * to become undefined.
     */
    void discard();

    /**
     * Clear the entire or rect of the render target, ignoring any clips.
     * @param rect  the rect to clear or the whole thing if rect is NULL.
     * @param color the color to clear to.
     * @param canIgnoreRect allows partial clears to be converted to whole
     *                      clears on platforms for which that is cheap
     */
    void clear(const SkIRect* rect, GrColor color, bool canIgnoreRect);

    /**
     *  Draw everywhere (respecting the clip) with the paint.
     */
    void drawPaint(const GrClip&, const GrPaint&, const SkMatrix& viewMatrix);

    /**
     * Draw the rect using a paint.
     * @param paint        describes how to color pixels.
     * @param GrAA         Controls whether rect is antialiased
     * @param viewMatrix   transformation matrix
     * @param style        The style to apply. Null means fill. Currently path effects are not
     *                     allowed.
     * The rects coords are used to access the paint (through texture matrix)
     */
    void drawRect(const GrClip&,
                  const GrPaint& paint,
                  GrAA,
                  const SkMatrix& viewMatrix,
                  const SkRect&,
                  const GrStyle* style  = nullptr);

    /**
     * Maps a rectangle of shader coordinates to a rectangle and fills that rectangle.
     *
     * @param paint        describes how to color pixels.
     * @param GrAA         Controls whether rect is antialiased
     * @param viewMatrix   transformation matrix which applies to rectToDraw
     * @param rectToDraw   the rectangle to draw
     * @param localRect    the rectangle of shader coordinates applied to rectToDraw
     */
    void fillRectToRect(const GrClip&,
                        const GrPaint& paint,
                        GrAA,
                        const SkMatrix& viewMatrix,
                        const SkRect& rectToDraw,
                        const SkRect& localRect);

    /**
     * Fills a rect with a paint and a localMatrix.
     */
    void fillRectWithLocalMatrix(const GrClip& clip,
                                 const GrPaint& paint,
                                 GrAA,
                                 const SkMatrix& viewMatrix,
                                 const SkRect& rect,
                                 const SkMatrix& localMatrix);

    /**
     * Draw a roundrect using a paint.
     *
     * @param paint       describes how to color pixels.
     * @param GrAA        Controls whether rrect is antialiased.
     * @param viewMatrix  transformation matrix
     * @param rrect       the roundrect to draw
     * @param style       style to apply to the rrect. Currently path effects are not allowed.
     */
    void drawRRect(const GrClip&,
                   const GrPaint&,
                   GrAA,
                   const SkMatrix& viewMatrix,
                   const SkRRect& rrect,
                   const GrStyle& style);

    /**
     * Draw a roundrect using a paint and a shadow shader. This is separate from drawRRect
     * because it uses different underlying geometry and GeometryProcessor
     *
     * @param paint        describes how to color pixels.
     * @param viewMatrix   transformation matrix
     * @param rrect        the roundrect to draw
     * @param blurRadius   amount of shadow blur to apply (in device space)
     * @param style        style to apply to the rrect. Currently path effects are not allowed.
     */
    void drawShadowRRect(const GrClip&,
                         const GrPaint&,
                         const SkMatrix& viewMatrix,
                         const SkRRect& rrect,
                         SkScalar blurRadius,
                         const GrStyle& style);

    /**
     * Shortcut for filling a SkPath consisting of nested rrects using a paint. The result is
     * undefined if outer does not contain inner.
     *
     * @param paint        describes how to color pixels.
     * @param GrAA         Controls whether rrects edges are antialiased
     * @param viewMatrix   transformation matrix
     * @param outer        the outer roundrect
     * @param inner        the inner roundrect
     */
    void drawDRRect(const GrClip&,
                    const GrPaint&,
                    GrAA,
                    const SkMatrix& viewMatrix,
                    const SkRRect& outer,
                    const SkRRect& inner);

    /**
     * Draws a path.
     *
     * @param paint         describes how to color pixels.
     * @param GrAA          Controls whether the path is antialiased.
     * @param viewMatrix    transformation matrix
     * @param path          the path to draw
     * @param style         style to apply to the path.
     */
    void drawPath(const GrClip&,
                  const GrPaint&,
                  GrAA,
                  const SkMatrix& viewMatrix,
                  const SkPath&,
                  const GrStyle& style);

    /**
     * Draws vertices with a paint.
     *
     * @param   paint           describes how to color pixels.
     * @param   viewMatrix      transformation matrix
     * @param   primitiveType   primitives type to draw.
     * @param   vertexCount     number of vertices.
     * @param   positions       array of vertex positions, required.
     * @param   texCoords       optional array of texture coordinates used
     *                          to access the paint.
     * @param   colors          optional array of per-vertex colors, supercedes
     *                          the paint's color field.
     * @param   indices         optional array of indices. If NULL vertices
     *                          are drawn non-indexed.
     * @param   indexCount      if indices is non-null then this is the
     *                          number of indices.
     */
    void drawVertices(const GrClip&,
                      const GrPaint& paint,
                      const SkMatrix& viewMatrix,
                      GrPrimitiveType primitiveType,
                      int vertexCount,
                      const SkPoint positions[],
                      const SkPoint texs[],
                      const GrColor colors[],
                      const uint16_t indices[],
                      int indexCount);

    /**
     * Draws textured sprites from an atlas with a paint. This currently does not support AA for the
     * sprite rectangle edges.
     *
     * @param   paint           describes how to color pixels.
     * @param   viewMatrix      transformation matrix
     * @param   spriteCount     number of sprites.
     * @param   xform           array of compressed transformation data, required.
     * @param   texRect         array of texture rectangles used to access the paint.
     * @param   colors          optional array of per-sprite colors, supercedes
     *                          the paint's color field.
     */
    void drawAtlas(const GrClip&,
                   const GrPaint& paint,
                   const SkMatrix& viewMatrix,
                   int spriteCount,
                   const SkRSXform xform[],
                   const SkRect texRect[],
                   const SkColor colors[]);

    /**
     * Draws a region.
     *
     * @param paint         describes how to color pixels
     * @param viewMatrix    transformation matrix
     * @param aa            should the rects of the region be antialiased.
     * @param region        the region to be drawn
     * @param style         style to apply to the region
     */
    void drawRegion(const GrClip&,
                    const GrPaint& paint,
                    GrAA aa,
                    const SkMatrix& viewMatrix,
                    const SkRegion& region,
                    const GrStyle& style);

    /**
     * Draws an oval.
     *
     * @param paint         describes how to color pixels.
     * @param GrAA          Controls whether the oval is antialiased.
     * @param viewMatrix    transformation matrix
     * @param oval          the bounding rect of the oval.
     * @param style         style to apply to the oval. Currently path effects are not allowed.
     */
    void drawOval(const GrClip&,
                  const GrPaint& paint,
                  GrAA,
                  const SkMatrix& viewMatrix,
                  const SkRect& oval,
                  const GrStyle& style);
    /**
     * Draws a partial arc of an oval.
     *
     * @param paint         describes how to color pixels.
     * @param GrGrAA          Controls whether the arc is antialiased.
     * @param viewMatrix    transformation matrix.
     * @param oval          the bounding rect of the oval.
     * @param startAngle    starting angle in degrees.
     * @param sweepAngle    angle to sweep in degrees. Must be in (-360, 360)
     * @param useCenter     true means that the implied path begins at the oval center, connects as
     *                      a line to the point indicated by the start contains the arc indicated by
     *                      the sweep angle. If false the line beginning at the center point is
     *                      omitted.
     * @param style         style to apply to the oval.
     */
    void drawArc(const GrClip&,
                 const GrPaint& paint,
                 GrAA,
                 const SkMatrix& viewMatrix,
                 const SkRect& oval,
                 SkScalar startAngle,
                 SkScalar sweepAngle,
                 bool useCenter,
                 const GrStyle& style);

    /**
     * Draw the image as a set of rects, specified by |iter|.
     */
    void drawImageLattice(const GrClip&,
                          const GrPaint& paint,
                          const SkMatrix& viewMatrix,
                          int imageWidth,
                          int imageHeight,
                          std::unique_ptr<SkLatticeIter> iter,
                          const SkRect& dst);

    /**
     * After this returns any pending surface IO will be issued to the backend 3D API and
     * if the surface has MSAA it will be resolved.
     */
    void prepareForExternalIO();

    /**
     * Reads a rectangle of pixels from the render target context.
     * @param dstInfo       image info for the destination
     * @param dstBuffer     destination pixels for the read
     * @param dstRowBytes   bytes in a row of 'dstBuffer'
     * @param x             x offset w/in the render target context from which to read
     * @param y             y offset w/in the render target context from which to read
     *
     * @return true if the read succeeded, false if not. The read can fail because of an
     *              unsupported pixel config.
     */
    bool readPixels(const SkImageInfo& dstInfo, void* dstBuffer, size_t dstRowBytes, int x, int y);

    /**
     * Writes a rectangle of pixels [srcInfo, srcBuffer, srcRowbytes] into the 
     * renderTargetContext at the specified position.
     * @param srcInfo       image info for the source pixels
     * @param srcBuffer     source for the write
     * @param srcRowBytes   bytes in a row of 'srcBuffer'
     * @param x             x offset w/in the render target context at which to write
     * @param y             y offset w/in the render target context at which to write
     *
     * @return true if the write succeeded, false if not. The write can fail because of an
     *              unsupported pixel config.
     */
    bool writePixels(const SkImageInfo& srcInfo, const void* srcBuffer, size_t srcRowBytes,
                     int x, int y);

    bool isStencilBufferMultisampled() const {
        return fRenderTargetProxy->isStencilBufferMultisampled();
    }
    bool isUnifiedMultisampled() const { return fRenderTargetProxy->isUnifiedMultisampled(); }
    bool hasMixedSamples() const { return fRenderTargetProxy->isMixedSampled(); }

    const GrCaps* caps() const { return fContext->caps(); }
    const GrSurfaceDesc& desc() const { return fRenderTargetProxy->desc(); }
    int width() const { return fRenderTargetProxy->width(); }
    int height() const { return fRenderTargetProxy->height(); }
    GrPixelConfig config() const { return fRenderTargetProxy->config(); }
    int numColorSamples() const { return fRenderTargetProxy->numColorSamples(); }
    bool isGammaCorrect() const { return SkToBool(fColorSpace.get()); }
    const SkSurfaceProps& surfaceProps() const { return fSurfaceProps; }
    SkColorSpace* getColorSpace() const { return fColorSpace.get(); }
    GrColorSpaceXform* getColorXformFromSRGB() const { return fColorXformFromSRGB.get(); }
    GrSurfaceOrigin origin() const { return fRenderTargetProxy->origin(); }

    bool wasAbandoned() const;

    GrRenderTarget* instantiate();

    GrRenderTarget* accessRenderTarget() {
        // TODO: usage of this entry point needs to be reduced and potentially eliminated
        // since it ends the deferral of the GrRenderTarget's allocation
        return fRenderTargetProxy->instantiate(fContext->textureProvider());
    }

    GrTextureProxy* asDeferredTexture();

    sk_sp<GrTexture> asTexture() {
        if (!this->accessRenderTarget()) {
            return nullptr;
        }

        // TODO: usage of this entry point needs to be reduced and potentially eliminated
        // since it ends the deferral of the GrRenderTarget's allocation
        // It's usage should migrate to asDeferredTexture
        return sk_ref_sp(this->accessRenderTarget()->asTexture());
    }

    // Provides access to functions that aren't part of the public API.
    GrRenderTargetContextPriv priv();
    const GrRenderTargetContextPriv priv() const;

    bool isWrapped_ForTesting() const;

protected:
    GrRenderTargetContext(GrContext*, GrDrawingManager*, sk_sp<GrRenderTargetProxy>,
                          sk_sp<SkColorSpace>, const SkSurfaceProps* surfaceProps, GrAuditTrail*,
                          GrSingleOwner*);

    GrDrawingManager* drawingManager() { return fDrawingManager; }

    SkDEBUGCODE(void validate() const;)

private:
    inline GrAAType decideAAType(GrAA aa, bool allowMixedSamples = false) {
        if (GrAA::kNo == aa) {
            return GrAAType::kNone;
        }
        if (this->isUnifiedMultisampled()) {
            return GrAAType::kMSAA;
        }
        if (allowMixedSamples && this->isStencilBufferMultisampled()) {
            return GrAAType::kMixedSamples;
        }
        return GrAAType::kCoverage;
    }

    friend class GrAtlasTextBlob; // for access to addDrawOp
    friend class GrStencilAndCoverTextContext; // for access to addDrawOp

    friend class GrDrawingManager; // for ctor
    friend class GrRenderTargetContextPriv;
    friend class GrTestTarget;  // for access to getOpList
    friend class GrSWMaskHelper;                 // for access to addDrawOp

    // All the path renderers currently make their own ops
    friend class GrSoftwarePathRenderer;         // for access to addDrawOp
    friend class GrAAConvexPathRenderer;         // for access to addDrawOp
    friend class GrDashLinePathRenderer;         // for access to addDrawOp
    friend class GrAAHairLinePathRenderer;       // for access to addDrawOp
    friend class GrAALinearizingConvexPathRenderer;  // for access to addDrawOp
    friend class GrAADistanceFieldPathRenderer;  // for access to addDrawOp
    friend class GrDefaultPathRenderer;          // for access to addDrawOp
    friend class GrPLSPathRenderer;              // for access to addDrawOp
    friend class GrMSAAPathRenderer;             // for access to addDrawOp
    friend class GrStencilAndCoverPathRenderer;  // for access to addDrawOp
    friend class GrTessellatingPathRenderer;     // for access to addDrawOp

    void internalClear(const GrFixedClip&, const GrColor, bool canIgnoreClip);

    bool drawFilledDRRect(const GrClip& clip,
                          const GrPaint& paint,
                          GrAA,
                          const SkMatrix& viewMatrix,
                          const SkRRect& origOuter,
                          const SkRRect& origInner);

    bool drawFilledRect(const GrClip& clip,
                        const GrPaint& paint,
                        GrAA,
                        const SkMatrix& viewMatrix,
                        const SkRect& rect,
                        const GrUserStencilSettings* ss);

    void drawNonAAFilledRect(const GrClip&,
                             const GrPaint&,
                             const SkMatrix& viewMatrix,
                             const SkRect& rect,
                             const SkRect* localRect,
                             const SkMatrix* localMatrix,
                             const GrUserStencilSettings* ss,
                             GrAAType hwOrNoneAAType);

    void internalDrawPath(const GrClip&,
                          const GrPaint&,
                          GrAA,
                          const SkMatrix&,
                          const SkPath&,
                          const GrStyle&);

    // This entry point allows the GrTextContext-derived classes to add their ops to the GrOpList.
    void addDrawOp(const GrPipelineBuilder&, const GrClip&, GrDrawOp*);

    GrRenderTargetOpList* getOpList();

    GrDrawingManager*                 fDrawingManager;
    sk_sp<GrRenderTargetProxy>        fRenderTargetProxy;

    // In MDB-mode the GrOpList can be closed by some other renderTargetContext that has picked
    // it up. For this reason, the GrOpList should only ever be accessed via 'getOpList'.
    GrRenderTargetOpList*             fOpList;
    GrInstancedPipelineInfo           fInstancedPipelineInfo;

    sk_sp<SkColorSpace>               fColorSpace;
    sk_sp<GrColorSpaceXform>          fColorXformFromSRGB;
    SkSurfaceProps                    fSurfaceProps;
};

#endif
