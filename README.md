# vkmGL

OpenGL 4.5 objects and state for a renderer that makes no `gl*` call of its own.
Every GL object - program, texture, buffer, vertex array, framebuffer, fence - is
a class here, edited by name through direct state access; pipeline state goes
through `Context`'s cache; every call is under `VKM_GL_CHECK`.

## Requirements

CMake 3.25 or above, a C++17 compiler and an OpenGL 4.5 driver.

## Use

Add it as a submodule and `add_subdirectory` it. It links OpenGL, glew, glm and
`vkm_log`, and vendors only glew: the consuming project adds the `glm` and
`vkm_log` targets first, as it uses them itself, and the build stops with the
reason when one is missing. It needs no window library: the consumer makes a
context current and initialises glew before the first call.

```
git submodule add https://github.com/V-KMilev/vkmGL.git modules/vkmGL
```

Headers are included flat: `#include "gl_texture.h"`.

## What it holds

| Folder    | What                                                                                   |
|-----------|----------------------------------------------------------------------------------------|
| `core`    | `Context` (state cache, limits, barriers), `GLObject`, `VKM_GL_CHECK`, debug output     |
| `buffer`  | `GLBuffer` and its vertex, index, uniform and storage kinds; `StreamRing`, `Fence`, `ReadbackBuffer` |
| `vertex`  | `VertexArray` (attribute formats, bindings, draws), `VertexBufferLayout`                |
| `texture` | `Texture2D`, `Texture2DArray`, `Texture3D`, `TextureCube`, `TextureCubeArray`, `MipChainTexture`, `Sampler` |
| `target`  | `FrameBuffer`, `RenderBuffer`                                                           |
| `shader`  | `Shader`, `ComputeShader`, the source preprocessor, parallel compiling, hot reload      |

## Tests

`vkm_gl_tests` covers what runs without a context - vertex layouts and the
preprocessor - so it runs on a machine with no GPU. What needs a context is
tested by the renderer that consumes the library.
