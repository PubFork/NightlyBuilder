#ifndef NBSTATEPULLING_H
#define NBSTATEPULLING_H

#include "state.h"
#include <QProcess>

class QTimer;

class NBStatePulling : public NBState
{
    Q_OBJECT
public:
    explicit NBStatePulling(QObject *parent = 0);
    ~NBStatePulling();

    void run();
    void shutUp();

public slots:
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void processError(QProcess::ProcessError error);
    void timeout();

private:
    QProcess *m_git;
    QTimer *m_waitTimer;
    bool m_running;
    bool m_isError;
};

#endif // NBSTATEPULLING_H
