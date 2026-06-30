#include "tutorialmanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>
#include <iostream>

TutorialManager::TutorialManager(QObject *parent)
    : QObject(parent), m_currentStepIndex(0), m_currentCompletion(0.0)
{
    loadSteps();

    // check state
    m_isTutorialCompleted = loadTutorialStatus();

    if (m_isTutorialCompleted) {
        m_currentStepIndex = m_steps.size();
    }

    updateCurrentState();
}

void TutorialManager::loadSteps() {
    m_steps.append({ "welcome to fraktalized! click 'next' to continue.", "bottom", 0.1 });
    m_steps.append({ "over here is the fractal selector! try selecting a few different fractal types, and click 'next' when you're ready to continue.", "bottom", 0.2, "selector" });
    m_steps.append({ "this is the iteration controller. the amount of iterations determines the fractal's detail. hit next to continue!", "bottom", 0.3, "iteration" });
    m_steps.append({ "over here, you can switch to 3d mode! try checking out the different 3d fractals! when you're ready, click 'next' to continue.", "bottom", 0.4, "toggle" });
    m_steps.append({ "here, you can switch between orbital and free camera modes. try exploring in 3d with these different modes!", "bottom", 0.5, "freeflytoggle" });
    m_steps.append({ "over here, you can export whatever you're currently looking at! you can set any resolution you like, then just hit 'save to file'", "bottom", 0.6, "save" });
    m_steps.append({ "here, you can change the colors that the fractal maps to! you can click and drag the handles, or double click them to change their color.", "bottom", 0.7, "color" });
    m_steps.append({ "thats pretty much it! have fun exploring these beautiful mathy functions!", "bottom", 0.9 });
}

QString TutorialManager::targetElement() const {
    if (m_currentStepIndex >= m_steps.size() || m_steps.isEmpty()) return "";
    return m_steps.at(m_currentStepIndex).targetElement;
}

QString TutorialManager::getSettingsFilePath() const {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(configDir);
    return QDir(configDir).filePath("presets.json");
}

bool TutorialManager::loadTutorialStatus() {
    QFile file(getSettingsFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (doc.isObject()) {
        QJsonObject masterRoot = doc.object();
        return masterRoot.value("tutorial_completed").toBool(false);
    }
    return false;
}

void TutorialManager::saveTutorialStatus(bool completed) {
    QString filePath = getSettingsFilePath();
    QJsonObject masterRoot;

    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isObject()) {
            masterRoot = doc.object();
        }
        file.close();
    }

    masterRoot["tutorial_completed"] = completed;

    // write changes
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(masterRoot);
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
        std::cout << "tutorial completion status updated to: " << (completed ? "true" : "false") << std::endl;
    }
}

void TutorialManager::updateCurrentState() {
    if (m_steps.isEmpty()) {
        m_currentText = "";
        m_currentLocation = "hidden";
        m_currentCompletion = 0.0;
        return;
    }

    if (m_currentStepIndex >= m_steps.size()) {
        m_currentText = "";
        m_currentLocation = "hidden";
        m_currentCompletion = 1.0;
        
        if (!m_isTutorialCompleted) {
            m_isTutorialCompleted = true;
            saveTutorialStatus(true);
        }
    } else {
        TutorialStep current = m_steps.at(m_currentStepIndex);
        m_currentText = current.text;
        m_currentLocation = current.location;
        m_currentCompletion = current.completion;
    }

    emit tutorialChanged();
}

void TutorialManager::nextStep() {
    if (m_currentStepIndex < m_steps.size()) {
        m_currentStepIndex++;
        updateCurrentState();
    }
}

void TutorialManager::resetTutorial() {
    m_currentStepIndex = 0;
    m_isTutorialCompleted = false;
    saveTutorialStatus(false);
    updateCurrentState();
}

QString TutorialManager::text() const { return m_currentText; }
QString TutorialManager::location() const { return m_currentLocation; }
double TutorialManager::completion() const { return m_currentCompletion; }