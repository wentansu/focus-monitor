// settings.hpp
// Created by Greg on 1/10/2024.
// Copyright (C) 2025 Presage Security, Inc.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once
// === configuration header ===
#include "configuration.hpp"
// === standard library includes (if any) ===
#include <optional>
// === third-party includes (if any) ===
// === local includes (if any) ===
#include <smartspectra/video_source/camera/camera.hpp>
#include <smartspectra/video_source/settings.hpp>

namespace presage::smartspectra::container::settings {

//TODO: find a better way to structure, in terms of module/namespace, where to place these enum classes
// (OperationMode, IntegrationMode)  and corresponding  Abseil parse/unparse ops + context classes.
// Perhaps, in their own separate sub-module, e.g. container::contexts?
// Currently, scattered between container::settings and container.
enum class OperationMode : int {
    Spot,
    Continuous,
    Unknown_EnumEnd
};

enum class IntegrationMode : int{
    Rest,
    Grpc,
    Unknown_EnumEnd
};

template<IntegrationMode TIntegrationMode>
struct IntegrationModeTraits {};

template<>
struct IntegrationModeTraits<IntegrationMode::Rest> {
    static constexpr char kPreprocessingDataFormat[] = "json";
};

template<>
struct IntegrationModeTraits<IntegrationMode::Grpc> {
    static constexpr char kPreprocessingDataFormat[] = "pb";
};

bool AbslParseFlag(absl::string_view text, OperationMode* mode, std::string* error);
std::string AbslUnparseFlag(OperationMode mode);
std::vector<std::string> GetOperationModeNames();
bool AbslParseFlag(absl::string_view text, IntegrationMode* mode, std::string* error);
std::string AbslUnparseFlag(IntegrationMode mode);
std::vector<std::string> GetIntegrationModeNames();

// region ================================ Operation Settings ==========================================================
template<OperationMode>
struct OperationSettings;

template<>
struct OperationSettings<OperationMode::Spot> {
    double spot_duration_s/* = 30.0*/;
};

template<>
struct OperationSettings<OperationMode::Continuous> {
    double preprocessed_data_buffer_duration_s/* = 0.2*/;
};

typedef OperationSettings<OperationMode::Spot> SpotSettings;
typedef OperationSettings<OperationMode::Continuous> ContinuousSettings;
// endregion ===========================================================================================================

// region ============================== Integration Settings ==========================================================
template<IntegrationMode>
struct IntegrationSettings;

template<>
struct IntegrationSettings<IntegrationMode::Grpc> {
    uint16_t port_number = 50051;
};

template<>
struct IntegrationSettings<IntegrationMode::Rest> {
    std::string api_key;
#ifdef ENABLE_CUSTOM_SERVER
    std::optional<std::string> continuous_server_url;
#endif
};

typedef IntegrationSettings<IntegrationMode::Grpc> GrpcSettings;
typedef IntegrationSettings<IntegrationMode::Rest> RestSettings;
// endregion ===========================================================================================================
// region =============================== Video Output Settings ========================================================
enum class VideoSinkMode : int {
    MJPG,
    GSTREAMER_TEMPLATED,
    Unknown_EnumEnd
};
std::vector<std::string> GetVideoSinkModeNames();
bool AbslParseFlag(absl::string_view text, VideoSinkMode* mode, std::string* error);
std::string AbslUnparseFlag(VideoSinkMode mode);

struct VideoSinkSettings {
    std::string destination;
    VideoSinkMode mode;
    bool passthrough;
};
// endregion ===========================================================================================================
// region ------------------------------- General Settings -------------------------------------------------------------
struct GeneralSettings {
    video_source::VideoSourceSettings video_source;
    VideoSinkSettings video_sink; // foreground-container only
    bool headless = false; // foreground-container only
    int interframe_delay_ms = 20; // foreground-container only
    bool start_with_recording_on = false; // foreground-container only
    int start_time_offset_ms = 0; // foreground-container only
    // graph internal settings
    bool scale_input = true;
    bool binary_graph = true;
    std::optional<bool> enable_phasic_bp;
    std::optional<bool> enable_eda;
    bool enable_dense_facemesh_points = false;
    std::optional<bool> use_full_range_face_detection;
    std::optional<bool> use_full_pose_landmarks;
    std::optional<bool> enable_pose_landmark_segmentation;
    std::optional<bool> enable_micromotion;
    // WARNING: enable_edge_metrics doesn't currently apply to spot mode
    bool enable_edge_metrics = false;
    bool print_graph_contents = false;
    bool log_transfer_timing_info = false;
    int verbosity_level = 0;
};
// endregion ===========================================================================================================
template<OperationMode, IntegrationMode>
struct Settings;

// Note on design: yes, breaking this down like this instead of just using the following fields would be much simpler.
// OperationSettings<OperationMode::Spot> operation;
// IntegrationSettings<IntegrationMode::JsonFileOnDisk> integration;
// However, it may be easier to use the settings if the field names themselves conveyed the semantic meanings of
// the setting categories,
// e.g. "settings.spot.<whatever>", "setting.continuous.<whatever>", "settings.grpc.<whatever>", etc.
// Since C++ doesn't have reflection, we would need to resort to preprocessor macros to generate the field names
// dynamically. Instead, to avoid bloat and preprocessor use, it makes sense to just write out the specializations
// with different fields for the scenarios for which we anticipate greater usage.

// region ================================ SPOT + OTHER SETTINGS =======================================================
template<IntegrationMode TIntegrationMode>
struct Settings<OperationMode::Spot, TIntegrationMode> : GeneralSettings {
    union{SpotSettings spot; SpotSettings operation;};
    IntegrationSettings<TIntegrationMode> integration;
};

template<>
struct Settings<OperationMode::Spot, IntegrationMode::Rest> : GeneralSettings {
    union{SpotSettings spot; SpotSettings operation;};
    RestSettings integration;
    //have to use function to provide an alias, because API key is not trivially-copiable
    RestSettings& rest() { return integration; }
};

template<>
struct Settings<OperationMode::Spot, IntegrationMode::Grpc> : GeneralSettings {
    union{SpotSettings spot; SpotSettings operation;};
    union{GrpcSettings grpc; GrpcSettings integration;};
};
// endregion ===========================================================================================================
// region ================================ CONTINUOUS + OTHER SETTINGS =================================================
template<IntegrationMode TIntegrationMode>
struct Settings<OperationMode::Continuous, TIntegrationMode> : GeneralSettings {
    union{ContinuousSettings continuous; ContinuousSettings operation;};
    IntegrationSettings<TIntegrationMode> integration;
};

template<>
struct Settings<OperationMode::Continuous, IntegrationMode::Rest> : GeneralSettings {
    union{ContinuousSettings continuous; ContinuousSettings operation;};
    RestSettings integration;
    //have to use function to provide an alias, because API key is not trivially-copiable
    RestSettings& rest() { return integration; }
};

template<>
struct Settings<OperationMode::Continuous, IntegrationMode::Grpc> : GeneralSettings {
    union{ContinuousSettings continuous; ContinuousSettings operation;};
    union{GrpcSettings grpc; GrpcSettings integration;};
};
// endregion ===========================================================================================================

} // namespace presage::smartspectra::container::settings
