//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// opencv_hud.cpp
// Created by greg on 1/3/25.
// Copyright (C) 2025 Presage Security, Inc.
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

// === standard library includes (if any) ===
// === third-party includes (if any) ===
#include <mediapipe/framework/deps/status_macros.h>

#include <utility>
// === local includes (if any) ===
#include "opencv_hud.hpp"
#include "opencv_element_fits.hpp"
#include "confidence_thresholding.hpp"


namespace presage::smartspectra::gui {

void OpenCvHud::UpdateWithNewMetrics(const physiology::MetricsBuffer& new_metrics) {
    auto pulse_rate_repeated_field = new_metrics.pulse().rate();
    if (!pulse_rate_repeated_field.empty()) {
        this->pulse_group->rate = pulse_rate_repeated_field.Get(pulse_rate_repeated_field.size() - 1);
        this->pulse_group->rate_is_high_confidence = is_pulse_high_confidence(this->pulse_group->rate.confidence());
        this->pulse_group->trace_plotter.UpdateTraceWithSampleRange(new_metrics.pulse().trace());
    }

    auto breathing_rate_repeated_field = new_metrics.breathing().rate();
    if (!breathing_rate_repeated_field.empty()) {
        this->upper_breathing_group->rate = breathing_rate_repeated_field.Get(breathing_rate_repeated_field.size() - 1);
        this->upper_breathing_group->rate_is_high_confidence = is_breathing_high_confidence(this->upper_breathing_group
                                                                                                ->rate.confidence());
        this->upper_breathing_group->trace_plotter.UpdateTraceWithSampleRange(new_metrics.breathing().upper_trace());
        // These two code lines are a little weird right now
        // (but I guess OK for now, since its expected to be fixed in Core later)
        // Since we don't yet have a separate lower breathing rate returned by Core,
        // so we use the same confidence as the upper breathing rate -- for now
        this->lower_breathing_group->rate_is_high_confidence = this->upper_breathing_group->rate_is_high_confidence;
        // And the rate copy needs to happen so that we accurately determine color for lower trace;
        // i.e. no_rate_value_to_display that it starts with is displayed w/ "confident" color regardless of
        // `rate_is_high_confidence`, whereas we need it to display w/ color of upper breathing rate/trace
        this->lower_breathing_group->rate = this->upper_breathing_group->rate;
    }

    this->lower_breathing_group->trace_plotter.UpdateTraceWithSampleRange(new_metrics.breathing().lower_trace());
}

const float OpenCvHud::no_rate_value_to_display = -1.0f;

const int OpenCvHud::top_plot_area_margin = 20;
const int OpenCvHud::bottom_plot_area_margin = 20;
const int OpenCvHud::minimal_plot_area_height = 90;
const int OpenCvHud::indicator_width = 200;
const int OpenCvHud::label_width = 150;
const int OpenCvHud::minimal_plot_area_width = 200;

const int OpenCvHud::minimal_width =
    OpenCvHud::indicator_width + OpenCvHud::minimal_plot_area_width;
const int OpenCvHud::minimal_height =
    OpenCvHud::top_plot_area_margin + OpenCvHud::minimal_plot_area_height + OpenCvHud::bottom_plot_area_margin;


OpenCvHud::OpenCvHud(
    int x, int y, int width, int height,
    int max_trace_points,
    cv::Scalar pulse_confident_color,
    cv::Scalar pulse_unconfident_color,
    cv::Scalar breathing_upper_confident_color,
    cv::Scalar breathing_upper_unconfident_color,
    cv::Scalar breathing_lower_confident_color,
    cv::Scalar breathing_lower_unconfident_color
) :
    width_sufficient(width >= OpenCvHud::minimal_width),
    height_sufficient(height >= OpenCvHud::minimal_height),
    hud_area(x, y, width, height), max_trace_points(max_trace_points) {

    if (width_sufficient && height_sufficient) {
        const int usable_plot_area_height =
            this->hud_area.height - OpenCvHud::top_plot_area_margin - OpenCvHud::bottom_plot_area_margin;
        const int single_trace_height = static_cast<int>(static_cast<float>(usable_plot_area_height) / 3.f - 1.f);
        const float sixth_trace_height = static_cast<float>(usable_plot_area_height) / 6.f;
        const int trace_width = this->hud_area.width - OpenCvHud::indicator_width - OpenCvHud::label_width;
        const int rate_indicator_x = this->hud_area.x + trace_width;
        const int label_x = rate_indicator_x + OpenCvHud::indicator_width;

        physiology::MeasurementWithConfidence rate;
        rate.set_value(no_rate_value_to_display);

        auto make_group =
            [this, &trace_width, &single_trace_height, &rate_indicator_x, &label_x, &rate]
                (
                    int y, cv::Scalar confident_color, cv::Scalar unconfident_color, const std::string& name,
                    bool indicator_visible = true
                ) {
                return std::make_unique<MetricsGroup>(MetricsGroup{
                    OpenCvTracePlotter{this->hud_area.x, y, trace_width, single_trace_height, this->max_trace_points},
                    OpenCvValueIndicator{rate_indicator_x, y + single_trace_height / 2,
                                         OpenCvHud::indicator_width, single_trace_height},
                    OpenCvLabel{indicator_visible ? label_x : rate_indicator_x,
                                y, OpenCvHud::label_width, single_trace_height, name},
                    rate, true, true,
                    std::move(confident_color), std::move(unconfident_color)
                });
            };
        int pulse_group_y = this->hud_area.y + static_cast<int>(OpenCvHud::top_plot_area_margin + sixth_trace_height);
        this->pulse_group = make_group(
            pulse_group_y,
            std::move(pulse_confident_color),
            std::move(pulse_unconfident_color),
            "Pulse (Skin Chroma)"
        );

        int upper_breathing_group_y =
            this->hud_area.y + static_cast<int>(OpenCvHud::top_plot_area_margin + 3 * sixth_trace_height);
        this->upper_breathing_group = make_group(
            upper_breathing_group_y,
            std::move(breathing_upper_confident_color),
            std::move(breathing_upper_unconfident_color),
            "Breathing (Chest)"
        );

        int lower_breathing_group_y =
            this->hud_area.y + static_cast<int>(OpenCvHud::top_plot_area_margin + 5 * sixth_trace_height);
        this->lower_breathing_group = make_group(
            lower_breathing_group_y,
            std::move(breathing_lower_confident_color),
            std::move(breathing_lower_unconfident_color),
            "Breathing (Abdomen)",
            /*indicator_visible=*/false
        );
        this->lower_breathing_group->display_rate = false;
    }
}

absl::Status OpenCvHud::Render(cv::Mat& image) {
    if (!this->width_sufficient) {
        return absl::InvalidArgumentError(
            "Width of HUD, " + std::to_string(this->hud_area.width) + ", is insufficient for adequate display."
        );
    }
    if (!this->height_sufficient) {
        return absl::InvalidArgumentError(
            "Height of HUD, " + std::to_string(this->hud_area.height) + ", is insufficient for adequate display."
        );
    }
    MP_RETURN_IF_ERROR(CheckThatElementFitsImage("OpenCvHud", this->hud_area, image));
    MP_RETURN_IF_ERROR(pulse_group->Render(image));
    MP_RETURN_IF_ERROR(upper_breathing_group->Render(image));
    MP_RETURN_IF_ERROR(lower_breathing_group->Render(image));
    return absl::OkStatus();
}


absl::Status OpenCvHud::MetricsGroup::Render(cv::Mat& image) {
    auto color = this->rate.value() == no_rate_value_to_display || this->rate_is_high_confidence ?
                 this->confident_color : this->unconfident_color;
    MP_RETURN_IF_ERROR(this->trace_plotter.Render(image, color));
    if (this->display_rate) {
        if (this->rate.value() == no_rate_value_to_display) {
            MP_RETURN_IF_ERROR(this->rate_indicator.RenderNA(image, color));
        } else {
            MP_RETURN_IF_ERROR(this->rate_indicator.Render(image, this->rate.value(), color));
        }
    }
    MP_RETURN_IF_ERROR(this->label.Render(image, color));
    return absl::OkStatus();
}
} // namespace presage::smartspectra::gui
