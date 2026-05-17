#pragma once

#include <GL/glew.h>

namespace Core {

/**
 * @brief Blit the colour buffer from one framebuffer to another.
 *
 * Encapsulates the read/draw bind + glBlitFramebuffer + restore triplet so
 * passes do not issue it raw. A multisample source is resolved when the
 * destination is single-sample. Header-only (no vkmGL source-list change).
 *
 * @param srcFbo  Source FBO id (0 = default framebuffer).
 * @param dstFbo  Destination FBO id (0 = default framebuffer).
 * @param srcW/H  Source rectangle size.
 * @param dstW/H  Destination rectangle size.
 * @param filter  GL_NEAREST (required if sizes differ / MSAA resolve) or GL_LINEAR.
 */
inline void blitColor(GLuint srcFbo, GLuint dstFbo,
                      int srcW, int srcH, int dstW, int dstH,
                      GLenum filter = GL_NEAREST) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFbo);
    glBlitFramebuffer(0, 0, srcW, srcH, 0, 0, dstW, dstH,
        GL_COLOR_BUFFER_BIT, filter);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

} // namespace Core
