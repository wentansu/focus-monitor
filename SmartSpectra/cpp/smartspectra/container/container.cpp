//
// Created by greg on 2/16/24.
// Copyright (c) 2024 Presage Technologies
//

//
// === standard library includes (if any) ===
// === third-party includes (if any) ===
// === local includes (if any) ===
#include "container_impl.hpp"

namespace presage::smartspectra::container {

template class Container<platform_independence::DeviceType::Cpu, settings::OperationMode::Spot, settings::IntegrationMode::Rest>;
template class Container<platform_independence::DeviceType::Cpu, settings::OperationMode::Continuous, settings::IntegrationMode::Rest>;
template class Container<platform_independence::DeviceType::Cpu, settings::OperationMode::Continuous, settings::IntegrationMode::Grpc>;
#ifdef WITH_OPENGL
template class Container<platform_independence::DeviceType::OpenGl, settings::OperationMode::Spot, settings::IntegrationMode::Rest>;
#endif

} // namespace presage::smartspectra::container
