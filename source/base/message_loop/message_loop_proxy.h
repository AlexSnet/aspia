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

#ifndef ASPIA_BASE__MESSAGE_LOOP__MESSAGE_LOOP_PROXY_H_
#define ASPIA_BASE__MESSAGE_LOOP__MESSAGE_LOOP_PROXY_H_

#include "base/message_loop/message_loop.h"
#include "base/message_loop/pending_task.h"

namespace aspia {

class MessageLoopProxy
{
public:
    static std::shared_ptr<MessageLoopProxy> current();

    bool postTask(PendingTask::Callback callback);
    bool postDelayedTask(PendingTask::Callback callback,
                         const std::chrono::milliseconds& delay);
    bool postQuit();
    bool belongsToCurrentThread() const;

private:
    friend class MessageLoop;

    explicit MessageLoopProxy(MessageLoop* loop);

    // Called directly by MessageLoop::~MessageLoop.
    void willDestroyCurrentMessageLoop();

    MessageLoop* loop_;
    mutable std::mutex loop_lock_;

    DISALLOW_COPY_AND_ASSIGN(MessageLoopProxy);
};

} // namespace aspia

#endif // ASPIA_BASE__MESSAGE_LOOP__MESSAGE_LOOP_PROXY_H_
