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

#ifndef ASPIA_DESKTOP_CAPTURE__CURSOR_CAPTURER_WIN_H_
#define ASPIA_DESKTOP_CAPTURE__CURSOR_CAPTURER_WIN_H_

#include "desktop_capture/cursor_capturer.h"

#include <qt_windows.h>
#include <memory>

#include "base/macros_magic.h"

namespace aspia {

class ScopedGetDC;

class CursorCapturerWin : public CursorCapturer
{
public:
    CursorCapturerWin();
    ~CursorCapturerWin() = default;

    MouseCursor* captureCursor() override;

private:
    std::unique_ptr<ScopedGetDC> desktop_dc_;
    CURSORINFO prev_cursor_info_;

    DISALLOW_COPY_AND_ASSIGN(CursorCapturerWin);
};

} // namespace aspia

#endif // ASPIA_DESKTOP_CAPTURE__CURSOR_CAPTURER_WIN_H_
