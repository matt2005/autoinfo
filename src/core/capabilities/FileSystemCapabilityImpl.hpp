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

#include "FileSystemCapability.hpp"
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QStorageInfo>

namespace opencardev::crankshaft::core { class CapabilityManager; }

namespace opencardev::crankshaft::core::capabilities {

class FileSystemCapabilityImpl : public FileSystemCapability {
  public:
    FileSystemCapabilityImpl(const QString& extension_id, core::CapabilityManager* manager, const QString& scope_path);
    QString extensionId() const override;
    bool isValid() const override;
    void invalidate();
    QFile* openFile(const QString& relativePath, QIODevice::OpenMode mode) override;
    QDir scopedDirectory() const override;
    QStringList listFiles(const QStringList& nameFilters) const override;
    bool fileExists(const QString& relativePath) const override;
    bool createDirectory(const QString& relativePath) override;
    bool deleteFile(const QString& relativePath) override;
    QString scopePath() const override;
    qint64 availableSpace() const override;
  private:
    QString extension_id_;
    core::CapabilityManager* manager_;
    bool is_valid_;
    QString scope_path_;
};

inline std::shared_ptr<FileSystemCapability> createFileSystemCapabilityInstance(const QString& extensionId, core::CapabilityManager* mgr, const QString& scopePath) {
    return std::static_pointer_cast<FileSystemCapability>(std::make_shared<FileSystemCapabilityImpl>(extensionId, mgr, scopePath));
}

} // namespace opencardev::crankshaft::core::capabilities
