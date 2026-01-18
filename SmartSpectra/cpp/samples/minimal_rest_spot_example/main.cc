#include <smartspectra/container/foreground_container.hpp>
#include <glog/logging.h>
#include <google/protobuf/util/json_util.h>
namespace spectra = presage::smartspectra;
namespace settings = presage::smartspectra::container::settings;
using DeviceType = presage::platform_independence::DeviceType;

int main(int argc, char** argv) {
    google::InitGoogleLogging(argv[0]);
    FLAGS_alsologtostderr = true;

    settings::Settings<settings::OperationMode::Spot, settings::IntegrationMode::Rest> settings;
    settings.integration.api_key = "YOUR_API_KEY_HERE";
    settings.spot.spot_duration_s = 30;

    spectra::container::SpotRestForegroundContainer<DeviceType::Cpu> container(settings);
    auto status = container.SetOnCoreMetricsOutput(
        [](const presage::physiology::MetricsBuffer& metrics, int64_t timestamp_microseconds) {
        std::string metrics_json;
        google::protobuf::util::JsonPrintOptions options;
        options.add_whitespace = true;
        google::protobuf::util::MessageToJsonString(metrics, &metrics_json, options);
        LOG(INFO) << "Got metrics from Physiology REST API at " << timestamp_microseconds << " microseconds from first frame: " << metrics_json;
        return absl::OkStatus();
    });
    if (status.ok()) { status = container.Initialize(); }
    if (status.ok()) { status = container.Run(); }

    if (!status.ok()) {
        LOG(ERROR) << "Run failed. " << status.message();
        return EXIT_FAILURE;
    } else {
        LOG(INFO) << "Success!";
    }
}
