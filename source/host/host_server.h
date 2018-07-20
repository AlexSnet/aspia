//
// Aspia Project
// Copyright (C) 2018 Dmitry Chapyshev <dmitry@aspia.ru>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
//

#ifndef ASPIA_HOST__HOST_SERVER_H_
#define ASPIA_HOST__HOST_SERVER_H_

#include "host/win/host_process.h"
#include "host/user.h"
#include "ipc/ipc_channel.h"
#include "network/network_server.h"

namespace aspia {

class Host;
class HostUserAuthorizer;

class HostServer : public QObject
{
    Q_OBJECT

public:
    HostServer(QObject* parent = nullptr);
    ~HostServer();

    bool start(int port, const QList<User>& user_list);
    void stop();
    void setSessionChanged(quint32 event, quint32 session_id);

signals:
    void sessionChanged(quint32 event, quint32 session_id);

protected:
    // QObject implementation.
    void timerEvent(QTimerEvent* event) override;

private slots:
    void onNewConnection();
    void onAuthorizationFinished(HostUserAuthorizer* authorizer);
    void onHostFinished(Host* host);
    void onIpcServerStarted(const QString& channel_id);
    void onIpcNewConnection(IpcChannel* channel);
    void onIpcMessageReceived(const QByteArray& buffer);
    void onNotifierProcessError(HostProcess::ErrorCode error_code);
    void restartNotifier();

private:
    enum class NotifierState
    {
        Stopped,
        Starting,
        Started
    };

    void startNotifier();
    void stopNotifier();
    void sessionToNotifier(const Host& host);
    void sessionCloseToNotifier(const Host& host);

    // Accepts incoming network connections.
    QPointer<NetworkServer> network_server_;

    // Contains the status of the notifier process.
    NotifierState notifier_state_ = NotifierState::Stopped;

    // Starts and monitors the status of the notifier process.
    QPointer<HostProcess> notifier_process_;

    // The channel is used to communicate with the notifier process.
    QPointer<IpcChannel> ipc_channel_;

    // Contains a list of users for authorization.
    QList<User> user_list_;

    // Contains a list of connected sessions.
    QList<QPointer<Host>> session_list_;

    int restart_timer_id_ = 0;

    DISALLOW_COPY_AND_ASSIGN(HostServer);
};

} // namespace aspia

#endif // ASPIA_HOST__HOST_SERVER_H_
