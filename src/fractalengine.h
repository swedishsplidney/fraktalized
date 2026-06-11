#ifndef FRACTALENGINE_H
#define FRACTALENGINE_H

#include <QQuickFramebufferObject>
#include <QOpenGLFunctions_4_0_Core>
#include <QOpenGLShaderProgram>
#include <QQuickItem>
#include <QVector3D>
#include <QOpenGLFramebufferObjectFormat>
#include <QImage>
#include <QDateTime>
#include <QDebug>

// handles the actual GPU rendering work inside an isolated fbo texture canvas
class FractalFBORenderer : public QQuickFramebufferObject::Renderer, protected QOpenGLFunctions_4_0_Core {
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

    void setFractalType(int type) { m_fractalType = type; }

    void setJuliaC(const QVector2D &c) { m_juliaC = c; }

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

    int m_fractalType = 0;

    QVector2D m_juliaC = QVector2D(-0.7f, 0.27015f);

    bool m_pendingExport = false;
    QString m_exportFilename;
    int m_exportWidth = 0;
    int m_exportHeight = 0;

    void init();
};

class FractalEngine : public QQuickFramebufferObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int maxIterations READ maxIterations WRITE setMaxIterations NOTIFY maxIterationsChanged)
    Q_PROPERTY(QVector3D colorTint READ colorTint WRITE setColorTint NOTIFY colorTintChanged)

    Q_PROPERTY(float zoomLevel READ zoomLevel WRITE setZoomLevel NOTIFY zoomLevelChanged)
    Q_PROPERTY(QVector2D zoomCenter READ zoomCenter WRITE setZoomCenter NOTIFY zoomCenterChanged)

    Q_PROPERTY(int fractalType READ fractalType WRITE setFractalType NOTIFY fractalTypeChanged)
    Q_PROPERTY(QVector2D juliaC READ juliaC WRITE setJuliaC NOTIFY juliaCChanged)

public:
    FractalEngine();

    Renderer *createRenderer() const override;

    // fractal type getter / setter
    int fractalType() const { return m_fractalType; }
    void setFractalType(int val) {
        if (m_fractalType != val) { m_fractalType = val; emit fractalTypeChanged(); update(); }
    }

    QVector2D juliaC() const { return m_juliaC; }
    void setJuliaC(const QVector2D &val) {
        if (m_juliaC != val) {
            m_juliaC = val;
            emit juliaCChanged();
            update();
        }
    }

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

    Q_INVOKABLE void renderToFile(const QString &filename, int width, int height) {
        m_pendingExport = true;
        m_exportFilename = filename;
        m_exportWidth = width;
        m_exportHeight = height;
        update();
    }

    bool hasPendingExport() const { return m_pendingExport; }
    QString exportFilename() const { return m_exportFilename; }
    int exportWidth() const { return m_exportWidth; }
    int exportHeight() const { return m_exportHeight; }

    void clearExportFlag() { m_pendingExport = false; }

signals:
    void maxIterationsChanged();
    void colorTintChanged();
    void zoomLevelChanged();
    void zoomCenterChanged();
    void fractalTypeChanged();
    void juliaCChanged();

private:
    int m_maxIterations = 100;
    QVector3D m_colorTint = QVector3D(0.2f, 0.0f, 0.6f);

    double m_zoomLevel = 2.0;
    double m_zoomCenterX = -0.5;
    double m_zoomCenterY = 0.0;

    int m_fractalType = 0;

    QVector2D m_juliaC = QVector2D(-0.7f, 0.27015f);

    bool m_pendingExport = false;
    QString m_exportFilename;
    int m_exportWidth = 0;
    int m_exportHeight = 0;
};

#endif