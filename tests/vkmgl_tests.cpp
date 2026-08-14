// vkmGL's test suite. Deliberately covers only what runs without a GL context:
// vertex layout arithmetic and the shader preprocessor. Anything needing a live
// context belongs in a rendering test, not here - this binary must stay runnable
// on a build machine with no GPU.

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

#include "logger.h"

#include "gl_vertex_buffer_layout.h"
#include "gl_shader_preprocess.h"

namespace fs = std::filesystem;

namespace {

int g_failures = 0;

void check(const char* what, bool ok) {
    if (!ok) ++g_failures;
    std::printf("  %-62s %s\n", what, ok ? "ok" : "<-- FAILED");
}

void write(const fs::path& p, const std::string& text) {
    std::ofstream out(p);
    out << text;
}

void testVertexBufferLayout() {
    std::printf("VertexBufferLayout:\n");

    Core::VertexBufferLayout layout;
    check("a fresh layout has zero stride", layout.getStride() == 0);
    check("a fresh layout has no elements", layout.getElements().empty());

    layout.push<float>(3);
    check("vec3 of float is 12 bytes of stride", layout.getStride() == 12);

    layout.push<float>(2);
    check("stride accumulates across pushes", layout.getStride() == 20);

    layout.push<uint8_t>(4);
    check("byte attributes add one byte each", layout.getStride() == 24);
    check("three elements recorded", layout.getElements().size() == 3);

    // uint8_t is the one type pushed as normalized - 0..255 reads as 0..1.
    check("uint8 pushes normalized", layout.getElements()[2].normalized == GL_TRUE);
    check("float pushes unnormalized", layout.getElements()[0].normalized == GL_FALSE);

    layout.clear();
    check("clear() resets the stride", layout.getStride() == 0);
    check("clear() drops the elements", layout.getElements().empty());

    // A layout describes vertex data and owns no GL handle, so it has to be
    // copyable and movable - it was deleted on both until 1.2.
    Core::VertexBufferLayout source;
    source.push<float>(3);
    Core::VertexBufferLayout copied = source;
    check("a layout can be copied", copied.getStride() == 12 && copied.getElements().size() == 1);
    check("the copy is independent", (copied.push<float>(1), source.getStride() == 12));

    Core::VertexBufferLayout moved = std::move(copied);
    check("a layout can be moved", moved.getStride() == 16);

    std::printf("VertexBufferElement::getSizeOfType:\n");
    using E = Core::VertexBufferElement;
    check("GL_FLOAT is 4",          E::getSizeOfType(GL_FLOAT) == 4);
    check("GL_UNSIGNED_INT is 4",   E::getSizeOfType(GL_UNSIGNED_INT) == 4);
    check("GL_UNSIGNED_BYTE is 1",  E::getSizeOfType(GL_UNSIGNED_BYTE) == 1);
    check("GL_SHORT is 2",          E::getSizeOfType(GL_SHORT) == 2);
    check("GL_DOUBLE is 8",         E::getSizeOfType(GL_DOUBLE) == 8);
}

void testShaderPreprocessor(const fs::path& dir) {
    std::printf("Shader preprocessor:\n");

    Core::setShaderVersion(430);

    write(dir / "plain.glsl", "void main() {}\n");
    const std::string plain = Core::preprocessShaderSource((dir / "plain.glsl").string());
    check("the #version is prepended", plain.rfind("#version 430 core", 0) == 0);
    check("a stage with no include is otherwise verbatim",
          plain.find("void main() {}") != std::string::npos);

    Core::setShaderVersion(460);
    const std::string bumped = Core::preprocessShaderSource((dir / "plain.glsl").string());
    check("setShaderVersion changes what is injected",
          bumped.rfind("#version 460 core", 0) == 0);
    Core::setShaderVersion(430);

    write(dir / "helper.glsl", "float helper() { return 1.0; }\n");
    write(dir / "uses.glsl",   "#include \"helper.glsl\"\nvoid main() {}\n");
    const std::string included = Core::preprocessShaderSource((dir / "uses.glsl").string());
    check("an included body is inlined",
          included.find("float helper()") != std::string::npos);
    check("the directive survives as a comment",
          included.find("// #include \"helper.glsl\"") != std::string::npos);

    // Includes resolve relative to the including file, not the process cwd.
    fs::create_directories(dir / "sub");
    write(dir / "sub" / "leaf.glsl",  "int leaf;\n");
    write(dir / "sub" / "inner.glsl", "#include \"leaf.glsl\"\n");
    write(dir / "outer.glsl",         "#include \"sub/inner.glsl\"\n");
    const std::string nested = Core::preprocessShaderSource((dir / "outer.glsl").string());
    check("includes nest, each relative to its own directory",
          nested.find("int leaf;") != std::string::npos);

    // The cycle guard: a file including itself must terminate, not recurse.
    write(dir / "cycle.glsl", "#include \"cycle.glsl\"\nint after;\n");
    const std::string cycle = Core::preprocessShaderSource((dir / "cycle.glsl").string());
    check("a self-include is skipped rather than looping",
          cycle.find("skipped duplicate include") != std::string::npos);
    check("parsing continues past the cycle",
          cycle.find("int after;") != std::string::npos);

    // A directive inside a comment is not a directive.
    write(dir / "commented.glsl", "// #include \"helper.glsl\"\nint only;\n");
    const std::string commented = Core::preprocessShaderSource((dir / "commented.glsl").string());
    check("a commented-out include is not resolved",
          commented.find("float helper()") == std::string::npos);

    const std::string missing = Core::preprocessShaderSource((dir / "nope.glsl").string());
    check("a missing file yields a marker instead of throwing",
          missing.find("failed to open") != std::string::npos);
}

} // namespace

int main() {
    // The preprocessor logs a warning for the deliberately-missing file below,
    // so the logger has to exist. ERROR level keeps the expected noise out of
    // the test output.
    const fs::path logPath = fs::temp_directory_path() / "vkmgl_tests.log";
    Logger::init(logPath.string(), "VKMGL-TESTS", LogLevel::ERROR);

    const fs::path dir = fs::temp_directory_path() / "vkmgl_tests";
    fs::remove_all(dir);
    fs::create_directories(dir);

    testVertexBufferLayout();
    testShaderPreprocessor(dir);

    fs::remove_all(dir);

    std::printf(g_failures ? "\n%d FAILURE(S)\n" : "\nALL OK\n", g_failures);
    return g_failures ? 1 : 0;
}
