# Atra Engine

A small game engine written in C99 using OpenGL 3.3 and SDL2 as a foundation. This project was part of a personal learning exercise in understanding how video games work at a low level. I had not used a game engine before making this project, and had no concept of good modern game engine design. As such, this engine is a bit of a mess, and would be entirely inappropriate for use in an actual product.

The most recent feature being developed before development stopped was reworking the engine architecture to be based around ECS, in order to allow for better cache-line use, and easier parallelism and behavioural decoupling.

## Features
- Forward and deferred rendering utilities
- glTF model and animation loading
- GPU skeletal animation skinning
- Flexible (but rather clunky) custom GUI library
- State stack for function scheduling and state preservation across states.
- Easy-to-use mesh generation interface for OpenGL.
