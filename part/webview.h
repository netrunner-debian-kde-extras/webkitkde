/*
 * This file is part of the KDE project.
 *
 * Copyright (C) 2007 Trolltech ASA
 * Copyright (C) 2008 Urs Wolfer <uwolfer @ kde.org>
 * Copyright (C) 2008 Laurent Montel <montel@kde.org>
 * Copyright (C) 2009 Dawit Alemayehu <adawit@kde.org>
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
#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <kdewebkit/kwebview.h>

#include <KDE/KParts/ReadOnlyPart>
#include <KDE/KParts/BrowserExtension>

#include "webkitkde_export.h"

class WebKitPart;
class QWebHitTestResult;

class WEBKITKDE_EXPORT WebView : public KWebView
{
    Q_OBJECT
public:
    WebView(WebKitPart *wpart, QWidget *parent);
    ~WebView();
    QWebHitTestResult contextMenuResult() const;

protected:
    void setNewPage();
    void contextMenuEvent(QContextMenuEvent *e);
    void selectActionPopupMenu(KParts::BrowserExtension::ActionGroupMap &selectGroupMap);
    void linkActionPopupMenu(KParts::BrowserExtension::ActionGroupMap &linkGroupMap);
    void partActionPopupMenu(KParts::BrowserExtension::ActionGroupMap &partGroupMap);

private Q_SLOTS:
    void openSelection();

private:
    class WebViewPrivate;
    WebViewPrivate* const d;
};

#endif // WEBVIEW_H
