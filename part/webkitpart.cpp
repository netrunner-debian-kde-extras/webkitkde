/*
 * This file is part of the KDE project.
 *
 * Copyright (C) 2007 Trolltech ASA
 * Copyright (C) 2008 - 2009 Urs Wolfer <uwolfer @ kde.org>
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

#include "webkitpart.h"

#include "webview.h"
#include "webpage.h"
#include "websslinfo.h"
#include "sslinfodialog_p.h"
#include <kdewebkit/settings/webkitsettings.h>

#include <KDE/KParts/GenericFactory>
#include <KDE/KParts/Plugin>
#include <KDE/KAboutData>
#include <KDE/KUriFilterData>
#include <KDE/KDesktopFile>
#include <KDE/KConfigGroup>
#include <KDE/KAction>
#include <KDE/KActionCollection>
#include <KDE/KRun>
#include <KDE/KTemporaryFile>
#include <KDE/KToolInvocation>
#include <KDE/KFileDialog>
#include <KDE/KMessageBox>
#include <KDE/KStandardDirs>
#include <KDE/KIconLoader>
#include <KDE/KGlobal>
#include <KDE/KStringHandler>
#include <KDE/KIO/NetAccess>
#include <kio/global.h>

#include <QHttpRequestHeader>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QFile>
#include <QClipboard>
#include <QApplication>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QPrintPreviewDialog>
#include <QPair>

#include <QtWebKit/QWebHistory>
#include <QtWebKit/QWebHitTestResult>

#define QL1(x)    QLatin1String(x)


static QString htmlError (int code, const QString& text, const KUrl& reqUrl)
{
  kDebug() << "errorCode" << code << "text" << text;

  QString errorName, techName, description;
  QStringList causes, solutions;

  QByteArray raw = KIO::rawErrorDetail( code, text, &reqUrl );
  QDataStream stream(raw);

  stream >> errorName >> techName >> description >> causes >> solutions;

  QString url, protocol, datetime;
  url = Qt::escape( reqUrl.prettyUrl() );
  protocol = reqUrl.protocol();
  datetime = KGlobal::locale()->formatDateTime( QDateTime::currentDateTime(),
                                                KLocale::LongDate );

  QString filename( KStandardDirs::locate( "data", "khtml/error.html" ) );
  QFile file( filename );
  bool isOpened = file.open( QIODevice::ReadOnly );
  if ( !isOpened )
    kWarning() << "Could not open error html template:" << filename;

  QString html = QString( QL1( file.readAll() ) );

  html.replace( QL1( "TITLE" ), i18n( "Error: %1 - %2", errorName, url ) );
  html.replace( QL1( "DIRECTION" ), QApplication::isRightToLeft() ? "rtl" : "ltr" );
  html.replace( QL1( "ICON_PATH" ), KIconLoader::global()->iconPath( "dialog-warning", -KIconLoader::SizeHuge ) );

  QString doc = QL1( "<h1>" );
  doc += i18n( "The requested operation could not be completed" );
  doc += QL1( "</h1><h2>" );
  doc += errorName;
  doc += QL1( "</h2>" );

  if ( !techName.isNull() ) {
    doc += QL1( "<h2>" );
    doc += i18n( "Technical Reason: " );
    doc += techName;
    doc += QL1( "</h2>" );
  }

  doc += QL1( "<h3>" );
  doc += i18n( "Details of the Request:" );
  doc += QL1( "</h3><ul><li>" );
  doc += i18n( "URL: %1" ,  url );
  doc += QL1( "</li><li>" );

  if ( !protocol.isNull() ) {
    doc += i18n( "Protocol: %1", protocol );
    doc += QL1( "</li><li>" );
  }

  doc += i18n( "Date and Time: %1" ,  datetime );
  doc += QL1( "</li><li>" );
  doc += i18n( "Additional Information: %1" ,  text );
  doc += QL1( "</li></ul><h3>" );
  doc += i18n( "Description:" );
  doc += QL1( "</h3><p>" );
  doc += description;
  doc += QL1( "</p>" );

  if ( causes.count() ) {
    doc += QL1( "<h3>" );
    doc += i18n( "Possible Causes:" );
    doc += QL1( "</h3><ul><li>" );
    doc += causes.join( "</li><li>" );
    doc += QL1( "</li></ul>" );
  }

  if ( solutions.count() ) {
    doc += QL1( "<h3>" );
    doc += i18n( "Possible Solutions:" );
    doc += QL1( "</h3><ul><li>" );
    doc += solutions.join( "</li><li>" );
    doc += QL1( "</li></ul>" );
  }

  html.replace( QL1("TEXT"), doc );

  return html;
}


class WebKitPart::WebKitPartPrivate
{
public:
  enum PageSecurity { Unencrypted, Encrypted, Mixed };
  WebKitPartPrivate() : updateHistory(true) {}

  bool updateHistory;

  WebView *webView;
  WebKitBrowserExtension *browserExtension;
};

WebKitPart::WebKitPart(QWidget *parentWidget, QObject *parent, const QStringList &/*args*/)
           :KParts::ReadOnlyPart(parent), d(new WebKitPart::WebKitPartPrivate())
{
    KAboutData about = KAboutData("webkitpart", "webkitkde", ki18n("WebKit HTML Component"),
                                  /*version*/ "0.2", /*ki18n("shortDescription")*/ KLocalizedString(),
                                  KAboutData::License_LGPL,
                                  ki18n("(c) 2009 Dawit Alemayehu\n"
                                        "(c) 2008-2009 Urs Wolfer\n"
                                        "(c) 2007 Trolltech ASA"));

    about.addAuthor(ki18n("Laurent Montel"), KLocalizedString(), "montel@kde.org");
    about.addAuthor(ki18n("Michael Howell"), KLocalizedString(), "mhowell123@gmail.com");
    about.addAuthor(ki18n("Urs Wolfer"), KLocalizedString(), "uwolfer@kde.org");
    about.addAuthor(ki18n("Dirk Mueller"), KLocalizedString(), "mueller@kde.org");
    about.addAuthor(ki18n("Dawit Alemayehu"), KLocalizedString(), "adawit@kde.org");
    KComponentData componentData(&about);
    setComponentData(componentData);

    // NOTE: If the application does not set its version number, we automatically
    // set it to KDE's version number so that the default user-agent string contains
    // proper application version number information. See QWebPage::userAgentForUrl...
    if (QCoreApplication::applicationVersion().isEmpty())
        QCoreApplication::setApplicationVersion(QString("%1.%2.%3")
                                                .arg(KDE::versionMajor())
                                                .arg(KDE::versionMinor())
                                                .arg(KDE::versionRelease()));

    setWidget(new QWidget(parentWidget));
    QVBoxLayout* lay = new QVBoxLayout(widget());
    lay->setMargin(0);
    lay->setSpacing(0);

    d->webView = new WebView(this, widget());
    lay->addWidget(d->webView);
    lay->addWidget(d->webView->searchBar());

    connect(d->webView, SIGNAL(titleChanged(const QString &)),
            this, SLOT(setWindowTitle(const QString &)));
    connect(d->webView, SIGNAL(loadFinished(bool)),
            this, SLOT(loadFinished(bool)));
    connect(d->webView, SIGNAL(urlChanged(const QUrl &)),
            this, SLOT(urlChanged(const QUrl &)));
#if 0
    connect(d->webView, SIGNAL(statusBarMessage(const QString&)),
            this, SIGNAL(setStatusBarText(const QString &)));
#endif

    WebPage* webPage = qobject_cast<WebPage*>(d->webView->page());
    Q_ASSERT(webPage);

    connect(webPage, SIGNAL(loadStarted()),
            this, SLOT(loadStarted()));
    connect(webPage, SIGNAL(loadMainPageFinished()),
            this, SLOT(loadMainPageFinished()));
    connect(webPage, SIGNAL(loadAborted(const QUrl &)),
            this, SLOT(loadAborted(const QUrl &)));
    connect(webPage, SIGNAL(loadError(int, const QString &)),
            this, SLOT(loadError(int, const QString &)));
    connect(webPage, SIGNAL(linkHovered(const QString &, const QString &, const QString &)),
            this, SLOT(linkHovered(const QString &, const QString&, const QString &)));

    connect(d->webView, SIGNAL(saveUrl(const KUrl &)),
            webPage, SLOT(saveUrl(const KUrl &)));

    d->browserExtension = new WebKitBrowserExtension(this);
    connect(webPage, SIGNAL(loadProgress(int)),
            d->browserExtension, SIGNAL(loadingProgress(int)));

    setXMLFile("webkitpart.rc");
    initAction();
}

WebKitPart::~WebKitPart()
{
    delete d;
}

void WebKitPart::initAction()
{
    KAction *action = actionCollection()->addAction(KStandardAction::SaveAs, "saveDocument",
                                                    d->browserExtension, SLOT(slotSaveDocument()));

    action = new KAction(i18n("Save &Frame As..."), this);
    actionCollection()->addAction("saveFrame", action);
    connect(action, SIGNAL(triggered(bool)), d->browserExtension, SLOT(slotSaveFrame()));

    action = new KAction(KIcon("document-print-frame"), i18n("Print Frame..."), this);
    actionCollection()->addAction("printFrame", action);
    connect(action, SIGNAL(triggered(bool)), d->browserExtension, SLOT(printFrame()));

    action = new KAction(KIcon("zoom-in"), i18n("Zoom In"), this);
    actionCollection()->addAction("zoomIn", action);
    action->setShortcut(KShortcut("CTRL++; CTRL+="));
    connect(action, SIGNAL(triggered(bool)), d->browserExtension, SLOT(zoomIn()));

    action = new KAction(KIcon("zoom-out"), i18n("Zoom Out"), this);
    actionCollection()->addAction("zoomOut", action);
    action->setShortcut(KShortcut("CTRL+-; CTRL+_"));
    connect(action, SIGNAL(triggered(bool)), d->browserExtension, SLOT(zoomOut()));

    action = new KAction(KIcon("zoom-original"), i18n("Actual Size"), this);
    actionCollection()->addAction("zoomNormal", action);
    action->setShortcut(KShortcut("CTRL+0"));
    connect(action, SIGNAL(triggered(bool)), d->browserExtension, SLOT(zoomNormal()));

#if QT_VERSION >= 0x040500
    action = new KAction(i18n("Zoom Text Only"), this);
    action->setCheckable(true);
    KConfigGroup cgHtml(KGlobal::config(), "HTML Settings");
    bool zoomTextOnly = cgHtml.readEntry("ZoomTextOnly", false);
    action->setChecked(zoomTextOnly);
    actionCollection()->addAction("zoomTextOnly", action);
    connect(action, SIGNAL(triggered(bool)), d->browserExtension, SLOT(toogleZoomTextOnly()));
#endif

    action = actionCollection()->addAction(KStandardAction::SelectAll, "selectAll",
                                           d->browserExtension, SLOT(slotSelectAll()));
    action->setShortcutContext(Qt::WidgetShortcut);
    d->webView->addAction(action);

    action = new KAction(i18n("View Do&cument Source"), this);
    actionCollection()->addAction("viewDocumentSource", action);
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_U));
    connect(action, SIGNAL(triggered(bool)), d->browserExtension, SLOT(slotViewDocumentSource()));

    action = new KAction(i18n("SSL"), this);
    actionCollection()->addAction("security", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(showSecurity()));

    action = actionCollection()->addAction(KStandardAction::Find, "find", d->webView->searchBar(), SLOT(show()));
    action->setWhatsThis(i18n("Find text<br /><br />"
                              "Shows a dialog that allows you to find text on the displayed page."));
}

void WebKitPart::guiActivateEvent(KParts::GUIActivateEvent *event)
{
    Q_UNUSED(event);
    // just overwrite, but do nothing for the moment
}

bool WebKitPart::openUrl(const KUrl &url)
{
    kDebug() << url;

   // Ignore empty requests...
   if (url.isEmpty())
      return false;

    if ( url.protocol() == "error" && url.hasSubUrl() ) {
        closeUrl();

        /**
         * The format of the error url is that two variables are passed in the query:
         * error = int kio error code, errText = QString error text from kio
         * and the URL where the error happened is passed as a sub URL.
         */
        KUrl::List urls = KUrl::split( url );
        //kDebug() << "Handling error URL. URL count:" << urls.count();

        if ( urls.count() > 1 ) {
            KUrl mainURL = urls.first();
            int error = mainURL.queryItem( "error" ).toInt();

            // error=0 isn't a valid error code, so 0 means it's missing from the URL
            if ( error == 0 )
                error = KIO::ERR_UNKNOWN;

            if (error == KIO::ERR_USER_CANCELED) {
                setUrl(d->webView->url());
                emit d->browserExtension->setLocationBarUrl(KUrl(d->webView->url()).prettyUrl());
                loadMainPageFinished();
            } else {
                QString errorText = mainURL.queryItem( "errText" );
                urls.pop_front();
                KUrl reqUrl = KUrl::join( urls );

                kDebug() << "Setting Url back to => " << reqUrl;
                emit d->browserExtension->setLocationBarUrl(reqUrl.prettyUrl());
                setUrl(reqUrl);

                emit started(0);
                showError(htmlError(error, errorText, reqUrl));
                emit completed();
            }

            return true;
        }
    }

    if (url.url() == "about:blank") {
        setWindowTitle (url.url());
    } else {
        setUrl(url);
        d->updateHistory = false;  // Do not update history when url is typed in since konqueror automatically does this itself!
        d->webView->loadUrl(url, arguments(), browserExtension()->browserArguments());
    }

    return true;
}

bool WebKitPart::closeUrl()
{
    d->webView->stop();
    return true;
}

WebKitBrowserExtension *WebKitPart::browserExtension() const
{
    return d->browserExtension;
}

bool WebKitPart::openFile()
{
    // never reached
    return false;
}

void WebKitPart::loadStarted()
{
    emit started(0);
}

void WebKitPart::loadFinished(bool ok)
{
    d->updateHistory = true;

    if (ok && d->webView->title().trimmed().isEmpty()) {
        // If the document title is null, then set it to the current url
        // squeezed at the center.
        QString caption (d->webView->url().toString((QUrl::RemoveQuery|QUrl::RemoveFragment)));
        emit setWindowCaption(KStringHandler::csqueeze(caption));

        // The urlChanged signal is emitted if and only if the main frame
        // receives the title of the page so we manually invoke the slot as
        // a work around here for pages that do not contain it, such as
        // text documents...
        urlChanged(d->webView->url());
    }

    /*
      NOTE #1: QtWebKit will not kill a META redirect request even if one
      triggers the WebPage::Stop action!! As such the code below is useless
      and disabled for now.

      NOTE #2: QWebFrame::metaData only provides access to META tags that
      contain a 'name' attribute and completely ignores those that do not.
      This of course includes yes the meta redirect tag, i.e. the 'http-equiv'
      attribute. Hence the convoluted code below just to check if we have a
      redirect request!
    */
#if 0
    bool refresh = false;
    QMapIterator<QString,QString> it (d->webView->page()->mainFrame()->metaData());
    while (it.hasNext()) {
      it.next();
      //kDebug() << "meta-key: " << it.key() << "meta-value: " << it.value();
      // HACK: QtWebKit does not parse the value of http-equiv property and
      // as such uses an empty key with a value when
      if (it.key().isEmpty() &&
          it.value().toLower().simplified().contains(QRegExp("[0-9];url"))) {
        refresh = true;
        break;
      }
    }
    emit completed(refresh);
#else
    emit completed();
#endif
}

void  WebKitPart::loadMainPageFinished()
{
     if (qobject_cast<WebPage*>(d->webView->page())->isSecurePage())
        d->browserExtension->setPageSecurity(WebKitPart::WebKitPartPrivate::Encrypted);
     else
        d->browserExtension->setPageSecurity(WebKitPart::WebKitPartPrivate::Unencrypted);
}

void WebKitPart::loadAborted(const QUrl & url)
{  
    closeUrl();
    if (url.isValid())
      emit d->browserExtension->openUrlRequest(url);
    else
      setUrl(d->webView->url());
}

void WebKitPart::loadError(int errCode, const QString &errStr)
{
    showError(htmlError(errCode, errStr, url()));
}

void WebKitPart::urlChanged(const QUrl& _url)
{
    QUrl currentUrl (url());
    if ((_url.toString(QUrl::RemoveFragment) == currentUrl.toString(QUrl::RemoveFragment)) &&
        (_url.fragment() != currentUrl.fragment()))
        updateHistory(true);

    setUrl(_url);
    emit d->browserExtension->setLocationBarUrl(KUrl(_url).prettyUrl());
}

void WebKitPart::showSecurity()
{
    WebPage* page = qobject_cast<WebPage*>(d->webView->page());
    if (page->isSecurePage()) {
        KSslInfoDialog *dlg = new KSslInfoDialog(0);
        page->setupSslDialog(*dlg);
        dlg->exec();
    } else {
        KMessageBox::information(0, i18n("The SSL information for this site "
                                         "appears to be corrupt."), i18n("SSL"));
    }
}

void WebKitPart::updateHistory(bool enable)
{
    // Send a history update request to Konqueror whenever QtWebKit requests
    // the clearing of the Window title which indicates content change...
    kDebug() <<  "update history ? "<< d->updateHistory;

    if (d->updateHistory) {
        emit d->browserExtension->openUrlNotify();
        d->updateHistory = enable;
    }
}

WebView * WebKitPart::view()
{
    return d->webView;
}

void WebKitPart::setStatusBarTextProxy(const QString &message)
{
    //kDebug() << message;
    emit setStatusBarText(message);
}

void WebKitPart::linkHovered(const QString &link, const QString &, const QString &)
{
    QString message;
    QUrl linkUrl (link);
    const QString scheme = linkUrl.scheme();

    if (QString::compare(scheme, QL1("mailto"), Qt::CaseInsensitive) == 0) {
        message += i18n("Email: ");

        // Workaround: QUrl's parsing deficiencies "mailto:foo@bar.com".
        if (!linkUrl.hasQuery())
          linkUrl = QUrl(scheme + '?' + linkUrl.path());

        QMap<QString, QStringList> fields;
        QPair<QString, QString> queryItem;

        Q_FOREACH (queryItem, linkUrl.queryItems()) {
            //kDebug() << "query: " << queryItem.first << queryItem.second;
            if (queryItem.first.contains(QChar('@')) && queryItem.second.isEmpty())
                fields["to"] << queryItem.first;
            if (QString::compare(queryItem.first, QL1("to"), Qt::CaseInsensitive) == 0)
                fields["to"] << queryItem.second;
            if (QString::compare(queryItem.first, QL1("cc"), Qt::CaseInsensitive) == 0)
                fields["cc"] << queryItem.second;
            if (QString::compare(queryItem.first, QL1("bcc"), Qt::CaseInsensitive) == 0)
                fields["bcc"] << queryItem.second;
            if (QString::compare(queryItem.first, QL1("subject"), Qt::CaseInsensitive) == 0)
                fields["subject"] << queryItem.second;
        }

        if (fields.contains(QL1("to")))
            message += fields.value(QL1("to")).join(QL1(", "));
        if (fields.contains(QL1("cc")))
            message += QL1(" - CC: ") + fields.value(QL1("cc")).join(QL1(", "));
        if (fields.contains(QL1("bcc")))
            message += QL1(" - BCC: ") + fields.value(QL1("bcc")).join(QL1(", "));
        if (fields.contains(QL1("subject")))
            message += QL1(" - Subject: ") + fields.value(QL1("subject")).join(QL1(" "));
    } else {
        message = link;
    }

    emit setStatusBarText(message);
}

void WebKitPart::showError(const QString& html)
{
    const bool signalsBlocked = d->webView->blockSignals(true);
    d->webView->setHtml(html);
    d->webView->blockSignals(signalsBlocked);
}

void WebKitPart::setWindowTitle(const QString &title)
{
    if (title.isEmpty())
      updateHistory();

    emit setWindowCaption(title);
}


WebKitBrowserExtension::WebKitBrowserExtension(WebKitPart *parent)
                       :KParts::BrowserExtension(parent), part(parent)
{
    connect(part->view()->page(), SIGNAL(selectionChanged()),
            this, SLOT(updateEditActions()));
    connect(part->view(), SIGNAL(openUrl(const KUrl &)),
            this, SIGNAL(openUrlRequest(const KUrl &)));
    connect(part->view(), SIGNAL(openUrlInNewTab(const KUrl &)),
            this, SIGNAL(createNewWindow(const KUrl &)));

    enableAction("cut", false);
    enableAction("copy", false);
    enableAction("paste", false);
    enableAction("print", true);
}

void WebKitBrowserExtension::cut()
{
    part->view()->page()->triggerAction(KWebPage::Cut);
}

void WebKitBrowserExtension::copy()
{
    part->view()->page()->triggerAction(KWebPage::Copy);
}

void WebKitBrowserExtension::paste()
{
    part->view()->page()->triggerAction(KWebPage::Paste);
}

void WebKitBrowserExtension::slotSaveDocument()
{
    qobject_cast<WebPage*>(part->view()->page())->saveUrl(part->view()->url());
}

void WebKitBrowserExtension::slotSaveFrame()
{
    qobject_cast<WebPage*>(part->view()->page())->saveUrl(part->view()->page()->currentFrame()->url());
}

void WebKitBrowserExtension::print()
{
    QPrintPreviewDialog dlg(part->view());
    connect(&dlg, SIGNAL(paintRequested(QPrinter *)),
            part->view(), SLOT(print(QPrinter *)));
    dlg.exec();
}

void WebKitBrowserExtension::printFrame()
{
    QPrintPreviewDialog dlg(part->view());
    connect(&dlg, SIGNAL(paintRequested(QPrinter *)),
            part->view()->page()->currentFrame(), SLOT(print(QPrinter *)));
    dlg.exec();
}

void WebKitBrowserExtension::updateEditActions()
{
    KWebPage *page = part->view()->page();
    enableAction("cut", page->action(KWebPage::Cut));
    enableAction("copy", page->action(KWebPage::Copy));
    enableAction("paste", page->action(KWebPage::Paste));
}

void WebKitBrowserExtension::searchProvider()
{
    // action name is of form "previewProvider[<searchproviderprefix>:]"
    const QString searchProviderPrefix = QString(sender()->objectName()).mid(14);

    const QString text = part->view()->page()->selectedText();
    KUriFilterData data;
    QStringList list;
    data.setData(searchProviderPrefix + text);
    list << "kurisearchfilter" << "kuriikwsfilter";

    if (!KUriFilter::self()->filterUri(data, list)) {
        KDesktopFile file("services", "searchproviders/google.desktop");
        const QString encodedSearchTerm = QUrl::toPercentEncoding(text);
        KConfigGroup cg(file.desktopGroup());
        data.setData(cg.readEntry("Query").replace("\\{@}", encodedSearchTerm));
    }

    KParts::BrowserArguments browserArgs;
    browserArgs.frameName = "_blank";

    emit openUrlRequest(data.uri(), KParts::OpenUrlArguments(), browserArgs);
}

void WebKitBrowserExtension::reparseConfiguration()
{
  // Force the configuration stuff to repase...
  WebKitSettings::self()->init();
}

void WebKitBrowserExtension::zoomIn()
{
#if QT_VERSION >= 0x040500
    part->view()->setZoomFactor(part->view()->zoomFactor() + 0.1);
#else
    part->view()->setTextSizeMultiplier(part->view()->textSizeMultiplier() + 0.1);
#endif
}

void WebKitBrowserExtension::zoomOut()
{
#if QT_VERSION >= 0x040500
    part->view()->setZoomFactor(part->view()->zoomFactor() - 0.1);
#else
    part->view()->setTextSizeMultiplier(part->view()->textSizeMultiplier() - 0.1);
#endif
}

void WebKitBrowserExtension::zoomNormal()
{
#if QT_VERSION >= 0x040500
    part->view()->setZoomFactor(1);
#else
    part->view()->setTextSizeMultiplier(1);
#endif
}

void WebKitBrowserExtension::toogleZoomTextOnly()
{
#if QT_VERSION >= 0x040500
    KConfigGroup cgHtml(KGlobal::config(), "HTML Settings");
    bool zoomTextOnly = cgHtml.readEntry( "ZoomTextOnly", false );
    cgHtml.writeEntry("ZoomTextOnly", !zoomTextOnly);
    KGlobal::config()->reparseConfiguration();

    part->view()->settings()->setAttribute(QWebSettings::ZoomTextOnly, !zoomTextOnly);
#endif
}

void WebKitBrowserExtension::slotSelectAll()
{
#if QT_VERSION >= 0x040500
    part->view()->page()->triggerAction(KWebPage::SelectAll);
#endif
}

void WebKitBrowserExtension::slotFrameInWindow()
{
    KParts::OpenUrlArguments args;// = d->m_khtml->arguments();
    args.metaData()["referrer"] = part->view()->contextMenuResult().linkText();
    args.metaData()["forcenewwindow"] = "true";
    emit createNewWindow(part->view()->contextMenuResult().linkUrl(), args);
}

void WebKitBrowserExtension::slotFrameInTab()
{
    KParts::OpenUrlArguments args;// = d->m_khtml->arguments();
    args.metaData()["referrer"] = part->view()->contextMenuResult().linkText();
    KParts::BrowserArguments browserArgs;//( d->m_khtml->browserExtension()->browserArguments() );
    browserArgs.setNewTab(true);
    emit createNewWindow(part->view()->contextMenuResult().linkUrl(), args, browserArgs);
}

void WebKitBrowserExtension::slotFrameInTop()
{
    KParts::OpenUrlArguments args;// = d->m_khtml->arguments();
    args.metaData()["referrer"] = part->view()->contextMenuResult().linkText();
    KParts::BrowserArguments browserArgs;//( d->m_khtml->browserExtension()->browserArguments() );
    browserArgs.frameName = "_top";
    emit openUrlRequest(part->view()->contextMenuResult().linkUrl(), args, browserArgs);
}

void WebKitBrowserExtension::slotSaveImageAs()
{
    QList<KUrl> urls;
    urls.append(part->view()->contextMenuResult().imageUrl());
    const int nbUrls = urls.count();
    for (int i = 0; i != nbUrls; i++) {
        QString file = KFileDialog::getSaveFileName(KUrl(), QString(), part->widget());
        KIO::NetAccess::file_copy(urls.at(i), file, part->widget());
    }
}

void WebKitBrowserExtension::slotSendImage()
{
    QStringList urls;
    urls.append(part->view()->contextMenuResult().imageUrl().path());
    const QString subject = part->view()->contextMenuResult().imageUrl().path();
    KToolInvocation::invokeMailer(QString(), QString(), QString(), subject,
                                  QString(), //body
                                  QString(),
                                  urls); // attachments
}

void WebKitBrowserExtension::slotCopyImage()
{
    KUrl safeURL(part->view()->contextMenuResult().imageUrl());
    safeURL.setPass(QString());

    // Set it in both the mouse selection and in the clipboard
    QMimeData* mimeData = new QMimeData;
    mimeData->setImageData(part->view()->contextMenuResult().pixmap());
    safeURL.populateMimeData(mimeData);
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);

    mimeData = new QMimeData;
    mimeData->setImageData(part->view()->contextMenuResult().pixmap());
    safeURL.populateMimeData(mimeData);
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Selection);
}

void WebKitBrowserExtension::slotCopyLinkLocation()
{
    KUrl safeURL(part->view()->contextMenuResult().linkUrl());
    safeURL.setPass(QString());
    // Set it in both the mouse selection and in the clipboard
    QMimeData* mimeData = new QMimeData;
    safeURL.populateMimeData(mimeData);
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);

    mimeData = new QMimeData;
    safeURL.populateMimeData(mimeData);
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Selection);
}

void WebKitBrowserExtension::slotSaveLinkAs()
{
    qobject_cast<WebPage*>(part->view()->page())->saveUrl(part->view()->contextMenuResult().linkUrl());
}

void WebKitBrowserExtension::slotViewDocumentSource()
{
    //TODO test http requests
    KUrl currentUrl(part->view()->page()->mainFrame()->url());
    bool isTempFile = false;
#if 0
    if (!(currentUrl.isLocalFile())/* && KHTMLPageCache::self()->isComplete(d->m_cacheId)*/) { //TODO implement
        KTemporaryFile sourceFile;
//         sourceFile.setSuffix(defaultExtension());
        sourceFile.setAutoRemove(false);
        if (sourceFile.open()) {
//             QDataStream stream (&sourceFile);
//             KHTMLPageCache::self()->saveData(d->m_cacheId, &stream);
            currentUrl = KUrl();
            currentUrl.setPath(sourceFile.fileName());
            isTempFile = true;
        }
    }
#endif

    KRun::runUrl(currentUrl, QL1("text/plain"), part->view(), isTempFile);
}

#include "webkitpart.moc"
