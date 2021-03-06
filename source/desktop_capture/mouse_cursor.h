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

#ifndef ASPIA_DESKTOP_CAPTURE__MOUSE_CURSOR_H_
#define ASPIA_DESKTOP_CAPTURE__MOUSE_CURSOR_H_

#include <memory>

#include "desktop_capture/desktop_geometry.h"

namespace aspia {

class MouseCursor
{
public:
    MouseCursor(std::unique_ptr<uint8_t[]> data,
                const DesktopSize& size,
                const DesktopPoint& hotspot);
    ~MouseCursor() = default;

    const DesktopSize& size() const { return size_; }
    const DesktopPoint& hotSpot() const { return hotspot_; }
    uint8_t* data() const { return data_.get(); }

    int stride() const;

    bool isEqual(const MouseCursor& other);

private:
    std::unique_ptr<uint8_t[]> const data_;
    const DesktopSize size_;
    const DesktopPoint hotspot_;
};

} // namespace aspia

#endif // ASPIA_DESKTOP_CAPTURE__MOUSE_CURSOR_H_
