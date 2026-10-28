# Changelog

Notable changes to vkmGL, newest first. Each release is tagged `vX.Y.Z`.

## 1.1.1

A host checks its programs one at a time, so no frame waits on them all.

- **`checkNextShader()`** checks one program the driver is done with: on some drivers asking for
  a link status is where the program is really built, so `checkShaders()` was one long frozen
  frame. `ShaderBase::checked()` says whether a program has been checked, and `countShaders()`
  counts those that have not (`ShaderCount::unchecked`).

## 1.1.0

What a renderer reached past vkmGL for, now in it: parallel shader builds, array and
multisampled textures, fences and readbacks, and the state a frame sets.

- **Parallel shader builds**: `ShaderBase::compileInParallel` hands each program to the
  driver's own threads and checks it at its first use; `ready()` asks without waiting,
  `countShaders()` counts the live shaders and how many are still compiling, for a progress bar,
  and `checkShaders()` checks every one, so a broken program is found before anything binds it.
- **`zeroUniforms`** sets every plain uniform of a program to zero, so a pass sharing it starts
  from nothing another pass left.
- **`Texture2DArray`**: layers of one size and format, sampled whole or through a view in
  another format, bound as a layered image, cleared a level at a time, its levels built from
  the first. `Texture2D` can be multisampled, binds a level as an image and clears a level;
  `TextureCubeArray` copies one cube over another.
- **`Fence`** owns a sync object, and `StreamRing` waits on them; a **`ReadbackBuffer`** is a
  persistently mapped copy the CPU reads once it has landed, never waiting. `GLBuffer` copies,
  clears and reads back by name.
- **`VertexArray`** sets attribute formats of each kind, its buffer bindings and index buffer,
  and draws instanced, multi and indirect.
- **`Context`** caches alpha to coverage, polygon offset, depth clamp, clip planes and a
  per-buffer colour mask, and sets clip control, pixel alignment and seamless cubes;
  `setDefaultState` resets them all. `FrameBuffer` clears its own attachments by name.
- The driver's debug messages log under the category `DRIVER`, and its performance notes below
  high severity at debug rather than as warnings.
- `BufferSpan` and `bindRange` live in `gl_buffer.h`, beside the buffer they name.
- vkm_gl requires only the `glm` and `vkm_log` targets; the consumer makes the context and
  starts glew. Its tests build when the consumer sets `VKM_GL_BUILD_TESTS`.

Fixes: a shader moved into place is in the registry, so hot reload and `countShaders()` find
it, and the moved-from one is not walked; the framebuffer status check goes through
`VKM_GL_CHECK`.

## 1.0.0

The first release with a changelog. vkmGL needs OpenGL 4.5 core and edits every object
through direct state access, so nothing it does disturbs what is bound.

- **Direct state access throughout.** Buffers, textures, framebuffers, render buffers, vertex
  arrays and samplers are made with `glCreate*` and edited by name; uniforms are set with
  `glProgramUniform*`, on a program whether it is bound or not.
- **Immutable texture storage.** A `Texture2D` allocates `Texture2DParams::levels` levels at
  creation (`levelCount`: the whole chain with `generateMipmaps`, else one); `uploadLevel`,
  `uploadCompressedLevel`, `generateMipmaps` and `setSwizzle` fill and shape them, and `setData`
  makes new storage only for a new size, keeping the swizzle. A texture of no size has none and samples black.
- **`StreamRing`**: per-frame data written straight into a persistently mapped buffer, three
  regions fenced in turn, so the CPU never writes what the GPU still reads nor waits on the
  frame it is building. `BufferSpan` and `bindRange` carry where a write went.
- **`DeviceStream`**: per-frame data the GPU reads many times a frame, written through the ring
  and copied once into a buffer in the GPU's own memory; `patch` rewrites a range of it and
  leaves the rest, so data that barely changes is not sent whole again, and `resize` changes
  its length, keeping what it holds.
- `FrameBuffer::attachTexture` and `attachLayer` replace `attachTexture2D` and the cube
  textures' `attachFace`; `FrameBuffer::blit` names its source and target.
- `Context::clear` takes the value depth clears to, for a reversed depth buffer.
- The GLSL version injected when none is set is 450.

Breaking: `Texture2D` and `Texture3D` lose `bind()`/`unbind()` and `bindSlot` its target;
`TextureCube::create` loses its transfer format and type; `Texture3D::download` takes the
destination's size; `VertexArray::setAttributeDivisor` is `setDivisor`, taking the buffer's
start index; `Texture2D::setData` keeps the internal format the texture was made with rather
than inferring one from its pixel format; `Texture2D::setSwizzle` is no longer const;
`FrameBufferDimension` and `FrameBufferAttachment` are gone.

## Before 1.0.0

Tags v0.0.3 to v0.1.0 (August to October 2026) carry no changelog: the OpenGL 4.3 wrappers the engine was
built on - buffers, textures, framebuffers, shaders with a shared prelude and hot reload, debug
output - grown and renumbered with the engine (see its history for what each one changed).
