#include "engine/GlGuard.hpp"

namespace {

void RestoreFlag(GLenum option, GLboolean shouldEnable) {
    if (shouldEnable) {
        glEnable(option);
    } else {
        glDisable(option);
    }
}

} // namespace

namespace engine::gl {

GlGuardAux::GlGuardAux() {
    GLCALL(glGetIntegerv(GL_ACTIVE_TEXTURE, reinterpret_cast<GLint*>(&activeTexture_)));
    GLCALL(glGetIntegerv(GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>(&program_)));
    // NOTE: probably a bad idea to store/restore glDrawBuffers, as it's state of framebuffer
    // for (size_t i = 0; i < sizeof(drawBuffers) / sizeof(drawBuffers[0]); ++i) {
    //     GLCALL(glGetIntegerv(GL_DRAW_BUFFER0 + i, reinterpret_cast<GLint*>(drawBuffers + i)));
    // }
    GLCALL(glGetIntegerv(GL_DISPATCH_INDIRECT_BUFFER_BINDING, reinterpret_cast<GLint*>(&dispatchIndirectBuffer_)));
    GLCALL(glGetIntegerv(GL_DRAW_INDIRECT_BUFFER_BINDING, reinterpret_cast<GLint*>(&drawIndirectBuffer_)));
    GLCALL(glGetIntegerv(GL_PROGRAM_PIPELINE_BINDING, reinterpret_cast<GLint*>(&programPipeline_)));

    GLCALL(glGetIntegerv(GL_TEXTURE_BUFFER_BINDING, reinterpret_cast<GLint*>(&textureBuffer_)));
    GLCALL(glGetIntegerv(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING, reinterpret_cast<GLint*>(&transformFeedbackBuffer_)));
}

GlGuardAux::~GlGuardAux() {
    GLCALL(glActiveTexture(activeTexture_));

    GLCALL(glUseProgram(program_));
    GLCALL(glBindProgramPipeline(programPipeline_));
    GLCALL(glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, dispatchIndirectBuffer_));
    GLCALL(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawIndirectBuffer_));
    GLCALL(glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, transformFeedbackBuffer_));
    GLCALL(glBindBuffer(GL_TEXTURE_BUFFER, textureBuffer_));
    // GLCALL(glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer));

    // GLCALL(glDrawBuffers(sizeof(drawBuffers) / sizeof(drawBuffers[0]), drawBuffers));
    // XLOG("~GlGuardAux", 0);
}

GlGuardFramebuffer::GlGuardFramebuffer(bool restoreRare)
    : restoreRare_(restoreRare) {
    GLCALL(glGetIntegerv(GL_RENDERBUFFER_BINDING, reinterpret_cast<GLint*>(&renderBuffer_)));
    GLCALL(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, reinterpret_cast<GLint*>(&drawFramebuffer_)));
    if (restoreRare_) {
        GLCALL(glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, reinterpret_cast<GLint*>(&readFramebuffer_)));
        GLCALL(glGetBooleanv(GL_FRAMEBUFFER_SRGB, &framebufferSrgb_));
        GLCALL(glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, reinterpret_cast<GLint*>(&pixelPackBuffer_)));
        GLCALL(glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, reinterpret_cast<GLint*>(&pixelUnpackBuffer_)));
    }
}

GlGuardFramebuffer::~GlGuardFramebuffer() {
    GLCALL(glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer_));
    GLCALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFramebuffer_));
    if (restoreRare_) {
        GLCALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, readFramebuffer_));
        RestoreFlag(GL_FRAMEBUFFER_SRGB, framebufferSrgb_);
        GLCALL(glBindBuffer(GL_PIXEL_PACK_BUFFER, pixelPackBuffer_));
        GLCALL(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixelUnpackBuffer_));
    }
}

GlGuardVertex::GlGuardVertex(bool restoreRare)
    : restoreRare_(restoreRare) {
    GLCALL(glGetIntegerv(GL_VERTEX_ARRAY_BINDING, reinterpret_cast<GLint*>(&vao_)));
    if (restoreRare_) {
        // NOTE: vbo and ebo are rare to restore, because they should be just once bound to vao
        GLCALL(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, reinterpret_cast<GLint*>(&vbo_)));
        GLCALL(glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, reinterpret_cast<GLint*>(&ebo_)));
        GLCALL(glGetBooleanv(GL_PRIMITIVE_RESTART, &primitiveRestart_));
        GLCALL(glGetIntegerv(GL_PRIMITIVE_RESTART_INDEX, reinterpret_cast<GLint*>(&primitiveRestartIndex_)));
        GLCALL(glGetBooleanv(GL_CULL_FACE, &cullFace_));
        GLCALL(glGetIntegerv(GL_CULL_FACE_MODE, reinterpret_cast<GLint*>(&cullFaceMode_)));
        GLCALL(glGetIntegerv(GL_PROVOKING_VERTEX, reinterpret_cast<GLint*>(&provokingVertex_)));
    }
}

GlGuardVertex::~GlGuardVertex() {
    // TODO: maybe glBindVertexArray(0) and after binding VBO/EBO bind VAO
    GLCALL(glBindVertexArray(vao_));
    if (restoreRare_) {
        GLCALL(glBindBuffer(GL_ARRAY_BUFFER, vbo_));
        GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_));
        RestoreFlag(GL_PRIMITIVE_RESTART, primitiveRestart_);
        GLCALL(glPrimitiveRestartIndex(primitiveRestartIndex_));
        RestoreFlag(GL_CULL_FACE, cullFace_);
        GLCALL(glCullFace(cullFaceMode_));
        GLCALL(glProvokingVertex(provokingVertex_));
    }
    // XLOG("~GlGuardVertex", 0);
}

GlGuardFlags::GlGuardFlags() {
    GLCALL(glGetBooleanv(GL_BLEND, &blend_));
    GLCALL(glGetBooleanv(GL_DEPTH_TEST, &depthTest_));
    GLCALL(glGetBooleanv(GL_MULTISAMPLE, &multisample_));
    GLCALL(glGetBooleanv(GL_STENCIL_TEST, &stencilTest_));
}

GlGuardFlags::~GlGuardFlags() {
    RestoreFlag(GL_BLEND, blend_);
    RestoreFlag(GL_DEPTH_TEST, depthTest_);
    RestoreFlag(GL_MULTISAMPLE, multisample_);
    RestoreFlag(GL_STENCIL_TEST, stencilTest_);
    // XLOG("~GlGuardFlags", 0);
}

GlGuardDepth::GlGuardDepth(bool restoreRare)
    : restoreRare_(restoreRare) {
    GLCALL(glGetBooleanv(GL_DEPTH_TEST, &depthTest_));
    GLCALL(glGetFloatv(GL_DEPTH_CLEAR_VALUE, &depthClearValue_));
    GLCALL(glGetIntegerv(GL_DEPTH_FUNC, reinterpret_cast<GLint*>(&depthFunc_)));
    GLCALL(glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWriteMask_));

    if (restoreRare_) {
        GLCALL(glGetBooleanv(GL_DEPTH_CLAMP, &depthClamp_));
        GLCALL(glGetFloatv(GL_DEPTH_RANGE, depthRange_));
        GLCALL(glGetBooleanv(GL_POLYGON_OFFSET_FILL, &polygonOffsetFill_));
        GLCALL(glGetBooleanv(GL_POLYGON_OFFSET_LINE, &polygonOffsetLine_));
        GLCALL(glGetBooleanv(GL_POLYGON_OFFSET_POINT, &polygonOffsetPoint_));
        GLCALL(glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &polygonOffsetFactor_));
        GLCALL(glGetFloatv(GL_POLYGON_OFFSET_UNITS, &polygonOffsetUnits_));
    }
}

GlGuardDepth::~GlGuardDepth() {
    RestoreFlag(GL_DEPTH_TEST, depthTest_);
    GLCALL(glClearDepth(depthClearValue_));
    GLCALL(glDepthFunc(depthFunc_));
    GLCALL(glDepthMask(depthWriteMask_));

    if (restoreRare_) {
        RestoreFlag(GL_DEPTH_CLAMP, depthClamp_);
        GLCALL(glDepthRange(depthRange_[0], depthRange_[1]));
        RestoreFlag(GL_POLYGON_OFFSET_FILL, polygonOffsetFill_);
        RestoreFlag(GL_POLYGON_OFFSET_LINE, polygonOffsetLine_);
        RestoreFlag(GL_POLYGON_OFFSET_POINT, polygonOffsetPoint_);
        GLCALL(glPolygonOffset(polygonOffsetFactor_, polygonOffsetUnits_));
    }
    // XLOG("~GlGuardDepth", 0);
}

GlGuardStencil::GlGuardStencil() {
    GLCALL(glGetBooleanv(GL_DEPTH_TEST, &stencilTest_));
    GLCALL(glGetIntegerv(GL_DEPTH_CLEAR_VALUE, &stencilClearValue_));

    GLCALL(glGetIntegerv(GL_STENCIL_BACK_FAIL, reinterpret_cast<GLint*>(&stencilBackFail_)));
    GLCALL(glGetIntegerv(GL_STENCIL_BACK_FUNC, reinterpret_cast<GLint*>(&stencilBackFunc_)));
    GLCALL(glGetIntegerv(GL_STENCIL_BACK_PASS_DEPTH_FAIL, reinterpret_cast<GLint*>(&stencilBackPassDepthFail_)));
    GLCALL(glGetIntegerv(GL_STENCIL_BACK_PASS_DEPTH_PASS, reinterpret_cast<GLint*>(&stencilBackPassDepthPass_)));
    GLCALL(glGetIntegerv(GL_STENCIL_BACK_REF, reinterpret_cast<GLint*>(&stencilBackRef_)));
    GLCALL(glGetIntegerv(GL_STENCIL_BACK_VALUE_MASK, reinterpret_cast<GLint*>(&stencilBackValueMask_)));
    GLCALL(glGetIntegerv(GL_STENCIL_BACK_WRITEMASK, reinterpret_cast<GLint*>(&stencilBackWriteMask_)));

    GLCALL(glGetIntegerv(GL_STENCIL_FAIL, reinterpret_cast<GLint*>(&stencilFrontFail_)));
    GLCALL(glGetIntegerv(GL_STENCIL_FUNC, reinterpret_cast<GLint*>(&stencilFrontFunc_)));
    GLCALL(glGetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL, reinterpret_cast<GLint*>(&stencilFrontPassDepthFail_)));
    GLCALL(glGetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, reinterpret_cast<GLint*>(&stencilFrontPassDepthPass_)));
    GLCALL(glGetIntegerv(GL_STENCIL_REF, reinterpret_cast<GLint*>(&stencilFrontRef_)));
    GLCALL(glGetIntegerv(GL_STENCIL_VALUE_MASK, reinterpret_cast<GLint*>(&stencilFrontValueMask_)));
    GLCALL(glGetIntegerv(GL_STENCIL_WRITEMASK, reinterpret_cast<GLint*>(&stencilFrontWriteMask_)));
}

GlGuardStencil::~GlGuardStencil() {
    RestoreFlag(GL_DEPTH_TEST, stencilTest_);
    GLCALL(glClearStencil(stencilClearValue_));
    GLCALL(glStencilOpSeparate(GL_BACK, stencilBackFail_, stencilBackPassDepthFail_, stencilBackPassDepthPass_));
    GLCALL(glStencilFuncSeparate(GL_BACK, stencilBackFunc_, stencilBackRef_, stencilBackValueMask_));
    GLCALL(glStencilMaskSeparate(GL_BACK, stencilBackWriteMask_));
    GLCALL(glStencilOpSeparate(GL_FRONT, stencilFrontFail_, stencilFrontPassDepthFail_, stencilFrontPassDepthPass_));
    GLCALL(glStencilFuncSeparate(GL_FRONT, stencilFrontFunc_, stencilFrontRef_, stencilFrontValueMask_));
    GLCALL(glStencilMaskSeparate(GL_FRONT, stencilFrontWriteMask_));
    // XLOG("~GlGuardStencil", 0);
}

GlGuardBlend::GlGuardBlend(bool restoreRare)
    : restoreRare_(restoreRare) {
    GLCALL(glGetIntegerv(GL_BLEND_DST_ALPHA, reinterpret_cast<GLint*>(&blendDstAlpha_)));
    GLCALL(glGetIntegerv(GL_BLEND_DST_RGB, reinterpret_cast<GLint*>(&blendDstRgb_)));
    GLCALL(glGetIntegerv(GL_BLEND_SRC_ALPHA, reinterpret_cast<GLint*>(&blendSrcAlpha_)));
    GLCALL(glGetIntegerv(GL_BLEND_SRC_RGB, reinterpret_cast<GLint*>(&blendSrcRgb_)));
    GLCALL(glGetIntegerv(GL_BLEND_EQUATION_ALPHA, reinterpret_cast<GLint*>(&blendEquationAlpha_)));
    GLCALL(glGetIntegerv(GL_BLEND_EQUATION_RGB, reinterpret_cast<GLint*>(&blendEquationRgb_)));
    if (restoreRare_) {
        GLCALL(glGetFloatv(GL_BLEND_COLOR, blendColor_));
        GLCALL(glGetBooleanv(GL_COLOR_LOGIC_OP, &colorLogicOp_));
        GLCALL(glGetIntegerv(GL_LOGIC_OP_MODE, reinterpret_cast<GLint*>(&colorLogicOpMode_)));
    }
}

GlGuardBlend::~GlGuardBlend() {
    GLCALL(glBlendFuncSeparate(blendSrcRgb_, blendDstRgb_, blendSrcAlpha_, blendDstAlpha_));
    GLCALL(glBlendEquationSeparate(blendEquationRgb_, blendEquationAlpha_));
    if (restoreRare_) {
        GLCALL(glBlendColor(blendColor_[0], blendColor_[1], blendColor_[2], blendColor_[3]));
        RestoreFlag(GL_COLOR_LOGIC_OP, colorLogicOp_);
        GLCALL(glLogicOp(colorLogicOpMode_));
    }
    // XLOG("~GlGuardBlend", 0);
}

GlGuardViewport::GlGuardViewport(bool restoreRare)
    : restoreRare_(restoreRare) {
    GLCALL(glGetInteger64v(GL_VIEWPORT, viewport_));
    if (restoreRare_) {
        GLCALL(glGetBooleanv(GL_SCISSOR_TEST, &scissorTest_));
        GLCALL(glGetInteger64v(GL_SCISSOR_BOX, scissor_));
    }
}

GlGuardViewport::~GlGuardViewport() {
    GLCALL(glViewport(viewport_[0], viewport_[1], viewport_[2], viewport_[3]));
    if (restoreRare_) {
        RestoreFlag(GL_SCISSOR_TEST, scissorTest_);
        GLCALL(glScissor(scissor_[0], scissor_[1], scissor_[2], scissor_[3]));
    }
    // XLOG("~GlGuardViewport", 0);
}

GlGuardColor::GlGuardColor() {
    GLCALL(glGetFloatv(GL_COLOR_CLEAR_VALUE, colorClearValue_));
    GLCALL(glGetBooleanv(GL_COLOR_WRITEMASK, colorWriteMask_));
}

GlGuardColor::~GlGuardColor() {
    GLCALL(glClearColor(colorClearValue_[0], colorClearValue_[1], colorClearValue_[2], colorClearValue_[3]));
    GLCALL(glColorMask(colorWriteMask_[0], colorWriteMask_[1], colorWriteMask_[2], colorWriteMask_[3]));

    // XLOG("~GlGuardColor", 0);
}

} // namespace engine::gl