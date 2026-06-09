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

    void setTargetZoom(double zoom) {m_targetZoom = zoom; }
    void setTargetCenter(double x, double y) { m_targetCenterX = x; m_targetCenterY = y; }

private:
    bool m_initialized = false;
    QOpenGLShaderProgram *m_program = nullptr;
    int m_maxIterations = 100;
    QVector3D m_colorTint = QVector3D(0.2f, 0.0f, 0.6f);

    double m_currentZoom = 2.0;
    double m_targetZoom = 2.0;
    double m_currentCenterX = -0.5;
    double m_currentCenterY = 0.0;
    double m_targetCenterX = -0.5;
    double m_targetCenterY = 0.0;

    void init();
};

class FractalEngine : public QQuickFramebufferObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int maxIterations READ maxIterations WRITE setMaxIterations NOTIFY maxIterationsChanged)
    Q_PROPERTY(QVector3D colorTint READ colorTint WRITE setColorTint NOTIFY colorTintChanged)

    Q_PROPERTY(float zoomLevel READ zoomLevel WRITE setZoomLevel NOTIFY zoomLevelChanged)
    Q_PROPERTY(QVector2D zoomCenter READ zoomCenter WRITE setZoomCenter NOTIFY zoomCenterChanged)

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

    double zoomLevel() const { return m_zoomLevel; }
    void setZoomLevel(double val) {
        if (m_zoomLevel != val) {
            m_zoomLevel = val;
            emit zoomLevelChanged();
            update();
        }
    }

    QVector2D zoomCenter() const { return QVector2D(static_cast<float>(m_zoomCenterX), static_cast<float>(m_zoomCenterY)); }
    void setZoomCenter(const QVector2D &val) {
        if (m_zoomCenterX != val.x() || m_zoomCenterY != val.y()) {
            m_zoomCenterX = val.x();
            m_zoomCenterY = val.y();
            emit zoomCenterChanged();
            update();
        }
    }

    double zoomCenterX() const { return m_zoomCenterX; }
    double zoomCenterY() const { return m_zoomCenterY; }

    Q_INVOKABLE void panCamera(double dx, double dy, double viewWidth, double viewHeight) {
        // calc aspect ratio
        double aspect = viewWidth / viewHeight;

        // calc delta
        double factorX = (m_zoomLevel * 2.0) / viewWidth;
        double factorY = (m_zoomLevel * 2.0) / viewHeight;

        // update target
        m_zoomCenterX -= (dx * factorX * aspect);
        m_zoomCenterY -= (dy * factorY);

        emit zoomCenterChanged();
        update();
    }

signals:
    void maxIterationsChanged();
    void colorTintChanged();
    void zoomLevelChanged();
    void zoomCenterChanged();

private:
    int m_maxIterations = 100;
    QVector3D m_colorTint = QVector3D(0.2f, 0.0f, 0.6f);

    double m_zoomLevel = 2.0;
    double m_zoomCenterX = -0.5;
    double m_zoomCenterY = 0.0;
};

#endif