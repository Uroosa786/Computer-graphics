# Aasiya Qadri 40263011
#Uroosa Lakhani 40227274
# Ashraqat Mansour 40276570

# Solar System with Shooting Stars – OpenGL Application

This application demonstrates a basic 3D solar system simulation using OpenGL. The scene features a textured Earth and Moon orbiting a glowing Sun, with randomly generated dynamic shooting stars. The camera system allows full navigation using both the keyboard and mouse.

---

## Controls

### Keyboard Controls

| Key | Action               |
| --- | -------------------- |
| W   | Move camera forward  |
| S   | Move camera backward |
| A   | Move camera left     |
| D   | Move camera right    |
| Q   | Move camera downward |
| E   | Move camera upward   |
| ESC | Exit the application |

### Mouse Controls

* Move the mouse to rotate the camera view (look around).
* Horizontal rotation (yaw) is clamped between 60° and 120°.
* Vertical rotation (pitch) is clamped between -25° and 25° to maintain a stable viewing range.

---

## Features

* **Free-Fly Camera**: Mouse and keyboard-based camera system for 3D scene exploration.
* **Textured Bodies**: Earth and Moon are rendered with texture maps.
* **Orbital Animation**:

  * Earth orbits the Sun.
  * Moon orbits the Earth.
* **Shooting Stars**:

  * Stars spawn periodically and move across the scene with a trailing effect.
  * Each star has a glowing head and a fading tail.
  * Shooting stars are removed automatically after their lifespan expires.

---

## Technical Overview

* **Graphics and Input**: Implemented using OpenGL, GLFW, and GLEW.
* **Math Library**: GLM is used for matrix operations and vector math.
* **Texture Loading**: Images are loaded using `stb_image.h`.
* **Shader Program**:

  * **Vertex Shader**: Transforms object coordinates using a Model-View-Projection (MVP) matrix and passes vertex data.
  * **Fragment Shader**: Handles color blending and texture sampling.
* **Mesh Generation**:

  * Sphere mesh generated at runtime for the Sun, Earth, and Moon.
  * Geometry is rendered using triangle strips for efficient drawing.
* **Dynamic Star System**:

  * Each shooting star maintains position, velocity, and a short trail buffer.
  * Trails fade based on lifespan and are rendered as point primitives with alpha blending.

---

## File Structure and Dependencies

### Required Files

* `main.cpp`: Main application source code.
* `earth.jpg`: Texture map for Earth.
* `moon.jpg`: Texture map for Moon.
* `stb_image.h`: Header-only image loader (STB library, included via macro).

### Required Libraries

* OpenGL 3.3+
* GLFW
* GLEW
* GLM

---

## Build Instructions

Ensure that all required dependencies are installed and properly linked. Then compile and run the application as follows.

### Linux/macOS

```bash
g++ main.cpp -o SolarApp -lglfw -lGL -lGLEW -lm
./SolarApp
```

### Windows (using MinGW or Visual Studio)

Link the following libraries: `glew32`, `glfw3`, `opengl32`. Ensure that any required DLLs (e.g., `glew32.dll`, `glfw3.dll`) are available in the runtime path.



