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

#include "client/ui/address_bar.h"

#include <QLineEdit>
#include <QMessageBox>
#include <QTreeView>
#include <QToolTip>

#include "client/ui/address_bar_model.h"
#include "client/ui/file_path_validator.h"

namespace aspia {

AddressBar::AddressBar(QWidget* parent)
    : QComboBox(parent)
{
    edit_ = new QLineEdit(this);
    view_ = new QTreeView(this);
    model_ = new AddressBarModel(this);

    view_->setHeaderHidden(true);
    view_->setRootIsDecorated(false);
    view_->setAllColumnsShowFocus(true);
    view_->setEditTriggers(QTreeView::NoEditTriggers);
    view_->setSelectionBehavior(QTreeView::SelectRows);
    view_->setFrameShape(QFrame::NoFrame);

    setLineEdit(edit_);
    setView(view_);
    setModel(model_);

    connect(model_, &AddressBarModel::pathIndexChanged,
            this, &AddressBar::onPathIndexChanged,
            Qt::QueuedConnection);

    connect(model_, &AddressBarModel::invalidPathEntered, [this]()
    {
        QMessageBox::warning(this,
                             tr("Warning"),
                             tr("An incorrect path to the folder was entered."),
                             QMessageBox::Ok);
    });

    FilePathValidator* validator = new FilePathValidator(this);
    edit_->setValidator(validator);

    connect(validator, &FilePathValidator::invalidPathEntered, [this]()
    {
        QString characters;

        for (const auto& character : FilePlatformUtil::invalidPathCharacters())
        {
            if (!characters.isEmpty())
                characters += QLatin1String(", ");

            characters += character;
        }

        QToolTip::showText(mapToGlobal(QPoint(0, 0)),
                           tr("The path can not contain characters %1.").arg(characters),
                           this);
    });

    connect(this, QOverload<int>::of(&AddressBar::activated), [this](int /* index */)
    {
        setCurrentPath(currentPath());
    });
}

void AddressBar::setDriveList(const proto::file_transfer::DriveList& list)
{
    model_->setDriveList(list);
    emit pathChanged(currentPath());
}

void AddressBar::setCurrentPath(const QString& path)
{
    onPathIndexChanged(model_->setCurrentPath(path));
}

QString AddressBar::currentPath() const
{
    return currentData().toString();
}

QString AddressBar::previousPath() const
{
    return model_->previousPath();
}

QString AddressBar::pathAt(const QModelIndex& index) const
{
    return model_->pathAt(index);
}

bool AddressBar::hasCurrentPath() const
{
    return currentPath() != AddressBarModel::computerPath();
}

void AddressBar::showPopup()
{
    setRootModelIndex(QModelIndex());

    for (int i = 1; i < model_->columnCount(); ++i)
        view_->hideColumn(i);

    view_->expandAll();
    view_->setItemsExpandable(false);

    QComboBox::showPopup();
}

void AddressBar::onPathIndexChanged(const QModelIndex& index)
{
    setRootModelIndex(index.parent());
    setCurrentIndex(index.row());

    emit pathChanged(currentPath());
}

} // namespace aspia
