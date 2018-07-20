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

#include "codec/pixel_translator.h"

#include "base/macros_magic.h"

namespace aspia {

namespace {

template<int kSourceBpp, int kTargetBpp>
class PixelTranslatorT : public PixelTranslator
{
public:
    PixelTranslatorT(const PixelFormat& source_format, const PixelFormat& target_format)
        : source_format_(source_format),
          target_format_(target_format)
    {
        red_table_ = std::make_unique<quint32[]>(source_format_.redMax() + 1);
        green_table_ = std::make_unique<quint32[]>(source_format_.greenMax() + 1);
        blue_table_ = std::make_unique<quint32[]>(source_format_.blueMax() + 1);

        for (quint32 i = 0; i <= source_format_.redMax(); ++i)
        {
            red_table_[i] = ((i * target_format_.redMax() + source_format_.redMax() / 2) /
                             source_format_.redMax()) << target_format_.redShift();
        }

        for (quint32 i = 0; i <= source_format_.greenMax(); ++i)
        {
            green_table_[i] = ((i * target_format_.greenMax() + source_format_.greenMax() / 2) /
                               source_format_.greenMax()) << target_format_.greenShift();
        }

        for (quint32 i = 0; i <= source_format_.blueMax(); ++i)
        {
            blue_table_[i] = ((i * target_format_.blueMax() + source_format_.blueMax() / 2) /
                              source_format_.blueMax()) << target_format_.blueShift();
        }
    }

    ~PixelTranslatorT() = default;

    void translate(const quint8* src, int src_stride,
                   quint8* dst, int dst_stride,
                   int width, int height) override
    {
        src_stride -= width * kSourceBpp;
        dst_stride -= width * kTargetBpp;

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                quint32 red;
                quint32 green;
                quint32 blue;

                if constexpr (kSourceBpp == 4)
                {
                    red = red_table_[
                        *(quint32*) src >> source_format_.redShift() & source_format_.redMax()];
                    green = green_table_[
                        *(quint32*) src >> source_format_.greenShift() & source_format_.greenMax()];
                    blue = blue_table_[
                        *(quint32*) src >> source_format_.blueShift() & source_format_.blueMax()];
                }
                else if constexpr (kSourceBpp == 2)
                {
                    red = red_table_[
                        *(quint16*) src >> source_format_.redShift() & source_format_.redMax()];
                    green = green_table_[
                        *(quint16*) src >> source_format_.greenShift() & source_format_.greenMax()];
                    blue = blue_table_[
                        *(quint16*) src >> source_format_.blueShift() & source_format_.blueMax()];
                }
                else
                {
                    static_assert(kSourceBpp == 1);

                    red = red_table_[
                        *(quint8*) src >> source_format_.redShift() & source_format_.redMax()];
                    green = green_table_[
                        *(quint8*) src >> source_format_.greenShift() & source_format_.greenMax()];
                    blue = blue_table_[
                        *(quint8*) src >> source_format_.blueShift() & source_format_.blueMax()];
                }

                if constexpr (kTargetBpp == 4)
                {
                    *(quint32*)dst = static_cast<quint32>(red | green | blue);
                }
                else if constexpr (kTargetBpp == 2)
                {
                    *(quint16*)dst = static_cast<quint16>(red | green | blue);
                }
                else
                {
                    static_assert(kTargetBpp == 1);
                    *(quint8*)dst = static_cast<quint8>(red | green | blue);
                }

                src += kSourceBpp;
                dst += kTargetBpp;
            }

            src += src_stride;
            dst += dst_stride;
        }
    }

private:
    std::unique_ptr<quint32[]> red_table_;
    std::unique_ptr<quint32[]> green_table_;
    std::unique_ptr<quint32[]> blue_table_;

    PixelFormat source_format_;
    PixelFormat target_format_;

    DISALLOW_COPY_AND_ASSIGN(PixelTranslatorT);
};

template<int kSourceBpp, int kTargetBpp>
class PixelTranslatorFrom8_16bppT : public PixelTranslator
{
public:
    PixelTranslatorFrom8_16bppT(const PixelFormat& source_format, const PixelFormat& target_format)
        : source_format_(source_format),
          target_format_(target_format)
    {
        size_t table_size;

        if constexpr (kSourceBpp == 2)
        {
            table_size = 65536;
        }
        else
        {
            static_assert(kSourceBpp == 1);
            table_size = 256;
        }

        table_ = std::make_unique<quint32[]>(table_size);

        quint32 source_red_mask = source_format.redMax() << source_format.redShift();
        quint32 source_green_mask = source_format.greenMax() << source_format.greenShift();
        quint32 source_blue_mask = source_format.blueMax() << source_format.blueShift();

        for (quint32 i = 0; i < table_size; ++i)
        {
            quint32 source_red = (i & source_red_mask) >> source_format.redShift();
            quint32 source_green = (i & source_green_mask) >> source_format.greenShift();
            quint32 source_blue = (i & source_blue_mask) >> source_format.blueShift();

            quint32 target_red =
                (source_red * target_format.redMax() / source_format.redMax()) << target_format.redShift();
            quint32 target_green =
                (source_green * target_format.greenMax() / source_format.greenMax()) << target_format.greenShift();
            quint32 target_blue =
                (source_blue * target_format.blueMax() / source_format.blueMax()) << target_format.blueShift();

            table_[i] = target_red | target_green | target_blue;
        }
    }

    ~PixelTranslatorFrom8_16bppT() = default;

    void translate(const quint8* src, int src_stride,
                   quint8* dst, int dst_stride,
                   int width, int height) override
    {
        src_stride -= width * kSourceBpp;
        dst_stride -= width * kTargetBpp;

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                quint32 target_pixel;

                if constexpr (kSourceBpp == 2)
                {
                    target_pixel = table_[*(quint16*)src];
                }
                else
                {
                    static_assert(kSourceBpp == 1);
                    target_pixel = table_[*(quint8*)src];
                }

                if constexpr (kTargetBpp == 4)
                {
                    *(quint32*)dst = static_cast<quint32>(target_pixel);
                }
                else if constexpr (kTargetBpp == 2)
                {
                    *(quint16*)dst = static_cast<quint16>(target_pixel);
                }
                else
                {
                    static_assert(kTargetBpp == 1);
                    *(quint8*)dst = static_cast<quint8>(target_pixel);
                }

                src += kSourceBpp;
                dst += kTargetBpp;
            }

            src += src_stride;
            dst += dst_stride;
        }
    }

private:
    std::unique_ptr<quint32[]> table_;

    PixelFormat source_format_;
    PixelFormat target_format_;

    DISALLOW_COPY_AND_ASSIGN(PixelTranslatorFrom8_16bppT);
};

} // namespace

// static
std::unique_ptr<PixelTranslator> PixelTranslator::create(const PixelFormat& source_format,
                                                         const PixelFormat& target_format)
{
    switch (target_format.bytesPerPixel())
    {
        case 4:
        {
            switch (source_format.bytesPerPixel())
            {
                case 4:
                    return std::make_unique<PixelTranslatorT<4, 4>>(source_format, target_format);

                case 2:
                    return std::make_unique<PixelTranslatorFrom8_16bppT<2, 4>>(source_format, target_format);

                case 1:
                    return std::make_unique<PixelTranslatorFrom8_16bppT<1, 4>>(source_format, target_format);
            }
        }
        break;

        case 2:
        {
            switch (source_format.bytesPerPixel())
            {
                case 4:
                    return std::make_unique<PixelTranslatorT<4, 2>>(source_format, target_format);

                case 2:
                    return std::make_unique<PixelTranslatorFrom8_16bppT<2, 2>>(source_format, target_format);

                case 1:
                    return std::make_unique<PixelTranslatorFrom8_16bppT<1, 2>>(source_format, target_format);
            }
        }
        break;

        case 1:
        {
            switch (source_format.bytesPerPixel())
            {
                case 4:
                    return std::make_unique<PixelTranslatorT<4, 1>>(source_format, target_format);

                case 2:
                    return std::make_unique<PixelTranslatorFrom8_16bppT<2, 1>>(source_format, target_format);

                case 1:
                    return std::make_unique<PixelTranslatorFrom8_16bppT<1, 1>>(source_format, target_format);
            }
        }
        break;
    }

    return nullptr;
}

} // namespace aspia
