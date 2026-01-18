//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// opencv_plotter.cpp
// Created by Greg on 12/13/24.
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
// standard library includes
#include <array>

// third-party includes
#include <physiology/modules/geometry.hpp>
#include <mediapipe/framework/deps/status_macros.h>
#include <opencv2/imgproc/imgproc.hpp>

// local includes
#include "opencv_trace_plotter.hpp"
#include "opencv_element_fits.hpp"

namespace presage::smartspectra::gui {


template<typename TMeasurement>
void AppendOverlappingTimeSeries(
    google::protobuf::RepeatedPtrField<TMeasurement>& target_series,
    const google::protobuf::RepeatedPtrField<TMeasurement>& source_series,
    int& target_start_index
) {
    if (!source_series.empty()) {
        float first_source_time = source_series.Get(0).time();
        int i_target_measurement = target_start_index;
        int i_source_measurement = 0;
        if (!target_series.empty()) {
            float current_target_time = target_series.Get(i_target_measurement).time();
            while (current_target_time < first_source_time && i_target_measurement < target_series.size()) {
                current_target_time = target_series.Get(i_target_measurement).time();
                i_target_measurement++;
            }
            float current_source_time = first_source_time;
            float first_target_time = current_target_time;
            // for cases when source data times are earlier than target data times (e.g. calibration trigger / re-trigger),
            // scroll to first source measurement that occurs after or at the first target measurement
            while (current_source_time < first_target_time && i_source_measurement < source_series.size()) {
                current_source_time = source_series.Get(i_source_measurement).time();
                i_source_measurement++;
            }
        }

        // start scanning next time from the source measurement time that started the overlap in this iteration
        target_start_index = i_target_measurement;

        // update existing measurements
        for (; i_target_measurement < target_series.size() &&
               i_source_measurement < source_series.size(); i_target_measurement++, i_source_measurement++) {
            TMeasurement* target_measurement = target_series.Mutable(i_target_measurement);
            const TMeasurement& source_measurement = source_series.Get(i_source_measurement);
            if (source_measurement.time() == target_measurement->time()) {
                *target_measurement = source_measurement;
            }
        }
        // add new measurements

        for (; i_source_measurement < source_series.size(); i_source_measurement++) {
            target_series.Add()->CopyFrom(source_series.Get(i_source_measurement));
        }
    }
}

void RenderTimeSeries(const std::vector<cv::Point2i>& points, cv::Mat& image, const cv::Scalar& color, int line_width);

/**
 * Update the trace with a range of samples. The range may have overlap with existing values, but must end at or after
 * the last range that was added this way.
 * @param new_values the sample range with updated values.
 */
void OpenCvTracePlotter::UpdateTraceWithSampleRange(
    const google::protobuf::RepeatedPtrField<physiology::Measurement>& new_values
) {
    AppendOverlappingTimeSeries(this->buffer, new_values, this->last_overlap_area_start);
    if (this->buffer.size() > this->max_points) {
        int deleted_point_count = this->buffer.size() - this->max_points;
        // clear out the oldest points
        this->buffer.DeleteSubrange(0, deleted_point_count);
        // push back start-check cursor in local buffer
        this->last_overlap_area_start = std::max(0, this->last_overlap_area_start - deleted_point_count);
    }
}

/**
 * Update the trace with a single sample, assuming the new sample follows existing samples in time
 * @param new_value the new sample
 */
void OpenCvTracePlotter::UpdateTraceWithSample(const physiology::Measurement& new_value) {
    this->buffer.Add()->CopyFrom(new_value);
    if (this->buffer.size() > this->max_points) {
        int deleted_point_count = this->buffer.size() - this->max_points;
        // clear out the oldest points
        this->buffer.DeleteSubrange(0, deleted_point_count);
    }
}

OpenCvTracePlotter::OpenCvTracePlotter(int x, int y, int width, int height, int max_points)
    : plot_area(x, y, width, height), max_points(max_points) {}


template<typename TMeasurement, typename TFunction>
void ApplyToMeasurements(
    const google::protobuf::RepeatedPtrField<TMeasurement>& measurements,
    TFunction&& function
) {
    for (const TMeasurement& measurement: measurements) {
        function(measurement);
    }
}

template<typename TMeasurement>
float GetMaxValue(const google::protobuf::RepeatedPtrField<TMeasurement>& measurements) {
    float max_value = 0.0f;
    auto process_for_max = [&max_value](const TMeasurement& measurement) {
        max_value = std::max(max_value, measurement.value());
    };
    ApplyToMeasurements(measurements, process_for_max);
    return max_value;
}

template<typename TMeasurement>
float GetMinValue(const google::protobuf::RepeatedPtrField<TMeasurement>& measurements) {
    float min_value = std::numeric_limits<float>::max();
    auto process_for_min = [&min_value](const TMeasurement& measurement) {
        min_value = std::min(min_value, measurement.value());
    };
    ApplyToMeasurements(measurements, process_for_min);
    return min_value;
}

template<typename TMeasurement>
std::vector<cv::Point2i> ComputeRenderableTimeSeries(
    const google::protobuf::RepeatedPtrField<TMeasurement>& trace_measurements,
    float value_scale_factor = 1.0, float time_scale_factor = 1.0, float y_offset = 0.0
) {
    float min_value = GetMinValue(trace_measurements);
    float max_value = GetMaxValue(trace_measurements);
    float value_range = max_value - min_value;
    float min_time = trace_measurements.Get(0).time();
    float max_time = trace_measurements.Get(trace_measurements.size() - 1).time();
    float time_range = max_time - min_time;

    std::vector<cv::Point2i> canvas_points;
    for (const TMeasurement& measurement: trace_measurements) {
        int normalized_value = static_cast<int>(
            (max_value - measurement.value()) * value_scale_factor / value_range + y_offset);;
        int normalized_time = static_cast<int>((measurement.time() - min_time) * time_scale_factor / time_range);
        canvas_points.emplace_back(normalized_time, normalized_value);
    }
    return canvas_points;
}

void RenderTimeSeries(const std::vector<cv::Point2i>& points, cv::Mat& image, const cv::Scalar& color, int line_width) {
    for (int i_point = 1; i_point < points.size(); i_point++) {
        cv::line(
            image, points[i_point - 1], points[i_point], color, line_width, cv::LINE_AA
        );
    }
}

absl::Status OpenCvTracePlotter::Render(cv::Mat& image, const cv::Scalar& color) {
    MP_RETURN_IF_ERROR(CheckThatElementFitsImage("OpenCvTracePlotter", this->plot_area, image));

    // Margins to avoid clipping
    const float trace_width = static_cast<float>(this->plot_area.width) - 1.f;

    if (this->buffer.size() >= 2) {
        auto points =
            ComputeRenderableTimeSeries(
                this->buffer,
                static_cast<float>(this->plot_area.height),
                trace_width,
                static_cast<float>(this->plot_area.y)
            );
        RenderTimeSeries(points, image, color, 1);
    }
    return absl::OkStatus();
}

} // presage::smartspectra::gui
