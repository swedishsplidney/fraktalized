#ifndef TUTORIALMANAGER_H
#define TUTORIALMANAGER_H

#include <QObject>
#include <QString>
#include <QVector>

struct TutorialStep {
    QString text;
    QString location;
    double completion;
    QString targetElement;
};

class TutorialManager : public QObject {
    Q_OBJECT

    // expose fields to qml
    Q_PROPERTY(QString text READ text NOTIFY tutorialChanged)
    Q_PROPERTY(QString location READ location NOTIFY tutorialChanged)
    Q_PROPERTY(double completion READ completion NOTIFY tutorialChanged)
    Q_PROPERTY(QString targetElement READ targetElement NOTIFY tutorialChanged)

public:
    explicit TutorialManager(QObject *parent = nullptr);

    QString text() const;
    QString location() const;
    double completion() const;
    QString targetElement() const;

    Q_INVOKABLE void nextStep(bool is3DModeActive);
    Q_INVOKABLE void resetTutorial();

signals:
    void tutorialChanged();

private:
    void loadSteps();
    void updateCurrentState();

    // get completion saved state
    QString getSettingsFilePath() const;
    bool loadTutorialStatus();
    void saveTutorialStatus(bool completed);

    int m_currentStepIndex;
    QVector<TutorialStep> m_steps;

    // fallback
    QString m_currentText;
    QString m_currentLocation;
    double m_currentCompletion;

    bool m_isTutorialCompleted;
};

#endif