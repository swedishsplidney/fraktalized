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

    m_program->bindAttributeLocation("aPos", 0);

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

    QQuickWindow *window = qobject_cast<QQuickWindow*>(sender());

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glViewport(0, 0, viewport[2], viewport[3]);

    if (window) window->beginExternalCommands();

    glClearColor(0.1f, 0.0f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    // activate shader program
    m_program->bind();
    m_vao.bind();

    glDrawArrays(GL_TRIANGLES, 0, 6); // draw 6 vertices

    m_vao.release();
    m_program->release();

    if (window) window->endExternalCommands();
}

// wrapper functions

FractalEngine::FractalEngine() {
    // notify qt that this item renders raw content
    setFlag(ItemHasContents, true);

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

        window()->setColor(QColor(Qt::transparent));

        connect(window(), &QQuickWindow::afterRendering, m_renderer, &FractalRenderer::paint, Qt::DirectConnection);
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