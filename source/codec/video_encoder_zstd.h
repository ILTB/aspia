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

#ifndef ASPIA_CODEC__VIDEO_ENCODER_ZSTD_H
#define ASPIA_CODEC__VIDEO_ENCODER_ZSTD_H

#include "base/aligned_memory.h"
#include "codec/scoped_zstd_stream.h"
#include "codec/video_encoder.h"
#include "desktop_capture/pixel_format.h"

namespace aspia {

class PixelTranslator;

class VideoEncoderZstd : public VideoEncoder
{
public:
    ~VideoEncoderZstd() = default;

    static VideoEncoderZstd* create(const PixelFormat& target_format, int compression_ratio);

    void encode(const DesktopFrame* frame, proto::desktop::VideoPacket* packet) override;

private:
    VideoEncoderZstd(std::unique_ptr<PixelTranslator> translator,
                     const PixelFormat& target_format,
                     int compression_ratio);
    void compressPacket(proto::desktop::VideoPacket* packet,
                        const uint8_t* input_data,
                        size_t input_size);

    // Client's pixel format
    PixelFormat target_format_;
    int compress_ratio_;
    ScopedZstdCStream stream_;
    std::unique_ptr<PixelTranslator> translator_;
    std::unique_ptr<uint8_t[], AlignedFreeDeleter> translate_buffer_;
    size_t translate_buffer_size_ = 0;

    DISALLOW_COPY_AND_ASSIGN(VideoEncoderZstd);
};

} // namespace aspia

#endif // ASPIA_CODEC__VIDEO_ENCODER_ZSTD_H
