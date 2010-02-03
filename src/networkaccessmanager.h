/*
 * This file is part of the KDE project.
 *
 * Copyright (C) 2009 Dawit Alemayehu <adawit @ kde.org>
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
#ifndef NETWORKACCESSMANAGER_H
#define NETWORKACCESSMANAGER_H

#include <kdeversion.h>

#if KDE_IS_VERSION(4,3,73)
#include <kio/accessmanager.h>
typedef KIO::AccessManager AccessManagerBase;
#else
#include "networkaccessmanager_p.h"
typedef KDEPrivate::NetworkAccessManager AccessManagerBase;
#endif

namespace KDEPrivate {

 /**
  * Re-implemented for internal reasons. API remains unaffected.
  */
class MyNetworkAccessManager : public AccessManagerBase
{
public:
    MyNetworkAccessManager(QObject *parent = 0);

protected:
    /**
     * Reimplemented for internal reasons, the API is not affected.
     *
     * @see KIO::AccessManager::createRequest.
     * @internal
     */
    virtual QNetworkReply *createRequest(Operation op, const QNetworkRequest &req, QIODevice *outgoingData = 0);
};

}

#endif // NETWORKACCESSMANAGER_P_H
