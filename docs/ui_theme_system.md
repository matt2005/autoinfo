# UI Theme System

## Overview

The CrankshaftReborn UI theme system provides a flexible, extensible way to customise the application's appearance. It supports multiple themes, including light and dark modes, with smooth transitions and persistent user preferences.

## Architecture

The theme system consists of three main components:

1. **Theme Data Structure** (`Theme.hpp/cpp`): Defines the theme schema with colours, properties, and JSON serialisation
2. **Theme Manager** (`ThemeManager.hpp/cpp`): Singleton manager for loading, switching, and persisting themes
3. **QML Integration** (`Theme.qml`): QML singleton providing reactive theme properties to the UI

## Features

- Light and dark mode built-in themes based on Material Design
- Custom theme support via JSON files
- Automatic theme persistence using QSettings
- Reactive QML property bindings for instant UI updates
- Extensible colour palette and properties
- Theme toggling and selection API

## Theme Structure

### Colour Palette

Each theme includes the following colour categories:

#### Primary Colours
- `primaryColor`: Main brand colour
- `primaryColorLight`: Lighter variant for hover states
- `primaryColorDark`: Darker variant for pressed states

#### Secondary Colours
- `secondaryColor`: Secondary brand colour
- `secondaryColorLight`: Lighter variant
- `secondaryColorDark`: Darker variant

#### Accent Colours
- `accentColor`: Highlight/accent colour
- `accentColorLight`: Lighter variant
- `accentColorDark`: Darker variant

#### Background Colours
- `backgroundColor`: Main background colour
- `surfaceColor`: Surface/card background
- `surfaceColorVariant`: Alternative surface colour

#### Text Colours
- `textColor`: Primary text colour
- `textColorSecondary`: Secondary text colour
- `textColorTertiary`: Tertiary/disabled text colour

#### Status Colours
- `errorColor`: Error states
- `warningColor`: Warning states
- `successColor`: Success states
- `infoColor`: Informational states

#### Other Colours
- `dividerColor`: Dividers and borders
- `overlayColor`: Overlay/modal backgrounds

### Properties

- `cornerRadius`: Default corner radius for rounded elements (px)
- `spacing`: Standard spacing unit (px)
- `padding`: Default padding (px)

### Metadata

- `displayName`: Human-readable theme name
- `isDark`: Boolean indicating if theme is dark mode

## Creating Custom Themes

### Theme JSON Format

Create a JSON file with the following structure:

```json
{
  "displayName": "My Custom Theme",
  "isDark": false,
  "primaryColor": "#2196F3",
  "primaryColorLight": "#64B5F6",
  "primaryColorDark": "#1976D2",
  "secondaryColor": "#FFC107",
  "secondaryColorLight": "#FFD54F",
  "secondaryColorDark": "#FFA000",
  "accentColor": "#FF5722",
  "accentColorLight": "#FF8A65",
  "accentColorDark": "#E64A19",
  "backgroundColor": "#FFFFFF",
  "surfaceColor": "#F5F5F5",
  "surfaceColorVariant": "#E0E0E0",
  "textColor": "#212121",
  "textColorSecondary": "#757575",
  "textColorTertiary": "#BDBDBD",
  "errorColor": "#F44336",
  "warningColor": "#FF9800",
  "successColor": "#4CAF50",
  "infoColor": "#2196F3",
  "dividerColor": "#E0E0E0",
  "overlayColor": "#000000",
  "cornerRadius": 8,
  "spacing": 16,
  "padding": 12
}
```

### Installing Custom Themes

1. Place your theme JSON file in `/usr/share/crankshaft_reborn/themes/`
2. The filename (without `.json`) becomes the theme name
3. Restart the application or call `ThemeManager::instance()->initialize()` to reload themes

## Usage

### C++ API

#### Initialising the Theme Manager

```cpp
#include "ui/ThemeManager.hpp"

// Initialise with default theme search paths
ThemeManager::instance()->initialize();

// Or specify a custom themes directory
ThemeManager::instance()->initialize("/path/to/themes");
```

#### Registering QML Type

```cpp
// In your main.cpp, before loading QML
#include "ui/ThemeManager.hpp"

ThemeManager::registerQmlType();
```

#### Switching Themes

```cpp
// Get available themes
QStringList themes = ThemeManager::instance()->availableThemes();

// Set a specific theme
ThemeManager::instance()->setCurrentTheme("dark");

// Toggle between light and dark
ThemeManager::instance()->toggleTheme();
```

#### Accessing Theme Properties

```cpp
ThemeManager* tm = ThemeManager::instance();
QColor primary = tm->primaryColor();
int radius = tm->cornerRadius();
bool isDark = tm->isDark();
```

#### Listening for Theme Changes

```cpp
connect(ThemeManager::instance(), &ThemeManager::themeChanged, this, [this]() {
    // React to theme change
    updateUI();
});
```

### QML API

#### Importing

```qml
import CrankshaftReborn.UI 1.0
```

#### Accessing Theme Properties

```qml
Rectangle {
    color: Theme.background
    
    Text {
        text: "Hello World"
        color: Theme.text
        font.pixelSize: 16
    }
    
    Rectangle {
        color: Theme.primary
        radius: Theme.cornerRadius
        width: 100
        height: 40
        
        Text {
            text: "Button"
            color: Theme.isDark ? Theme.background : Theme.surface
            anchors.centerIn: parent
        }
    }
}
```

#### Using Theme Manager Methods

```qml
Button {
    text: "Toggle Theme"
    onClicked: Theme.toggleTheme()
}

ComboBox {
    model: Theme.availableThemes
    onCurrentTextChanged: Theme.setTheme(currentText)
}
```

#### Helper Functions

```qml
Rectangle {
    // Create semi-transparent overlay
    color: Theme.alpha(Theme.overlay, 0.5)
    
    // Lighten a colour
    border.color: Theme.lighten(Theme.primary, 1.2)
    
    // Darken a colour
    Rectangle {
        color: Theme.darken(Theme.surface, 1.3)
    }
}
```

### Using Styled Components

The theme system includes pre-styled components that automatically adapt to the current theme:

#### StyledButton

```qml
import CrankshaftReborn.UI 1.0

StyledButton {
    text: "Primary"
    buttonType: "primary"  // or "secondary", "accent", "outline"
    onClicked: console.log("Clicked!")
}
```

Button types:
- `primary`: Primary colour background
- `secondary`: Secondary colour background
- `accent`: Accent colour background
- `outline`: Transparent background with coloured border

## Default Themes

### Light Theme

- Primary: Blue (#2196F3)
- Secondary: Amber (#FFC107)
- Accent: Deep Orange (#FF5722)
- Background: White (#FFFFFF)
- Text: Dark Grey (#212121)

### Dark Theme

- Primary: Light Blue (#90CAF9)
- Secondary: Light Amber (#FFE082)
- Accent: Light Deep Orange (#FF8A65)
- Background: Very Dark Grey (#121212)
- Text: White (#FFFFFF)

Both themes follow Material Design guidelines for optimal readability and accessibility.

## Best Practices

### Use Semantic Colour Names

Always use semantic colour properties (e.g., `Theme.primary`, `Theme.error`) rather than hardcoded colours. This ensures your UI adapts correctly to different themes.

```qml
// Good
Text { color: Theme.text }

// Bad
Text { color: "#000000" }
```

### Respect isDark Property

When choosing contrasting colours, check the `Theme.isDark` property:

```qml
Rectangle {
    color: Theme.primary
    
    Text {
        // Use light text on dark themes, dark text on light themes
        color: Theme.isDark ? Theme.background : Theme.text
        text: "Adaptive Text"
    }
}
```

### Use Theme Properties for Spacing

Use `Theme.spacing`, `Theme.padding`, and `Theme.cornerRadius` for consistent spacing:

```qml
Rectangle {
    radius: Theme.cornerRadius
    padding: Theme.padding
    spacing: Theme.spacing
}
```

### Animate Colour Changes

Add smooth transitions when colours change:

```qml
Rectangle {
    color: Theme.surface
    
    Behavior on color {
        ColorAnimation { duration: 150 }
    }
}
```

### Test Both Light and Dark Modes

Always test your UI components in both light and dark themes to ensure proper contrast and readability.

## Troubleshooting

### Theme Not Loading

If a custom theme isn't loading:

1. Check the JSON syntax is valid
2. Ensure the file is in the correct directory (`/usr/share/crankshaft_reborn/themes/`)
3. Verify all required colour properties are present
4. Check application logs for error messages

### QML Type Not Found

If you get "Theme is not a type" errors:

1. Ensure `ThemeManager::registerQmlType()` is called before loading QML
2. Check the QML import: `import CrankshaftReborn.UI 1.0`
3. Verify the qmldir file exists in the QML module path

### Colours Not Updating

If colours don't update when switching themes:

1. Ensure you're using property bindings (`:`) not assignments (`=`)
2. Check you're accessing theme properties via `Theme.` prefix
3. Verify the ThemeManager singleton is properly initialised

### Theme Preference Not Persisting

If the theme resets on restart:

1. Check QSettings has write permission to the config directory
2. Verify the organisation and application name are set correctly in QSettings
3. Look for saved preferences in `~/.config/OpenCarDev/CrankshaftReborn.conf`

## File Locations

### Installed Paths

- Themes: `/usr/share/crankshaft_reborn/themes/`
- QML modules: `/usr/share/crankshaft_reborn/qml/`
- User config: `~/.config/OpenCarDev/CrankshaftReborn.conf`

### Source Paths

- Theme headers: `src/ui/Theme.hpp`, `src/ui/ThemeManager.hpp`
- Theme implementation: `src/ui/Theme.cpp`, `src/ui/ThemeManager.cpp`
- Default themes: `assets/themes/light.json`, `assets/themes/dark.json`
- QML module: `assets/qml/Theme.qml`, `assets/qml/qmldir`
- Styled components: `assets/qml/StyledButton.qml`

## Future Enhancements

Planned features for the theme system:

- [ ] Theme editor GUI
- [ ] Theme import/export functionality
- [ ] Per-extension theme overrides
- [ ] Automatic theme switching based on time of day
- [ ] Theme preview without applying
- [ ] Colour scheme validation and accessibility checking
- [ ] Theme marketplace integration
- [ ] Gradients and pattern support
- [ ] Font family and size customisation
- [ ] Animation timing customisation

## Contributing

When adding new UI components:

1. Use theme properties for all colours and spacing
2. Test in both light and dark modes
3. Follow Material Design guidelines for consistency
4. Document any new theme properties in this file
5. Update default theme JSON files with new properties

For theme property additions:

1. Add property to `Theme` struct in `Theme.hpp`
2. Add Q_PROPERTY to `ThemeManager` in `ThemeManager.hpp`
3. Update JSON parsing in `Theme.cpp::fromFile()`
4. Update JSON saving in `Theme.cpp::toFile()`
5. Update default themes in `Theme.cpp::defaultLight()` and `defaultDark()`
6. Update `assets/themes/*.json` files
7. Document the new property in this file

## License

This theme system is part of the CrankshaftReborn project and is licensed under the GNU General Public License v3.0.
