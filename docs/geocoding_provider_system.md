# Geocoding Provider System

## Overview
The navigation extension uses a pluggable provider system for geocoding (address search) and reverse geocoding (coordinates to address). This allows easy addition of new mapping services without modifying core navigation code.

## Architecture

### Provider Interface
All geocoding providers implement the `GeocodingProvider` abstract interface defined in `GeocodingProvider.hpp`.

**Key Methods:**
- `search(query)` - Forward geocoding (address → coordinates)
- `reverseGeocode(lat, lng)` - Reverse geocoding (coordinates → address)
- `id()` - Unique provider identifier
- `displayName()` - Name shown in UI
- `requiresApiKey()` - Whether API key is needed
- `isAvailable()` - Whether provider is configured and ready

**Signals:**
- `searchResultsReady(results)` - Emitted with search results
- `reverseGeocodeComplete(address, details)` - Emitted with reverse geocode result
- `errorOccurred(error)` - Emitted on errors

### Provider Factory
`GeocodingProviderFactory` manages provider registration and instantiation.

**Key Methods:**
- `registerProvider(id, creator, ...)` - Register a new provider
- `createProvider(id)` - Create provider instance
- `availableProviders()` - List all registered providers
- `getProviderInfo(id)` - Get provider metadata

### Integration
`NavigationBridge` uses the provider system and exposes it to QML.

**Properties:**
- `geocodingProvider` (QString) - Currently selected provider ID
- `availableProviders` (QVariantList) - List of available providers

**Methods:**
- `setGeocodingProvider(id)` - Switch to different provider
- `searchLocation(query)` - Delegates to current provider

## Built-in Providers

### Nominatim (OpenStreetMap)
**ID:** `nominatim`
**API Key:** Not required
**URL:** `https://nominatim.openstreetmap.org`

Free geocoding service by OpenStreetMap. No API key required but has usage policy.

**Features:**
- Forward geocoding (address/place search)
- Reverse geocoding
- Detailed address components
- Configurable result limit
- Self-hostable (can use custom Nominatim server)

**Configuration:**
```cpp
auto* provider = new NominatimProvider();
provider->setServerUrl("https://nominatim.example.com"); // Optional: custom server
provider->setResultLimit(20); // Optional: default is 10
```

**Usage Policy:**
- Max 1 request per second
- Include User-Agent header (done automatically)
- Do not send large batch requests
- Consider self-hosting for heavy usage

## Adding New Providers

### Step 1: Implement Provider Class

Create `YourProvider.hpp`:
```cpp
#include "GeocodingProvider.hpp"

class YourProvider : public GeocodingProvider {
    Q_OBJECT
public:
    explicit YourProvider(QObject* parent = nullptr);
    
    QString id() const override { return "your_provider"; }
    QString displayName() const override { return "Your Provider Name"; }
    QString description() const override { return "Description"; }
    bool requiresApiKey() const override { return true; } // or false
    bool isAvailable() const override { return !apiKey_.isEmpty(); }
    
    void setApiKey(const QString& key) override { apiKey_ = key; }
    void search(const QString& query) override;
    void reverseGeocode(double lat, double lng) override;
    
private:
    QNetworkAccessManager* networkManager_;
    QString apiKey_;
};
```

### Step 2: Implement Search Method

Create `YourProvider.cpp`:
```cpp
void YourProvider::search(const QString& query) {
    // 1. Build API URL
    QUrl url("https://api.yourservice.com/geocode");
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("q", query);
    urlQuery.addQueryItem("key", apiKey_);
    url.setQuery(urlQuery);
    
    // 2. Create request
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "Crankshaft/1.0");
    
    // 3. Send request
    QNetworkReply* reply = networkManager_->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleSearchReply(reply);
    });
}

void YourProvider::handleSearchReply(QNetworkReply* reply) {
    reply->deleteLater();
    
    // 1. Check for errors
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(reply->errorString());
        return;
    }
    
    // 2. Parse response
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonArray results = doc.array(); // Or doc.object()["results"].toArray()
    
    // 3. Convert to standard format
    QVariantList resultsList;
    for (const QJsonValue& value : results) {
        QJsonObject obj = value.toObject();
        QVariantMap result;
        
        // REQUIRED fields
        result["latitude"] = obj["lat"].toDouble();
        result["longitude"] = obj["lng"].toDouble();
        result["display_name"] = obj["formatted_address"].toString();
        
        // OPTIONAL fields
        result["name"] = obj["name"].toString();
        result["type"] = obj["type"].toString();
        
        resultsList.append(result);
    }
    
    // 4. Emit results
    emit searchResultsReady(resultsList);
}
```

### Step 3: Register Provider

Edit `GeocodingProviderFactory.cpp`:
```cpp
#include "YourProvider.hpp"

void GeocodingProviderFactory::registerBuiltInProviders() {
    auto& factory = instance();
    
    // Existing providers...
    
    // Register your provider
    factory.registerProvider(
        "your_provider",
        [](QObject* parent) -> GeocodingProvider* {
            return new YourProvider(parent);
        },
        "Your Provider Name",
        "Description of your provider",
        true // requiresApiKey
    );
}
```

### Step 4: Update Build System

Edit `extensions/navigation/CMakeLists.txt`:
```cmake
set(PROVIDER_SOURCES
    GeocodingProvider.hpp
    GeocodingProviderFactory.hpp
    GeocodingProviderFactory.cpp
    NominatimProvider.hpp
    NominatimProvider.cpp
    YourProvider.hpp      # Add these
    YourProvider.cpp      # Add these
)
```

### Step 5: Test

Build and run:
```bash
cmake --build build
./build/CrankshaftReborn
```

The new provider will appear in Settings → Services → Map Provider dropdown.

## Result Format Standard

All providers must return results in this format:

```cpp
QVariantMap result;

// REQUIRED fields
result["latitude"] = 51.5074;           // double
result["longitude"] = -0.1278;          // double
result["display_name"] = "Full address"; // QString

// OPTIONAL fields (but recommended)
result["name"] = "Place name";          // QString
result["type"] = "city";                // QString (city, road, building, etc.)
result["city"] = "London";              // QString
result["country"] = "United Kingdom";   // QString
result["postcode"] = "SW1A 1AA";        // QString
result["road"] = "Downing Street";      // QString
result["house_number"] = "10";          // QString

// Provider-specific fields (optional)
result["importance"] = 0.95;            // double (relevance score)
result["class"] = "place";              // QString (OSM class)
```

## Provider Examples

### Google Maps Geocoding API

```cpp
class GoogleMapsProvider : public GeocodingProvider {
    QString id() const override { return "google"; }
    QString displayName() const override { return "Google Maps"; }
    bool requiresApiKey() const override { return true; }
    
    void search(const QString& query) override {
        QUrl url("https://maps.googleapis.com/maps/api/geocode/json");
        QUrlQuery urlQuery;
        urlQuery.addQueryItem("address", query);
        urlQuery.addQueryItem("key", apiKey_);
        url.setQuery(urlQuery);
        // ... make request
    }
};
```

### Mapbox Geocoding API

```cpp
class MapboxProvider : public GeocodingProvider {
    QString id() const override { return "mapbox"; }
    QString displayName() const override { return "Mapbox"; }
    bool requiresApiKey() const override { return true; }
    
    void search(const QString& query) override {
        QString encoded = QUrl::toPercentEncoding(query);
        QUrl url(QString("https://api.mapbox.com/geocoding/v5/mapbox.places/%1.json").arg(encoded));
        QUrlQuery urlQuery;
        urlQuery.addQueryItem("access_token", apiKey_);
        url.setQuery(urlQuery);
        // ... make request
    }
};
```

### HERE Geocoding API

```cpp
class HereProvider : public GeocodingProvider {
    QString id() const override { return "here"; }
    QString displayName() const override { return "HERE Maps"; }
    bool requiresApiKey() const override { return true; }
    
    void search(const QString& query) override {
        QUrl url("https://geocode.search.hereapi.com/v1/geocode");
        QUrlQuery urlQuery;
        urlQuery.addQueryItem("q", query);
        urlQuery.addQueryItem("apiKey", apiKey_);
        url.setQuery(urlQuery);
        // ... make request
    }
};
```

### Photon (Self-hosted OSM)

```cpp
class PhotonProvider : public GeocodingProvider {
    QString id() const override { return "photon"; }
    QString displayName() const override { return "Photon (OSM)"; }
    bool requiresApiKey() const override { return false; }
    
    void search(const QString& query) override {
        QUrl url(serverUrl_ + "/api");
        QUrlQuery urlQuery;
        urlQuery.addQueryItem("q", query);
        url.setQuery(urlQuery);
        // ... make request
    }
    
private:
    QString serverUrl_ = "https://photon.komoot.io";
};
```

## Configuration Storage

Provider settings are saved in `config/navigation_settings.json`:

```json
{
  "gpsDevice": "Internal",
  "geocodingProvider": "nominatim"
}
```

Future enhancement: Add API keys storage:
```json
{
  "gpsDevice": "Internal",
  "geocodingProvider": "google",
  "providerApiKeys": {
    "google": "AIza...",
    "mapbox": "pk.ey...",
    "here": "abc123..."
  }
}
```

## UI Integration

The provider selection is integrated into NavigationView settings panel under "Services" section.

**Features:**
- Dropdown list of available providers
- Shows provider display name
- Shows provider description
- Automatically saves selection
- Restarts search with new provider immediately

**QML Code:**
```qml
ComboBox {
    id: providerCombo
    model: ListModel { id: providersModel }
    textRole: "displayName"
    
    Component.onCompleted: {
        var providers = NavigationBridge.availableProviders
        providersModel.clear()
        for (var i = 0; i < providers.length; i++) {
            providersModel.append(providers[i])
        }
    }
    
    onActivated: function(index) {
        var providerId = providersModel.get(index).id
        NavigationBridge.setGeocodingProvider(providerId)
    }
}
```

## Error Handling

Providers should emit `errorOccurred()` signal for:

1. **Network Errors:**
   - Connection failed
   - Timeout
   - DNS resolution failed

2. **API Errors:**
   - Invalid API key
   - Rate limit exceeded
   - Quota exceeded
   - Service unavailable

3. **Parse Errors:**
   - Invalid response format
   - Missing required fields
   - Unexpected data types

4. **Validation Errors:**
   - Empty query
   - Invalid coordinates
   - Query too long

## Best Practices

### 1. Async Operations
Always use asynchronous network requests to avoid blocking UI.

### 2. User-Agent Header
Set appropriate User-Agent to identify your application:
```cpp
request.setHeader(QNetworkRequest::UserAgentHeader, "Crankshaft/1.0");
```

### 3. Rate Limiting
Respect API rate limits. Consider implementing:
- Request throttling
- Result caching
- Debounced search

### 4. Error Messages
Provide clear, actionable error messages:
```cpp
emit errorOccurred("API key invalid. Check settings.");
emit errorOccurred("Rate limit exceeded. Try again in 60 seconds.");
```

### 5. Logging
Use Qt logging for debugging:
```cpp
qDebug() << "Provider: Searching for" << query;
qInfo() << "Provider: Returned" << results.size() << "results";
qWarning() << "Provider: API error" << errorCode;
```

### 6. Resource Cleanup
Always delete network replies:
```cpp
void handleReply(QNetworkReply* reply) {
    reply->deleteLater(); // IMPORTANT!
    // ... process reply
}
```

## Testing

### Unit Tests
Test each provider independently:
1. Valid search queries
2. Invalid/empty queries
3. Network errors
4. API errors
5. Result parsing

### Integration Tests
Test provider switching:
1. Switch between providers
2. Verify settings persistence
3. Verify results format consistency

### Manual Tests
1. Search for known addresses
2. Search for places
3. Check error handling
4. Verify UI updates

## Future Enhancements

### API Key Management UI
Add UI for entering/managing API keys per provider.

### Provider Health Monitoring
Track provider availability and response times.

### Fallback Providers
Automatically switch to backup provider on failure.

### Result Caching
Cache recent searches to reduce API calls.

### Batch Geocoding
Support geocoding multiple addresses at once.

### Custom Providers
Allow users to configure custom geocoding endpoints.
