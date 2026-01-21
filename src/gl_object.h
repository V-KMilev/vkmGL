#pragma once

#include <cstdint>
#include <GL/glew.h>

namespace Core {

/**
 * @brief Abstract base class representing a generic OpenGL object.
 *
 * GLObject serves as the foundational interface for OpenGL objects that are
 * identified by an integer handle. It ensures objects provide basic operations
 * such as binding, unbinding, querying their OpenGL ID, and setting debug labels.
 * Copy and move construction are explicitly disabled to prevent accidental
 * resource duplication or invalidation.
 */
class GLObject {
    public:
        GLObject() = delete;

        GLObject(const GLObject& other) = delete;
        GLObject& operator=(const GLObject& other) = delete;

        GLObject(GLObject && other) = delete;
        GLObject& operator=(GLObject && other) = delete;

        virtual ~GLObject() = default;

        protected:
            GLObject(
                GLenum target,
                GLenum identifier,
                uint32_t id
            );

    public:
        /**
         * @brief Bind this OpenGL object to its target.
         * @param target Optional override for the target; default is GL_NONE.
         */
        virtual void bind(GLenum target = GL_NONE) const = 0;

        /**
         * @brief Unbind this OpenGL object from its target.
         * @param target Optional override for the target; default is GL_NONE.
         */
        virtual void unbind(GLenum target = GL_NONE) const = 0;

        /**
         * @brief Get the OpenGL id/name for this object.
         * @return The GLuint identifier for this GL object.
         */
        uint32_t getID() const;

        /**
         * @brief Check whether the GL object holds a valid OpenGL id.
         * @return True if the object is valid, false otherwise.
         */
        bool isValid() const;

        /**
         * @brief Set a debug label for this OpenGL object, if supported.
         * @param label Null-terminated C-string to set as the label.
         */
        void setLabel(const char* label);

        protected:
            uint32_t m_id;

            GLenum m_target;
            GLenum m_identifier;
    };
};
