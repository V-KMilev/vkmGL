#pragma once

#include <cstdint>
#include <GL/glew.h>

namespace VkmGL {

/**
 * @brief Base for the things OpenGL identifies by an integer handle.
 *
 * Carries the handle, the object type glObjectLabel needs, and the ownership
 * rules: non-copyable, since a GL handle has a single owner, and movable so a
 * wrapper can be returned by value or stored in a container. A move zeros the
 * source's m_id, which makes the moved-from destructor a no-op.
 *
 * Deliberately declares no bind()/unbind(). Nothing in the codebase holds a
 * GLObject by pointer or reference, so a virtual here dispatched nothing while
 * obliging every subclass to supply the same signature - which four of six
 * could not honour: a program, a VAO and a renderbuffer have no target to
 * select, and a sampler binds to a texture unit rather than a target at all.
 * Each subclass declares the binding call that suits it instead.
 */
class GLObject {
    public:
        GLObject() = delete;

        GLObject(const GLObject& other) = delete;
        GLObject& operator=(const GLObject& other) = delete;

        GLObject(GLObject && other) noexcept;
        GLObject& operator=(GLObject && other) noexcept;

        virtual ~GLObject() = default;

    protected:
        GLObject(
            GLenum target,
            GLenum identifier,
            uint32_t id
        );

    public:
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
        // Declared in constructor-parameter order: initialising out of that
        // order is what the members are read in regardless, and -Wreorder
        // flags the mismatch.
        GLenum   m_target;
        GLenum   m_identifier;
        uint32_t m_id;
};

} // namespace VkmGL
