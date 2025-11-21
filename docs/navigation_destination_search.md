# Navigation Extension - Destination Search Implementation

## Overview
Added comprehensive destination search functionality to the navigation extension with three input methods:
1. **Geocoding Search** (Primary) - Search by address or place name using OpenStreetMap Nominatim API
2. **Favourites** - Save and quickly access frequently used locations
3. **Advanced Coordinates** - Direct GPS coordinate input for precise navigation

## Architecture

### Component Structure
The implementation follows a modular architecture with separated, maintainable QML components:

```
extensions/navigation/qml/
├── NavigationView.qml          # Main navigation UI (updated)
├── DestinationSearch.qml       # Main search coordinator with tabs
├── GeocodingSearch.qml         # Address/place search interface
├── FavouritesList.qml          # Saved locations manager
├── CoordinateInput.qml         # Direct GPS coordinate input
└── qmldir                      # QML module definition
```

### Backend Integration

**NavigationBridge** (`src/ui/NavigationBridge.hpp/cpp`)
- Added `QNetworkAccessManager` for HTTP requests
- Implemented geocoding API integration with Nominatim
- Added favourites persistence (JSON file storage)
- Signals for search results and errors

**Key Methods:**
- `searchLocation(query)` - Performs geocoding search
- `loadFavourites()` - Loads saved locations from config
- `saveFavourites(list)` - Persists favourites to JSON

## Features

### 1. Geocoding Search (Primary Interface)
- **Search Bar**: Natural language address/place search
- **Live Results**: Displays up to 10 results from Nominatim API
- **Result Details**: Shows full address, coordinates, place type
- **User Agent**: Identifies as "Crankshaft/1.0" per OSM guidelines

**API Details:**
- Endpoint: `https://nominatim.openstreetmap.org/search`
- Format: JSON with address details
- Limit: 10 results per search
- Network: Asynchronous via Qt Network module

### 2. Favourites System
- **Save Locations**: Star button during navigation
- **Custom Names**: User-defined labels for saved locations
- **Persistent Storage**: JSON file in config/navigation_favourites.json
- **Quick Access**: One-tap navigation to saved locations
- **Delete Capability**: Remove unwanted favourites with confirmation

**Storage Format:**
```json
[
  {
    "name": "Home",
    "latitude": 51.5074,
    "longitude": -0.1278,
    "address": "Full address string",
    "timestamp": 1700000000000
  }
]
```

### 3. Advanced Coordinate Input
- **Direct Entry**: Latitude/longitude input fields
- **Validation**: Real-time coordinate validation (-90/+90, -180/+180)
- **Optional Labels**: Name coordinates for clarity
- **Examples**: Built-in examples (London, New York, Tokyo)
- **Format Guide**: Decimal degrees with up to 8 decimal places

## UI/UX Design

### Search Overlay
- **Modal Interface**: Full-screen overlay over map
- **Tab Navigation**: Switch between Search/Favourites/Advanced
- **Close Button**: Easy dismissal with X button
- **Theme Integration**: Uses ThemeManager colours throughout

### Control Panel Updates
- **Set Destination Button**: Primary action to open search
- **Change Destination**: Update destination during navigation
- **Save Favourite**: Star button appears during navigation
- **Context Aware**: Buttons adapt based on navigation state

### Empty States
- **No Results**: Helpful message when search returns nothing
- **No Favourites**: Guide users to save their first location
- **Loading States**: BusyIndicator during network requests

## Integration Points

### Theme System
All components use ThemeManager properties:
- `surfaceColor`, `surfaceVariant`, `cardColor`
- `accentColor`, `errorColor`
- `textColor`, `textSecondary`
- `outlineColor`, `dividerColor`
- `padding`, `spacing`

### Event Flow
1. User enters search query → `GeocodingSearch.performSearch()`
2. NavigationBridge.searchLocation() → HTTP request to Nominatim
3. Results received → `searchResultsReady` signal emitted
4. UI updated → Results displayed in list
5. User selects location → `destinationSelected` signal
6. Navigation starts → Updates map and info overlay

### Data Persistence
- **Settings**: `config/navigation_settings.json` (GPS device)
- **Favourites**: `config/navigation_favourites.json` (saved locations)
- **Auto-load**: Favourites loaded on component initialization
- **Auto-save**: Immediate persistence on add/delete

## Dependencies

### Qt Modules Required
- `Qt6::Network` - HTTP requests for geocoding
- `Qt6::Core` - JSON parsing and file I/O
- `Qt6::Qml` - QML integration
- `Qt6::Gui` - UI components

### External Services
- **Nominatim/OpenStreetMap**: Free geocoding API
- **No API Key Required**: Public service with usage limits
- **Attribution**: Should credit OSM in production

## Configuration

### Network Settings
The geocoding service can be configured by modifying:
```cpp
// In NavigationBridge::searchLocation()
QUrl url("https://nominatim.openstreetmap.org/search");
```

For self-hosted Nominatim or alternative geocoding services, update the URL.

### Result Limits
Default: 10 results per search
Modify in `searchLocation()`:
```cpp
urlQuery.addQueryItem("limit", "10");  // Change as needed
```

## Usage

### Basic Search
1. Click "Set Destination" in control panel
2. Search tab is active by default
3. Enter address or place name
4. Press Enter or click Search button
5. Select result from list
6. Navigation starts automatically

### Save Favourite
1. Navigate to a destination
2. Click star button (⭐) in control panel
3. Enter custom name
4. Click Save
5. Access from Favourites tab anytime

### Advanced Input
1. Click "Advanced" tab in search overlay
2. Enter latitude (-90 to +90)
3. Enter longitude (-180 to +180)
4. Optional: Add label
5. Click "Set Destination"

## Error Handling

### Network Errors
- Connection failures show error message
- Invalid responses handled gracefully
- User can retry search

### Validation
- Empty searches prevented
- Invalid coordinates rejected
- Favourites with missing data filtered

### User Feedback
- Loading indicators during searches
- Error messages via signals
- Success feedback on save

## Future Enhancements

### Potential Improvements
1. **Search History**: Recent searches list
2. **Offline Caching**: Store recent geocoding results
3. **Route Preview**: Show route before starting navigation
4. **Multi-stop Routes**: Add waypoints to journey
5. **Voice Search**: Speech-to-text for hands-free operation
6. **POI Categories**: Quick access to nearby restaurants, fuel, etc.
7. **Import/Export**: Share favourites between devices
8. **Cloud Sync**: Backup favourites to cloud service

### Alternative Geocoding Services
- Google Maps Geocoding API (requires API key)
- Mapbox Geocoding (requires API key)
- HERE Geocoding (requires API key)
- Self-hosted Nominatim instance

## Testing

### Manual Test Cases
1. ✅ Search for known address
2. ✅ Search for place name (e.g., "Eiffel Tower")
3. ✅ Invalid search (gibberish) - should show no results
4. ✅ Add favourite with custom name
5. ✅ Delete favourite with confirmation
6. ✅ Enter valid coordinates in advanced mode
7. ✅ Enter invalid coordinates (should be disabled)
8. ✅ Switch between tabs
9. ✅ Close overlay without selecting
10. ✅ Select location and start navigation

### Network Testing
- Test with no internet connection
- Test with slow connection
- Test with API rate limiting
- Test with invalid API responses

## Performance Considerations

### Optimizations
- Asynchronous network requests (non-blocking UI)
- Lazy loading of favourites
- Minimal redraws with property bindings
- Efficient JSON parsing

### Resource Usage
- Network: ~5-20KB per search (depends on results)
- Storage: <1KB per favourite
- Memory: Minimal QML component overhead

## Maintenance

### Code Organisation
- Each component in separate file for clarity
- Clear signal/slot connections
- Documented functions and properties
- Consistent naming conventions

### File Locations
- QML Components: `extensions/navigation/qml/`
- Backend Logic: `src/ui/NavigationBridge.*`
- Config Files: `config/navigation_*.json`

### Adding New Features
1. Add QML component to `qml/` directory
2. Register in `qmldir` if needed
3. Expose backend functions in NavigationBridge
4. Connect signals between components
5. Test thoroughly before commit

## License
All new files include Crankshaft GPL-3.0 header as per project standards.

## British English
All documentation and user-facing strings use British English spelling per project guidelines.
