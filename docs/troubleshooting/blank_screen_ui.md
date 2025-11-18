# Troubleshooting: Blank Screen UI

## Problem Description
The application starts but displays only a blank/black screen with no UI elements visible.

## Common Causes

### 1. QML Loading Failures

**Symptoms:**
- Black/blank window
- Application appears to run but nothing displays
- No error messages in console (potentially)

**Diagnosis:**
```bash
# Run with QML debugging enabled
QT_LOGGING_RULES="qt.qml.*=true" ./CrankshaftReborn

# Or with more verbose Qt debugging
QT_DEBUG_PLUGINS=1 QT_LOGGING_RULES="*=true" ./CrankshaftReborn
```

**Common Issues:**
- Missing QML files in installation
- Incorrect QML module paths
- Missing Qt Quick modules

**Solutions:**
```bash
# Check if QML files are installed
ls -la /usr/share/crankshaft_reborn/qml/
ls -la /usr/lib/crankshaft_reborn/

# Verify Qt Quick modules are available
qml6 -h  # Should show Qt QML runtime
dpkg -l | grep qt6-declarative  # Check if installed

# Install missing Qt Quick components (Debian/Raspberry Pi OS)
sudo apt-get install qt6-declarative-dev qml6-module-qtquick \
    qml6-module-qtquick-controls qml6-module-qtquick-layouts \
    qml6-module-qtquick-window
```

### 2. Graphics/OpenGL Issues

**Symptoms:**
- Blank screen with warnings about OpenGL
- Messages like "Failed to create OpenGL context"
- Works on some systems but not others

**Diagnosis:**
```bash
# Check OpenGL support
glxinfo | grep "OpenGL version"  # For X11
eglinfo  # For EGL/embedded systems

# Run with software rendering
QT_QUICK_BACKEND=software ./CrankshaftReborn

# Check Qt platform information
QT_DEBUG_PLUGINS=1 ./CrankshaftReborn 2>&1 | grep -i platform
```

**Solutions:**

For Raspberry Pi:
```bash
# Ensure proper GPU drivers
sudo raspi-config
# Navigate to: Advanced Options -> GL Driver -> Full KMS

# Add to /boot/config.txt
dtoverlay=vc4-kms-v3d

# Reboot
sudo reboot
```

For X11 systems:
```bash
# Install Mesa drivers
sudo apt-get install mesa-utils libgl1-mesa-dri

# Test OpenGL
glxgears  # Should show spinning gears
```

For headless/framebuffer systems:
```bash
# Use EGL backend
export QT_QPA_PLATFORM=eglfs
./CrankshaftReborn

# Or VNC for remote display
export QT_QPA_PLATFORM=vnc
./CrankshaftReborn
```

### 3. Missing Display Platform

**Symptoms:**
- "Could not find the Qt platform plugin" error
- Application fails to start or shows blank window

**Diagnosis:**
```bash
# List available Qt platforms
ls /usr/lib/*/qt6/plugins/platforms/

# Check which platform Qt is trying to use
QT_DEBUG_PLUGINS=1 ./CrankshaftReborn 2>&1 | grep platform
```

**Solutions:**
```bash
# Install platform plugins
sudo apt-get install qt6-qpa-plugins

# Explicitly set platform
export QT_QPA_PLATFORM=xcb  # For X11
# OR
export QT_QPA_PLATFORM=wayland  # For Wayland
# OR
export QT_QPA_PLATFORM=eglfs  # For embedded/framebuffer

./CrankshaftReborn
```

### 4. XKB/Keyboard Configuration Issues

**Symptoms:**
- Blank screen
- Qt warnings about XKB or keyboard layout
- Works with some Qt versions but not others

**Diagnosis:**
```bash
# Check XKB installation
dpkg -l | grep xkb

# Run with XKB debugging
QT_XKB_DEBUG=1 ./CrankshaftReborn
```

**Solutions:**
```bash
# Install XKB components
sudo apt-get install libxkbcommon-dev libxkbcommon-x11-0

# Disable XKB if not needed (embedded systems)
export QT_XKB_CONFIG_ROOT=/usr/share/X11/xkb
```

### 5. Extension Loading Failures

**Symptoms:**
- Blank screen after splash (if any)
- Extensions fail to load silently
- UI framework initializes but no content

**Diagnosis:**
```bash
# Check extension directory
ls -la /usr/lib/crankshaft_reborn/extensions/

# Verify extension manifests
cat /usr/lib/crankshaft_reborn/extensions/*/manifest.json

# Run with verbose logging
./CrankshaftReborn --verbose
# OR
QT_LOGGING_RULES="*.debug=true" ./CrankshaftReborn
```

**Solutions:**
```bash
# Check extension permissions
sudo chmod -R 755 /usr/lib/crankshaft_reborn/extensions/

# Verify manifest format
jq . /usr/lib/crankshaft_reborn/extensions/ui/manifest.json

# Check for missing extension dependencies
ldd /usr/lib/crankshaft_reborn/extensions/ui/*.so
```

### 6. Display Resolution/Scaling Issues

**Symptoms:**
- Window appears but is completely blank
- UI elements might be rendering off-screen
- Happens on specific display configurations

**Diagnosis:**
```bash
# Check display information
xrandr  # For X11
fbset  # For framebuffer

# Override display scaling
export QT_SCALE_FACTOR=1
export QT_AUTO_SCREEN_SCALE_FACTOR=0
./CrankshaftReborn

# Set specific window size
./CrankshaftReborn --window-size=800x480
```

**Solutions:**
```bash
# Force specific resolution
export QT_QPA_EGLFS_WIDTH=1920
export QT_QPA_EGLFS_HEIGHT=1080
./CrankshaftReborn

# Disable high DPI scaling
export QT_ENABLE_HIGHDPI_SCALING=0
```

## Debugging Workflow

### Step 1: Enable Maximum Logging
```bash
export QT_LOGGING_RULES="*=true"
export QT_DEBUG_PLUGINS=1
./CrankshaftReborn 2>&1 | tee debug.log
```

### Step 2: Check Core Dependencies
```bash
# Verify Qt6 installation
dpkg -l | grep qt6

# Check library dependencies
ldd ./CrankshaftReborn

# Verify all shared libraries load
ldd ./CrankshaftReborn | grep "not found"
```

### Step 3: Test with Minimal Configuration
```bash
# Try software rendering (slower but more compatible)
QT_QUICK_BACKEND=software ./CrankshaftReborn

# Use basic platform
QT_QPA_PLATFORM=minimal ./CrankshaftReborn

# Disable hardware acceleration
QT_XCB_FORCE_SOFTWARE_OPENGL=1 ./CrankshaftReborn
```

### Step 4: Verify Installation Integrity
```bash
# Check installed files
dpkg -L crankshaft-reborn | grep -E '\.(so|qml)$'

# Verify file permissions
find /usr/lib/crankshaft_reborn -type f -exec ls -l {} \;
find /usr/share/crankshaft_reborn -type f -exec ls -l {} \;
```

### Step 5: Test Qt Quick Separately
```bash
# Create minimal QML test
cat > test.qml << 'EOF'
import QtQuick 2.15
import QtQuick.Window 2.15

Window {
    visible: true
    width: 640
    height: 480
    title: "Test Window"
    
    Rectangle {
        anchors.fill: parent
        color: "red"
        
        Text {
            anchors.centerIn: parent
            text: "Qt Quick Works!"
            font.pixelSize: 32
            color: "white"
        }
    }
}
EOF

# Run test
qml6 test.qml
```

If this test shows a red window with text, Qt Quick is working and the issue is application-specific.

## Environment Variables Reference

| Variable | Purpose | Example |
|----------|---------|---------|
| `QT_LOGGING_RULES` | Control Qt logging output | `"*=true"` or `"qt.qml.*=true"` |
| `QT_DEBUG_PLUGINS` | Show plugin loading details | `1` |
| `QT_QPA_PLATFORM` | Set Qt platform plugin | `xcb`, `wayland`, `eglfs`, `vnc` |
| `QT_QUICK_BACKEND` | Set Qt Quick rendering backend | `software`, `opengl` |
| `QT_SCALE_FACTOR` | Override display scaling | `1`, `1.5`, `2` |
| `QT_XCB_FORCE_SOFTWARE_OPENGL` | Force software OpenGL on X11 | `1` |
| `QT_QPA_EGLFS_WIDTH/HEIGHT` | Set display resolution for EGLFS | `1920`, `1080` |

## Getting Help

If none of these solutions work, gather diagnostic information:

```bash
# Collect system information
uname -a > diagnostics.txt
cat /etc/os-release >> diagnostics.txt
echo "=== Qt Version ===" >> diagnostics.txt
qmake6 --version >> diagnostics.txt 2>&1 || echo "qmake not found" >> diagnostics.txt
echo "=== Installed Qt Packages ===" >> diagnostics.txt
dpkg -l | grep qt6 >> diagnostics.txt
echo "=== OpenGL Info ===" >> diagnostics.txt
glxinfo | head -20 >> diagnostics.txt 2>&1 || echo "glxinfo not available" >> diagnostics.txt
echo "=== Application Debug Log ===" >> diagnostics.txt
QT_LOGGING_RULES="*=true" QT_DEBUG_PLUGINS=1 ./CrankshaftReborn >> diagnostics.txt 2>&1
```

Then share `diagnostics.txt` when reporting the issue on GitHub or forums.

## Platform-Specific Notes

### Raspberry Pi 4/5
- Ensure Full KMS driver is enabled in `raspi-config`
- Use EGLFS platform for best performance: `QT_QPA_PLATFORM=eglfs`
- May need to increase GPU memory in `/boot/config.txt`: `gpu_mem=256`

### Virtual Machines
- 3D acceleration may not work properly
- Use software rendering: `QT_QUICK_BACKEND=software`
- Ensure guest additions/tools are installed

### SSH/Headless Systems
- Use VNC platform: `QT_QPA_PLATFORM=vnc:size=800x480:port=5900`
- Or offscreen platform for testing: `QT_QPA_PLATFORM=offscreen`
- Connect with VNC client to see the UI

### Wayland
- Some Qt6 versions have Wayland issues
- Try forcing X11/XWayland: `QT_QPA_PLATFORM=xcb`
- Or use native Wayland: `QT_QPA_PLATFORM=wayland`
