//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// packet_helpers.h
// Created by Greg on 2/16/2024.
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
// === third-party includes (if any) ===
#include <mediapipe/framework/output_stream_poller.h>
#include <physiology/modules/messages/status.h>
#include <google/protobuf/message.h>
// === local includes (if any) ===

namespace presage::smartspectra::container::packet_helpers {

// Generic operator<< for protobuf messages
// Allows streaming any protobuf message using ShortDebugString()
template <typename T, typename = std::enable_if_t<std::is_base_of<google::protobuf::Message, T>::value>>
std::ostream& operator<<(std::ostream& os, const T& proto) {
    return os << proto.DebugString();
}

// Lightweight operator<< for std::pair to support simple debug logging
template <typename A, typename B>
inline std::ostream& operator<<(std::ostream& os, const std::pair<A, B>& p) {
    os << "(" << p.first << ", " << p.second << ")";
    return os;
}

// functional predicate, with grabbing packet timestamp
template<typename TPacketContentsType, typename TReportPredicate, bool TPrintTimestamp = false>
inline absl::Status GetPacketContentsIfAny(
    TPacketContentsType& contents,
    bool& nonempty_packet_received,
    mediapipe::OutputStreamPoller& poller,
    const char* stream_name,
    mediapipe::Timestamp& timestamp,
    TReportPredicate&& report_if
) {
    nonempty_packet_received = false;
    if (poller.QueueSize() > 0) {
        mediapipe::Packet packet;
        if (!poller.Next(&packet)) {
            return absl::UnknownError(
                "Failed to get packet from output stream " + std::string(stream_name) + ".");
        } else {
            if (!packet.IsEmpty()) {
                nonempty_packet_received = true;
                contents = packet.Get<TPacketContentsType>();
                std::string extra_information = "";
                timestamp = packet.Timestamp();
                if (TPrintTimestamp) {
                    extra_information = " (timestamp: " + std::to_string(timestamp.Value()) + ")";
                }
                if (report_if()) {
                    LOG(INFO) << "Got " + std::string(stream_name) + " packet: " << contents << extra_information;
                }
            }
        }
    }
    return absl::OkStatus();
}

// functional predicate, without grabbing packet timestamp
template<typename TPacketContentsType, typename TReportPredicate, bool TPrintTimestamp = false>
inline absl::Status GetPacketContentsIfAny(
    TPacketContentsType& contents,
    bool& nonempty_packet_received,
    mediapipe::OutputStreamPoller& poller,
    const char* stream_name,
    TReportPredicate&& report_if
) {
    mediapipe::Timestamp timestamp;
    return GetPacketContentsIfAny<TPacketContentsType, TReportPredicate, TPrintTimestamp>(
        contents, nonempty_packet_received, poller, stream_name, timestamp, std::forward<TReportPredicate>(report_if)
    );
}

// constant boolean predicate, with grabbing packet timestamp
template<typename TPacketContentsType>
inline absl::Status GetPacketContentsIfAny(
    TPacketContentsType& contents,
    bool& nonempty_packet_received,
    mediapipe::OutputStreamPoller& poller,
    const char* string_name,
    mediapipe::Timestamp& timestamp,
    bool report_on_package_retrieval
) {
    return GetPacketContentsIfAny(
        contents, nonempty_packet_received, poller, string_name, timestamp, [&report_on_package_retrieval]() {
            return report_on_package_retrieval;
        }
    );
}

// constant boolean predicate, without grabbing packet timestamp
template<typename TPacketContentsType>
inline absl::Status GetPacketContentsIfAny(
    TPacketContentsType& contents,
    bool& nonempty_packet_received,
    mediapipe::OutputStreamPoller& poller,
    const char* string_name,
    bool report_on_package_retrieval
) {
    mediapipe::Timestamp timestamp;
    return GetPacketContentsIfAny(
        contents, nonempty_packet_received, poller, string_name, timestamp, [&report_on_package_retrieval]() {
            return report_on_package_retrieval;
        }
    );
}

} // namespace presage::smartspectra::container::packet_helpers
