#ifndef FRACTALENGINE_H
#define FRACTALENGINE_H

#include <QQuickFramebufferObject>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QQuickItem>
#include <QVector3D>

// handles the actual GPU rendering work inside an isolated fbo texture canvas
class FractalFBORenderer : public QQuickFramebufferObject::Renderer, protected QOpenGLFunctions_3_3_Core {
public:
    FractalFBORenderer();
    ~FractalFBORenderer() override;

    // fbo rendering hooks
    void render() override;
    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;

    // state synchronization
    void synchronize(QQuickFramebufferObject *item) override;

    void setMaxIterations(int iterations) {m_maxIterations = iterations; }

    void setColorTint(const QVector3D &tint) { m_colorTint = tint; }

private:
    bool m_initialized = false;
    QOpenGLShaderProgram *m_program = nullptr;
    int m_maxIterations = 100;
    QVector3D m_colorTint = QVector3D(0.2f, 0.0f, 0.6f);
    void init();
};

class FractalEngine : public QQuickFramebufferObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int maxIterations READ maxIterations WRITE setMaxIterations NOTIFY maxIterationsChanged)
    Q_PROPERTY(QVector3D colorTint READ colorTint WRITE setColorTint NOTIFY colorTintChanged)

public:
    FractalEngine();

    Renderer *createRenderer() const override;

    // getter and setter for qml stuff
    int maxIterations() const { return m_maxIterations; }
    void setMaxIterations(int val) {
        if (m_maxIterations != val) {
            m_maxIterations = val;
            emit maxIterationsChanged();
            update();
        }
    }

    QVector3D colorTint() const { return m_colorTint; }
    void setColorTint(const QVector3D &val) {
        if (m_colorTint != val) {
            m_colorTint = val;
            emit colorTintChanged();
            update();
        }
    }

signals:
    void maxIterationsChanged();
    void colorTintChanged();

private:
    int m_maxIterations = 100;
    QVector3D m_colorTint = QVector3D(0.2f, 0.0f, 0.6f);
};

#endif