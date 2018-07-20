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

#ifndef ASPIA_DESKTOP_CAPTURE__WIN__SCREEN_CAPTURE_UTILS_H_
#define ASPIA_DESKTOP_CAPTURE__WIN__SCREEN_CAPTURE_UTILS_H_

#include <QRect>
#include <QString>
#include <QVector>

#include "base/macros_magic.h"
#include "desktop_capture/screen_capturer.h"

namespace aspia {

class ScreenCaptureUtils
{
public:
    // Output the list of active screens into |screens|. Returns true if succeeded, or false if it
    // fails to enumerate the display devices.
    static bool screenList(ScreenCapturer::ScreenList* screens);

    // Returns true if |screen| is a valid screen. The screen device key is returned through
    // |device_key| if the screen is valid. The device key can be used in screenRect to verify the
    // screen matches the previously obtained id.
    static bool isScreenValid(ScreenCapturer::ScreenId screen, QString* device_key);

    // Get the rect of the entire system in system coordinate system. I.e. the primary monitor
    // always starts from (0, 0).
    static QRect fullScreenRect();

    // Get the rect of the screen identified by |screen|, relative to the primary display's
    // top-left.
    // If the screen device key does not match |device_key|, or the screen does not exist, or any
    // error happens, an empty rect is returned.
    static QRect screenRect(ScreenCapturer::ScreenId screen, const QString& device_key);

private:
    DISALLOW_COPY_AND_ASSIGN(ScreenCaptureUtils);
};

} // namespace aspia

#endif // ASPIA_DESKTOP_CAPTURE__WIN__SCREEN_CAPTURE_UTILS_H_
