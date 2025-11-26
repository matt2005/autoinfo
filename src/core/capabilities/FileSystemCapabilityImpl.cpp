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
#include "FileSystemCapabilityImpl.hpp"
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QStorageInfo>
#include "CapabilityManager.hpp"

using namespace opencardev::crankshaft::core::capabilities;
using opencardev::crankshaft::core::CapabilityManager;

FileSystemCapabilityImpl::FileSystemCapabilityImpl(const QString& extension_id,
                                                   CapabilityManager* manager,
                                                   const QString& scope_path)
    : extension_id_(extension_id), manager_(manager), is_valid_(true), scope_path_(scope_path) {
    QDir dir;
    if (!dir.mkpath(scope_path_)) {
        qWarning() << "Failed to create filesystem scope:" << scope_path_;
    }
}

QString FileSystemCapabilityImpl::extensionId() const {
    return extension_id_;
}
bool FileSystemCapabilityImpl::isValid() const {
    return is_valid_;
}
void FileSystemCapabilityImpl::invalidate() {
    is_valid_ = false;
}

QFile* FileSystemCapabilityImpl::openFile(const QString& relativePath, QIODevice::OpenMode mode) {
    if (!is_valid_)
        return nullptr;
    if (relativePath.contains("..") || relativePath.startsWith("/")) {
        qWarning() << "Rejected suspicious file path:" << relativePath;
        return nullptr;
    }
    QString absolutePath = QDir(scope_path_).filePath(relativePath);
    manager_->logCapabilityUsage(extension_id_, "filesystem", "openFile",
                                 QString("%1 (mode=%2)").arg(relativePath).arg((int)mode));
    QFile* file = new QFile(absolutePath);
    if (!file->open(mode)) {
        qWarning() << "Failed to open file:" << absolutePath;
        delete file;
        return nullptr;
    }
    return file;
}

QDir FileSystemCapabilityImpl::scopedDirectory() const {
    return QDir(scope_path_);
}

QStringList FileSystemCapabilityImpl::listFiles(const QStringList& nameFilters) const {
    if (!is_valid_)
        return {};
    QDir dir(scope_path_);
    QStringList files;
    QDirIterator it(scope_path_, nameFilters.isEmpty() ? QStringList{"*"} : nameFilters,
                    QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString absPath = it.next();
        files << dir.relativeFilePath(absPath);
    }
    return files;
}

bool FileSystemCapabilityImpl::fileExists(const QString& relativePath) const {
    if (!is_valid_ || relativePath.contains("..") || relativePath.startsWith("/"))
        return false;
    return QFile::exists(QDir(scope_path_).filePath(relativePath));
}

bool FileSystemCapabilityImpl::createDirectory(const QString& relativePath) {
    if (!is_valid_ || relativePath.contains("..") || relativePath.startsWith("/"))
        return false;
    QString absolutePath = QDir(scope_path_).filePath(relativePath);
    manager_->logCapabilityUsage(extension_id_, "filesystem", "createDirectory", relativePath);
    return QDir().mkpath(absolutePath);
}

bool FileSystemCapabilityImpl::deleteFile(const QString& relativePath) {
    if (!is_valid_ || relativePath.contains("..") || relativePath.startsWith("/"))
        return false;
    QString absolutePath = QDir(scope_path_).filePath(relativePath);
    manager_->logCapabilityUsage(extension_id_, "filesystem", "deleteFile", relativePath);
    return QFile::remove(absolutePath);
}

QString FileSystemCapabilityImpl::scopePath() const {
    return scope_path_;
}
qint64 FileSystemCapabilityImpl::availableSpace() const {
    return QStorageInfo(scope_path_).bytesAvailable();
}
