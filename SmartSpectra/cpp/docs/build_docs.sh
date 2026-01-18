#!/usr/bin/env bash
set -e

# Build the Doxygen documentation

BUILD_DIR="cmake-build-release"
mkdir -p "$BUILD_DIR"

cmake -S . -B "$BUILD_DIR" -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_DOCS=ON
cmake --build "$BUILD_DIR" --target doxygen

# Copy presage-common.css to the generated HTML output directory
HTML_OUTPUT_DIR="$BUILD_DIR/docs/generated/html"
if [ -d "$HTML_OUTPUT_DIR" ]; then
    # Check if we're running from repo root or smartspectra/cpp directory
    if [ -f "smartspectra/cpp/docs/presage-common.css" ]; then
        # Running from repo root (GitLab CI)
        cp smartspectra/cpp/docs/presage-common.css "$HTML_OUTPUT_DIR/"
        echo "Copied presage-common.css to $HTML_OUTPUT_DIR (from repo root)"
    elif [ -f "docs/presage-common.css" ]; then
        # Running from smartspectra/cpp directory (local)
        cp docs/presage-common.css "$HTML_OUTPUT_DIR/"
        echo "Copied presage-common.css to $HTML_OUTPUT_DIR (from cpp dir)"
    else
        echo "Error: presage-common.css not found in expected locations"
        exit 1
    fi
else
    echo "Warning: HTML output directory $HTML_OUTPUT_DIR not found"
fi

# Generate protobuf documentation for OnPrem services
echo "Generating protobuf documentation for OnPrem services..."

# Check if we're running from repo root or smartspectra/cpp directory
if [ -f "smartspectra/cpp/on_prem/docs/generate_protobuf_docs.sh" ]; then
    # Running from repo root (GitLab CI)
    bash smartspectra/cpp/on_prem/docs/generate_protobuf_docs.sh
    echo "Generated protobuf documentation (from repo root)"
elif [ -f "on_prem/docs/generate_protobuf_docs.sh" ]; then
    # Running from smartspectra/cpp directory (local)
    cd ../..  # Go to repo root
    bash smartspectra/cpp/on_prem/docs/generate_protobuf_docs.sh
    cd smartspectra/cpp  # Return to cpp directory
    echo "Generated protobuf documentation (from cpp dir)"
else
    echo "Warning: protobuf documentation generation script not found"
    echo "Looked for:"
    echo "  - smartspectra/cpp/on_prem/docs/generate_protobuf_docs.sh (from repo root)"
    echo "  - on_prem/docs/generate_protobuf_docs.sh (from cpp dir)"
fi

echo "Documentation build complete!"
