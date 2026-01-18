// stdlib includes
#include <vector>
#include <set>
#include <map>

// third-party includes
#include <opencv2/videoio/registry.hpp>

// local includes
#include "camera.hpp"
#include "camera_opencv.hpp"

namespace presage::camera::opencv {

const std::map<CaptureCodec, int> kCvCodecFlagByCaptureCodec = {
    {CaptureCodec::MJPG, cv::VideoWriter::fourcc('M', 'J', 'P', 'G')},
    {CaptureCodec::UYVY, cv::VideoWriter::fourcc('I', 'Y', 'U', 'V')}
};

bool CheckCameraOpens(int camera_device_index, int cv_api_index) {
    cv::VideoCapture capture;
    if (!capture.open(camera_device_index, cv_api_index)) {
        return false;
    }
    return capture.get(cv::CAP_PROP_FRAME_WIDTH) != 0 || capture.get(cv::CAP_PROP_FRAME_HEIGHT) != 0;
}

int DeterminePreferredBackendForCamera(int camera_device_index) {
    const std::map<cv::VideoCaptureAPIs, int> backendPriorities = {
        {cv::VideoCaptureAPIs::CAP_V4L,          20},
        {cv::VideoCaptureAPIs::CAP_AVFOUNDATION, 10},
        {cv::VideoCaptureAPIs::CAP_GSTREAMER,    0},
    };
    std::vector<cv::VideoCaptureAPIs> backends = cv::videoio_registry::getCameraBackends();
    std::sort(
        backends.begin(), backends.end(),
        [&backendPriorities](const cv::VideoCaptureAPIs& backend_a, const cv::VideoCaptureAPIs& backend_b) {
            auto backend_a_priority_position = backendPriorities.find(backend_a);
            auto backend_b_priority_position = backendPriorities.find(backend_b);

            if (backend_a_priority_position == backendPriorities.end()) {
                if (backend_b_priority_position == backendPriorities.end()) {
                    return true;
                }
                return false; // [no priority for backend_a, but something for backend_b] => [a is ordered after b]
            }
            if (backend_b_priority_position == backendPriorities.end()) {
                return true; // backend_a has some priority defined, place it before backend_b
            } else {
                return backend_a_priority_position->second > backend_b_priority_position->second; // greater priority means earlier placement
            }
        }
    );

    for (const cv::VideoCaptureAPIs& backend: backends) {
        if (CheckCameraOpens(camera_device_index, backend)) {
            return backend;
        }
    }
    return -1;
}

UncertainBool CheckCameraInterfaceSupportsTimestamp(int camera_device_index) {
    int backend = DeterminePreferredBackendForCamera(camera_device_index);
    static std::set<cv::VideoCaptureAPIs> backends_known_to_support_timestamp = {
        //TODO: list is probably incomplete. Test with each new backend encountered and populate this list.
        cv::CAP_V4L2
    };
    static std::set<cv::VideoCaptureAPIs> backends_known_to_not_support_timestamp = {
        //TODO: list is probably incomplete. Test with each new backend encountered and populate this list.
        cv::CAP_AVFOUNDATION
    };
    if (backend == -1) {
        return UncertainBool::False;
    }
    // TODO: replace with .contains when we finally get to C++20
    if (backends_known_to_support_timestamp.find(static_cast<cv::VideoCaptureAPIs>(backend)) !=
        backends_known_to_support_timestamp.end()) {
        return UncertainBool::True;
    }
    if (backends_known_to_not_support_timestamp.find(static_cast<cv::VideoCaptureAPIs>(backend)) !=
        backends_known_to_not_support_timestamp.end()) {
        return UncertainBool::False;
    } else {
        return UncertainBool::Unknown;
    }
}

std::string DeterminePreferredBackendNameForCamera(int camera_device_index) {
    int backend = DeterminePreferredBackendForCamera(camera_device_index);
    if (backend == -1) {
        return "Undefined";
    }
    return cv::videoio_registry::getBackendName(static_cast<cv::VideoCaptureAPIs>(backend));
}

} // presage::camera::opencv
