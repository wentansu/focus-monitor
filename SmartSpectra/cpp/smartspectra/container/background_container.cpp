//
// Created by greg on 4/29/24.
// Copyright (c) 2024 Presage Technologies
//

// === standard library includes (if any) ===
// === third-party includes (if any) ===
// === local includes (if any) ===
#include "background_container_impl.hpp"
namespace presage::smartspectra::container {
template class BackgroundContainer<platform_independence::DeviceType::Cpu, settings::OperationMode::Spot, settings::IntegrationMode::Rest>;
template class BackgroundContainer<platform_independence::DeviceType::Cpu, settings::OperationMode::Continuous, settings::IntegrationMode::Rest>;
template class BackgroundContainer<platform_independence::DeviceType::Cpu, settings::OperationMode::Continuous, settings::IntegrationMode::Grpc>;
#ifdef WITH_OPENGL
template class BackgroundContainer<platform_independence::DeviceType::OpenGl, settings::OperationMode::Spot, settings::IntegrationMode::Rest>;
#endif

} // namespace presage::smartspectra::container
