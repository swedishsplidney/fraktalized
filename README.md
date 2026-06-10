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

* **multithreaded fbo rendering:** isolates raw openGL stuff from the main ui system via ``` QQuickFramebufferObject::Render ```.
* **hybrid precision camera:** uses a 64-bit ``` double ``` camera tracking system to prevent goofy camera issues when zoomed in, while using 32-bit ``` float ``` at the gpu level
* **camera smoothing:** the camera system uses ``` lerp ``` to make the camera feel smooth and cinematic
* **procedural color:** user controlled, real time, user controlled rgb channel control for tint mapping, utilizing a quadratic brightness boost at the edges.

---

## controls:

* **left click + drag:** camera pan
* **scroll wheel:** zoom
* **sidebar controls for iterations and color**

---

## ai disclosure:

no generative ai or LLMs were used to write any of this stuff

---

coded using C++ 20 / Qt 6 in JetBrains CLion

created by SwedishSplidney for Hack Club Stardance 2026