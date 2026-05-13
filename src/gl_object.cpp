#include "gl_object.h"

#include "gl_error_handle.h"

namespace Core {

GLObject::GLObject(
    GLenum target,
    GLenum identifier,
    uint32_t id
) : m_target(target),
    m_identifier(identifier),
    m_id(id) {}

GLObject::GLObject(GLObject&& other) noexcept
    : m_target(other.m_target),
      m_identifier(other.m_identifier),
      m_id(other.m_id) {
    other.m_id = 0;
}

GLObject& GLObject::operator=(GLObject&& other) noexcept {
    // Note: derived classes must release their GL handle before delegating
    // to this operator — this base move-assignment simply transfers state.
    if (this != &other) {
        m_target     = other.m_target;
        m_identifier = other.m_identifier;
        m_id         = other.m_id;
        other.m_id   = 0;
    }
    return *this;
}

uint32_t GLObject::getID() const { return m_id; }
bool GLObject::isValid() const { return m_id != 0; }

void GLObject::setLabel(const char* label) {
    VKM_GL_CHECK(glObjectLabel(m_identifier, m_id, -1, label));
}
}
