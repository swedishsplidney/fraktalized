#include "fractalengine.h"
#include <iostream>
#include <QOpenGLFramebufferObject>
#include <QPainter>
#include <QQuickWindow>
#include <QOpenGLVersionFunctionsFactory>
#include <QOpenGLFunctions_4_3_Core>

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

    // trajectory engine
    QOpenGLShader *compShader = new QOpenGLShader(QOpenGLShader::Compute);
    if (compShader->compileSourceFile("shaders/trajectory.glsl")) {
        m_compute_program = glCreateProgram();
        glAttachShader(m_compute_program, compShader->shaderId());
        glLinkProgram(m_compute_program);

        // check linkage status
        GLint linkSuccess = 0;
        glGetProgramiv(m_compute_program, GL_LINK_STATUS, &linkSuccess);
        if (!linkSuccess) {
            char infoLog[512];
            glGetProgramInfoLog(m_compute_program, 512, nullptr, infoLog);
            std::cout << "compute Shader link error: " << infoLog << std::endl;
        } else {
            std::cout << "compute shader linked completely: ok" << std::endl;
        }
    } else {
        std::cout << "compute shader compilation failed!" << std::endl;
    }
    delete compShader;

    // ifs engine
    QOpenGLShader *ifsShader = new QOpenGLShader(QOpenGLShader::Compute);
    if (ifsShader->compileSourceFile("shaders/ifs.glsl")) {
        m_ifs_program = glCreateProgram();
        glAttachShader(m_ifs_program, ifsShader->shaderId());
        glLinkProgram(m_ifs_program);

        GLint linkSuccess = 0;
        glGetProgramiv(m_ifs_program, GL_LINK_STATUS, &linkSuccess);
        if (!linkSuccess) {
            char infoLog[512];
            glGetProgramInfoLog(m_ifs_program, 512, nullptr, infoLog);
            std::cout << "ifs shader link error: " << infoLog << std::endl;
        } else {
            std::cout << "ifs shader link: ok" << std::endl;
        }
    } else {
        std::cout << "ifs shader compilation failed!" << std::endl;
    }
    delete ifsShader;

    // 3d initializer
    m_3dProgram = new QOpenGLShaderProgram();
    m_3dProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, "shaders/3d.vert");
    m_3dProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, "shaders/3d.frag");
    m_3dProgram->bindAttributeLocation("aPos", 0);
    m_3dProgram->bindAttributeLocation("aColor", 1);
    m_3dProgram->link();

    // cube vertex data
    GLfloat cubeVertices[] = {
        // front face
        -0.5f, -0.5f, 0.5f,   1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.5f,   0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, 0.5f,   0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f,   1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, 0.5f,   0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, 0.5f,   1.0f, 1.0f, 0.0f,
        // back face
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.5f, 0.5f, 0.5f,
        // left face
        -0.5f,  0.5f,  0.5f,  1.0f, 0.1f, 0.4f,
        -0.5f,  0.5f, -0.5f,  0.2f, 0.8f, 0.2f,
        -0.5f, -0.5f, -0.5f,  0.9f, 0.9f, 0.1f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.1f, 0.4f,
        -0.5f, -0.5f, -0.5f,  0.9f, 0.9f, 0.1f,
        -0.5f, -0.5f,  0.5f,  0.3f, 0.2f, 0.7f,
        // right face
         0.5f,  0.5f,  0.5f,  0.7f, 0.1f, 0.9f,
         0.5f, -0.5f,  0.5f,  0.1f, 0.8f, 0.6f,
         0.5f, -0.5f, -0.5f,  0.4f, 0.4f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.7f, 0.1f, 0.9f,
         0.5f, -0.5f, -0.5f,  0.4f, 0.4f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.2f, 0.9f, 0.3f,
        // top face
        -0.5f,  0.5f, -0.5f,  1.0f, 0.5f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.5f,
         0.5f,  0.5f,  0.5f,  0.5f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 0.5f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.5f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.1f, 0.7f, 0.8f,
        // bottom face
        -0.5f, -0.5f, -0.5f,  0.5f, 0.5f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 0.5f, 0.5f,
         0.5f, -0.5f,  0.5f,  0.5f, 0.0f, 0.5f,
        -0.5f, -0.5f, -0.5f,  0.5f, 0.5f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.5f, 0.0f, 0.5f,
        -0.5f, -0.5f,  0.5f,  0.8f, 0.2f, 0.2f
    };

    m_cubeVAO = new QOpenGLVertexArrayObject();
    m_cubeVAO->create();
    m_cubeVAO->bind();

    m_cubeVBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_cubeVBO->create();
    m_cubeVBO->setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_cubeVBO->bind();
    m_cubeVBO->allocate(cubeVertices, sizeof(cubeVertices));

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    m_cubeVAO->release();
    m_cubeVBO->release();

    m_initialized = true;
}

QOpenGLFramebufferObject *FractalFBORenderer::createFramebufferObject(const QSize &size) {
    // scale fbo storage
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::NoAttachment);

    QSize targetSize = m_scaledSize;
    if (targetSize.isEmpty()) {
        targetSize = size;
    }

    return new QOpenGLFramebufferObject(targetSize, format);
}

void FractalFBORenderer::render() {
    if (!m_initialized) init();

    // get the texture surface constraints directly froam the active fbo container
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

    if (m_currentZoom != m_targetZoom || m_currentCenterX != m_targetCenterX || m_currentCenterY != m_targetCenterY || m_fractalType >= 4) {
        update();
    }

    // isolate state adjustments
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_is3DMode) {
        // 3d depth buffers
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
    }
    glDisable(GL_BLEND);

    float screenAspect = (float)width / (float)height;
    float scaleX = 1.0f;
    float scaleY = 1.0f;

    if (width >= height) {
        // landscape
        scaleX = 1.0f;
        scaleY = 1.0f / screenAspect;
    } else {
        // portrait
        scaleX = screenAspect;
        scaleY = 1.0f;
    }

    // color
    QVector3D colors[4];
    float stops[4];
    for (int i = 0; i < 4; ++i) {
        colors[i] = (i < m_gradientColors.size()) ? m_gradientColors[i] : QVector3D(0.0f, 0.0f, 0.0f);
        stops[i]  = (i < m_gradientStops.size()) ? m_gradientStops[i] : (static_cast<float>(i) / 3.0f);
    }

    // local coordinate structure passed down to the pipeline unit
    GLfloat rawVertices[] = {
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
    };

    // escape time
    if (m_fractalType <= 3) {
        m_program->bind();
        m_program->setUniformValue("u_tile_bounds", QVector4D(-scaleX, -scaleY, scaleX, scaleY));
        m_program->setUniformValue("u_resolution", QVector2D(width, height));
        m_program->setUniformValue("u_max_iter", static_cast<float>(m_maxIterations));
        m_program->setUniformValueArray("u_gradient_colors", colors, 4);
        m_program->setUniformValueArray("u_gradient_stops", stops, 4, 1);
        m_program->setUniformValue("u_fractal_type", m_fractalType);
        m_program->setUniformValue("u_aaSamples", m_aaSamples);

        int locZoomLevel = m_program->uniformLocation("u_zoom_level");
        int locZoomCenter = m_program->uniformLocation("u_zoom_center");
        int locJuliaC = m_program->uniformLocation("u_julia_c");

        if (locZoomLevel != -1)  this->glUniform1d(locZoomLevel, m_currentZoom);
        if (locZoomCenter != -1) this->glUniform2d(locZoomCenter, m_currentCenterX, m_currentCenterY);
        if (locJuliaC != -1)     this->glUniform2d(locJuliaC, static_cast<double>(m_juliaC.x()), static_cast<double>(m_juliaC.y()));

        m_program->enableAttributeArray(0);
        m_program->setAttributeArray(0, GL_FLOAT, rawVertices, 3);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        m_program->disableAttributeArray(0);
        m_program->release();
    }
    // trajectory/ifs accumulation
    else if (m_fractalType == 4 || m_fractalType == 5 || m_fractalType == 6) {
        QOpenGLContext *currentContext = QOpenGLContext::currentContext();
        auto *gl43 = currentContext ? QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_4_3_Core>(currentContext) : nullptr;

        if (!gl43) {
            std::cout << "failed to obtain openGL 4.3 function pointers! compute shaders not supported :(" << std::endl;
            return;
        }

        // gen texture id (if id doesnt alr exist)
        static bool texGenerated = false;
        if (!texGenerated) {
            glGenTextures(1, &m_accumulation_texture);
            texGenerated = true;
        }

        // dynamically track texture size and reallocate gpu if size changed
        static int lastWidth = 0;
        static int lastHeight = 0;
        glBindTexture(GL_TEXTURE_2D, m_accumulation_texture);

        if (width != lastWidth || height != lastHeight) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            lastWidth = width;
            lastHeight = height;

            std::vector<uint32_t> zeroData(width * height, 0u);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED_INTEGER, GL_UNSIGNED_INT, zeroData.data());
        }

        // clear accumulation buffers if camera changes
        static double lastCurrentX = 0;
        static double lastCurrentY = 0;
        static double lastCurrentZoom = 0;
        static int lastType = 0;

        bool cameraMoved = (m_currentCenterX != lastCurrentX ||
                            m_currentCenterY != lastCurrentY ||
                            m_currentZoom != lastCurrentZoom ||
                            m_fractalType != lastType);

        if (cameraMoved) {
            std::vector<uint32_t> zeroData(width * height, 0u);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED_INTEGER, GL_UNSIGNED_INT, zeroData.data());

            lastCurrentX = m_currentCenterX;
            lastCurrentY = m_currentCenterY;
            lastCurrentZoom = m_currentZoom;
            lastType = m_fractalType;

            m_viewPassCount = 0;
        }

        int maxRealtimePasses = 1;

        if (m_viewPassCount < maxRealtimePasses) {
            // bind image to unit 0
            gl43->glBindImageTexture(0, m_accumulation_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

            GLuint activeComputeProg = (m_fractalType >= 6) ? m_ifs_program : m_compute_program;

            // compute workers
            glUseProgram(activeComputeProg);
            glUniform2d(glGetUniformLocation(activeComputeProg, "u_zoom_center"), m_currentCenterX, m_currentCenterY);
            glUniform1d(glGetUniformLocation(activeComputeProg, "u_zoom_level"), m_currentZoom);
            glUniform2f(glGetUniformLocation(activeComputeProg, "u_resolution"), static_cast<float>(width), static_cast<float>(height));
            glUniform1i(glGetUniformLocation(activeComputeProg, "u_fractal_type"), m_fractalType);

            if (m_fractalType <= 6) {
                glUniform1f(glGetUniformLocation(activeComputeProg, "u_max_iter"), static_cast<float>(m_maxIterations));
            }

            if (m_fractalType >= 6) {
                double totalWorkers = 100000.0;
                GLuint groups_x = (totalWorkers + 255) / 256;
                gl43->glDispatchCompute(groups_x, 1, 1);
            } else {
                GLuint groups_x = (width + 15) / 16;
                GLuint groups_y = (height + 15) / 16;
                gl43->glDispatchCompute(groups_x, groups_y, 1);
            }

            // wait until atomic stores finish
            gl43->glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            glUseProgram(0);

            m_viewPassCount++;

            // only trigger a new update if still need to fulfill passes
            if (m_viewPassCount < maxRealtimePasses) {
                update();
            }
        }

        m_program->bind();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_accumulation_texture);
        m_program->setUniformValue("u_accumulation_sampler", 0);

        m_program->setUniformValue("u_resolution", QVector2D(width, height));
        m_program->setUniformValue("u_fractal_type", m_fractalType);
        m_program->setUniformValue("u_max_iter", static_cast<float>(m_maxIterations));
        m_program->setUniformValueArray("u_gradient_colors", colors, 4);
        m_program->setUniformValueArray("u_gradient_stops", stops, 4, 1);

        m_program->enableAttributeArray(0);
        m_program->setAttributeArray(0, GL_FLOAT, rawVertices, 3);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        m_program->disableAttributeArray(0);
        m_program->release();
    }

    // 3d render
    if (m_is3DMode) {
        m_3dProgram->bind();

        QVector3D bgColor = !m_gradientColors.isEmpty() ? m_gradientColors[0] : QVector3D(0.0f, 0.0f, 0.0f);
        this->glClearColor(bgColor.x(), bgColor.y(), bgColor.z(), 1.0f);

        this->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float fov = 45.0f * M_PI / 180.0f;
        float aspect = (float)width / (float)height;
        float nearPlane = 0.1f;
        float farPlane = 100.0f;
        float f = 1.0f / tan(fov / 2.0f);

        QMatrix4x4 projection;
        projection.setToIdentity();
        projection.setRow(0, QVector4D(f / aspect, 0.0f, 0.0f, 0.0f));
        projection.setRow(1, QVector4D(0.0f, f, 0.0f, 0.0f));
        projection.setRow(2, QVector4D(0.0f, 0.0f, (farPlane + nearPlane) / (nearPlane - farPlane), (2.0f * farPlane * nearPlane) / (nearPlane - farPlane)));
        projection.setRow(3, QVector4D(0.0f, 0.0f, -1.0f, 0.0f));

        QMatrix4x4 view;
        view.setToIdentity();
        view.setRow(2, QVector4D(0.0f, 0.0f, 1.0f, -3.0f));

        m_rotationAngle += 0.01f;
        float cosA = cos(m_rotationAngle);
        float sinA = sin(m_rotationAngle);

        QMatrix4x4 model;
        model.setToIdentity();
        model.setRow(0, QVector4D(cosA,  0.0f, sinA, 0.0f));
        model.setRow(2, QVector4D(-sinA, 0.0f, cosA, 0.0f));

        m_3dProgram->setUniformValue("u_model", model);
        m_3dProgram->setUniformValue("u_view", view);
        m_3dProgram->setUniformValue("u_projection", projection);

        m_cubeVAO->bind();
        glDrawArrays(GL_TRIANGLES, 0, 36);
        m_cubeVAO->release();

        m_3dProgram->release();
        update();
    }

    // render to file engine
    if (m_pendingExport && m_exportWidth > 0 && m_exportHeight > 0) {
        m_pendingExport = false;

        QOpenGLContext *currentContext = QOpenGLContext::currentContext();
        auto *gl43 = currentContext ? QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_4_3_Core>(currentContext) : nullptr;

        if ((m_fractalType == 4 || m_fractalType == 5 || m_fractalType == 6) && !gl43) {
            qWarning() << "cannot export accumulation fractal: OpenGL 4.3 functions unavailable.";
            return;
        }

        qDebug() << "initializing high-res render sequence:" << m_exportWidth << "x" << m_exportHeight;

        // setup the high res fbo buffer layout
        QOpenGLFramebufferObjectFormat exportFormat;
        exportFormat.setAttachment(QOpenGLFramebufferObject::NoAttachment);
        exportFormat.setInternalTextureFormat(GL_RGBA8);
        m_exportFbo = new QOpenGLFramebufferObject(m_exportWidth, m_exportHeight, exportFormat);

        if (m_fractalType == 4 || m_fractalType == 5 || m_fractalType == 6) {
            // allocate giant high res texture storage unit
            glGenTextures(1, &m_exportAccumulationTex);
            glBindTexture(GL_TEXTURE_2D, m_exportAccumulationTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, m_exportWidth, m_exportHeight, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            std::vector<uint32_t> zeroData(m_exportWidth * m_exportHeight, 0u);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_exportWidth, m_exportHeight, GL_RED_INTEGER, GL_UNSIGNED_INT, zeroData.data());

            m_isExportingBuddhabrot = true;
            m_exportPassCount = 0;
        } else {
            // traditional escape fractals don't need multi-frame splits
            if (m_exportFbo->bind()) {
                glViewport(0, 0, m_exportWidth, m_exportHeight);
                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);

                m_program->bind();
                m_program->setUniformValue("u_tile_bounds", QVector4D(-scaleX, -scaleY, scaleX, scaleY));

                int expZoomLevel = m_program->uniformLocation("u_zoom_level");
                int expZoomCenter = m_program->uniformLocation("u_zoom_center");
                if (expZoomLevel != -1)  this->glUniform1d(expZoomLevel, m_targetZoom);
                if (expZoomCenter != -1) this->glUniform2d(expZoomCenter, m_targetCenterX, m_targetCenterY);

                m_program->setUniformValue("u_resolution", QVector2D(m_exportWidth, m_exportHeight));
                m_program->setUniformValue("u_fractal_type", m_fractalType);
                m_program->setUniformValue("u_max_iter", static_cast<float>(m_maxIterations));
                m_program->setUniformValueArray("u_gradient_colors", colors, 4);
                m_program->setUniformValueArray("u_gradient_stops", stops, 4, 1);

                m_program->enableAttributeArray(0);
                m_program->setAttributeArray(0, GL_FLOAT, rawVertices, 3);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                m_program->disableAttributeArray(0);
                m_program->release();

                QImage masterImage = m_exportFbo->toImage().flipped(Qt::Vertical);
                masterImage.save(m_exportFilename);
                m_exportFbo->release();
                delete m_exportFbo;
                m_exportFbo = nullptr;
                qDebug() << "saved standard high-res image successfully.";
            }
        }
    }

    // passthrough loop engine
    if (m_isExportingBuddhabrot && m_exportFbo) {
        QOpenGLContext *currentContext = QOpenGLContext::currentContext();
        auto *gl43 = currentContext ? QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_4_3_Core>(currentContext) : nullptr;

        GLuint activeComputeProg = (m_fractalType >= 6) ? m_ifs_program : m_compute_program;

        // process 1 calculation step per frame context update
        glUseProgram(activeComputeProg);
        glUniform2d(glGetUniformLocation(activeComputeProg, "u_zoom_center"), m_targetCenterX, m_targetCenterY);
        glUniform1d(glGetUniformLocation(activeComputeProg, "u_zoom_level"), m_targetZoom);
        glUniform2f(glGetUniformLocation(activeComputeProg, "u_resolution"), static_cast<float>(m_exportWidth), static_cast<float>(m_exportHeight));
        glUniform1i(glGetUniformLocation(activeComputeProg, "u_fractal_type"), m_fractalType);

        if (m_fractalType <= 6) {
            glUniform1f(glGetUniformLocation(activeComputeProg, "u_max_iter"), static_cast<float>(m_maxIterations));
        }

        gl43->glBindImageTexture(0, m_exportAccumulationTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

        if (m_fractalType >= 6) {
            int totalWorkers = 200000;
            GLuint groups_x = (totalWorkers + 255) / 256;
            gl43->glDispatchCompute(groups_x, 1, 1);
        } else {
            GLuint groups_x = (m_exportWidth + 15) / 16;
            GLuint groups_y = (m_exportHeight + 15) / 16;

            // execute 1 pass, then return control immediately back to system window manager loop
            gl43->glDispatchCompute(groups_x, groups_y, 1);
        }


        gl43->glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glUseProgram(0);

        m_exportPassCount++;
        qDebug() << "compute export progress:" << m_exportPassCount << "/" << m_maxExportPasses;

        // keep pushing updates until we collect all samples
        if (m_exportPassCount < m_maxExportPasses) {
            update();
        }
        // final comp and writing
        else {
            m_isExportingBuddhabrot = false;

            if (m_exportFbo->bind()) {
                glViewport(0, 0, m_exportWidth, m_exportHeight);
                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);

                m_program->bind();
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, m_exportAccumulationTex);
                m_program->setUniformValue("u_accumulation_sampler", 0);

                m_program->setUniformValue("u_resolution", QVector2D(m_exportWidth, m_exportHeight));
                m_program->setUniformValue("u_fractal_type", m_fractalType);
                m_program->setUniformValue("u_max_iter", static_cast<float>(m_maxIterations));
                m_program->setUniformValueArray("u_gradient_colors", colors, 4);
                m_program->setUniformValueArray("u_gradient_stops", stops, 4, 1);

                m_program->enableAttributeArray(0);
                m_program->setAttributeArray(0, GL_FLOAT, rawVertices, 3);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                m_program->disableAttributeArray(0);
                m_program->release();

                glFlush();

                QImage masterImage = m_exportFbo->toImage().flipped(Qt::Vertical);
                if (masterImage.save(m_exportFilename)) {
                    qDebug() << "completed export:" << m_exportFilename;
                } else {
                    qWarning() << "failed to save file to disk path.";
                }

                m_exportFbo->release();
            }

            // deallocate assets
            glDeleteTextures(1, &m_exportAccumulationTex);
            m_exportAccumulationTex = 0;
            delete m_exportFbo;
            m_exportFbo = nullptr;

            // reset viewport
            glViewport(0, 0, width, height);
        }
    }
}

// fractalengine implementation

FractalEngine::FractalEngine() : m_viewportScale(1.0f) {
    m_gradientColors.append(QVector3D(0.0f, 0.0f, 0.02f));
    m_gradientColors.append(QVector3D(0.0f, 0.5f, 1.0f));
    m_gradientColors.append(QVector3D(1.0f, 1.0f, 1.0f));
    m_gradientColors.append(QVector3D(1.0f, 0.4f, 1.0f));

    m_gradientStops.append(0.0f);
    m_gradientStops.append(0.33f);
    m_gradientStops.append(0.66f);
    m_gradientStops.append(1.0f);
}

QQuickFramebufferObject::Renderer *FractalEngine::createRenderer() const {
    return new FractalFBORenderer();
}

// convert qvector3d to qvariant list
QVariantList FractalEngine::gradientColors() const {
    QVariantList list;
    for (const auto &color : m_gradientColors) {
        list.append(QColor::fromRgbF(color.x(), color.y(), color.z()));
    }
    return list;
}

void FractalEngine::setGradientColors(const QVariantList &colors) {
    QList<QVector3D> newColors;
    for (const auto &element : colors) {
        QColor col = element.value<QColor>();
        newColors.append(QVector3D(col.redF(), col.greenF(), col.blueF()));
    }
    if (m_gradientColors != newColors) {
        m_gradientColors = newColors;
        emit gradientColorsChanged();
        update();
    }
}

// convert floats to variant lists
QVariantList FractalEngine::gradientPositions() const {
    QVariantList list;
    for (float stop : m_gradientStops) {
        list.append(stop);
    }
    return list;
}

void FractalEngine::setGradientPositions(const QVariantList &positions) {
    QList<float> newStops;
    for (const auto &element : positions) {
        newStops.append(element.toFloat());
    }
    if (m_gradientStops != newStops) {
        m_gradientStops = newStops;
        emit gradientPositionsChanged();
        update();
    }
}

void FractalFBORenderer::synchronize(QQuickFramebufferObject *item) {
    auto *engine = static_cast<FractalEngine *>(item);

    this->m_is3DMode = engine->is3DMode();

    this->setMaxIterations(engine->maxIterations());

    this->setGradientData(engine->internalGradientColors(), engine->internalGradientStops());

    this->setTargetZoom(engine->zoomLevel());
    this->setTargetCenter(engine->zoomCenterX(), engine->zoomCenterY());

    this->setFractalType(engine->fractalType());

    this->setJuliaC(engine->juliaC());

    this->m_aaSamples = engine->aaSamples();

    // calc scaled viewport size
    QSize baseSize = engine->window() ? engine->window()->size() : QSize(1920, 1080);
    QSize newScaledSize = QSize(baseSize.width() * engine->viewportScale(), baseSize.height() * engine->viewportScale());

    if (this->m_scaledSize != newScaledSize) {
        this->m_scaledSize = newScaledSize;

        this->invalidateFramebufferObject();
    }

    engine->updateComputedBoundsDirect(newScaledSize.width(), newScaledSize.height());

    if (engine->hasPendingExport()) {
        this->m_pendingExport = true;
        this->m_exportFilename = engine->exportFilename();
        this->m_exportWidth = engine->exportWidth();
        this->m_exportHeight = engine->exportHeight();
        engine->clearExportFlag();
    }
}

// compute coordinate space
void FractalEngine::updateComputedBoundsDirect(double w, double h) {
    if (w <= 0 || h <= 0) return;

    double screenAspect = w / h;
    double scaleX = 1.0;
    double scaleY = 1.0;

    if (w >= h) {
        scaleX = 1.0;
        scaleY = 1.0 / screenAspect;
    } else {
        scaleX = screenAspect;
        scaleY = 1.0;
    }

    m_minX = m_zoomCenterX - (scaleX * m_zoomLevel);
    m_maxX = m_zoomCenterX + (scaleX * m_zoomLevel);
    m_minY = m_zoomCenterY - (scaleY * m_zoomLevel);
    m_maxY = m_zoomCenterY + (scaleY * m_zoomLevel);

    emit boundsChanged();
}

void FractalEngine::updateComputedBounds() {
    updateComputedBoundsDirect(this->width(), this->height());
}

// save presets
void FractalEngine::savePreset(const QString & name, const QVariantList &positions, const QVariantList &colors, int fractalType, double zoom, const QVector2D &center) {
    if (name.isEmpty()) return;

    QString filePath = getPresetsFilePath();
    QJsonObject masterRoot;

    // read existing presets
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isObject()) {
            masterRoot = doc.object();
        }
        file.close();
    }

    // add new preset entry
    QJsonObject presetObj;

    // gradient
    QJsonArray stopsArray;
    for (int i = 0; i < positions.size() && i < colors.size(); ++i) {
        QJsonObject stop;
        stop["pos"] = positions[i].toDouble();
        stop["color"] = colors[i].toString();
        stopsArray.append(stop);
    }
    presetObj["stops"] = stopsArray;
    presetObj["set"] = fractalType;
    presetObj["zoom"] = zoom;

    // convert coords
    QJsonObject centerObj;
    centerObj["x"] = static_cast<double>(center.x());
    centerObj["y"] = static_cast<double>(center.y());
    presetObj["center"] = centerObj;

    // insert preset
    masterRoot[name] = presetObj;

    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(masterRoot);
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
        std::cout << "saved preset: " << name.toStdString() << " to " << filePath.toStdString() << std::endl;
    }
}

// load preset names
QStringList FractalEngine::loadPresetNames() {
    QStringList names;
    QFile file (getPresetsFilePath());

    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isObject()) {
            names = doc.object().keys();
        }
        file.close();
    }

    // fallback
    if (names.isEmpty()) {
        names << "no presets saved!";
    }
    return names;
}

// load preset data
QVariantMap FractalEngine::loadPresetData(const QString &name) {
    QVariantMap result;
    QFile file(getPresetsFilePath());

    if (!file.open(QIODevice::ReadOnly)) {
        return result;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject()) return result;

    QJsonObject masterRoot = doc.object();
    if (!masterRoot.contains(name)) return result;

    QJsonObject presetObj = masterRoot[name].toObject();

    // convert into raw QVariant structures
    QVariantList retrievedStops;
    QJsonArray stopsArr = presetObj["stops"].toArray();
    for (const auto &stopVal : stopsArr) {
        QJsonObject stopObj = stopVal.toObject();
        QVariantMap stopMap;
        stopMap["pos"] = stopObj["pos"].toDouble();
        stopMap["color"] = stopObj["color"].toString();
        retrievedStops.append(stopMap);
    }

    result["stops"] = retrievedStops;
    result["set"] = presetObj["set"].toInt();
    result["zoom"] = presetObj["zoom"].toDouble();

    // convert center to qvector2d
    QJsonObject centerObj = presetObj["center"].toObject();
    QVector2D centerVec(centerObj["x"].toDouble(), centerObj["y"].toDouble());
    result["center"] = centerVec;

    return result;
}

void FractalEngine::deletePreset(const QString &name) {
    if (name.isEmpty() || name == "no presets saved!") return;

    QString filePath = getPresetsFilePath();
    QJsonObject masterRoot;

    // read the preset file
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isObject()) {
            masterRoot = doc.object();
        }
        file.close();
    }

    // remove the preset
    if (masterRoot.contains(name)) {
        masterRoot.remove(name);
        std::cout << "removed preset: " << name.toStdString()<< ", at: " << filePath.toStdString() << std::endl;
    } else {
        return;
    }

    // update json
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(masterRoot);
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    }
}