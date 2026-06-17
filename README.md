```
                    ____           __   __        ___                __
                   / __/________ _/ /__/ /_____ _/ (_)___  ___  ____/ /
                  / /_/ ___/ __ `/ //_/ __/ __ `/ / /_  / / _ \/ __  / 
                 / __/ /  / /_/ / ,< / /_/ /_/ / / / / /_/  __/ /_/ /  
                /_/ /_/   \__,_/_/|_|\__/\__,_/_/_/ /___/\___/\__,_/
                                the open-source, cross-platform fractal explorer
```
an open-source, cross-platform ultra smooth real time fractal explorer
built using a hybrid architecture of Qt6 (QML/C++) and openGL (GLSL 330 core).

the program uses a split-threaded system where a floating sidebar interface
controls a high-performance fragment shader inside a framebuffer object.

---

## technical features:

* **multithreaded fbo rendering:** isolates raw openGL stuff from the main ui system via `QQuickFramebufferObject::Render`.
* **full 64 bit depth:** uses 64 bit `double` systems for maximum zooming depth and accuracy.
* **camera smoothing:** the camera system uses `lerp` to make the camera feel smooth and cinematic
* **procedural gradient colors:** user controlled, real time, gradient color mapping system for beautiful, fully customizable color setup with a slight quadratic brightness boost at the edges.
* **render to file:** you can render any fractal at any location easily with any custom resolution, just position the camera and change your settings to what you want, and hit render. to help prevent your graphics card from exploding, it also renders in 2000x2000px tiles.
* **SSAA anti-aliasing:** utilizes super-sampling anti-aliasing to give super smooth looking visuals. you can choose off, 2x, or 3x ssaa, givinig smoother images at the cost of gpu usage
* **JSON preset system:** you can save any color, position and zoom setup to a JSON file on your computer for later reference, in case you forget. it is also super easy to delete said JSON entries if you make a mistake
* **adjustable resolution scale:** use the resolution scale slider to set your resolution anywhere from 25% to 150% of the active window resolution

---

## available fractals:

fraktalized currently has 7 fractal sets:

* **mandelbrot set:** the classic fractal set
* **julia set:** cool swirly stuff, complete with adjustable constant coords
* **burning ship set:** cool pointy stuff
* **newton set:** kinda spidery looking
* **buddhabrot:** like the mandelbrot but galaxy looking
* **anti-buddhabrot:** reverse buddhabrot
* **barnsley fern:** looks very naturey

### current fractal types:

fraktalized currently has 3 fractal types:

* escape time (the classic)
* trajectory (nebula-looking)
* iterated function system (self-same repeating stuff)

---

## controls:

* **left click + drag:** camera pan
* **scroll wheel:** zoom
* **sidebar controls:** for iterations, color, adjusting julia set constant, rendering to file, and more
* **settings panel:** for resolution scale and anti-aliasing

---

## techy information:

* **language:** C++ 20
* **framework:** Qt 6 (QML, quick)
* **graphics api:** openGL (GLSL 330 core)
* **ide:** JetBrains CLion
* **platform support:** since its Qt, basically all of them (tested on fedora 44)

---

## ai disclosure:

no generative ai or LLMs were used to write or debug any of this code or documentation

---

created by **SwedishSplidney** for Hack Club Stardance 2026