//
// Aspia Project
// Copyright (C) 2018 Dmitry Chapyshev <dmitry@aspia.ru>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
//

#ifndef ASPIA_CONSOLE__CONSOLE_SETTINGS_H
#define ASPIA_CONSOLE__CONSOLE_SETTINGS_H

#include <QSettings>

#include "base/macros_magic.h"
#include "protocol/common.pb.h"

namespace aspia {

class ConsoleSettings
{
public:
    ConsoleSettings();
    ~ConsoleSettings() = default;

    QString locale() const;
    void setLocale(const QString& locale);

    QString lastDirectory() const;
    void setLastDirectory(const QString& directory_path);

    QByteArray windowGeometry() const;
    void setWindowGeometry(const QByteArray& geometry);

    QByteArray windowState() const;
    void setWindowState(const QByteArray& state);

    QByteArray splitterState() const;
    void setSplitterState(const QByteArray& state);

    QByteArray columnsState() const;
    void setColumnsState(const QByteArray& state);

    QStringList recentOpen() const;
    void setRecentOpen(const QStringList& list);

    QStringList pinnedFiles() const;
    void setPinnedFiles(const QStringList& list);

    bool isToolBarEnabled() const;
    void setToolBarEnabled(bool enable);

    bool isStatusBarEnabled() const;
    void setStatusBarEnabled(bool enable);

    bool minimizeToTray() const;
    void setMinimizeToTray(bool enable);

    bool alwaysShowTrayIcon() const;
    void setAlwaysShowTrayIcon(bool enable);

    proto::SessionType sessionType() const;
    void setSessionType(proto::SessionType session_type);

    bool checkUpdates() const;
    void setCheckUpdates(bool check);

    QString updateServer() const;
    void setUpdateServer(const QString& server);

private:
    QSettings settings_;

    DISALLOW_COPY_AND_ASSIGN(ConsoleSettings);
};

} // namespace aspia

#endif // ASPIA_CONSOLE__CONSOLE_SETTINGS_H
