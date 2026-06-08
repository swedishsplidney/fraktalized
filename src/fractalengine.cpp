#include "fractalengine.h"
#include <QQuickWindow>
#include <QDebug>

FractalRenderer::FractalRenderer() {
    // constructor
}

FractalRenderer::~FractalRenderer() {
    // cleanup gpu resources
    m_vao.destroy();
    m_vbo.destroy();
    delete m_program;
}

void FractalRenderer::initShaders() {
    m_program = new QOpenGLShaderProgram();

    // compile shaders directly
    if (!m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "shaders/fractal.vert")) {
        qWarning() << "fragment shader error: " << m_program->log();
    }

    if (!m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "shaders/fractal.frag")) {
        qWarning() << "fragment shader error: " << m_program->log();
    }

    // link them together for the GPU
    if (!m_program->link()) {
        qWarning() << "shader program linking error: " << m_program->log();
    }
}

void FractalRenderer::initGeometry() {
    // corner coords for two triangles that cover a modern clip space screen
    GLfloat vertices[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
    };

    m_vao.create();
    m_vao.bind();

    m_vbo.create();
    m_vbo.bind();
    m_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_vbo.allocate(vertices, sizeof(vertices));

    // tell gpu that attribute 0 means xyz coords
    m_program->enableAttributeArray(0);
    m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));

    m_vao.release();
    m_vbo.release();
}

void FractalRenderer::paint() {
    if (!m_initialized) {
        initializeOpenGLFunctions();
        initShaders();
        initGeometry();
        m_initialized = true;
    }

    glViewport(0, 0, m_width, m_height);
    glClear(GL_COLOR_BUFFER_BIT);

    // activate shader program
    m_program->bind();
    m_vao.bind();

    glDrawArrays(GL_TRIANGLES, 0, 6); // draw 6 vertices

    m_vao.release();
    m_program->release();
}

// wrapper functions

FractalEngine::FractalEngine() {
    connect(this, &QQuickItem::windowChanged, this, &FractalEngine::handleWindowChanged);
}

void FractalEngine::handleWindowChanged(QQuickWindow *window) {
    if (window) {
        connect(window, &QQuickWindow::beforeSynchronizing, this, &FractalEngine::sync, Qt::DirectConnection);
        connect(window, &QQuickWindow::sceneGraphInvalidated, this, &FractalEngine::cleanup, Qt::DirectConnection);
    }
}

void FractalEngine::sync() {
    if (!m_renderer) {
        m_renderer = new FractalRenderer();
        connect(window(), &QQuickWindow::beforeRenderPassRecording, m_renderer, &FractalRenderer::paint, Qt::DirectConnection);
    }
    m_renderer->setWindowSize(window()->width() * window()->devicePixelRatio(),
        window()->height() * window()->devicePixelRatio());
}

void FractalEngine::cleanup() {
    if (m_renderer) {
        delete m_renderer;
        m_renderer = nullptr;
    }
}

void FractalEngine::releaseResources() {
    // safe destruction placeholder
}