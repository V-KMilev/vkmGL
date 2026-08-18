#define VKM_LOG_SUFFIX "VKM-GL"
#define VKM_LOG_CATEGORY "SHADER"

#include "gl_shader_preprocess.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_set>

#include "logger.h"

namespace fs = std::filesystem;

namespace Vkm::GL {

namespace {

// Mutable process-wide state behind an accessor, matching how gl_shader_reload
// holds its registry: a function-local static initialises on first use and
// needs no ordering guarantee against other translation units.
int& glslVersion() {
    static int version = DEFAULT_GLSL_VERSION;
    return version;
}

// Resolve `#include "relative/path"` directives, inlining referenced files
// relative to each including file's directory. Cycle-safe via `visited`; the
// directive is preserved as a comment so compile errors in the inlined body
// stay navigable.
std::string resolveIncludes(const fs::path& filePath, std::unordered_set<std::string>& visited) {
    std::error_code ec;
    const fs::path canonical = fs::weakly_canonical(filePath, ec);
    const std::string key = ec ? filePath.string() : canonical.string();
    if (!visited.insert(key).second) {
        return "// (skipped duplicate include: " + filePath.string() + ")\n";
    }

    std::ifstream in(filePath);
    if (!in) {
        LOG_ERROR("Shader preprocessor: cannot open '%s'", filePath.string().c_str());
        return "// (failed to open: " + filePath.string() + ")\n";
    }

    const fs::path basedir = filePath.parent_path();
    std::ostringstream out;
    std::string line;
    while (std::getline(in, line)) {
        // Only a leading `#include` token is a directive (not one in a comment).
        size_t firstNonWS = 0;
        while (firstNonWS < line.size() &&
               (line[firstNonWS] == ' ' || line[firstNonWS] == '\t')) {
            ++firstNonWS;
        }
        if (line.compare(firstNonWS, 8, "#include") == 0) {
            const size_t q1 = line.find('"', firstNonWS + 8);
            const size_t q2 = q1 != std::string::npos ? line.find('"', q1 + 1) : std::string::npos;
            if (q1 != std::string::npos && q2 != std::string::npos && q2 > q1) {
                out << "// " << line << "\n";
                out << resolveIncludes(basedir / line.substr(q1 + 1, q2 - q1 - 1), visited);
                continue;
            }
            LOG_WARNING("Shader preprocessor: malformed #include in '%s': %s",
                filePath.string().c_str(), line.c_str());
        }
        out << line << "\n";
    }
    return out.str();
}

} // namespace

void setShaderVersion(int glslVersion_) {
    glslVersion() = glslVersion_;
}

std::string preprocessShaderSource(const std::string& filePath) {
    std::unordered_set<std::string> visited;
    // #version must be the first line, and #included files never carry one.
    return "#version " + std::to_string(glslVersion()) + " core\n"
         + resolveIncludes(fs::path(filePath), visited);
}

} // namespace Vkm::GL
