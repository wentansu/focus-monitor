#!/usr/bin/env python3
"""
Artifact Monitor - Automatically copies generated heart rate images to /images folder

This script monitors the SAM artifact directory and copies any new .png files
to the project's /images folder for easy access.
"""

import os
import shutil
import time
import logging
from pathlib import Path
from datetime import datetime

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)
log = logging.getLogger(__name__)

# Configuration
ARTIFACT_BASE_PATH = "/tmp/samv2/sam_dev_user"
OUTPUT_DIR = Path(__file__).parent / "images"
CHECK_INTERVAL = 2  # seconds
WATCH_EXTENSIONS = [".png", ".jpg", ".jpeg", ".svg"]

# Track processed files to avoid duplicates
processed_files = set()

def ensure_output_dir():
    """Create the output directory if it doesn't exist."""
    OUTPUT_DIR.mkdir(exist_ok=True)
    log.info(f"Output directory: {OUTPUT_DIR}")

def find_new_images():
    """Find new image files in the artifact directory."""
    new_images = []
    
    if not os.path.exists(ARTIFACT_BASE_PATH):
        return new_images
    
    # Walk through all subdirectories
    for root, dirs, files in os.walk(ARTIFACT_BASE_PATH):
        for file in files:
            file_path = os.path.join(root, file)
            
            # Check for versioned files (files without extension that are actually images) 
            # OR standard image files
            is_image = False
            
            # Case 1: Standard extension
            if any(file.endswith(ext) for ext in WATCH_EXTENSIONS):
                is_image = True
                
            # Case 2: Version file (numeric) inside an image-named directory
            elif file.isdigit() or file == "0":
                parent_dir = os.path.basename(root)
                # If parent dir looks like an image filename (e.g. "plot.png")
                if any(ext in parent_dir for ext in WATCH_EXTENSIONS):
                    is_image = True
            
            if is_image and file_path not in processed_files:
                try:
                    # Verify it's actually an image by checking header
                    with open(file_path, 'rb') as f:
                        header = f.read(16)
                        if (header.startswith(b'\x89PNG') or  # PNG
                            header.startswith(b'\xff\xd8\xff') or  # JPEG
                            header.startswith(b'<svg')):  # SVG
                            
                            # Use parent dir name as artifact name for versioned files
                            if file.isdigit() or file == "0":
                                artifact_name = os.path.basename(root)
                            else:
                                artifact_name = file
                                
                            new_images.append((file_path, artifact_name))
                            processed_files.add(file_path)
                            log.info(f"Found new image: {artifact_name} at {file_path}")
                            
                except Exception as e:
                    log.debug(f"Skipping {file_path}: {e}")
    
    return new_images

def copy_image(source_path, artifact_name):
    """Copy an image file to the output directory with a timestamped name."""
    try:
        # Extract the base name without version number
        base_name = artifact_name
        
        # Create a timestamped filename
        timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
        
        # Determine extension from artifact name
        ext = None
        for extension in WATCH_EXTENSIONS:
            if extension in artifact_name:
                ext = extension
                base_name = artifact_name.replace(extension, '')
                break
        
        if not ext:
            ext = '.png'  # Default to PNG
        
        # Create destination filename
        dest_filename = f"{base_name}_{timestamp}{ext}"
        dest_path = OUTPUT_DIR / dest_filename
        
        # Copy the file
        shutil.copy2(source_path, dest_path)
        log.info(f"✅ Copied: {artifact_name} -> {dest_filename}")
        
        # Also create a "latest" symlink/copy
        latest_path = OUTPUT_DIR / f"{base_name}_latest{ext}"
        if latest_path.exists():
            latest_path.unlink()
        shutil.copy2(source_path, latest_path)
        log.info(f"   Updated: {base_name}_latest{ext}")
        
        return True
    
    except Exception as e:
        log.error(f"❌ Failed to copy {artifact_name}: {e}")
        return False

def main():
    """Main monitoring loop."""
    log.info("=" * 60)
    log.info("Heart Rate Image Monitor - Starting")
    log.info("=" * 60)
    log.info(f"Monitoring: {ARTIFACT_BASE_PATH}")
    log.info(f"Output to: {OUTPUT_DIR}")
    log.info(f"Check interval: {CHECK_INTERVAL}s")
    log.info("")
    log.info("Watching for new heart rate visualization images...")
    log.info("Press Ctrl+C to stop")
    log.info("")
    
    ensure_output_dir()
    
    try:
        while True:
            new_images = find_new_images()
            
            if new_images:
                log.info(f"Found {len(new_images)} new image(s)")
                for source_path, artifact_name in new_images:
                    copy_image(source_path, artifact_name)
            
            time.sleep(CHECK_INTERVAL)
    
    except KeyboardInterrupt:
        log.info("")
        log.info("=" * 60)
        log.info("Monitor stopped by user")
        log.info(f"Total images processed: {len(processed_files)}")
        log.info("=" * 60)

if __name__ == "__main__":
    main()
