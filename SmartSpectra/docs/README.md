# SmartSpectra SDK Documentation - Unified Header & Footer System

This directory contains the unified header and footer system for SmartSpectra SDK documentation across all platforms (Android, Swift, and C++).

## Overview

The unified system provides consistent branding and navigation across all SDK documentation sites while accommodating the different documentation generation tools:

- **C++ SDK**: Uses Doxygen with custom header/footer HTML templates
- **Android SDK**: Uses Dokka with JavaScript injection for header/footer
- **Swift SDK**: Uses DoCC with JavaScript injection for header/footer

## Files Structure

```
smartspectra/
├── docs/
│   └── README.md               # This documentation file
├── android/docs/
│   ├── presage-dokka.css       # Android-specific styles
│   └── presage-inject.js       # JavaScript to inject header/footer
├── swift/docs/
│   ├── presage-docc.css        # Swift-specific styles
│   └── presage-inject.js       # JavaScript to inject header/footer
└── cpp/docs/
    ├── unified_header.html     # Doxygen header template
    ├── unified_footer.html     # Doxygen footer template
    ├── presage-common.css      # C++ documentation styles
    ├── build_docs.sh           # Build script for C++ docs
    └── Doxyfile.in             # Doxygen configuration
```

## How It Works

### For C++ (Doxygen)
- Uses `unified_header.html` and `unified_footer.html` templates
- Configured in `Doxyfile.in` with `HTML_HEADER` and `HTML_FOOTER` options
- Includes `presage-common.css` for styling
- Header/footer are built into the generated HTML

### For Android (Dokka)
- Uses JavaScript injection via `presage-inject.js`
- CSS styling via `presage-dokka.css`
- CI/CD pipeline injects the JavaScript into generated HTML files
- Header/footer are added dynamically when pages load

### For Swift (DoCC)
- Uses JavaScript injection via `presage-inject.js`
- CSS styling via `presage-docc.css`
- CI/CD pipeline injects the JavaScript into generated HTML files
- Header/footer are added dynamically when pages load

## CI/CD Integration

The unified system is integrated into the GitLab CI/CD pipeline:

### Android (`android_dokka` job)
```yaml
# Copy unified header/footer assets
- cp docs/presage-dokka.css "$OUTPUT_DIR/"
- cp docs/presage-inject.js "$OUTPUT_DIR/"
# Inject the header/footer JavaScript into each HTML file
- find "$OUTPUT_DIR" -name "*.html" -type f -exec sh -c '
    sed -i "s|</head>|<script src=\"/android/presage-inject.js\"></script></head>|" "$1"
  ' _ {} \;
```

### Swift (`swift_docc` job)
```yaml
# Copy unified header/footer assets
- cp docs/presage-docc.css "$OUTPUT_DIR/"
- cp docs/presage-inject.js "$OUTPUT_DIR/"
# Inject the header/footer JavaScript into each HTML file
- find "$OUTPUT_DIR" -name "*.html" -type f -exec sh -c '
    sed -i "" "s|</head>|<script src=\"/swift/presage-inject.js\"></script></head>|" "$1"
  ' _ {} \;
```

### C++ (`generate_doxygen_docs` job)
- Header/footer are automatically included during Doxygen build
- No additional CI/CD steps required

## Navigation Links

The unified header includes navigation to:
- Home (main documentation portal)
- Android SDK documentation
- Swift SDK documentation
- C++ SDK documentation
- Developer Portal

The active SDK is highlighted in the navigation.

## Footer Content

The unified footer includes:
- Links to all SDK documentation
- Resource links (Developer Portal, GitHub, Support)
- Company information
- Copyright notice

## Customization

To modify the header/footer:

1. **For all platforms**: Update the styles in each platform-specific CSS file
2. **For specific platforms**: Update the platform-specific files  
3. **For navigation**: Update the navigation links in all header templates/injection scripts
4. **For styling**: Update the CSS files for each platform

## Maintenance

When updating the unified system:

1. Test changes with all three documentation generation tools
2. Ensure responsive design works across all platforms
3. Verify navigation links are correct and functional
4. Check that the CI/CD pipeline properly deploys the changes

## Brand Guidelines

The unified system follows Presage Technologies brand guidelines:
- Primary color: `#c91e1e`
- Secondary color: `#425a76`
- Hover color: `#405238`
- Typography: Inter font family (loaded with SRI protection)
- Logo: Presage Technologies logo from the main site

## Security Features

### Subresource Integrity (SRI)
All external resources (Google Fonts) are loaded with SRI protection:
- **SRI Hash**: `sha384-8ja2fRQufUmQXQFDsDcFl7WKysfokCEbehppUwEusw4hGvP7WtVsgMvg9B7wSPSc`
- **Cross-Origin**: `anonymous` attribute for proper CORS handling
- **Protection**: Prevents tampering with external font resources

### Fixed Header Implementation
Headers are implemented as fixed-position elements that stay visible while scrolling:
- **Android**: `padding-top: 120px` added to body
- **Swift**: `padding-top: 130px` added to body
- **C++**: Uses relative positioning with Doxygen templates

## Support

For issues or questions about the unified documentation system, please contact the development team or create an issue in the project repository.
