//
// Aspia Project
// Copyright (C) 2019 Dmitry Chapyshev <dmitry@aspia.ru>
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

#include "client/client_file_transfer.h"

#include <QMetaType>
#include <QThread>

#include "client/ui/file_manager_window.h"
#include "common/file_request.h"
#include "common/file_worker.h"

namespace aspia {

ClientFileTransfer::ClientFileTransfer(const ConnectData& connect_data, QObject* parent)
    : Client(connect_data, parent)
{
    qRegisterMetaType<proto::file_transfer::Request>();
    qRegisterMetaType<proto::file_transfer::Reply>();

    worker_thread_ = new QThread(this);
    worker_.reset(new FileWorker());
    worker_->moveToThread(worker_thread_);
    worker_thread_->start();
}

ClientFileTransfer::~ClientFileTransfer()
{
    worker_thread_->quit();
    worker_thread_->wait();

    for (auto request : requests_)
        delete request;
}

FileWorker* ClientFileTransfer::localWorker() { return worker_.get(); }

void ClientFileTransfer::messageReceived(const QByteArray& buffer)
{
    proto::file_transfer::Reply reply;

    if (!reply.ParseFromArray(buffer.constData(), buffer.size()))
    {
        emit errorOccurred(tr("Session error: Invalid message from host."));
        return;
    }

    if (reply.status() == proto::file_transfer::STATUS_NO_LOGGED_ON_USER)
    {
        emit errorOccurred(
            tr("Session error: There are no logged in users. File transfer is not available."));
        return;
    }

    QScopedPointer<FileRequest> request(requests_.front());
    requests_.pop_front();

    request->sendReply(reply);
}

void ClientFileTransfer::remoteRequest(FileRequest* request)
{
    requests_.push_back(QPointer<FileRequest>(request));
    sendMessage(request->request());
}

} // namespace aspia
