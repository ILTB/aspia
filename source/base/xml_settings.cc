﻿//
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

#include "base/xml_settings.h"

#include <QtXml>

namespace aspia {

namespace {

QString variantToType(const QVariant& value)
{
    QString result;

    switch (value.type())
    {
        case QMetaType::UnknownType:
            result = QLatin1String("Unknown");
            break;

        case QMetaType::Bool:
        case QMetaType::Int:
        case QMetaType::UInt:
        case QMetaType::LongLong:
        case QMetaType::ULongLong:
        case QMetaType::Double:
        case QMetaType::QString:
        case QMetaType::QKeySequence:
            // Default type.
            break;

        case QMetaType::QByteArray:
            result = QLatin1String("ByteArray");
            break;

        case QMetaType::QRect:
            result = QLatin1String("Rect");
            break;

        case QMetaType::QSize:
            result = QLatin1String("Size");
            break;

        case QMetaType::QPoint:
            result = QLatin1String("Point");
            break;

        default:
            result = QLatin1String("Variant");
            break;
    }

    return result;
}

QString variantToString(const QVariant& value)
{
    QString result;

    switch (value.type())
    {
        case QMetaType::UnknownType:
            break;

        case QMetaType::Bool:
        case QMetaType::Int:
        case QMetaType::UInt:
        case QMetaType::LongLong:
        case QMetaType::ULongLong:
        case QMetaType::Double:
        case QMetaType::QString:
        case QMetaType::QKeySequence:
            result = value.toString();
            break;

        case QMetaType::QByteArray:
            result = value.toByteArray().toBase64();
            break;

        case QMetaType::QRect:
        {
            QRect rect = value.toRect();
            result = QString::asprintf("%d %d %d %d",
                                       rect.x(), rect.y(),
                                       rect.width(), rect.height());
        }
        break;

        case QMetaType::QSize:
        {
            QSize size = value.toSize();
            result = QString::asprintf("%d %d", size.width(), size.height());
        }
        break;

        case QMetaType::QPoint:
        {
            QPoint point = value.toPoint();
            result = QString::asprintf("%d %d", point.x(), point.y());
        }
        break;

        default:
        {
            QByteArray buffer;

            {
                QDataStream stream(&buffer, QIODevice::WriteOnly);
                stream.setVersion(QDataStream::Qt_5_12);
                stream << value;
            }

            result = buffer.toBase64();
        }
        break;
    }

    return result;
}

QVariant stringToVariant(const QString& value, const QString& type)
{
    if (type == QLatin1String("ByteArray"))
    {
        return QVariant(QByteArray::fromBase64(value.toUtf8()));
    }
    else if (type == QLatin1String("Variant"))
    {
        QByteArray buffer = QByteArray::fromBase64(value.toUtf8());

        QDataStream stream(&buffer, QIODevice::ReadOnly);
        stream.setVersion(QDataStream::Qt_5_12);

        QVariant result;
        stream >> result;
        return result;
    }
    else if (type == QLatin1String("Rect"))
    {
        QStringList list = value.split(QLatin1Char(' '));
        if (list.size() == 4)
        {
            return QVariant(QRect(list[0].toInt(),
                                  list[1].toInt(),
                                  list[2].toInt(),
                                  list[3].toInt()));
        }
    }
    else if (type == QLatin1String("Size"))
    {
        QStringList list = value.split(QLatin1Char(' '));
        if (list.size() == 2)
            return QVariant(QSize(list[0].toInt(), list[1].toInt()));
    }
    else if (type == QLatin1String("Point"))
    {
        QStringList list = value.split(QLatin1Char(' '));
        if (list.size() == 2)
            return QVariant(QPoint(list[0].toInt(), list[1].toInt()));
    }
    else if (type == QLatin1String("Unknown"))
    {
        return QVariant();
    }

    return QVariant(value);
}

} // namespace

// static
QSettings::Format XmlSettings::registerFormat(Qt::CaseSensitivity case_sensitivity)
{
    return QSettings::registerFormat(QLatin1String("xml"), readFunc, writeFunc, case_sensitivity);
}

// static
bool XmlSettings::readFunc(QIODevice& device, QSettings::SettingsMap& map)
{
    QXmlStreamReader xml(&device);
    QStringList segments;
    QString type;

    while (!xml.atEnd() && !xml.hasError())
    {
        switch (xml.readNext())
        {
            case QXmlStreamReader::StartElement:
            {
                if (xml.name() == QLatin1String("Settings"))
                    continue;

                segments.append(xml.attributes().value(QLatin1String("Name")).toString());
                type = xml.attributes().value(QLatin1String("Type")).toString();
            }
            break;

            case QXmlStreamReader::EndElement:
            {
                if (!segments.isEmpty())
                    segments.removeLast();
            }
            break;

            case QXmlStreamReader::Characters:
            {
                if (xml.isWhitespace())
                    continue;

                QString key;

                for (int i = 0; i < segments.size(); ++i)
                {
                    if (i != 0)
                        key += QLatin1Char('/');
                    key += segments.at(i);
                }

                map[key] = stringToVariant(xml.text().toString(), type);
            }
            break;
        }
    }

    return !xml.hasError();
}

// static
bool XmlSettings::writeFunc(QIODevice& device, const QSettings::SettingsMap& map)
{
    QXmlStreamWriter xml(&device);

    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement(QLatin1String("Settings"));

    QStringList prev;

    for (auto it = map.constBegin(); it != map.constEnd(); ++it)
    {
        QStringList segments = it.key().split(QLatin1Char('/'));

        int x = 0;

        while (x < prev.size() && segments.at(x) == prev.at(x))
            ++x;

        for (int i = prev.size() - 1; i >= x; --i)
            xml.writeEndElement();

        for (int i = x; i < segments.size(); ++i)
        {
            if (i == segments.size() - 1)
                xml.writeStartElement(QLatin1String("Value"));
            else
                xml.writeStartElement(QLatin1String("Group"));

            xml.writeAttribute(QLatin1String("Name"), segments.at(i));
        }

        QString type = variantToType(it.value());
        if (!type.isEmpty())
            xml.writeAttribute(QLatin1String("Type"), type);

        xml.writeCharacters(variantToString(it.value()));

        prev.swap(segments);
    }

    for (int i = 0; i < prev.size(); ++i)
        xml.writeEndElement();

    xml.writeEndElement();
    xml.writeEndDocument();

    return !xml.hasError();
}

} // namespace aspia
