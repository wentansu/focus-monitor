// SmartSpectra C++ SDK Documentation - Header/Footer Injection
(function() {
    'use strict';

    // Wait for DOM to be ready
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }

    function init() {
        // Add Google Fonts
        const fontsLink = document.createElement('link');
        fontsLink.href = 'https://fonts.googleapis.com/css2?family=Montserrat:wght@300;400;500;600;700;800&family=Source+Serif+Pro:wght@400;600&display=swap';
        fontsLink.rel = 'stylesheet';
        document.head.appendChild(fontsLink);

        // Wait for Doxygen to fully load, then inject
        setTimeout(() => {
            injectHeader();
            injectFooter();
        }, 500);
    }

    function injectHeader() {
        // Remove existing header if it exists
        const existingHeader = document.querySelector('.presage-header');
        if (existingHeader) {
            existingHeader.remove();
        }

        const header = document.createElement('header');
        header.className = 'presage-header';
        header.innerHTML = `
            <div class="presage-header-content">
                <a href="https://physiology.presagetech.com" class="presage-logo">
                    <img src="https://physiology.presagetech.com/assets/images/logo.webp" alt="Presage Technologies Logo">
                </a>
                <nav class="presage-nav">
                    <a href="https://docs.physiology.presagetech.com/">Documentation</a>
                    <a href="https://docs.physiology.presagetech.com/android/index.html">Android</a>
                    <a href="https://docs.physiology.presagetech.com/swift/documentation/smartspectraswiftsdk/">iOS/Swift</a>
                    <a href="https://docs.physiology.presagetech.com/cpp/index.html" class="active">C++</a>
                    <a href="https://physiology.presagetech.com">Portal</a>
                </nav>
            </div>
        `;

        // Insert header as the very first element in body
        document.body.insertBefore(header, document.body.firstChild);
        
        // Header injected successfully
    }

    function injectFooter() {
        // Remove existing footer if it exists
        const existingFooter = document.querySelector('.presage-footer');
        if (existingFooter) {
            existingFooter.remove();
        }

        const footer = document.createElement('footer');
        footer.className = 'presage-footer';
        footer.innerHTML = `
            <div class="presage-footer-content">
                <div class="presage-footer-bottom">
                    &copy; 2025 Presage Technologies
                </div>
                
                <div class="presage-footer-center">
                    <a href="https://docs.physiology.presagetech.com/android/index.html">Android</a>
                    <a href="https://docs.physiology.presagetech.com/swift/documentation/smartspectraswiftsdk/">iOS/Swift</a>
                    <a href="https://docs.physiology.presagetech.com/cpp/index.html">C++</a>
                    <a href="https://physiology.presagetech.com">Portal</a>
                </div>

                <div class="presage-footer-links">
                    <div class="presage-footer-section">
                        <ul>
                            <li><a href="https://presagetechnologies.com/support">Support</a></li>
                            <li><span class="presage-footer-separator">|</span></li>
                            <li><a href="https://presagetechnologies.com/contact-us">Contact</a></li>
                            <li><span class="presage-footer-separator">|</span></li>
                            <li><a href="https://presagetechnologies.com/">Company</a></li>
                        </ul>
                    </div>
                </div>
            </div>
        `;

        document.body.appendChild(footer);
        
        // Footer injected successfully
    }
})();