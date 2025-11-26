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

#pragma once

#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QStringList>
#include "Capability.hpp"

namespace opencardev::crankshaft {
namespace core {
namespace capabilities {

/**
 * FileSystem capability for scoped file access.
 *
 * Extensions with this capability can:
 * - Read/write files within their scope directory
 * - List files in their scope
 * - Create/delete files/directories in their scope
 *
 * Extensions CANNOT access files outside their scope.
 * Typical scopes: $CACHE/extensions/{extension-id}/
 *                 $DATA/extensions/{extension-id}/
 */
class FileSystemCapability : public Capability {
  public:
    ~FileSystemCapability() override = default;

    QString id() const override { return "filesystem"; }

    /**
     * Open a file within the capability's scope.
     * Path is relative to scope root.
     *
     * @param relativePath Path relative to scope (e.g., "maps/tile_0_0.png")
     * @param mode Open mode (ReadOnly, WriteOnly, ReadWrite, etc.)
     * @return QFile pointer (caller owns) or nullptr if access denied
     */
    virtual QFile* openFile(const QString& relativePath, QIODevice::OpenMode mode) = 0;

    /**
     * Get the scoped directory (read-only access to QDir).
     * Extensions can use this to check file existence, but cannot
     * access files outside scope.
     */
    virtual QDir scopedDirectory() const = 0;

    /**
     * List all files in the scope (recursive).
     *
     * @param nameFilters Optional filters (e.g., {"*.png", "*.jpg"})
     * @return List of relative file paths within scope
     */
    virtual QStringList listFiles(const QStringList& nameFilters = QStringList()) const = 0;

    /**
     * Check if a file exists within scope.
     *
     * @param relativePath Path relative to scope
     */
    virtual bool fileExists(const QString& relativePath) const = 0;

    /**
     * Create a directory within scope.
     *
     * @param relativePath Path relative to scope
     * @return true if created or already exists
     */
    virtual bool createDirectory(const QString& relativePath) = 0;

    /**
     * Delete a file within scope.
     *
     * @param relativePath Path relative to scope
     * @return true if deleted successfully
     */
    virtual bool deleteFile(const QString& relativePath) = 0;

    /**
     * Get absolute path to scope root (for display only).
     * Extensions should not use this for file operations.
     */
    virtual QString scopePath() const = 0;

    /**
     * Get available disk space in scope (bytes).
     */
    virtual qint64 availableSpace() const = 0;

  protected:
    FileSystemCapability() = default;
};

}  // namespace capabilities
}  // namespace core
}  // namespace opencardev::crankshaft
