// swift-tools-version: 5.10
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
    name: "SmartSpectraSwiftSDK",
    platforms: [
        .iOS(.v15)
    ],
    products: [
        // Products define the executables and libraries a package produces, making them visible to other packages.
        .library(
            name: "SmartSpectraSwiftSDK",
            targets: ["SmartSpectraSwiftSDK"]),
    ],
    dependencies: [
        .package(url: "https://github.com/apple/swift-protobuf.git", from: "1.28.1"),
    ],
    targets: [
        // Targets are the basic building blocks of a package, defining a module or a test suite.
        // Targets can depend on other targets in this package and products from dependencies.
        .target(
            name: "SmartSpectraSwiftSDK",
            dependencies: [
                "PresagePreprocessing",
                .product(name: "SwiftProtobuf", package: "swift-protobuf")
            ],
            path: "swift/sdk/Sources/SmartSpectraSwiftSDK",
            resources: [
                .process("Resources/PrivacyInfo.xcprivacy")
            ]
        ),
        .testTarget(
            name: "SmartSpectraSwiftSDKTests",
            dependencies: ["SmartSpectraSwiftSDK"],
            path: "swift/sdk/Tests/SmartSpectraSwiftSDKTests"
        ),
        .binaryTarget(
            name: "PresagePreprocessing",
            path: "swift/sdk/Sources/Frameworks/PresagePreprocessing.xcframework"
        )
    ]
)
