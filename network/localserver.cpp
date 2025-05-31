#include <QLocalSocket>

#include "network/localserver.h"

LocalServer::LocalServer(QObject* pParent)
    : NetworkInterface(pParent)
{
    setObjectName("LocalServer");
    m_isServer = true;
    m_isConnected = true;
}

LocalServer::~LocalServer()
{
    disconnect();
    LocalServer::disconnectTCP();
    CONSOLE_PRINT("Server is closed", GameConsole::eLogLevels::eDEBUG);
}

void LocalServer::connectTCP(QString primaryAdress, quint16 port, QString secondaryAdress, bool sendAll)
{
    m_pTCPServer = MemoryManagement::create<QLocalServer>(this);
    m_pTCPServer->listen(primaryAdress);
    connect(m_pTCPServer.get(), &QLocalServer::newConnection, this, &LocalServer::onConnect, Qt::QueuedConnection);
    connect(this, &LocalServer::sigDisconnectClient, this, &LocalServer::disconnectClient, Qt::QueuedConnection);
    connect(this, &LocalServer::sigDisconnectTCP, this, &LocalServer::disconnectTCP, Qt::QueuedConnection);
    connect(this, &LocalServer::sigForwardData, this, &LocalServer::forwardData, Qt::QueuedConnection);
    connect(this, &LocalServer::sigContinueListening, this, &LocalServer::continueListening, Qt::QueuedConnection);
    connect(this, &LocalServer::sigPauseListening, this, &LocalServer::pauseListening, Qt::QueuedConnection);
    CONSOLE_PRINT("Local Server is running. " + primaryAdress, GameConsole::eLogLevels::eDEBUG);
}

void LocalServer::disconnectTCP()
{
    while (m_pTCPSockets.size() > 0)
    {
        if (m_pTCPSockets[0]->isOpen())
        {
            // realize correct deletion
            m_pTCPSockets[0]->disconnect();
            m_pTCPSockets[0]->close();
            CONSOLE_PRINT("Client disconnected.", GameConsole::eLogLevels::eDEBUG);
        }
        m_pRXTasks.removeAt(0);
        m_pTXTasks.removeAt(0);
        m_pTCPSockets.removeAt(0);
    }
    if (m_pTCPServer != nullptr)
    {
        m_pTCPServer->close();
        m_pTCPServer.reset();
    }
}

void LocalServer::disconnectClient(quint64 socketID)
{
    for (qint32 i = 0; i < m_SocketIDs.size(); i++)
    {
        if (m_SocketIDs[i] == socketID)
        {
            CONSOLE_PRINT("Local Server Client disconnected.", GameConsole::eLogLevels::eDEBUG);
            if (m_pTCPSockets[i]->isOpen())
            {
                // realize correct deletion
                m_pTCPSockets[i]->disconnect(this);
                m_pTCPSockets[i]->disconnectFromServer();
                m_pTCPSockets[i]->close();
            }
            m_pRXTasks.removeAt(i);
            m_pTXTasks.removeAt(i);
            m_pTCPSockets.removeAt(i);
            emit sigDisconnected(socketID);
            break;
        }
    }
}

void LocalServer::onConnect()
{
    if (m_pTCPServer != nullptr)
    {
        QLocalSocket* nextSocket = m_pTCPServer->nextPendingConnection();
        m_pTCPSockets.append(nextSocket);
        connect(nextSocket, &QLocalSocket::errorOccurred, this, &LocalServer::displayLocalError, Qt::QueuedConnection);
        m_idCounter++;
        // Start RX-Task
        spRxTask pRXTask = MemoryManagement::create<RxTask>(nextSocket, m_idCounter, this, true);
        m_pRXTasks.append(pRXTask);
        connect(nextSocket, &QLocalSocket::readyRead, pRXTask.get(), &RxTask::recieveData, Qt::QueuedConnection);

        // start TX-Task
        spTxTask pTXTask = MemoryManagement::create<TxTask>(nextSocket, m_idCounter, this, true);
        m_pTXTasks.append(pTXTask);
        connect(this, &LocalServer::sig_sendData, pTXTask.get(), &TxTask::send, Qt::QueuedConnection);
        quint64 socket = m_idCounter;
        connect(nextSocket, &QLocalSocket::disconnected, this, [this, socket]()
        {
            emit sigDisconnectClient(socket);
        });
        CONSOLE_PRINT("Client connection.", GameConsole::eLogLevels::eDEBUG);
        emit sigConnected(m_idCounter);
    }
}

void LocalServer::forwardData(quint64 socketID, QByteArray data, NetworkInterface::NetworkSerives service)
{
    CONSOLE_PRINT("Forwarding data from local server to all clients except " + QString::number(socketID), GameConsole::eDEBUG);
    for (qint32 i = 0; i < m_SocketIDs.size(); i++)
    {
        if (m_SocketIDs[i] != socketID)
        {
            for (qint32 i2 = 0; i2 < m_pTXTasks.size(); i2++)
            {
                m_pTXTasks[i2]->send(m_SocketIDs[i], data, service, false);
            }
        }
    }
}

void LocalServer::pauseListening()
{
}

void LocalServer::continueListening()
{
}

QVector<quint64> LocalServer::getConnectedSockets()
{
    return m_SocketIDs;
}

void LocalServer::changeThread(quint64 socketID, QThread*)
{
    CONSOLE_PRINT("Unsupported call to change thread on local server for socekt " + QString::number(socketID), GameConsole::eFATAL);
}

void LocalServer::addSocket(quint64 socket)
{
    CONSOLE_PRINT("Local Server added socket " + QString::number(socket), GameConsole::eLogLevels::eDEBUG);
    m_SocketIDs.append(socket);
}

void LocalServer::removeSocket(quint64 socket)
{
    CONSOLE_PRINT("Local Server removed socket " + QString::number(socket), GameConsole::eLogLevels::eDEBUG);
    m_SocketIDs.removeAll(socket);
}
