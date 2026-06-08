#pragma once

#include <QQuickItem>
#include <QOpenGLFunctions>
#include <QQuickWindow>

// renderer handles heavy lifting on graphics thread
class FractalRenderer : public QObject, protected QOpenGLFunctions {
    Q_OBJECT
public:
    FractalRenderer();
    void paint();
    void setWindowSize(int w, int h) {m_width = w; m_height = h; }

private:
    int m_width = 1280;
    int m_height = 720;
    bool m_initialized = false;
};

// custom qml item
class FractalEngine : public QQuickItem {
    Q_OBJECT
    QML_ELEMENT

public:
    FractalEngine();

protected:
    void releaseResources() override;

private slots:
    void handleWindowChanged(QQuickWindow *window);
    void sync();
    void cleanup();

private:
    FractalRenderer *m_renderer = nullptr;
};