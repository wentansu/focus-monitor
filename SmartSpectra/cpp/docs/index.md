# SmartSpectra C++ SDK Documentation {#docs_index}

Welcome to the SmartSpectra C++ SDK documentation! This SDK enables you to measure human vitals (heart rate and respiration) from video streams in real-time.

## 🚀 Quick Start

### Prerequisites

#### Build Tools & CMake 3.27.0+

Install essential build tools and CMake:

```shell
# Install build tools (required for CMAKE_MAKE_PROGRAM and CMAKE_CXX_COMPILER)
sudo apt update
sudo apt install -y build-essential git lsb-release libcurl4-openssl-dev libssl-dev pkg-config libv4l-dev libgles2-mesa-dev libunwind-dev

# Remove old cmake if installed
sudo apt remove --purge --auto-remove cmake

# Install dependencies for CMake
sudo apt install -y software-properties-common lsb-release wget gnupg

# Download and install Kitware's signing key
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null

# Add Kitware repository
echo "deb https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main" | sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null
sudo apt update

# Install CMake
sudo apt install cmake

# Optional: Install Ninja for faster builds
sudo apt install ninja-build
```

Verify installation: `cmake --version` (should be 3.27.0+) and `gcc --version`

#### SmartSpectra SDK Installation

```shell
# Add Presage repository
curl -s "https://presage-security.github.io/PPA/KEY.gpg" | gpg --dearmor | sudo tee /etc/apt/trusted.gpg.d/presage-technologies.gpg >/dev/null
sudo curl -s --compressed -o /etc/apt/sources.list.d/presage-technologies.list "https://presage-security.github.io/PPA/presage-technologies.list"

# Install SDK
sudo apt update
sudo apt install libsmartspectra-dev
```

### Hello World Example

Here's a minimal example that measures vitals from your camera:

```cpp
// hello_vitals.cpp
// SmartSpectra Hello Vitals - Minimal Example

#include <smartspectra/container/foreground_container.hpp>
#include <smartspectra/container/settings.hpp>
#include <smartspectra/gui/opencv_hud.hpp>
#include <physiology/modules/messages/metrics.h>
#include <physiology/modules/messages/status.h>
#include <glog/logging.h>
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace presage::smartspectra;

int main(int argc, char** argv) {
    // Initialize logging
    google::InitGoogleLogging(argv[0]);
    FLAGS_alsologtostderr = true;
    
    // Get API key
    std::string api_key;
    if (argc > 1) {
        api_key = argv[1];
    } else if (const char* env_key = std::getenv("SMARTSPECTRA_API_KEY")) {
        api_key = env_key;
    } else {
        std::cout << "Usage: ./hello_vitals YOUR_API_KEY\n";
        std::cout << "Or set SMARTSPECTRA_API_KEY environment variable\n";
        std::cout << "Get your API key from: https://physiology.presagetech.com\n";
        return 1;
    }
    
    std::cout << "Starting SmartSpectra Hello Vitals...\n";
    
    try {
        // Create settings
        container::settings::Settings<
            container::settings::OperationMode::Continuous,
            container::settings::IntegrationMode::Rest
        > settings;
        
        // Configure camera (defaults work for most cases)
        settings.video_source.device_index = 0;
        settings.video_source.resolution_selection_mode = video_source::ResolutionSelectionMode::Auto;
        settings.video_source.capture_width_px = 1280;
        settings.video_source.capture_height_px = 720;
        settings.video_source.codec = presage::camera::CaptureCodec::MJPG;
        settings.video_source.auto_lock = true;
        
        // Basic settings
        settings.headless = false;
        settings.enable_edge_metrics = true;
        settings.verbosity_level = 1;
        
        // Continuous mode buffer
        settings.continuous.preprocessed_data_buffer_duration_s = 0.5;
        
        // API key for REST
        settings.integration.api_key = api_key;
        
        // Create container and HUD
        auto container = std::make_unique<container::CpuContinuousRestForegroundContainer>(settings);
        auto hud = std::make_unique<gui::OpenCvHud>(10, 0, 1260, 400);
        
        // Set up callbacks
        if (auto status = container->SetOnCoreMetricsOutput(
            [&hud](const presage::physiology::MetricsBuffer& metrics, int64_t timestamp) {
                int pulse = static_cast<int>(metrics.pulse().strict().value());
                int breathing = static_cast<int>(metrics.breathing().strict().value());
                
                std::cout << "Vitals - Pulse: " << pulse << " BPM, Breathing: " << breathing << " BPM\n";
                hud->UpdateWithNewMetrics(metrics);
                return absl::OkStatus();
            }); !status.ok()) {
            std::cerr << "Failed to set metrics callback: " << status.message() << "\n";
            return 1;
        }
        
        if (auto status = container->SetOnVideoOutput(
            [&hud](cv::Mat& frame, int64_t timestamp) {
                hud->Render(frame);
                cv::imshow("SmartSpectra Hello Vitals", frame);
                
                char key = cv::waitKey(1) & 0xFF;
                if (key == 'q' || key == 27) {
                    return absl::CancelledError("User quit");
                }
                return absl::OkStatus();
            }); !status.ok()) {
            std::cerr << "Failed to set video callback: " << status.message() << "\n";
            return 1;
        }
        
        // Initialize and run
        std::cout << "Initializing camera and processing...\n";
        if (auto status = container->Initialize(); !status.ok()) {
            std::cerr << "Failed to initialize: " << status.message() << "\n";
            return 1;
        }
        
        std::cout << "Ready! Press 'q' to quit.\n";
        if (auto status = container->Run(); !status.ok()) {
            std::cerr << "Processing failed: " << status.message() << "\n";
            return 1;
        }
        
        cv::destroyAllWindows();
        std::cout << "Done!\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
```

When everything works correctly, you'll see a camera window with real-time vital sign measurements displayed as an overlay, like this:

![SmartSpectra C++ SDK Demo](images/cpp-quickstart.gif)

*The demo shows the camera feed with pulse and breathing rate measurements overlaid in real-time.*

### Build Your Application

Create a `CMakeLists.txt` file:

```cmake
cmake_minimum_required(VERSION 3.27.0)
project(SmartSpectraHelloVitals CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(SmartSpectra REQUIRED)
find_package(OpenCV REQUIRED)

# Create executable
add_executable(hello_vitals hello_vitals.cpp)

# Link libraries
target_link_libraries(hello_vitals
    SmartSpectra::Container
    SmartSpectra::Gui
    ${OpenCV_LIBS}
)
```

Then build and run:

```shell
# Build
mkdir build && cd build
cmake .. && make

# Run with API key
./hello_vitals YOUR_API_KEY

# Or set environment variable
export SMARTSPECTRA_API_KEY=YOUR_KEY
./hello_vitals
```

## 📚 Documentation Structure

This documentation is generated with [Doxygen](https://www.doxygen.nl/) and styled with [Doxygen Awesome](https://jothepro.github.io/doxygen-awesome-css/).

### Core Components

#### [Containers](@ref container)

- **`ForegroundContainer`** – Complete solution with built-in video source and optional GUI
- **`BackgroundContainer`** – Headless processing for server/embedded applications  
- **`Container`** – Base class with common functionality

#### [Video Sources](@ref video_source)

- **`VideoSource`** interface for custom video inputs
- Factory helpers for camera selection and configuration
- Support for live cameras, video files, and custom streams

#### Utilities & Helpers

- **Graph Initialization** – Device setup and MediaPipe graph configuration
- **Benchmarking Tools** – Performance monitoring and telemetry
- **Frame Transfer** – Efficient video frame handling
- **JSON I/O** – Configuration and metrics serialization
- **Input Handling** – Keyboard controls and user interaction

## 🔧 Key Features

- **Real-time Processing** – Low-latency vital sign measurement
- **Multiple Operation Modes** – Spot measurements or continuous monitoring
- **Flexible Integration** – REST API or OnPrem solutions
- **Cross-Platform** – Linux, macOS, and Windows support
- **GPU Acceleration** – Optional CUDA support for enhanced performance

## 📖 Usage Patterns

### Spot Measurements

Perfect for one-time health checks or periodic monitoring:

```cpp
settings::Settings<settings::OperationMode::Spot, settings::IntegrationMode::Rest> settings;
settings.spot.spot_duration_s = 30;  // 30-second measurement
```

### Continuous Monitoring  

Ideal for real-time applications and live dashboards:

```cpp
settings::Settings<settings::OperationMode::Continuous, settings::IntegrationMode::Rest> settings;
settings.continuous.enable_edge_metrics = true;  // Real-time metrics
```

## 🎯 Next Steps

1. **Get your API key** from [physiology.presagetech.com](https://physiology.presagetech.com)
2. **Try the examples** in the [samples](samples) directory
3. **Read the full guide** in the main [README](@ref mainpage)
4. **Explore the API reference** using the navigation above

## 🛠 Generating Documentation

To build this documentation locally:

```shell
# From the smartspectra/cpp directory
bash docs/build_docs.sh
```

Open `docs/generated/html/index.html` in your browser.

For detailed build instructions, see [Linux Build Guide](build_linux.md) or [macOS Build Guide](build_macos.md).

## 💡 Support

- **Documentation Issues**: Check the [samples](samples) directory for working examples
- **Technical Support**: Contact [support@presagetech.com](mailto:support@presagetech.com)  
- **Bug Reports**: [Submit a GitHub issue](https://github.com/Presage-Security/SmartSpectra/issues)

---

*Ready to measure vitals from video? Start with the [Hello World](#hello-world-example) example above!*
