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

    // isolate state adjustments
    glViewport(0, 0, width, height);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    m_program->bind();

    m_program->setUniformValue("u_resolution", QVector2D(width, height));
    m_program->setUniformValue("u_max_iter", static_cast<float>(m_maxIterations));

    m_program->setUniformValue("u_zoom_level", 0.4f);
    m_program->setUniformValue("u_zoom_center", QVector2D(-1.1f, 0.0f));

    m_program->setUniformValue("u_color_tint", m_colorTint);

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
}