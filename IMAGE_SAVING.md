# Saving Generated Images to /images Folder

The orchestrator automatically creates visualizations of your heart rate data. By default, these are stored in the SAM artifact service (`/tmp/samv2`). 

To automatically save all generated images to the `/images` folder, use the artifact monitor script.

## Quick Start

**Terminal 1** - Run SAM:
```bash
uv run sam run configs/
```

**Terminal 2** - Run heart rate monitor:
```bash
uv run python src/main.py
```

**Terminal 3** - Run artifact monitor (saves images):
```bash
uv run python monitor_artifacts.py
```

## What It Does

The `monitor_artifacts.py` script:
- ✅ Watches the SAM artifact directory for new .png images
- ✅ Automatically copies them to `/images` with timestamps
- ✅ Creates a `_latest` version for easy access
- ✅ Runs continuously in the background

## Output Examples

When new images are detected, you'll see:
```
2026-01-18 05:00:15 - INFO - Found 2 new image(s)
2026-01-18 05:00:15 - INFO - ✅ Copied: heart_rate_distribution.png -> heart_rate_distribution_20260118_050015.png
2026-01-18 05:00:15 - INFO -    Updated: heart_rate_distribution_latest.png
2026-01-18 05:00:15 - INFO - ✅ Copied: heart_rate_analysis.png -> heart_rate_analysis_20260118_050015.png
2026-01-18 05:00:15 - INFO -    Updated: heart_rate_analysis_latest.png
```

## File Organization

```
/images/
├── heart_rate_distribution_20260118_050015.png  # Timestamped version
├── heart_rate_distribution_latest.png           # Always the newest
├── heart_rate_analysis_20260118_050015.png
├── heart_rate_analysis_latest.png
├── heart_rate_distribution_20260118_050230.png  # Next batch
├── heart_rate_analysis_20260118_050230.png
└── ...
```

## Configuration

Edit `monitor_artifacts.py` to customize:

```python
ARTIFACT_BASE_PATH = "/tmp/samv2/sam_dev_user"  # Source directory
OUTPUT_DIR = Path(__file__).parent / "images"    # Destination
CHECK_INTERVAL = 2  # How often to check (seconds)
```

## Stopping the Monitor

Press `Ctrl+C` in the terminal running `monitor_artifacts.py`:

```
2026-01-18 05:15:30 - INFO - Monitor stopped by user
2026-01-18 05:15:30 - INFO - Total images processed: 12
```

## Alternative: Manual Copy

If you don't want to run the monitor continuously, you can manually copy images:

```bash
# Find all heart rate images
find /tmp/samv2/sam_dev_user -name "*.png" -path "*/heart_rate*" -type f

# Copy them manually
cp /tmp/samv2/sam_dev_user/web-session-*/heart_rate*.png/0 images/
```

## Integration with Other Tools

The `/images` folder makes it easy to:
- Share visualizations with others
- Include in reports or presentations
- Process with other scripts
- Upload to cloud storage
- Archive for long-term tracking

## Troubleshooting

### "No images found"
- Make sure SAM is running (`uv run sam run configs/`)
- Make sure the heart rate monitor is running and sending data
- Check that you've sent at least 10 BPM readings (triggers orchestrator analysis)

### "Permission denied"
- Make sure the script is executable: `chmod +x monitor_artifacts.py`
- Check that `/tmp/samv2` is readable

### "Wrong directory"
- The script looks for `ARTIFACT_BASE_PATH = "/tmp/samv2/sam_dev_user"`
- If your SAM uses a different path, edit the script
