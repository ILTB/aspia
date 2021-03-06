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

#ifndef ASPIA_CLIENT__UI__SELECT_SCREEN_ACTION_H
#define ASPIA_CLIENT__UI__SELECT_SCREEN_ACTION_H

#include <QAction>

#include "base/macros_magic.h"
#include "protocol/desktop_session_extensions.pb.h"

namespace aspia {

class SelectScreenAction : public QAction
{
    Q_OBJECT

public:
    explicit SelectScreenAction(QObject* parent)
        : QAction(parent)
    {
        setText(tr("Full Desktop"));

        setCheckable(true);
        setChecked(true);

        screen_.set_id(-1);
    }

    SelectScreenAction(const proto::desktop::Screen& screen, QObject* parent)
        : QAction(parent),
          screen_(screen)
    {
        setText(QString::fromStdString(screen_.title()));
        setCheckable(true);
    }

    const proto::desktop::Screen& screen() const { return screen_; }

private:
    proto::desktop::Screen screen_;

    DISALLOW_COPY_AND_ASSIGN(SelectScreenAction);
};

} // namespace aspia

#endif // ASPIA_CLIENT__UI__SELECT_SCREEN_ACTION_H
