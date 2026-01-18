//
// Created by greg on 2/16/24.
// Copyright (c) 2024 Presage Technologies
//

// === config header ===
#include <physiology/modules/configuration.h>
// === standard library includes (if any) ===
// === third-party includes (if any) ===
#ifdef WITH_OPENGL
// third-party GPU-only includes
#include <mediapipe/gpu/gpu_buffer.h>
#include <mediapipe/gpu/gpu_shared_data_internal.h>
#include <mediapipe/gpu/gl_calculator_helper.h>
#endif
#include <mediapipe/framework/formats/image_frame_opencv.h>
// === local includes (if any) ===
#include "image_transfer.hpp"



namespace presage::smartspectra::container::image_transfer {

template<>
absl::Status FeedFrameToGraph<platform_independence::DeviceType::Cpu>(
    std::unique_ptr<mediapipe::ImageFrame> input_frame,
    mediapipe::CalculatorGraph& graph,
    presage::platform_independence::DeviceContext<platform_independence::DeviceType::Cpu>& device_context,
    const int64_t& frame_timestamp,
    const char* video_stream
) {
    return graph.AddPacketToInputStream(
        video_stream, mediapipe::Adopt(input_frame.release()).At(mediapipe::Timestamp(frame_timestamp))
    );
}

template<>
absl::Status GetFrameFromPacket<platform_independence::DeviceType::Cpu>(
    cv::Mat& output_frame_rgb,
    presage::platform_independence::DeviceContext<platform_independence::DeviceType::Cpu>& device_context,
    const mediapipe::Packet& output_video_packet
) {
    auto& output_frame = output_video_packet.Get<mediapipe::ImageFrame>();
    // Convert back to opencv for display or saving.
    output_frame_rgb = mediapipe::formats::MatView(&output_frame);
    return absl::OkStatus();
}

#ifdef WITH_OPENGL
template<>
absl::Status FeedFrameToGraph<platform_independence::DeviceType::OpenGl>(
    std::unique_ptr<mediapipe::ImageFrame> input_frame,
    mediapipe::CalculatorGraph& graph,
    presage::platform_independence::DeviceContext<platform_independence::DeviceType::OpenGl>& device_context,
    const int64_t& frame_timestamp,
    const char* video_stream
) {
    return device_context.gpu_helper.RunInGlContext(
        [&input_frame, &frame_timestamp, &device_context, &graph, video_stream]() -> absl::Status {
            // Convert ImageFrame to GpuBuffer.
            auto texture = device_context.gpu_helper.CreateSourceTexture(*input_frame);
            auto gpu_frame = texture.GetFrame<mediapipe::GpuBuffer>();
            glFlush();
            texture.Release();
            // Send GPU image packet into the graph.
            MP_RETURN_IF_ERROR(
                graph.AddPacketToInputStream(
                    video_stream,
                    mediapipe::Adopt(gpu_frame.release()).At(mediapipe::Timestamp(frame_timestamp))
                )
            );
            return absl::OkStatus();
        }
    );
}


template<>
absl::Status GetFrameFromPacket<platform_independence::DeviceType::OpenGl>(
    cv::Mat& output_frame_rgb,
    presage::platform_independence::DeviceContext<platform_independence::DeviceType::OpenGl>& device_context,
    const mediapipe::Packet& output_video_packet
) {
    std::unique_ptr<mediapipe::ImageFrame> output_frame;

    // Convert GpuBuffer to ImageFrame.
    MP_RETURN_IF_ERROR(
        device_context.gpu_helper.RunInGlContext(
            [&output_video_packet, &output_frame, &device_context]() -> absl::Status {
                auto& gpu_frame = output_video_packet.Get<mediapipe::GpuBuffer>();
                auto texture = device_context.gpu_helper.CreateSourceTexture(gpu_frame);
                output_frame = absl::make_unique<mediapipe::ImageFrame>(
                    mediapipe::ImageFormatForGpuBufferFormat(gpu_frame.format()),
                    gpu_frame.width(), gpu_frame.height(),
                    mediapipe::ImageFrame::kGlDefaultAlignmentBoundary
                );
                device_context.gpu_helper.BindFramebuffer(texture);
                const auto info = mediapipe::GlTextureInfoForGpuBufferFormat(
                    gpu_frame.format(), 0, device_context.gpu_helper.GetGlVersion());
                glReadPixels(
                    0, 0, texture.width(), texture.height(), info.gl_format,
                    info.gl_type, output_frame->MutablePixelData());
                glFlush();
                texture.Release();
                return absl::OkStatus();
            }
        )
    );
    output_frame_rgb = mediapipe::formats::MatView(output_frame.get());
    return absl::OkStatus();
}
#endif

absl::Status FeedFrameToGraph(
    std::unique_ptr<mediapipe::ImageFrame> input_frame,
    mediapipe::CalculatorGraph& graph,
    const int64_t& frame_timestamp,
    const char* video_stream
) {
    static platform_independence::DeviceContext<platform_independence::DeviceType::Cpu> cpu_context;
    return FeedFrameToGraph<platform_independence::DeviceType::Cpu>(
        std::move(input_frame),
        graph,
        cpu_context,
        frame_timestamp,
        video_stream
    );
}

absl::Status GetFrameFromPacket(
    cv::Mat& output_frame_rgb,
    const mediapipe::Packet& output_video_packet
) {
    static platform_independence::DeviceContext<platform_independence::DeviceType::Cpu> cpu_context;
    return GetFrameFromPacket<platform_independence::DeviceType::Cpu>(
        output_frame_rgb,
        cpu_context,
        output_video_packet
    );
}

} // namespace presage::smartspectra::container::image_transfer
