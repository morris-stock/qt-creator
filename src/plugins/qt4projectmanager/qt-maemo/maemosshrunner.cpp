/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Creator.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "maemosshrunner.h"

#include "maemodeploystep.h"
#include "maemodeviceconfigurations.h"
#include "maemoglobal.h"
#include "maemoremotemounter.h"
#include "maemoremotemountsmodel.h"
#include "maemorunconfiguration.h"

#include <coreplugin/ssh/sshconnection.h>
#include <coreplugin/ssh/sshremoteprocess.h>

#include <QtCore/QFileInfo>

#include <limits>

#define ASSERT_STATE(state) assertState(state, Q_FUNC_INFO)

using namespace Core;

namespace Qt4ProjectManager {
namespace Internal {

MaemoSshRunner::MaemoSshRunner(QObject *parent,
    MaemoRunConfiguration *runConfig, bool debugging)
    : QObject(parent), m_runConfig(runConfig),
      m_mounter(new MaemoRemoteMounter(this)),
      m_devConfig(runConfig->deviceConfig()),
      m_debugging(debugging), m_state(Inactive)
{
    m_procsToKill
        << QFileInfo(m_runConfig->localExecutableFilePath()).fileName()
        << QLatin1String("utfs-client");
    if (debugging)
        m_procsToKill << QLatin1String("gdbserver");
    connect(m_mounter, SIGNAL(mounted()), this, SLOT(handleMounted()));
    connect(m_mounter, SIGNAL(unmounted()), this, SLOT(handleUnmounted()));
    connect(m_mounter, SIGNAL(error(QString)), this,
        SLOT(handleMounterError(QString)));
    connect(m_mounter, SIGNAL(reportProgress(QString)), this,
        SIGNAL(reportProgress(QString)));
    connect(m_mounter, SIGNAL(debugOutput(QString)), this,
        SIGNAL(mountDebugOutput(QString)));
}

MaemoSshRunner::~MaemoSshRunner() {}

void MaemoSshRunner::start()
{
    ASSERT_STATE(QList<State>() << Inactive << StopRequested);

    setState(Connecting);
    m_exitStatus = -1;
    if (m_connection)
        disconnect(m_connection.data(), 0, this, 0);
    m_connection = m_runConfig->deployStep()->sshConnection();
    const bool reUse = isConnectionUsable();
    if (!reUse)
        m_connection = SshConnection::create();
    connect(m_connection.data(), SIGNAL(connected()), this,
        SLOT(handleConnected()));
    connect(m_connection.data(), SIGNAL(error(Core::SshError)), this,
        SLOT(handleConnectionFailure()));
    if (reUse) {        
        handleConnected();
    } else {
        emit reportProgress(tr("Connecting to device..."));
        m_connection->connectToHost(m_devConfig.server);
    }
}

void MaemoSshRunner::stop()
{
    if (m_state == PostRunCleaning || m_state == StopRequested
        || m_state == Inactive)
        return;

    setState(StopRequested);
    cleanup();
}

void MaemoSshRunner::handleConnected()
{
    ASSERT_STATE(QList<State>() << Connecting << StopRequested);
    if (m_state == StopRequested) {
        setState(Inactive);
    } else {
        setState(PreRunCleaning);
        cleanup();
    }
}

void MaemoSshRunner::handleConnectionFailure()
{
    if (m_state == Inactive)
        qWarning("Unexpected state %d in %s.", m_state, Q_FUNC_INFO);

    const QString errorTemplate = m_state == Connecting
        ? tr("Could not connect to host: %1") : tr("Connection failed: %1");
    emitError(errorTemplate.arg(m_connection->errorString()));
}

void MaemoSshRunner::cleanup()
{
    ASSERT_STATE(QList<State>() << PreRunCleaning << PostRunCleaning
        << StopRequested);

    emit reportProgress(tr("Killing remote process(es)..."));

    // pkill behaves differently on Fremantle and Harmattan.
    const char *const killTemplate = "pkill -%2 '^%1$'; pkill -%2 '/%1$';";
    QString niceKill;
    QString brutalKill;
    foreach (const QString &proc, m_procsToKill) {
        niceKill += QString::fromLocal8Bit(killTemplate).arg(proc).arg("SIGTERM");
        brutalKill += QString::fromLocal8Bit(killTemplate).arg(proc).arg("SIGKILL");
    }
    QString remoteCall = niceKill + QLatin1String("sleep 1; ") + brutalKill;
    remoteCall.remove(remoteCall.count() - 1, 1); // Get rid of trailing semicolon.

    m_cleaner = m_connection->createRemoteProcess(remoteCall.toUtf8());
    connect(m_cleaner.data(), SIGNAL(closed(int)), this,
        SLOT(handleCleanupFinished(int)));
    m_cleaner->start();
}

void MaemoSshRunner::handleCleanupFinished(int exitStatus)
{
    Q_ASSERT(exitStatus == SshRemoteProcess::FailedToStart
        || exitStatus == SshRemoteProcess::KilledBySignal
        || exitStatus == SshRemoteProcess::ExitedNormally);

    ASSERT_STATE(QList<State>() << PreRunCleaning << PostRunCleaning
        << StopRequested << Inactive);

    if (m_state == Inactive)
        return;
    if (m_state == StopRequested || m_state == PostRunCleaning) {
        unmount();
        return;
    }

    if (exitStatus != SshRemoteProcess::ExitedNormally) {
        emitError(tr("Initial cleanup failed: %1")
            .arg(m_cleaner->errorString()));
    } else {
        m_mounter->setConnection(m_connection);
        unmount();
    }
}

void MaemoSshRunner::handleUnmounted()
{
    ASSERT_STATE(QList<State>() << PreRunCleaning << PreMountUnmounting
        << PostRunCleaning << StopRequested);

    switch (m_state) {
    case PreRunCleaning: {
        m_mounter->resetMountSpecifications();
        MaemoPortList portList = m_devConfig.freePorts();
        if (m_debugging) { // gdbserver and QML inspector need one port each.
            MaemoRunConfiguration::DebuggingType debuggingType
                = m_runConfig->debuggingType();
            if (debuggingType != MaemoRunConfiguration::DebugQmlOnly
                    && !m_runConfig->useRemoteGdb())
                portList.getNext();
            if (debuggingType != MaemoRunConfiguration::DebugCppOnly)
                portList.getNext();
        }
        m_mounter->setToolchain(m_runConfig->toolchain());
        m_mounter->setPortList(portList);
        const MaemoRemoteMountsModel * const remoteMounts
            = m_runConfig->remoteMounts();
        for (int i = 0; i < remoteMounts->mountSpecificationCount(); ++i) {
            if (!addMountSpecification(remoteMounts->mountSpecificationAt(i)))
                return;
        }
        if (m_debugging && m_runConfig->useRemoteGdb()) {
            if (!addMountSpecification(MaemoMountSpecification(
                m_runConfig->localDirToMountForRemoteGdb(),
                MaemoGlobal::remoteProjectSourcesMountPoint())))
                return;
        }
        setState(PreMountUnmounting);
        unmount();
        break;
    }
    case PreMountUnmounting:
        mount();
        break;
    case PostRunCleaning:
    case StopRequested:
        m_mounter->resetMountSpecifications();
        if (m_state == StopRequested) {
            emit remoteProcessFinished(InvalidExitCode);
        } else if (m_exitStatus == SshRemoteProcess::ExitedNormally) {
            emit remoteProcessFinished(m_runner->exitCode());
        } else {
            emit error(tr("Error running remote process: %1")
                .arg(m_runner->errorString()));
        }
        setState(Inactive);
        break;
    default: ;
    }
}

void MaemoSshRunner::handleMounted()
{
    ASSERT_STATE(QList<State>() << Mounting << StopRequested);

    if (m_state == Mounting) {
        setState(ReadyForExecution);
        emit readyForExecution();
    }
}

void MaemoSshRunner::handleMounterError(const QString &errorMsg)
{
    ASSERT_STATE(QList<State>() << PreRunCleaning << PostRunCleaning
        << PreMountUnmounting << Mounting << StopRequested << Inactive);

    emitError(errorMsg);
}

void MaemoSshRunner::startExecution(const QByteArray &remoteCall)
{
    ASSERT_STATE(ReadyForExecution);

    if (m_runConfig->remoteExecutableFilePath().isEmpty()) {
        emitError(tr("Cannot run: No remote executable set."));
        return;
    }

    m_runner = m_connection->createRemoteProcess(remoteCall);
    connect(m_runner.data(), SIGNAL(started()), this,
        SIGNAL(remoteProcessStarted()));
    connect(m_runner.data(), SIGNAL(closed(int)), this,
        SLOT(handleRemoteProcessFinished(int)));
    connect(m_runner.data(), SIGNAL(outputAvailable(QByteArray)), this,
        SIGNAL(remoteOutput(QByteArray)));
    connect(m_runner.data(), SIGNAL(errorOutputAvailable(QByteArray)), this,
        SIGNAL(remoteErrorOutput(QByteArray)));
    setState(ProcessStarting);
    m_runner->start();
}

void MaemoSshRunner::handleRemoteProcessFinished(int exitStatus)
{
    Q_ASSERT(exitStatus == SshRemoteProcess::FailedToStart
        || exitStatus == SshRemoteProcess::KilledBySignal
        || exitStatus == SshRemoteProcess::ExitedNormally);
    ASSERT_STATE(QList<State>() << ProcessStarting << StopRequested << Inactive);

    m_exitStatus = exitStatus;
    if (m_state != StopRequested && m_state != Inactive) {
        setState(PostRunCleaning);
        cleanup();
    }
}

bool MaemoSshRunner::addMountSpecification(const MaemoMountSpecification &mountSpec)
{
    if (!m_mounter->addMountSpecification(mountSpec, false)) {
        emitError(tr("The device does not have enough free ports "
            "for this run configuration."));
        return false;
    }
    return true;
}

bool MaemoSshRunner::isConnectionUsable() const
{
    return m_connection && m_connection->state() == SshConnection::Connected
        && m_connection->connectionParameters() == m_devConfig.server;
}

void MaemoSshRunner::assertState(State expectedState, const char *func)
{
    assertState(QList<State>() << expectedState, func);
}

void MaemoSshRunner::assertState(const QList<State> &expectedStates,
    const char *func)
{
    if (!expectedStates.contains(m_state))
        qWarning("Unexpected state %d at %s.", m_state, func);
}

void MaemoSshRunner::setState(State newState)
{
    if (newState == Inactive) {
        m_mounter->setConnection(SshConnection::Ptr());
        if (m_connection) {
            disconnect(m_connection.data(), 0, this, 0);
            m_connection = SshConnection::Ptr();
        }
        if (m_cleaner)
            disconnect(m_cleaner.data(), 0, this, 0);
    }
    m_state = newState;
}

void MaemoSshRunner::emitError(const QString &errorMsg)
{
    if (m_state != Inactive) {
        emit error(errorMsg);
        setState(Inactive);
    }
}

void MaemoSshRunner::mount()
{
    setState(Mounting);
    if (m_mounter->hasValidMountSpecifications()) {
        emit reportProgress(tr("Mounting host directories..."));
        m_mounter->mount();
    } else {
        handleMounted();
    }
}

void MaemoSshRunner::unmount()
{
    ASSERT_STATE(QList<State>() << PreRunCleaning << PreMountUnmounting
        << PostRunCleaning << StopRequested);
    if (m_mounter->hasValidMountSpecifications()) {
        QString message;
        switch (m_state) {
        case PreRunCleaning:
            message = tr("Unmounting left-over host directory mounts...");
            break;
        case PreMountUnmounting:
            message = tr("Potentially unmounting left-over host directory mounts...");
        case StopRequested: case PostRunCleaning:
            message = tr("Unmounting host directories...");
            break;
        default:
            break;
        }
        emit reportProgress(message);
        m_mounter->unmount();
    } else {
        handleUnmounted();
    }
}


const qint64 MaemoSshRunner::InvalidExitCode
    = std::numeric_limits<qint64>::min();

} // namespace Internal
} // namespace Qt4ProjectManager

