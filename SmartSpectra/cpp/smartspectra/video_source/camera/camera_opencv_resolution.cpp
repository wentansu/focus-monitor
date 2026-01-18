/*
 * Created by Gregory Kramida on 9/18/23.
 * Copyright (c) 2023 Presage Technologies. All rights reserved.
 */
// local includes
#include "camera_opencv.hpp"

namespace presage::camera::opencv {
bool CheckCameraWorksWithResolution(cv::VideoCapture& capture, const cv::Size& resolution, int cv_capture_api) {
  assert (capture.isOpened());
  capture.set(cv::CAP_PROP_FRAME_WIDTH, resolution.width);
  capture.set(cv::CAP_PROP_FRAME_HEIGHT, resolution.height);
  cv::Size resolution_out(
      static_cast<int>(capture.get(cv::CAP_PROP_FRAME_WIDTH)),
      static_cast<int>(capture.get(cv::CAP_PROP_FRAME_HEIGHT))
  );
  return resolution_out == resolution;
}

std::tuple<bool, cv::Size>
GetMaximumCameraResolutionFromRange(int camera_device_index, CameraResolutionRange range_to_check, int cv_capture_api) {
  cv::VideoCapture capture;
  if (!capture.open(camera_device_index, cv_capture_api)) return std::make_tuple(false, cv::Size{});
  int max_width = 0;
  int max_height = 0;
  bool some_working_resolution_found = false;
  const auto& range = kCommonCameraResolutionRanges.at(range_to_check);
  // TODO: redo using std::span when C++20 is supported
  for (int i_resolution = range.first; i_resolution <= range.second; i_resolution++) {
    const auto& resolution = kCommonCameraResolutions[i_resolution];
    if (CheckCameraWorksWithResolution(capture, resolution, cv_capture_api)) {
      max_width = std::max(max_width, resolution.width);
      max_height = std::max(max_height, resolution.height);
      some_working_resolution_found = true;
    }
  }
  return std::make_tuple(some_working_resolution_found, cv::Size(max_width, max_height));
}

const std::map<CameraResolutionRange, std::pair<int, int>> kCommonCameraResolutionRanges = {
    {CameraResolutionRange::Low,      {0,   16}},
    {CameraResolutionRange::Mid,      {16,  35}},
    {CameraResolutionRange::High,     {35,  61}},
    {CameraResolutionRange::Ultra,    {61,  91}},
    {CameraResolutionRange::FourK,    {91,  102}},
    {CameraResolutionRange::Giant,    {102, 103}},
    {CameraResolutionRange::Complete, {0,   103}}
};

const std::vector<CameraResolutionRange> kCommonCameraResolutionRangeValues = [](){
  std::vector<CameraResolutionRange> ranges;
  for (const auto& range : kCommonCameraResolutionRanges) {
    ranges.push_back(range.first);
  }
  return ranges;
}();

const std::vector<cv::Size> kCommonCameraResolutions =
    {
        {160,   120}, // Low
        {192,   144},
        {256,   144},
        {240,   160},
        {320,   240},
        {360,   240},
        {384,   240},
        {400,   240},
        {432,   240},
        {480,   320},
        {480,   360},
        {640,   360},
        {800,   433},
        {600,   480},
        {640,   480},
        {720,   480},
        {768,   480}, //->16
        {800,   480}, //16+, Mid
        {854,   480},
        {960,   480},
        {675,   540},
        {960,   540},
        {720,   576},
        {768,   576},
        {1024,  576},
        {750,   600},
        {800,   600},
        {1024,  600},
        {960,   640},
        {1024,  640},
        {1136,  640},
        {960,   720},
        {1152,  720},
        {1280,  720},
        {1440,  720},
        {960,   768},
        {1024,  768}, //->35
        {1152,  768}, // High
        {1280,  768},
        {1366,  768},
        {1280,  800},
        {1152,  864},
        {1280,  864},
        {1536,  864},
        {1200,  900},
        {1440,  900},
        {1600,  900},
        {1280,  960},
        {1440,  960},
        {1536,  960},
        {1280,  1024},
        {1600,  1024},
        {1400,  1050},
        {1680,  1050},
        {1440,  1080},
        {1920,  1080},
        {2160,  1080},
        {2280,  1080},
        {2560,  1080},
        {2048,  1152},
        {1500,  1200},
        {1600,  1200},
        {1920,  1200}, //->61
        {1920,  1280}, // 61+, Ultra
        {2048,  1280},
        {1920,  1440},
        {2160,  1440},
        {2304,  1440},
        {2560,  1440},
        {2880,  1440},
        {2960,  1440},
        {3040,  1440},
        {3120,  1440},
        {3200,  1440},
        {3440,  1440},
        {5120,  1440},
        {2048,  1536},
        {2304,  1536}, //->76
        {2400,  1600}, // 76+
        {2560,  1600},
        {3840,  1600},
        {2880,  1620},
        {2880,  1800},
        {3200,  1800},
        {2560,  1920},
        {2880,  1920},
        {3072,  1920},
        {2560,  2048},
        {2732,  2048},
        {3200,  2048},
        {2880,  2160},
        {3240,  2160},
        {3840,  2160}, //->91, FourK
        {4320,  2160},
        {5120,  2160},
        {3200,  2400},
        {3840,  2400},
        {3840,  2560},
        {4096,  2560},
        {5120,  2880},
        {5760,  2880}, // Giant
        {4096,  3072},
        {7680,  4320},
        {10240, 4320},
    };
} // namespace presage::camera::opencv
