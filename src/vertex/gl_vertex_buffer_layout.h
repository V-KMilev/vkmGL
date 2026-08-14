#pragma once

#include <cstdint>
#include <vector>

#include <GL/glew.h>

#include "l_assert.h"

namespace Core {
/**
 * @brief Represents a single vertex attribute element in a vertex buffer layout.
 * 
 * Used to describe how an individual attribute (such as position, normal, color, etc.)
 * is stored in the vertex buffer, including its data type, how many components per vertex,
 * and whether values should be normalized when accessed.
 */
struct VertexBufferElement {
    GLenum type;
    uint32_t count;
    GLboolean normalized;

    /**
     * @brief Helper to get the size in bytes of a given OpenGL type.
     * @param type An OpenGL enum representing the data type.
     * @return Size in bytes of a single element of this type.
     */
    static uint32_t getSizeOfType(GLenum type) {
        switch (type) {
            case GL_FLOAT:         return sizeof(GLfloat);
            case GL_UNSIGNED_INT:  return sizeof(GLuint);
            case GL_INT:           return sizeof(GLint);
            case GL_UNSIGNED_BYTE: return sizeof(GLubyte);
            case GL_BYTE:          return sizeof(GLbyte);
            case GL_SHORT:         return sizeof(GLshort);
            case GL_UNSIGNED_SHORT: return sizeof(GLushort);
            case GL_DOUBLE:        return sizeof(GLdouble);
            default: break;
        }
        // Returning 0 would silently stop the attribute offsets advancing, so
        // every later attribute in the layout would read the same bytes.
        VKM_ASSERT(false);
        return 0;
    }
};

/**
 * @brief Describes the layout of vertex buffer attributes.
 *
 * Holds a list of attribute elements with type/count/normalization and tracks
 * the overall stride for interleaved vertex data. This class is typically used to
 * tell OpenGL how to interpret the data for each vertex in a buffer.
 */
class VertexBufferLayout {
    public:
        VertexBufferLayout();
        ~VertexBufferLayout() = default;

        VertexBufferLayout(const VertexBufferLayout& other) = default;
        VertexBufferLayout& operator=(const VertexBufferLayout& other) = default;

        VertexBufferLayout(VertexBufferLayout && other) noexcept = default;
        VertexBufferLayout& operator=(VertexBufferLayout && other) noexcept = default;

    public:
        /**
         * @brief Adds a new attribute element of type T with the given component count.
         * Template specializations cover supported types.
         * @tparam T Data type of the attribute (float, uint32_t, etc.)
         * @param count Number of components (e.g. 3 for vec3).
         */
        template<typename T>
        void push(uint32_t count);

        /**
         * @brief Returns the stride (total byte size of a single interleaved vertex).
         */
        uint32_t getStride() const;

        /**
         * @brief Returns a vector of all attribute elements in the layout.
         */
        const std::vector<VertexBufferElement>& getElements() const;

        /**
         * @brief Clears all attribute elements and resets stride to zero.
         */
        void clear();

    private:
        uint32_t m_stride;
        std::vector<VertexBufferElement> m_elements;
};

// Template specializations for supported types.
template<> void VertexBufferLayout::push<float>(uint32_t count);
template<> void VertexBufferLayout::push<uint32_t>(uint32_t count);
template<> void VertexBufferLayout::push<int32_t>(uint32_t count);
template<> void VertexBufferLayout::push<uint8_t>(uint32_t count);

} // namespace Core
