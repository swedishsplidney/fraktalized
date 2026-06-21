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
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QColor>
#include <QMatrix4x4>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QQuaternion>

#include "fractalengine.h"

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

    void setGradientData(const QList<QVector3D> &colors, const QList<float> &stops) {
        m_gradientColors = colors;
        m_gradientStops = stops;
    }

    void setTargetZoom(double zoom) {m_targetZoom = zoom; }
    void setTargetCenter(double x, double y) { m_targetCenterX = x; m_targetCenterY = y; }

    void setFractalType(int type) { m_fractalType = type; }

    void setJuliaC(const QVector2D &c) { m_juliaC = c; }

    void setMandelbulbPower(float power) { m_mandelbulbPower = power; }
    void setFractalBrightness(float b) { m_fractalBrightness = b; }

private:
    bool m_initialized = false;
    QOpenGLShaderProgram *m_program = nullptr;
    int m_maxIterations = 100;

    QList<QVector3D> m_gradientColors = {
        QVector3D(0.0f, 0.0f, 0.02f), QVector3D(0.0f, 0.5f, 1.0f),
        QVector3D(1.0f, 1.0f, 1.0f), QVector3D(1.0f, 0.4f, 1.0f)
    };
    QList<float> m_gradientStops = { 0.0f, 0.33f, 0.66f, 1.0f };

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

    int m_aaSamples = 1;
    QSize m_scaledSize;

    GLuint m_compute_program = 0;
    GLuint m_accumulation_texture = 0;
    GLuint m_ifs_program = 0;
    int m_current_fbo_w = 0;
    int m_current_fbo_h = 0;

    void runTrajectoryCompute(int width, int height);
    void checkAccumulationStorage(int width, int height);

    bool m_isExportingBuddhabrot = false;
    int m_exportPassCount = 0;
    const int m_maxExportPasses = 50;
    GLuint m_exportAccumulationTex = 0;
    QOpenGLFramebufferObject* m_exportFbo = nullptr;

    int m_viewPassCount = 0;
    const int m_maxViewPasses = 10;

    QOpenGLShaderProgram *m_3dProgram = nullptr;
    bool m_is3DMode = false;

    QOpenGLVertexArrayObject *m_cubeVAO = nullptr;
    QOpenGLBuffer *m_cubeVBO = nullptr;

    float m_rotationAngle = 0.0f;

    QQuaternion m_rotation3D;
    QVector3D m_pan3D = QVector3D(0.0f, 0.0f, 0.0f);
    float m_zoom3D = 3.0f;

    QQuaternion m_targetRotation3D = QQuaternion();
    QVector3D m_targetPan3D = QVector3D(0.0f, 0.0f, 0.0f);
    float m_targetZoom3D = 3.0f;
    const float m_lerpFactor = 0.12f;

    void updateAnimations();

    float m_mandelbulbPower = 8.0f;
    float m_fractalBrightness = 1.0f;

    void init();
};

class FractalEngine : public QQuickFramebufferObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int maxIterations READ maxIterations WRITE setMaxIterations NOTIFY maxIterationsChanged)

    Q_PROPERTY(QVariantList gradientPositions READ gradientPositions WRITE setGradientPositions NOTIFY gradientPositionsChanged)
    Q_PROPERTY(QVariantList gradientColors READ gradientColors WRITE setGradientColors NOTIFY gradientColorsChanged)

    Q_PROPERTY(double zoomLevel READ zoomLevel WRITE setZoomLevel NOTIFY zoomLevelChanged)
    Q_PROPERTY(QVector2D zoomCenter READ zoomCenter WRITE setZoomCenter NOTIFY zoomCenterChanged)

    Q_PROPERTY(int fractalType READ fractalType WRITE setFractalType NOTIFY fractalTypeChanged)
    Q_PROPERTY(QVector2D juliaC READ juliaC WRITE setJuliaC NOTIFY juliaCChanged)

    Q_PROPERTY(int aaSamples READ aaSamples WRITE setAaSamples NOTIFY aaSamplesChanged)
    Q_PROPERTY(float viewportScale READ viewportScale WRITE setViewportScale NOTIFY viewportScaleChanged)

    Q_PROPERTY(bool is3DMode READ is3DMode WRITE setIs3DMode NOTIFY is3DModeChanged)

    Q_PROPERTY(double currentMinX READ currentMinX NOTIFY boundsChanged)
    Q_PROPERTY(double currentMaxX READ currentMaxX NOTIFY boundsChanged)
    Q_PROPERTY(double currentMinY READ currentMinY NOTIFY boundsChanged)
    Q_PROPERTY(double currentMaxY READ currentMaxY NOTIFY boundsChanged)

    Q_PROPERTY(QQuaternion rotation3D READ rotation3D WRITE setRotation3D NOTIFY rotation3DChanged)
    Q_PROPERTY(QVector3D pan3D READ pan3D WRITE setPan3D NOTIFY pan3DChanged)
    Q_PROPERTY(float zoom3D READ zoom3D WRITE setZoom3D NOTIFY zoom3DChanged)

    Q_PROPERTY(float mandelbulbPower READ mandelbulbPower WRITE setMandelbulbPower NOTIFY mandelbulbPowerChanged)
    Q_PROPERTY(float fractalBrightness READ fractalBrightness WRITE setFractalBrightness NOTIFY fractalBrightnessChanged)

public:
    FractalEngine();

    Renderer *createRenderer() const override;

    int aaSamples() const { return m_aaSamples; }
    void setAaSamples(int samples) {
        if (m_aaSamples != samples) {
            m_aaSamples = samples;
            emit aaSamplesChanged();
            update();
        }
    }

    float viewportScale() const { return m_viewportScale; }
    void setViewportScale(float scale) {
        if (m_viewportScale != scale) {
            m_viewportScale = scale;
            emit viewportScaleChanged();
            update();
        }
    }

    // 3d toggle
    bool is3DMode() const { return m_is3DMode; }
    void setIs3DMode(bool enabled) {
        if (m_is3DMode != enabled) {
            m_is3DMode = enabled;
            emit is3DModeChanged();
            update();
        }
    }

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

    QVariantList gradientColors() const;
    void setGradientColors(const QVariantList &colors);

    QVariantList gradientPositions() const;
    void setGradientPositions(const QVariantList &positions);

    QList<QVector3D> internalGradientColors() const { return m_gradientColors; }
    QList<float> internalGradientStops() const { return m_gradientStops; }

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

    double currentMinX() const { return m_minX; }
    double currentMaxX() const { return m_maxX; }
    double currentMinY() const { return m_minY; }
    double currentMaxY() const { return m_maxY; }

    void updateComputedBoundsDirect(double w, double h);

    Q_INVOKABLE void savePreset(const QString &name, const QVariantList &positions, const QVariantList &colors, int fractalType, double zoom, const QVector2D &center);
    Q_INVOKABLE QStringList loadPresetNames();
    Q_INVOKABLE QVariantMap loadPresetData(const QString &name);

    Q_INVOKABLE void deletePreset(const QString &name);

    // 3d stuff
    QVector3D pan3D() const { return m_targetPan3D; }
    void setPan3D(const QVector3D &val) {
        if (m_targetPan3D != val) {
            m_targetPan3D = val;
            emit pan3DChanged();
            update();
        }
    }

    float zoom3D() const { return m_targetZoom3D; }
    void setZoom3D(float val) {
        if (m_targetZoom3D != val) {
            m_targetZoom3D = val;
            emit zoom3DChanged();
            update();
        }
    }

    QQuaternion rotation3D() const { return m_targetRotation3D; }
    void setRotation3D(const QQuaternion &val) {
        if (m_targetRotation3D != val) {
            m_targetRotation3D = val;
            emit rotation3DChanged();
            update();
        }
    }

    Q_INVOKABLE void handle3DDrag(float dx, float dy, bool isShiftPressed) {
        if (isShiftPressed) {
            float speed = m_targetZoom3D * 0.00095f;
            m_targetPan3D.setX(m_targetPan3D.x() + (dx * speed));
            m_targetPan3D.setY(m_targetPan3D.y() + (dy * speed));
        } else {
            float rotationSpeed = (m_targetZoom3D / 3.0f) * 0.3f;
            if (rotationSpeed < 0.02f) rotationSpeed = 0.02f;
            if (rotationSpeed > 0.5f) rotationSpeed = 0.5f;

            QVector3D targetUp = m_targetRotation3D.rotatedVector(QVector3D(0.0f, 1.0f, 0.0f));
            float flipDirection = (targetUp.y() < 0.0f) ? -1.0f : 1.0f;

            QQuaternion globalPitch = QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, -dy * rotationSpeed);
            QQuaternion localYaw = QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, dx * rotationSpeed * flipDirection);

            m_targetRotation3D = globalPitch * m_targetRotation3D * localYaw;
            m_targetRotation3D.normalize();
        }
        update();
    }

    Q_INVOKABLE void handle3DZoom(float angleDelta) {
        float dynamicSensitivity = 0.0005f * (m_targetZoom3D * m_targetZoom3D);

        if (dynamicSensitivity < 0.00001f) dynamicSensitivity = 0.00001f;

        m_targetZoom3D -= angleDelta * dynamicSensitivity;

        if (m_targetZoom3D < 1e-6f) m_targetZoom3D = 1e-6f;

        update();
    }

    float mandelbulbPower() const { return m_mandelbulbPower; }
    void setMandelbulbPower(float val) {
        if (m_mandelbulbPower != val) {
            m_mandelbulbPower = val;
            emit mandelbulbPowerChanged();
            update();
        }
    }

    float fractalBrightness() const { return m_fractalBrightness; }
    void setFractalBrightness(float val) {
       if (m_fractalBrightness != val) {
           m_fractalBrightness = val;
           emit fractalBrightnessChanged();
           update();
       }
    }

signals:
    void maxIterationsChanged();
    void gradientColorsChanged();
    void gradientPositionsChanged();
    void zoomLevelChanged();
    void zoomCenterChanged();
    void fractalTypeChanged();
    void juliaCChanged();
    void aaSamplesChanged();
    void viewportScaleChanged();
    void boundsChanged();
    void is3DModeChanged();
    void rotation3DChanged();
    void pan3DChanged();
    void zoom3DChanged();
    void mandelbulbPowerChanged();
    void fractalBrightnessChanged();

private:
    int m_maxIterations = 100;

    QList<QVector3D> m_gradientColors;
    QList<float> m_gradientStops;

    double m_zoomLevel = 2.0;
    double m_zoomCenterX = -0.5;
    double m_zoomCenterY = 0.0;

    int m_fractalType = 0;

    QVector2D m_juliaC = QVector2D(-0.7f, 0.27015f);

    bool m_pendingExport = false;
    QString m_exportFilename;
    int m_exportWidth = 0;
    int m_exportHeight = 0;

    int m_aaSamples = 1;
    float m_viewportScale = 1.0f;

    bool m_is3DMode = false;

    void updateComputedBounds();

    double m_minX = -2.5;
    double m_maxX = 1.5;
    double m_minY = -1.5;
    double m_maxY = 1.5;

    QString getPresetsFilePath() const {
        QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(appDataPath);
        return appDataPath + "/presets.json";
    }

    QQuaternion m_rotation3D =  QQuaternion();
    QVector3D m_pan3D = QVector3D(0.0f, 0.0f, 0.0f);
    float m_zoom3D = 3.0f;

    QQuaternion m_targetRotation3D = QQuaternion();
    QVector3D m_targetPan3D = QVector3D(0.0f, 0.0f, 0.0f);
    float m_targetZoom3D = 3.0f;
    const float m_lerpFactor = 0.12f;

    float m_mandelbulbPower = 8.0f;
    float m_fractalBrightness = 1.0f;
};

#endif