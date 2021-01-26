/****************************************************************************
**
** Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebSockets module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

/*!
    \class QSslServer

    \inmodule QtWebSockets

    \brief Implements a secure TCP server over SSL.

    \internal
*/

#include "qsslserver_p.h"

#include <QtNetwork/QSslSocket>
#include <QtNetwork/QSslCipher>

QT_BEGIN_NAMESPACE

/*!
    Constructs a new QSslServer with the given \a parent.

    \internal
*/
QSslServer::QSslServer(QObject *parent) :
    QTcpServer(parent),
    m_sslConfiguration(QSslConfiguration::defaultConfiguration())
{
}

/*!
    Destroys the QSslServer.

    All open connections are closed.

    \internal
*/
QSslServer::~QSslServer()
{
}

/*!
    Sets the \a sslConfiguration to use.

    \sa QSslSocket::setSslConfiguration()

    \internal
*/
void QSslServer::setSslConfiguration(const QSslConfiguration &sslConfiguration)
{
    m_sslConfiguration = sslConfiguration;
}

/*!
    Returns the current ssl configuration.

    \internal
*/
QSslConfiguration QSslServer::sslConfiguration() const
{
    return m_sslConfiguration;
}

/*!
    Called when a new connection is established.

    Converts \a socket to a QSslSocket.

    \internal
*/
void QSslServer::incomingConnection(qintptr socket)
{
    QSslSocket *pSslSocket = new QSslSocket(this);

    if (Q_LIKELY(pSslSocket)) {
        pSslSocket->setSslConfiguration(m_sslConfiguration);

        if (Q_LIKELY(pSslSocket->setSocketDescriptor(socket))) {
            connect(pSslSocket, &QSslSocket::peerVerifyError, this, &QSslServer::peerVerifyError);

            connect(pSslSocket, QOverload<const QList<QSslError>&>::of(&QSslSocket::sslErrors),
                    this, &QSslServer::sslErrors);
            connect(pSslSocket, &QSslSocket::encrypted,
                    this, &QSslServer::socketEncrypted);
            connect(pSslSocket, &QSslSocket::preSharedKeyAuthenticationRequired,
                    this, &QSslServer::preSharedKeyAuthenticationRequired);

            Q_EMIT startedEncryptionHandshake(pSslSocket);

            pSslSocket->startServerEncryption();
        } else {
           delete pSslSocket;
        }
    }
}

void QSslServer::socketEncrypted()
{
    QSslSocket *pSslSocket = qobject_cast<QSslSocket *>(sender());

    // We do not add the connection until the encryption handshake is complete.
    // In case the handshake is aborted, we would be left with a stale
    // connection in the queue otherwise.
    addPendingConnection(pSslSocket);
    Q_EMIT newEncryptedConnection();
}

QT_END_NAMESPACE
