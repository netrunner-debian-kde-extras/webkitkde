/*
 * This file is part of the KDE project.
 *
 * Copyright (C) 2008 - 2009 Urs Wolfer <uwolfer @ kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef KNETWORKACCESSMANAGER_H
#define KNETWORKACCESSMANAGER_H

#include <kdemacros.h>

#include <QtNetwork/QNetworkAccessManager>

#include <KDE/KIO/MetaData>

/**
 * KDE implementation of QNetworkAccessManager. Use this class instead of QNetworkAccessManager
 * for any KDE application since it uses KIO for network operations.
 */

class KDE_EXPORT KNetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    KNetworkAccessManager(QObject *parent);
    virtual ~KNetworkAccessManager();
    void setExternalContentAllowed(bool allowed);
    bool isExternalContentAllowed() const;

protected:
    virtual QNetworkReply *createRequest(Operation op, const QNetworkRequest &req, QIODevice *outgoingData = 0);

private:
    class KNetworkAccessManagerPrivate;
    KNetworkAccessManagerPrivate* const d;
};

#endif // KNETWORKACCESSMANAGER_H
