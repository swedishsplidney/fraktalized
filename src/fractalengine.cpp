#include "fractalengine.h"
#include <iostream>
#include <QOpenGLFramebufferObject>

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
    // let qt allocate a color-managed texture surface box automatically
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    return new QOpenGLFramebufferObject(size, format);
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
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    m_program->bind();

    m_program->setUniformValue("u_resolution", QVector2D(width, height));
    m_program->setUniformValue("u_max_iter", static_cast<float>(m_maxIterations));
    m_program->setUniformValue("u_color_tint", m_colorTint);

    m_program->setUniformValue("u_zoom_level", static_cast<float>(m_currentZoom));
    m_program->setUniformValue("u_zoom_center", QVector2D(static_cast<float>(m_currentCenterX), static_cast<float>(m_currentCenterY)));

    m_program->setUniformValue("u_fractal_type", m_fractalType);

    m_program->setUniformValue("u_julia_c", m_juliaC);

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
}

// fractalengine implementation

FractalEngine::FractalEngine() {
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
}