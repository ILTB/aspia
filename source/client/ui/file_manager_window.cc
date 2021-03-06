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

#include "client/ui/file_manager_window.h"

#include "base/logging.h"
#include "client/ui/address_bar_model.h"
#include "client/ui/file_remove_dialog.h"
#include "client/ui/file_transfer_dialog.h"
#include "client/ui/file_manager_settings.h"
#include "client/ui/file_mime_data.h"
#include "client/client_file_transfer.h"
#include "common/file_worker.h"

namespace aspia {

FileManagerWindow::FileManagerWindow(const ConnectData& connect_data, QWidget* parent)
    : ClientWindow(parent)
{
    createClient<ClientFileTransfer>(connect_data);
    ui.setupUi(this);

    setWindowTitle(createWindowTitle(connect_data));

    FileManagerSettings settings;
    restoreGeometry(settings.windowGeometry());
    restoreState(settings.windowState());

    QString mime_type = FileMimeData::createMimeType();

    ui.local_panel->setPanelName(tr("Local Computer"));
    ui.local_panel->setMimeType(mime_type);

    ui.remote_panel->setPanelName(tr("Remote Computer"));
    ui.remote_panel->setMimeType(mime_type);

    connect(ui.local_panel, &FilePanel::removeItems, this, &FileManagerWindow::removeItems);
    connect(ui.remote_panel, &FilePanel::removeItems, this, &FileManagerWindow::removeItems);
    connect(ui.local_panel, &FilePanel::sendItems, this, &FileManagerWindow::sendItems);
    connect(ui.remote_panel, &FilePanel::sendItems, this, &FileManagerWindow::sendItems);
    connect(ui.local_panel, &FilePanel::receiveItems, this, &FileManagerWindow::receiveItems);
    connect(ui.remote_panel, &FilePanel::receiveItems, this, &FileManagerWindow::receiveItems);
    connect(ui.local_panel, &FilePanel::pathChanged, this, &FileManagerWindow::onPathChanged);
    connect(ui.remote_panel, &FilePanel::pathChanged, this, &FileManagerWindow::onPathChanged);

    ClientFileTransfer* client = static_cast<ClientFileTransfer*>(currentClient());

    connect(ui.local_panel, &FilePanel::newRequest, client->localWorker(), &FileWorker::executeRequest);
    connect(ui.remote_panel, &FilePanel::newRequest, client, &ClientFileTransfer::remoteRequest);

    ui.local_panel->setFocus();
}

QByteArray FileManagerWindow::saveState() const
{
    QByteArray buffer;

    {
        QDataStream stream(&buffer, QIODevice::WriteOnly);
        stream.setVersion(QDataStream::Qt_5_12);
        stream << ui.splitter->saveState();
        stream << ui.local_panel->saveState();
        stream << ui.remote_panel->saveState();
    }

    return buffer;
}

void FileManagerWindow::restoreState(const QByteArray& state)
{
    QDataStream stream(state);
    stream.setVersion(QDataStream::Qt_5_12);

    QByteArray value;

    stream >> value;
    ui.splitter->restoreState(value);

    stream >> value;
    ui.local_panel->restoreState(value);

    stream >> value;
    ui.remote_panel->restoreState(value);
}

void FileManagerWindow::refresh()
{
    ui.local_panel->refresh();
    ui.remote_panel->refresh();
}

void FileManagerWindow::closeEvent(QCloseEvent* event)
{
    FileManagerSettings settings;

    settings.setWindowGeometry(saveGeometry());
    settings.setWindowState(saveState());

    emit windowClose();
    ClientWindow::closeEvent(event);
}

void FileManagerWindow::sessionStarted()
{
    refresh();
}

void FileManagerWindow::removeItems(FilePanel* sender, const QList<FileRemover::Item>& items)
{
    FileRemoveDialog* dialog = new FileRemoveDialog(this);
    FileRemover* remover = new FileRemover(dialog);

    ClientFileTransfer* client = static_cast<ClientFileTransfer*>(currentClient());

    if (sender == ui.local_panel)
    {
        connect(remover, &FileRemover::newRequest,
                client->localWorker(), &FileWorker::executeRequest);
    }
    else
    {
        DCHECK(sender == ui.remote_panel);

        connect(remover, &FileRemover::newRequest,
                client, &ClientFileTransfer::remoteRequest);
    }

    connect(remover, &FileRemover::started, dialog, &FileRemoveDialog::open);
    connect(remover, &FileRemover::finished, dialog, &FileRemoveDialog::close);
    connect(remover, &FileRemover::finished, sender, &FilePanel::refresh);
    connect(remover, &FileRemover::progressChanged, dialog, &FileRemoveDialog::setProgress);
    connect(remover, &FileRemover::error, dialog, &FileRemoveDialog::showError);

    connect(dialog, &FileRemoveDialog::finished, dialog, &FileRemoveDialog::deleteLater);
    connect(this, &FileManagerWindow::windowClose, dialog, &FileRemoveDialog::close);

    remover->start(sender->currentPath(), items);
}

void FileManagerWindow::sendItems(FilePanel* sender, const QList<FileTransfer::Item>& items)
{
    if (sender == ui.local_panel)
    {
        transferItems(FileTransfer::Uploader,
                      ui.local_panel->currentPath(),
                      ui.remote_panel->currentPath(),
                      items);
    }
    else
    {
        DCHECK(sender == ui.remote_panel);

        transferItems(FileTransfer::Downloader,
                      ui.remote_panel->currentPath(),
                      ui.local_panel->currentPath(),
                      items);
    }
}

void FileManagerWindow::receiveItems(FilePanel* sender,
                                     const QString& target_folder,
                                     const QList<FileTransfer::Item>& items)
{
    if (sender == ui.local_panel)
    {
        transferItems(FileTransfer::Downloader,
                      ui.remote_panel->currentPath(),
                      target_folder,
                      items);
    }
    else
    {
        DCHECK(sender == ui.remote_panel);

        transferItems(FileTransfer::Uploader,
                      ui.local_panel->currentPath(),
                      target_folder,
                      items);
    }
}

void FileManagerWindow::transferItems(FileTransfer::Type type,
                                      const QString& source_path,
                                      const QString& target_path,
                                      const QList<FileTransfer::Item>& items)
{
    FileTransferDialog* dialog = new FileTransferDialog(this);
    FileTransfer* transfer = new FileTransfer(type, dialog);

    if (type == FileTransfer::Uploader)
    {
        connect(transfer, &FileTransfer::finished, ui.remote_panel, &FilePanel::refresh);
    }
    else
    {
        DCHECK(type == FileTransfer::Downloader);
        connect(transfer, &FileTransfer::finished, ui.local_panel, &FilePanel::refresh);
    }

    connect(transfer, &FileTransfer::started, dialog, &FileTransferDialog::open);
    connect(transfer, &FileTransfer::finished, dialog, &FileTransferDialog::onTransferFinished);

    connect(transfer, &FileTransfer::currentItemChanged, dialog, &FileTransferDialog::setCurrentItem);
    connect(transfer, &FileTransfer::progressChanged, dialog, &FileTransferDialog::setProgress);

    connect(transfer, &FileTransfer::error, dialog, &FileTransferDialog::showError);

    connect(dialog, &FileTransferDialog::transferCanceled, transfer, &FileTransfer::stop);
    connect(dialog, &FileTransferDialog::finished, dialog, &FileTransferDialog::deleteLater);

    ClientFileTransfer* client = static_cast<ClientFileTransfer*>(currentClient());

    connect(transfer, &FileTransfer::localRequest, client->localWorker(), &FileWorker::executeRequest);
    connect(transfer, &FileTransfer::remoteRequest, client, &ClientFileTransfer::remoteRequest);

    connect(this, &FileManagerWindow::windowClose,
            dialog, &FileTransferDialog::onTransferFinished);

    transfer->start(source_path, target_path, items);
}

void FileManagerWindow::onPathChanged(FilePanel* sender, const QString& path)
{
    bool allow = path != AddressBarModel::computerPath();

    if (sender == ui.local_panel)
    {
        ui.remote_panel->setTransferAllowed(allow);
    }
    else
    {
        DCHECK(sender == ui.remote_panel);
        ui.local_panel->setTransferAllowed(allow);

    }
}

// static
QString FileManagerWindow::createWindowTitle(const ConnectData& connect_data)
{
    QString computer_name;
    if (!connect_data.computer_name.isEmpty())
        computer_name = connect_data.computer_name;
    else
        computer_name = connect_data.address;

    return tr("%1 - Aspia File Transfer").arg(computer_name);
}

} // namespace aspia
