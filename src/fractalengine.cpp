#include "fractalengine.h"
#include <QQuickWindow>

FractalRenderer::FractalRenderer() {
    // constructor
}

void FractalRenderer::paint() {
    // raw opengl code for fractals
    if (!m_initialized) {
        initializeOpenGLFunctions();
        m_initialized = true;
    }

    // clear viewport
    glViewport(0, 0, m_width, m_height);
    glClearColor(0.2f, 0.0f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

FractalEngine::FractalEngine() {
    // tell qt to trigger sync slot every frame
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
        // force new window to connect paint function to frame cycle
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
    // safe destruction
}