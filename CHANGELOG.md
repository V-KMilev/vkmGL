# Changelog

Notable changes to vkmGL, newest first. Each release is tagged `vX.Y.Z`.

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
