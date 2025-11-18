/*
 * Project: OpenAuto
 * This file is part of openauto project.
 * Copyright (C) 2025 OpenCarDev Team
 *
 *  openauto is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  openauto is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with openauto. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Theme.hpp"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

namespace CrankshaftReborn {
namespace UI {

Theme Theme::fromFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open theme file:" << filePath;
        return defaultLight();
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    file.close();
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse theme file:" << error.errorString();
        return defaultLight();
    }
    
    QJsonObject json = doc.object();
    Theme theme;
    
    theme.name = json["name"].toString();
    theme.displayName = json["displayName"].toString(theme.name);
    theme.isDark = json["isDark"].toBool(false);
    
    // Helper lambda to parse color from JSON
    auto parseColor = [&json](const QString& key, const QColor& defaultColor) -> QColor {
        if (json.contains(key)) {
            return QColor(json[key].toString());
        }
        return defaultColor;
    };
    
    // Load colors
    theme.primaryColor = parseColor("primaryColor", "#2196F3");
    theme.primaryLightColor = parseColor("primaryLightColor", "#64B5F6");
    theme.primaryDarkColor = parseColor("primaryDarkColor", "#1976D2");
    
    theme.secondaryColor = parseColor("secondaryColor", "#FFC107");
    theme.secondaryLightColor = parseColor("secondaryLightColor", "#FFD54F");
    theme.secondaryDarkColor = parseColor("secondaryDarkColor", "#FFA000");
    
    theme.accentColor = parseColor("accentColor", "#FF5722");
    theme.accentLightColor = parseColor("accentLightColor", "#FF8A65");
    theme.accentDarkColor = parseColor("accentDarkColor", "#E64A19");
    
    theme.backgroundColor = parseColor("backgroundColor", "#FAFAFA");
    theme.surfaceColor = parseColor("surfaceColor", "#FFFFFF");
    theme.cardColor = parseColor("cardColor", "#FFFFFF");
    
    theme.textColor = parseColor("textColor", "#212121");
    theme.textSecondaryColor = parseColor("textSecondaryColor", "#757575");
    theme.textDisabledColor = parseColor("textDisabledColor", "#BDBDBD");
    
    theme.dividerColor = parseColor("dividerColor", "#E0E0E0");
    theme.borderColor = parseColor("borderColor", "#BDBDBD");
    theme.shadowColor = parseColor("shadowColor", "#000000");
    
    theme.successColor = parseColor("successColor", "#4CAF50");
    theme.warningColor = parseColor("warningColor", "#FF9800");
    theme.errorColor = parseColor("errorColor", "#F44336");
    theme.infoColor = parseColor("infoColor", "#2196F3");
    
    // Load properties
    theme.cornerRadius = json["cornerRadius"].toInt(8);
    theme.spacing = json["spacing"].toInt(8);
    theme.padding = json["padding"].toInt(16);
    
    return theme;
}

bool Theme::toFile(const QString& filePath) const {
    QJsonObject json;
    
    json["name"] = name;
    json["displayName"] = displayName;
    json["isDark"] = isDark;
    
    // Save colors
    json["primaryColor"] = primaryColor.name();
    json["primaryLightColor"] = primaryLightColor.name();
    json["primaryDarkColor"] = primaryDarkColor.name();
    
    json["secondaryColor"] = secondaryColor.name();
    json["secondaryLightColor"] = secondaryLightColor.name();
    json["secondaryDarkColor"] = secondaryDarkColor.name();
    
    json["accentColor"] = accentColor.name();
    json["accentLightColor"] = accentLightColor.name();
    json["accentDarkColor"] = accentDarkColor.name();
    
    json["backgroundColor"] = backgroundColor.name();
    json["surfaceColor"] = surfaceColor.name();
    json["cardColor"] = cardColor.name();
    
    json["textColor"] = textColor.name();
    json["textSecondaryColor"] = textSecondaryColor.name();
    json["textDisabledColor"] = textDisabledColor.name();
    
    json["dividerColor"] = dividerColor.name();
    json["borderColor"] = borderColor.name();
    json["shadowColor"] = shadowColor.name();
    
    json["successColor"] = successColor.name();
    json["warningColor"] = warningColor.name();
    json["errorColor"] = errorColor.name();
    json["infoColor"] = infoColor.name();
    
    // Save properties
    json["cornerRadius"] = cornerRadius;
    json["spacing"] = spacing;
    json["padding"] = padding;
    
    QJsonDocument doc(json);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to save theme file:" << filePath;
        return false;
    }
    
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

Theme Theme::defaultLight() {
    Theme theme;
    theme.name = "light";
    theme.displayName = "Light";
    theme.isDark = false;
    
    // Material Design Light Theme colors
    theme.primaryColor = QColor("#2196F3");
    theme.primaryLightColor = QColor("#64B5F6");
    theme.primaryDarkColor = QColor("#1976D2");
    
    theme.secondaryColor = QColor("#FFC107");
    theme.secondaryLightColor = QColor("#FFD54F");
    theme.secondaryDarkColor = QColor("#FFA000");
    
    theme.accentColor = QColor("#FF5722");
    theme.accentLightColor = QColor("#FF8A65");
    theme.accentDarkColor = QColor("#E64A19");
    
    theme.backgroundColor = QColor("#FAFAFA");
    theme.surfaceColor = QColor("#FFFFFF");
    theme.cardColor = QColor("#FFFFFF");
    
    theme.textColor = QColor("#212121");
    theme.textSecondaryColor = QColor("#757575");
    theme.textDisabledColor = QColor("#BDBDBD");
    
    theme.dividerColor = QColor("#E0E0E0");
    theme.borderColor = QColor("#BDBDBD");
    theme.shadowColor = QColor("#000000");
    
    theme.successColor = QColor("#4CAF50");
    theme.warningColor = QColor("#FF9800");
    theme.errorColor = QColor("#F44336");
    theme.infoColor = QColor("#2196F3");
    
    theme.cornerRadius = 8;
    theme.spacing = 8;
    theme.padding = 16;
    
    return theme;
}

Theme Theme::defaultDark() {
    Theme theme;
    theme.name = "dark";
    theme.displayName = "Dark";
    theme.isDark = true;
    
    // Material Design Dark Theme colors
    theme.primaryColor = QColor("#90CAF9");
    theme.primaryLightColor = QColor("#BBDEFB");
    theme.primaryDarkColor = QColor("#42A5F5");
    
    theme.secondaryColor = QColor("#FFE082");
    theme.secondaryLightColor = QColor("#FFF9C4");
    theme.secondaryDarkColor = QColor("#FFD54F");
    
    theme.accentColor = QColor("#FF7043");
    theme.accentLightColor = QColor("#FFAB91");
    theme.accentDarkColor = QColor("#F4511E");
    
    theme.backgroundColor = QColor("#121212");
    theme.surfaceColor = QColor("#1E1E1E");
    theme.cardColor = QColor("#2C2C2C");
    
    theme.textColor = QColor("#FFFFFF");
    theme.textSecondaryColor = QColor("#B0B0B0");
    theme.textDisabledColor = QColor("#808080");
    
    theme.dividerColor = QColor("#404040");
    theme.borderColor = QColor("#606060");
    theme.shadowColor = QColor("#000000");
    
    theme.successColor = QColor("#66BB6A");
    theme.warningColor = QColor("#FFA726");
    theme.errorColor = QColor("#EF5350");
    theme.infoColor = QColor("#42A5F5");
    
    theme.cornerRadius = 8;
    theme.spacing = 8;
    theme.padding = 16;
    
    return theme;
}

} // namespace UI
} // namespace CrankshaftReborn
