/*
 * This file is part of the KDE project.
 *
 * Copyright (C) 2008 Laurent Montel <montel@kde.org>
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

#include "kwebkitpartfactory.h"

#include "kwebkitpart.h"

#include <KDE/KParts/GenericFactory>

WebKitFactory::WebKitFactory()
{
    kDebug() << this;
}

WebKitFactory::~WebKitFactory()
{
    kDebug() << this;
}

KParts::Part *WebKitFactory::createPartObject(QWidget *parentWidget, QObject *parent, const char *className, const QStringList &args)
{
    Q_UNUSED(className);
    Q_UNUSED(args);
    return new KWebKitPart(parentWidget, parent, QStringList());
}

extern "C" KDE_EXPORT void *init_kwebkitpart()
{
    return new WebKitFactory;
}

#include "kwebkitpartfactory.moc"
