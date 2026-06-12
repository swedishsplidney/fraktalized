#include "fractalengine.h"
#include <iostream>
#include <QOpenGLFramebufferObject>
#include <QPainter>
#include <QQuickWindow>

FractalFBORenderer::FractalFBORenderer() {
    // constructor runs safely on the dedicated graphics render thread context loop
}

FractalFBORenderer::~FractalFBORenderer() {
    delete m_program;
}

void FractalFBORenderer::init() {
    if (m_initialized) return;

    initializeOpenGLFunctions();
    m_program = new QOpenGLShaderProgram();

    bool vLoaded = m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "shaders/fractal.vert");
    bool fLoaded = m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "shaders/fractal.frag");

    std::cout << "shaders -> vertex: " << (vLoaded ? "ok" : "fail")
              << " | fragment: " << (fLoaded ? "ok" : "fail") << std::endl;

    m_program->bindAttributeLocation("aPos", 0);
    m_program->link();

    m_initialized = true;
}

QOpenGLFramebufferObject *FractalFBORenderer::createFramebufferObject(const QSize &size) {
    // scale fbo storage
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::NoAttachment);

    QSize targetSize = m_scaledSize;
    if (targetSize.isEmpty()) {
        targetSize = size;
    }

    return new QOpenGLFramebufferObject(targetSize, format);
}

void FractalFBORenderer::render() {
    if (!m_initialized) init();

    // get the texture surface constraints directly from the active fbo container
    QOpenGLFramebufferObject *fbo = framebufferObject();
    int width = fbo->width();
    int height = fbo->height();

    // smoothing lerp
    double lerpSpeed = 0.12f;
    m_currentZoom += (m_targetZoom - m_currentZoom) * lerpSpeed;
    m_currentCenterX += (m_targetCenterX - m_currentCenterX) * lerpSpeed;
    m_currentCenterY += (m_targetCenterY - m_currentCenterY) * lerpSpeed;

    // thresholds scaled by zoom amount
    double zoomSnapLimit = m_targetZoom * 0.0005f;
    double centerSnapLimit = (m_targetZoom * m_targetZoom) * 0.000005f;

    if (std::abs(m_targetZoom - m_currentZoom) < zoomSnapLimit) {
        m_currentZoom = m_targetZoom;
    }

    if (std::abs(m_targetCenterX - m_currentCenterX) < centerSnapLimit) m_currentCenterX = m_targetCenterX;
    if (std::abs(m_targetCenterY - m_currentCenterY) < centerSnapLimit) m_currentCenterY = m_targetCenterY;

    if (m_currentZoom != m_targetZoom || m_currentCenterX != m_targetCenterX || m_currentCenterY != m_targetCenterY) {
        update();
    }

    // isolate state adjustments
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    m_program->bind();

    float screenAspect = (float)width / (float)height;
    float scaleX = 1.0f;
    float scaleY = 1.0f;

    if (width >= height) {
        // landscape
        scaleX = 1.0f;
        scaleY = 1.0f / screenAspect;
    } else {
        // portrait
        scaleX = screenAspect;
        scaleY = 1.0f;
    }

    m_program->setUniformValue("u_tile_bounds", QVector4D(-scaleX, -scaleY, scaleX, scaleY));

    m_program->setUniformValue("u_resolution", QVector2D(width, height));
    m_program->setUniformValue("u_max_iter", static_cast<float>(m_maxIterations));
    m_program->setUniformValue("u_color_tint", m_colorTint);
    m_program->setUniformValue("u_fractal_type", m_fractalType);

    m_program->setUniformValue("u_aaSamples", m_aaSamples);

    int locZoomLevel = m_program->uniformLocation("u_zoom_level");
    int locZoomCenter = m_program->uniformLocation("u_zoom_center");
    int locJuliaC = m_program->uniformLocation("u_julia_c");

    // pass double values directly to the resolver
    if (locZoomLevel != -1) {
        this->glUniform1d(locZoomLevel, m_currentZoom);
    }
    if (locZoomCenter != -1) {
        this->glUniform2d(locZoomCenter, m_currentCenterX, m_currentCenterY);
    }
    if (locJuliaC != -1) {
        this->glUniform2d(locJuliaC, static_cast<double>(m_juliaC.x()), static_cast<double>(m_juliaC.y()));
    }

    // local coordinate structure passed down to the pipeline unit
    GLfloat rawVertices[] = {
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
    };

    m_program->enableAttributeArray(0);
    m_program->setAttributeArray(0, GL_FLOAT, rawVertices, 3);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    m_program->disableAttributeArray(0);
    m_program->release();

    // render to file engine
    if (m_pendingExport && m_exportWidth > 0 && m_exportHeight > 0) {
        m_pendingExport = false;

        qDebug() << "started tiling render:" << m_exportWidth << "x" << m_exportHeight;

        //create the final master image canvas
        QImage masterImage(m_exportWidth, m_exportHeight, QImage::Format_RGBA8888);
        masterImage.fill(Qt::black);

        // define a tile size
        const int tileSize = 2000;

        // allocate a reusable fbo area for the gpu
        QOpenGLFramebufferObjectFormat tileFormat;
        tileFormat.setAttachment(QOpenGLFramebufferObject::NoAttachment);
        tileFormat.setInternalTextureFormat(GL_RGBA8);
        QOpenGLFramebufferObject tileFbo(tileSize, tileSize, tileFormat);

        if (tileFbo.bind()) {
            m_program->bind();

            // pass layout constraints to the shader
            float highResIterations = m_maxIterations * 1.0f;
            m_program->setUniformValue("u_max_iter", highResIterations);
            m_program->setUniformValue("u_color_tint", m_colorTint);
            m_program->setUniformValue("u_fractal_type", m_fractalType);

            // the shader needs the total image resolution
            m_program->setUniformValue("u_resolution", QVector2D(m_exportWidth, m_exportHeight));

            QPainter painter(&masterImage);

            // loop through the image grid one tile at a time
            for (int y = 0; y < m_exportHeight; y += tileSize) {
                for (int x = 0; x < m_exportWidth; x += tileSize) {

                    int currentTileWidth = std::min(tileSize, m_exportWidth - x);
                    int currentTileHeight = std::min(tileSize, m_exportHeight - y);

                    // resize to tile's size
                    glViewport(0, 0, currentTileWidth, currentTileHeight);
                    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                    glClear(GL_COLOR_BUFFER_BIT);

                    // flip y if needed
                    GLfloat tileVertices[] = {
                        -1.0f, -1.0f, 0.0f,
                        1.0f, -1.0f, 0.0f,
                        -1.0f,  1.0f, 0.0f,

                        -1.0f,  1.0f, 0.0f,
                        1.0f, -1.0f, 0.0f,
                        1.0f,  1.0f, 0.0f,
                    };

                    // calculate render aspect ratio
                    float exportAspect = (float)m_exportWidth / (float)m_exportHeight;
                    float expScaleX = 1.0f;
                    float expScaleY = 1.0f;

                    if (m_exportWidth >= m_exportHeight) {
                        // landscape
                        expScaleX = 1.0f;
                        expScaleY = 1.0f / exportAspect;
                    } else {
                        // portrait
                        expScaleX = exportAspect;
                        expScaleY = 1.0f;
                    }

                    // map the x dimension
                    float xStart = -expScaleX + 2.0f * expScaleX * (float)x / m_exportWidth;
                    float xEnd   = -expScaleX + 2.0f * expScaleX * (float)(x + currentTileWidth) / m_exportWidth;

                    float openGLYTop    = m_exportHeight - y;
                    float openGLYBottom = m_exportHeight - (y + currentTileHeight);

                    float yStart = -expScaleY + 2.0f * expScaleY * (openGLYBottom / m_exportHeight);
                    float yEnd   = -expScaleY + 2.0f * expScaleY * (openGLYTop / m_exportHeight);

                    m_program->setUniformValue("u_tile_bounds", QVector4D(xStart, yStart, xEnd, yEnd));

                    // re-bind double uniform camera stuff
                    int expZoomLevel = m_program->uniformLocation("u_zoom_level");
                    int expZoomCenter = m_program->uniformLocation("u_zoom_center");
                    int expJuliaC = m_program->uniformLocation("u_julia_c");

                    if (expZoomLevel != -1)  this->glUniform1d(expZoomLevel, m_targetZoom);
                    if (expZoomCenter != -1) this->glUniform2d(expZoomCenter, m_targetCenterX, m_targetCenterY);
                    if (expJuliaC != -1)     this->glUniform2d(expJuliaC, static_cast<double>(m_juliaC.x()), static_cast<double>(m_juliaC.y()));

                    // render the tile
                    m_program->enableAttributeArray(0);
                    m_program->setAttributeArray(0, GL_FLOAT, tileVertices, 3);
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                    m_program->disableAttributeArray(0);

                    // flush commands
                    glFlush();

                    int openGLYSource = tileSize - currentTileHeight;
                    QImage tileImage = tileFbo.toImage().copy(0, openGLYSource, currentTileWidth, currentTileHeight);

                    // mirror vertically
                    tileImage = tileImage.flipped(Qt::Vertical);

                    int canvasYDestination = m_exportHeight - y - currentTileHeight;

                    // stitch tile into master image
                    painter.drawImage(x, canvasYDestination, tileImage);
                }
            }

            painter.end();

            m_program->release();
            tileFbo.release();

            // save the master image
            if (masterImage.save(m_exportFilename)) {
                qDebug() << "successfully rendered:" << m_exportFilename;
            } else {
                qWarning() << "failed to render image";
            }

            // restore viewport
            glViewport(0, 0, width, height);
        }
    }
}

// fractalengine implementation

FractalEngine::FractalEngine() : m_viewportScale(1.0f) {
    // class construction properties managed implicitly by the framework
}

QQuickFramebufferObject::Renderer *FractalEngine::createRenderer() const {
    return new FractalFBORenderer();
}

void FractalFBORenderer::synchronize(QQuickFramebufferObject *item) {
    auto *engine = static_cast<FractalEngine *>(item);
    this->setMaxIterations(engine->maxIterations());
    this->setColorTint(engine->colorTint());

    this->setTargetZoom(engine->zoomLevel());
    this->setTargetCenter(engine->zoomCenterX(), engine->zoomCenterY());

    this->setFractalType(engine->fractalType());

    this->setJuliaC(engine->juliaC());

    this->m_aaSamples = engine->aaSamples();

    // calc scaled viewport size
    QSize baseSize = engine->window() ? engine->window()->size() : QSize(1920, 1080);
    QSize newScaledSize = QSize(baseSize.width() * engine->viewportScale(), baseSize.height() * engine->viewportScale());

    if (this->m_scaledSize != newScaledSize) {
        this->m_scaledSize = newScaledSize;

        this->invalidateFramebufferObject();
    }

    if (engine->hasPendingExport()) {
        this->m_pendingExport = true;
        this->m_exportFilename = engine->exportFilename();
        this->m_exportWidth = engine->exportWidth();
        this->m_exportHeight = engine->exportHeight();
        engine->clearExportFlag();
    }
}