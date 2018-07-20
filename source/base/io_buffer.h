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

#ifndef ASPIA_BASE__IO_BUFFER_H_
#define ASPIA_BASE__IO_BUFFER_H_

#include <cstdint>
#include <memory>

#include "base/macros_magic.h"

namespace aspia {

class IOBuffer
{
public:
    IOBuffer(size_t size)
        : size_(size),
          data_(std::make_unique<uint8_t[]>(size))
    {
        // Nothing
    }

    size_t size() const { return size_; }
    uint8_t* data() const { return data_.get(); }

private:
    std::unique_ptr<uint8_t[]> data_;
    const size_t size_;

    DISALLOW_COPY_AND_ASSIGN(IOBuffer);
};

} // namespace aspia

#endif // ASPIA_BASE__IO_BUFFER_H_
