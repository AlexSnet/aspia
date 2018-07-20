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

#ifndef ASPIA_HOST__HOST_SESSION_FILE_TRANSFER_H_
#define ASPIA_HOST__HOST_SESSION_FILE_TRANSFER_H_

#include "base/macros_magic.h"
#include "host/host_session.h"

namespace aspia {

class FileWorker;

class HostSessionFileTransfer : public HostSession
{
    Q_OBJECT

public:
    explicit HostSessionFileTransfer(const QString& channel_id);
    ~HostSessionFileTransfer() = default;

public slots:
    // HostSession implementation.
    void messageReceived(const QByteArray& buffer) override;
    void messageWritten(int message_id) override;

protected:
    void startSession() override;
    void stopSession() override;

private:
    QPointer<FileWorker> worker_;

    DISALLOW_COPY_AND_ASSIGN(HostSessionFileTransfer);
};

} // namespace aspia

#endif // ASPIA_HOST__HOST_SESSION_FILE_TRANSFER_H_
