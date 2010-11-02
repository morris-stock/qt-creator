/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Commercial Usage
**
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://qt.nokia.com/contact.
**
**************************************************************************/

#include "remoteplaingdbadapter.h"

#include <debugger/debuggeractions.h>
#include <debugger/debuggerstringutils.h>
#include <utils/qtcassert.h>


namespace Debugger {
namespace Internal {

RemotePlainGdbAdapter::RemotePlainGdbAdapter(GdbEngine *engine, QObject *parent)
    : AbstractPlainGdbAdapter(engine, parent),
      m_gdbProc(engine->startParameters().connParams, this)
{
    connect(&m_gdbProc, SIGNAL(started()), this, SLOT(handleGdbStarted()));
    connect(&m_gdbProc, SIGNAL(startFailed()), this,
        SLOT(handleGdbStartFailed()));
}

void RemotePlainGdbAdapter::startAdapter()
{
    QTC_ASSERT(state() == EngineSetupRequested, qDebug() << state());
    showMessage(QLatin1String("TRYING TO START ADAPTER"));

    emit requestSetup();
}

void RemotePlainGdbAdapter::setupInferior()
{
    AbstractPlainGdbAdapter::setupInferior();
    m_engine->postCommand("directory "
        + m_engine->startParameters().remoteSourcesDir);
}

void RemotePlainGdbAdapter::interruptInferior()
{
    m_gdbProc.interruptInferior();
}

QByteArray RemotePlainGdbAdapter::execFilePath() const
{
    return startParameters().executable.toUtf8();
}

bool RemotePlainGdbAdapter::infoTargetNecessary() const
{
    return true;
}

QByteArray RemotePlainGdbAdapter::toLocalEncoding(const QString &s) const
{
    return s.toUtf8();
}

QString RemotePlainGdbAdapter::fromLocalEncoding(const QByteArray &b) const
{
    return QString::fromUtf8(b);
}

void RemotePlainGdbAdapter::handleApplicationOutput(const QByteArray &output)
{
    showMessage(QString::fromUtf8(output), AppOutput);
}

void RemotePlainGdbAdapter::shutdownInferior()
{
    m_engine->defaultInferiorShutdown("kill");
}

void RemotePlainGdbAdapter::shutdownAdapter()
{
    m_engine->notifyAdapterShutdownOk();
}

void RemotePlainGdbAdapter::handleSetupDone(int qmlPort)
{
    QTC_ASSERT(state() == EngineSetupRequested, qDebug() << state());

    if (qmlPort != -1)
        startParameters().qmlServerPort = qmlPort;
    if (!startParameters().workingDirectory.isEmpty())
        m_gdbProc.setWorkingDirectory(startParameters().workingDirectory);
    if (!startParameters().environment.isEmpty())
        m_gdbProc.setEnvironment(startParameters().environment);
    m_gdbProc.realStart(m_engine->startParameters().debuggerCommand,
        QStringList() << QLatin1String("-i") << QLatin1String("mi"),
        m_engine->startParameters().executable);
}

void RemotePlainGdbAdapter::handleGdbStarted()
{
    if (m_engine->startGdb(QStringList(),
            m_engine->startParameters().debuggerCommand))
        m_engine->handleAdapterStarted();
}

void RemotePlainGdbAdapter::handleGdbStartFailed()
{
    m_engine->handleAdapterStartFailed(m_gdbProc.errorString());
}

void RemotePlainGdbAdapter::handleSetupFailed(const QString &reason)
{
    QTC_ASSERT(state() == EngineSetupRequested, qDebug() << state());

    m_engine->handleAdapterStartFailed(reason);
}

} // namespace Internal
} // namespace Debugger
