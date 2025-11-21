/*
 * Project: Crankshaft
 * This file is part of Crankshaft project.
 * Copyright (C) 2025 OpenCarDev Team
 *
 *  Crankshaft is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Crankshaft is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Crankshaft. If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

/**
 * Destination search component with geocoding, direct coordinates, and favourites
 */
Item {
    id: root
    
    // Signals
    signal destinationSelected(real latitude, real longitude, string address)
    signal searchLocation(string query)
    signal favouriteSelected(var favourite)
    
    // Properties
    property bool showAdvanced: false
    property var searchResults: []
    property var favourites: []
    property bool isSearching: false
    
    // Theme properties
    readonly property color surfaceColor: ThemeManager.surfaceColor
    readonly property color surfaceVariant: ThemeManager.cardColor
    readonly property color accentColor: ThemeManager.accentColor
    readonly property color errorColor: ThemeManager.errorColor
    readonly property color textColor: ThemeManager.textColor
    readonly property color textSecondary: ThemeManager.textSecondaryColor
    readonly property color outlineColor: ThemeManager.borderColor
    readonly property color dividerColor: ThemeManager.dividerColor
    readonly property int paddingSize: ThemeManager.padding
    readonly property int spacingSize: ThemeManager.spacing
    
    Column {
        anchors.fill: parent
        spacing: spacingSize
        
        // Header with tabs
        Rectangle {
            width: parent.width
            height: 50
            color: surfaceVariant
            radius: 8
            
            Row {
                anchors.fill: parent
                anchors.margins: 4
                spacing: 4
                
                // Search tab
                Button {
                    width: (parent.width - 12) / 3
                    height: parent.height
                    text: "🔍 Search"
                    
                    background: Rectangle {
                        color: !showAdvanced && searchTab.active ? accentColor : "transparent"
                        radius: 6
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 14
                        font.bold: !showAdvanced && searchTab.active
                        color: !showAdvanced && searchTab.active ? "white" : textColor
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    
                    onClicked: {
                        showAdvanced = false
                        searchTab.active = true
                        favouritesTab.active = false
                    }
                    
                    property bool active: true
                    Component.onCompleted: searchTab.active = true
                    id: searchTab
                }
                
                // Favourites tab
                Button {
                    width: (parent.width - 12) / 3
                    height: parent.height
                    text: "⭐ Favourites"
                    
                    background: Rectangle {
                        color: !showAdvanced && favouritesTab.active ? accentColor : "transparent"
                        radius: 6
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 14
                        font.bold: !showAdvanced && favouritesTab.active
                        color: !showAdvanced && favouritesTab.active ? "white" : textColor
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    
                    onClicked: {
                        showAdvanced = false
                        searchTab.active = false
                        favouritesTab.active = true
                    }
                    
                    property bool active: false
                    id: favouritesTab
                }
                
                // Advanced tab
                Button {
                    width: (parent.width - 12) / 3
                    height: parent.height
                    text: "⚙️ Advanced"
                    
                    background: Rectangle {
                        color: showAdvanced ? accentColor : "transparent"
                        radius: 6
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 14
                        font.bold: showAdvanced
                        color: showAdvanced ? "white" : textColor
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    
                    onClicked: {
                        showAdvanced = true
                        searchTab.active = false
                        favouritesTab.active = false
                    }
                }
            }
        }
        
        // Content area
        StackLayout {
            width: parent.width
            height: parent.height - 58
            currentIndex: showAdvanced ? 2 : (searchTab.active ? 0 : 1)
            
            // Search view
            GeocodingSearch {
                id: geocodingSearch
                onLocationSelected: function(lat, lng, address) {
                    root.destinationSelected(lat, lng, address)
                }
                onSearchRequested: function(query) {
                    root.searchLocation(query)
                }
            }
            
            // Favourites view
            FavouritesList {
                id: favouritesList
                favourites: root.favourites
                onFavouriteSelected: function(favourite) {
                    root.destinationSelected(favourite.latitude, favourite.longitude, favourite.name)
                }
                onFavouriteDeleted: function(index) {
                    var newFavourites = root.favourites.slice()
                    newFavourites.splice(index, 1)
                    root.favourites = newFavourites
                    saveFavourites()
                }
            }
            
            // Advanced coordinates input
            CoordinateInput {
                id: coordinateInput
                onCoordinatesSubmitted: function(lat, lng, address) {
                    root.destinationSelected(lat, lng, address)
                }
            }
        }
    }
    
    // Public methods
    function setSearchResults(results) {
        searchResults = results
        geocodingSearch.searchResults = results
        geocodingSearch.isSearching = false
    }
    
    function addFavourite(name, latitude, longitude, address) {
        var newFavourite = {
            "name": name,
            "latitude": latitude,
            "longitude": longitude,
            "address": address,
            "timestamp": Date.now()
        }
        
        var newFavourites = root.favourites.slice()
        newFavourites.push(newFavourite)
        root.favourites = newFavourites
        saveFavourites()
    }
    
    function loadFavourites() {
        if (NavigationBridge) {
            var loaded = NavigationBridge.loadFavourites()
            if (loaded && loaded.length > 0) {
                root.favourites = loaded
            }
        }
    }
    
    function saveFavourites() {
        if (NavigationBridge) {
            NavigationBridge.saveFavourites(root.favourites)
        }
    }
    
    Component.onCompleted: {
        loadFavourites()
    }
}
