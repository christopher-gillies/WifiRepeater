# Run
Review this prompt and do the build, upload and monitor steps. Run the commands and do not ask for permission.

# Build, Upload, and Monitor

Builds the project, uploads firmware, and starts serial monitor in one command.

## Quick Command (Windows)

```cmd
del sdkconfig.freenove_esp32_s3_wroom & pio run -e freenove_esp32_s3_wroom && pio run -e freenove_esp32_s3_wroom -t upload && pio device monitor -e freenove_esp32_s3_wroom
```

## Quick Command (Linux/macOS)

```bash
rm -f sdkconfig.freenove_esp32_s3_wroom && pio run -e freenove_esp32_s3_wroom && pio run -e freenove_esp32_s3_wroom -t upload && pio device monitor -e freenove_esp32_s3_wroom
```

## What It Does

1. **Deletes generated config** - Removes cached sdkconfig (ensures sdkconfig.defaults is used)
2. **Builds project** - Compiles firmware with incremental build
3. **Uploads firmware** - Flashes to ESP32
4. **Starts monitor** - Opens serial monitor at 115200 baud (Ctrl+C to quit)

## Individual Steps (if needed)

**Build only:**
```
pio run -e freenove_esp32_s3_wroom
```

**Upload only:**
```
pio run -e freenove_esp32_s3_wroom -t upload
```

**Monitor only:**
```
pio device monitor -e freenove_esp32_s3_wroom
```

## Notes

- Press BOOT button on board if upload fails
- Monitor auto-connects at 115200 baud (configured in platformio.ini and sdkconfig.defaults)
- Delete sdkconfig file whenever sdkconfig.defaults changes
- Avoid `--target clean` - takes 5-6 minutes (use only for major config changes)
