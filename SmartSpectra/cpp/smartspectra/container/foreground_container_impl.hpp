//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// foreground_container_impl.h
// Created by Greg on 4/29/2024.
// Copyright (C) 2024 Presage Security, Inc.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
// === standard library includes (if any) ===
#include <thread>
// === third-party includes (if any) ===
#include <mediapipe/framework/port/opencv_imgproc_inc.h>
#include <mediapipe/framework/port/opencv_highgui_inc.h>
#include <mediapipe/framework/formats/image_frame.h>
#include <mediapipe/framework/formats/image_frame_opencv.h>
#include <physiology/graph/stream_and_packet_names.h>
// === local includes (if any) ===
#include "foreground_container.hpp"
#include "initialization.hpp"
#include "image_transfer.hpp"
#include "packet_helpers.hpp"
#include "benchmarking.hpp"
#include "keyboard_input.hpp"
#include <smartspectra/video_source/factory.hpp>


namespace presage::smartspectra::container {

namespace pe = physiology::edge;
namespace pi = platform_independence;
namespace init = initialization;
namespace ph = packet_helpers;
namespace keys = keyboard_input;
namespace it = image_transfer;
namespace bench = benchmarking;
using json = nlohmann::json;


template<platform_independence::DeviceType TDeviceType, settings::OperationMode TOperationMode, settings::IntegrationMode TIntegrationMode>
ForegroundContainer<TDeviceType, TOperationMode, TIntegrationMode>::ForegroundContainer(
    SettingsType settings
): Base(settings),
   load_video(!this->settings.video_source.input_video_path.empty()),
   video_source(nullptr), keep_grabbing_frames(false) {}

template<platform_independence::DeviceType TDeviceType, settings::OperationMode TOperationMode, settings::IntegrationMode TIntegrationMode>
std::string ForegroundContainer<TDeviceType, TOperationMode, TIntegrationMode>::GenerateGuiWindowName() {
    return "Presage SmartSpectra C++ SDK "
           "[device: " + pi::AbslUnparseFlag(TDeviceType) +
           "; operation mode: " + settings::AbslUnparseFlag(TOperationMode) +
           "; integration mode: " + settings::AbslUnparseFlag(TIntegrationMode) + "]";
}

template<platform_independence::DeviceType TDeviceType, settings::OperationMode TOperationMode, settings::IntegrationMode TIntegrationMode>
const std::string ForegroundContainer<TDeviceType, TOperationMode, TIntegrationMode>::kWindowName = ForegroundContainer<
    TDeviceType,
    TOperationMode,
    TIntegrationMode>::GenerateGuiWindowName();
/**
 * Called from Run() at each frame iteration.
 * @tparam TDeviceType
 * @tparam TOperationMode
 * @param frame_timestamp
 * @return
 */
template<platform_independence::DeviceType TDeviceType, settings::OperationMode TOperationMode, settings::IntegrationMode TIntegrationMode>
absl::Status ForegroundContainer<TDeviceType,
    TOperationMode,
    TIntegrationMode>::HandleOutputData(int64_t frame_timestamp) {
    bool got_core_metrics_output;
    physiology::MetricsBuffer metrics_buffer;
    MP_RETURN_IF_ERROR(ph::GetPacketContentsIfAny(
        metrics_buffer,
        got_core_metrics_output,
        this->core_metrics_poller.Get(),
        pe::graph::output_streams::kMetricsBuffer,
        this->settings.verbosity_level > 2
    ));
    if (got_core_metrics_output) {
        MP_RETURN_IF_ERROR(this->OnCoreMetricsOutput(metrics_buffer, frame_timestamp));
        if (TOperationMode == settings::OperationMode::Spot) {
            // reset to start state
            this->recording = false;
            if (this->load_video) {
                keep_grabbing_frames = false;
            } else if (this->settings.video_source.auto_lock &&
                       this->video_source->SupportsExposureControls()) {
                MP_RETURN_IF_ERROR(this->video_source->TurnOnAutoExposure());
            }
        } else {
            MP_RETURN_IF_ERROR(this->ComputeCorePerformanceTelemetry(metrics_buffer));
        }
    }
    // A separate outer if-clause used here to increase the likelihood of compiler optimizing this out
    // when we're in spot mode.
    if (TOperationMode == settings::OperationMode::Continuous) {
        if (this->settings.enable_edge_metrics) {
            bool got_edge_metrics_output;
            do {
                physiology::Metrics edge_metrics;
                MP_RETURN_IF_ERROR(ph::GetPacketContentsIfAny(
                    edge_metrics, got_edge_metrics_output, this->edge_metrics_poller.Get(),
                    pe::graph::output_streams::kEdgeMetrics,
                    this->settings.verbosity_level > 2
                ));
                if (got_edge_metrics_output) {
                    MP_RETURN_IF_ERROR(this->OnEdgeMetricsOutput(edge_metrics));
                }
            } while (got_edge_metrics_output);
        }
    }

    return absl::OkStatus();
}

/**
 * Called from Run() to initialize the output data pollers.
 * @tparam TDeviceType
 * @tparam TOperationMode
 * @param frame_timestamp
 * @return status of the initialization
 */
template<platform_independence::DeviceType TDeviceType, settings::OperationMode TOperationMode, settings::IntegrationMode TIntegrationMode>
absl::Status ForegroundContainer<TDeviceType, TOperationMode, TIntegrationMode>::InitializeOutputDataPollers() {
    MP_RETURN_IF_ERROR(this->core_metrics_poller.Initialize(this->graph, pe::graph::output_streams::kMetricsBuffer));
    if (TOperationMode == settings::OperationMode::Spot) {
        return absl::OkStatus();
    } else {
        return this->edge_metrics_poller.Initialize(this->graph, pe::graph::output_streams::kEdgeMetrics);
    }
}

template<platform_independence::DeviceType TDeviceType, settings::OperationMode TOperationMode, settings::IntegrationMode TIntegrationMode>
absl::Status ForegroundContainer<TDeviceType, TOperationMode, TIntegrationMode>::Initialize() {
    LOG(INFO) << "Begin to initialize preprocessing container.";
    MP_RETURN_IF_ERROR(Base::Initialize());
    MP_ASSIGN_OR_RETURN(this->video_source, video_source::BuildVideoSource(this->settings.video_source));

    MP_RETURN_IF_ERROR(init::InitializeGui(this->settings, kWindowName));
    // legacy behavior: assume user wants to start with recording=on when a video file is supplied.
    if (this->load_video || this->settings.start_with_recording_on) {
        this->recording = true;
    } else {
        // turn on auto-exposure on launch *before* recording (if it's supported by this video source)
        if (this->settings.video_source.auto_lock && this->video_source->SupportsExposureControls()) {
            auto status = this->video_source->TurnOnAutoExposure();
            if (absl::IsUnavailable(status)) {
                LOG(INFO)
                    << "Warning: video source does not support auto-exposure controls. Please try manual controls or address the video source code.";
            }
        }
    }

#ifdef WITH_VIDEO_OUTPUT
    RET_CHECK(this->video_source->HasFrameDimensions());
    cv::Size input_video_size(this->video_source->GetWidth(), this->video_source->GetHeight());
    MP_RETURN_IF_ERROR(
        init::InitializeVideoSink<TDeviceType>(
            this->stream_writer,
            input_video_size,
            this->settings.video_sink.destination,
            30,
            this->settings.video_sink.mode
        )
    );
#endif

    LOG(INFO) << "Finish preprocessing container initialization.";
    return absl::OkStatus();
}

template<platform_independence::DeviceType TDeviceType, settings::OperationMode TOperationMode, settings::IntegrationMode TIntegrationMode>
void ForegroundContainer<TDeviceType,
    TOperationMode,
    TIntegrationMode>::ScrollPastTimeOffset() {
    // skip the first settings.start_time_offset_ms milliseconds of video
    if (this->settings.start_time_offset_ms > 0 && this->load_video) {
        cv::Mat camera_frame_raw;
        // get first frame
        *(this->video_source) >> camera_frame_raw;
        if (!camera_frame_raw.empty()) {
            // calculate correct recording start time
            int64_t frame_timestamp = this->video_source->GetFrameTimestamp();
            int64_t recording_start_time = frame_timestamp + this->settings.start_time_offset_ms * 1e3;
            // skip frames until recording time is reached
            while ((frame_timestamp < recording_start_time) && !camera_frame_raw.empty()) {
                *(this->video_source) >> camera_frame_raw;
                frame_timestamp = this->video_source->GetFrameTimestamp();
            }
        }
    }
}

template<platform_independence::DeviceType TDeviceType, settings::OperationMode TOperationMode, settings::IntegrationMode TIntegrationMode>
absl::Status ForegroundContainer<TDeviceType, TOperationMode, TIntegrationMode>::Run() {
    this->operation_context.Reset();
    if (!this->initialized) {
        return absl::PermissionDeniedError("Client not initialized.");
    }
    this->running = true;
    LOG(INFO) << "Set up output pollers.";

    //TODO: check that callbacks aren't nullptr (potentially, move the checks out into container base class and call
    // from both here and background container's StartGraph, instead of duplicating the code that's already there.)

    MP_ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller output_video_poller,
                        this->graph.AddOutputStreamPoller(pe::graph::output_streams::kOutputVideo));
    MP_ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller status_code_poller,
                        this->graph.AddOutputStreamPoller(pe::graph::output_streams::kStatusCode));
    MP_ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller blue_tooth_poller,
                        this->graph.AddOutputStreamPoller(pe::graph::output_streams::kBlueTooth));

    // frame rate diagnostics
    MP_ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller frame_sent_through_poller,
                        this->graph.AddOutputStreamPoller(pe::graph::output_streams::kFrameSentThrough));


    MP_RETURN_IF_ERROR(this->InitializeOutputDataPollers());

    MP_RETURN_IF_ERROR(this->operation_context.InitializePollers(this->graph));

    LOG(INFO) << "Start running the calculator graph.";
    MP_RETURN_IF_ERROR(this->graph.StartRun({}));

    LOG(INFO) << "Start to grab and process frames.";
    this->keep_grabbing_frames = true;

    double blue_tooth;

#ifdef BENCHMARK_CAMERA_CAPTURE
    int64_t i_frame = 0;
    std::chrono::duration<double> interval_capture_time(0.);
    std::chrono::duration<double> interval_frame_time(0.);
    int64 frame_interval = 30;
#endif

    //TODO: this function needs to be moved into VideoSourceInterface and implemented in related subclasses.
    // This way, video sources such as CaptureVideoFileSource can do the scrolling, whereas other sources
    // can ignore the command (still not sure what FileStreamVideoSource should do for scroll behavior).
    this->ScrollPastTimeOffset();

    physiology::StatusCode previous_status_code = physiology::StatusCode::PROCESSING_NOT_STARTED;

    // loop over frames
    while (this->keep_grabbing_frames) {
        cv::Mat camera_frame_raw;
#ifdef BENCHMARK_CAMERA_CAPTURE
        auto frame_loop_start = std::chrono::high_resolution_clock::now();
#endif
        // Capture frame from camera or video.
        *this->video_source >> camera_frame_raw;
#ifdef WITH_VIDEO_OUTPUT
        if (this->stream_writer.isOpened() && this->settings.video_sink.passthrough) {
                  this->stream_writer.write(camera_frame_raw);
        }
#endif
#ifdef BENCHMARK_CAMERA_CAPTURE
        auto frame_capture_end = std::chrono::high_resolution_clock::now();
#endif
        if (camera_frame_raw.empty()) {
            LOG(INFO) << "Encountered empty frame: assuming end of video or stream reached.";
            this->keep_grabbing_frames = false;
        } else {
            // === got new frame, now process it and handle output ===

            // compute timestamp
            int64_t frame_timestamp = this->video_source->GetFrameTimestamp();
            auto mp_frame_timestamp = mediapipe::Timestamp(frame_timestamp);
            this->AddFrameTimestampToBenchmarkingInfo(mp_frame_timestamp);

            // === handle output
            cv::Mat camera_frame;
            cv::cvtColor(camera_frame_raw, camera_frame, cv::COLOR_BGR2RGB);

            // Wrap Mat into an ImageFrame.
            auto input_frame = absl::make_unique<mediapipe::ImageFrame>(
                mediapipe::ImageFormat::SRGB, camera_frame.cols, camera_frame.rows,
                mediapipe::ImageFrame::kDefaultAlignmentBoundary
            );
            cv::Mat input_frame_mat = mediapipe::formats::MatView(input_frame.get());
            // transfer camera_frame data to input_frame
            camera_frame.copyTo(input_frame_mat);

            // Send recording state to the graph.
            MP_RETURN_IF_ERROR(
                this->graph
                    .AddPacketToInputStream(
                        pe::graph::input_streams::kRecording,
                        mediapipe::MakePacket<bool>(this->recording).At(mp_frame_timestamp)
                    )
            );
            // Send image packet into the graph.
            MP_RETURN_IF_ERROR(
                it::FeedFrameToGraph(std::move(input_frame), this->graph, this->device_context, frame_timestamp,
                                     pe::graph::input_streams::kInputVideo)
            );

            // region ========================================== HANDLE GRAPH OUTPUT ===================================
            // Get the graph video output packet, or stop if that fails.
            mediapipe::Packet output_video_packet;
            if (output_video_poller.QueueSize() > 0) {
                if (!output_video_poller.Next(&output_video_packet)) break;
                cv::Mat output_frame_rgb;
                MP_RETURN_IF_ERROR(it::GetFrameFromPacket<TDeviceType>(output_frame_rgb,
                                                                       this->device_context,
                                                                       output_video_packet));

                // Convert to BGR and display.
                cv::cvtColor(output_frame_rgb, this->output_frame_bgr, cv::COLOR_RGB2BGR);

                // Envoke Callback on the video
                MP_RETURN_IF_ERROR(this->OnVideoOutput(this->output_frame_bgr, frame_timestamp));

                // only display output window when we're not in headless mode.
                if (!this->settings.headless) {
                    cv::imshow(kWindowName, this->output_frame_bgr);
                }
#ifdef WITH_VIDEO_OUTPUT
                if (this->stream_writer.isOpened() && !this->settings.video_sink.passthrough) {
                  this->stream_writer.write(output_frame_bgr);
                }
#endif
            }

            bool got_status_code_packet;
            physiology::StatusValue status_value;
            MP_RETURN_IF_ERROR(ph::GetPacketContentsIfAny(
                status_value, got_status_code_packet, status_code_poller, pe::graph::output_streams::kStatusCode,
                this->settings.verbosity_level > 2
            ));

            if (got_status_code_packet){
                this->status = status_value;
                if (this->status.value() != previous_status_code) {
                    MP_RETURN_IF_ERROR(this->OnStatusChange(this->status));
                    previous_status_code = this->status.value();
                }
            }

            bool got_blue_tooth_packet;
            MP_RETURN_IF_ERROR(ph::GetPacketContentsIfAny(
                blue_tooth, got_blue_tooth_packet, blue_tooth_poller, pe::graph::output_streams::kBlueTooth,
                this->settings.verbosity_level > 0
            ));

            bool operation_state_changed;
            MP_RETURN_IF_ERROR(this->operation_context
                                   .QueryPollers(operation_state_changed, this->settings.verbosity_level > 1));

            bool got_frame_sent_through_packet;
            bool frame_sent_through;
            mediapipe::Timestamp frame_sent_through_timestamp;
            MP_RETURN_IF_ERROR(ph::GetPacketContentsIfAny(
                frame_sent_through, got_frame_sent_through_packet, frame_sent_through_poller,
                pe::graph::output_streams::kFrameSentThrough, frame_sent_through_timestamp,
                this->settings.verbosity_level > 4
            ));
            if(got_frame_sent_through_packet){
                MP_RETURN_IF_ERROR(this->OnFrameSentThrough(frame_sent_through, frame_sent_through_timestamp.Value()));
            }

            MP_RETURN_IF_ERROR(this->HandleOutputData(frame_timestamp));

            // endregion ===============================================================================================
            if (this->settings.headless) {
                if (!this->load_video) {
                    // if we loaded video, that means we started recording already.
                    // Otherwise, start recording iff status code is OK
                    if (!this->recording && this->status.value() == physiology::StatusCode::OK) {
                        if (this->settings.video_source.auto_lock && this->video_source->SupportsExposureControls()) {
                            return this->video_source->TurnOffAutoExposure();
                        }
                        this->recording = true;
                        LOG(INFO) << "====== Recording started after timestamp:" << frame_timestamp << " ======";
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(this->settings.interframe_delay_ms));
            } else {
                MP_RETURN_IF_ERROR(keys::HandleKeyboardInput(
                    this->keep_grabbing_frames, this->recording, *(this->video_source), this->settings, this->status
                ));
            }
        }

#ifdef BENCHMARK_CAMERA_CAPTURE
        MP_RETURN_IF_ERROR(
            bench::HandleCameraBenchmarking(
                i_frame, interval_capture_time, interval_frame_time, frame_loop_start, frame_capture_end,
                frame_interval, this->settings.interframe_delay_ms, this->settings.verbosity_level
            )
        );
#endif
    }

    LOG(INFO) << "Shutting down.";
    MP_RETURN_IF_ERROR(this->graph.CloseAllInputStreams());
    MP_RETURN_IF_ERROR(this->graph.CloseAllPacketSources());
#ifdef WITH_VIDEO_OUTPUT
    if (this->stream_writer.isOpened()) {
        this->stream_writer.release();
    }
#endif
    MP_RETURN_IF_ERROR(this->graph.WaitUntilDone());
    this->running = false;
    return absl::OkStatus();
}
} // namespace presage::smartspectra::container
