//
// Created by greg on 2/15/24.
// Copyright (c) 2024 Presage Technologies
//

// === standard library includes (if any) ===
// === third-party includes (if any) ===
#include <mediapipe/framework/port/opencv_highgui_inc.h>
#include <mediapipe/framework/port/logging.h>
// === local includes (if any) ===
#include "keyboard_input.hpp"
#include "modules/messages/status.h"


namespace presage::smartspectra::container::keyboard_input {

using StatusCode = physiology::StatusCode;

absl::Status HandleKeyboardInput(
    bool& grab_frames,
    bool& recording,
    video_source::VideoSource& v_source,
    const settings::GeneralSettings& settings,
    physiology::StatusValue status
) {
    const int pressed_key = cv::waitKey(settings.interframe_delay_ms);
    if (pressed_key != -1) {
        switch (pressed_key) {
            case 'q':
            case 27:  // ESC
                grab_frames = false;
                break;
            case 'e':
                return v_source.ToggleAutoExposure();
            case '-':
                return v_source.DecreaseExposure();
            case '=':
                return v_source.IncreaseExposure();
            case 's':
                if (status.value() == StatusCode::OK || status.value() == StatusCode::PROCESSING_NOT_STARTED) {
                    recording = !recording;
                    LOG(INFO) << (
                        recording ?
                        "====== Recording started after timestamp " + std::to_string(status.timestamp()) + ". ======"
                                  :
                        "====== Recording stopped after timestamp " + std::to_string(status.timestamp()) + ". ======"
                    );
                    if (recording) {
                        // lock exposure when recording commences (if it's supported by this video source)
                        if (settings.video_source.auto_lock && v_source.SupportsExposureControls()) {
                            return v_source.TurnOffAutoExposure();
                        }
                    } else {
                        // turn on auto-exposure after recording (if it's supported by this video source)
                        if (settings.video_source.auto_lock && v_source.SupportsExposureControls()) {
                            return v_source.TurnOnAutoExposure();
                        }
                    }
                } else {
                    LOG(INFO) << "Not ready to start recording. Preprocessing input issue detected: "
                    << physiology::GetStatusDescription(status.value()) << ". Status code " << status.value()
                    << " at timestamp " << status.timestamp() << ". " << physiology::GetStatusHint(status.value());
                }
                break;
            default:
                LOG(INFO) << "User pressed key with code '" << pressed_key
                          << "'. This key is not yet mapped to any action.";
                break;
        }
    }
    return absl::OkStatus();
}

} // namespace presage::smartspectra::container::keyboard_input
