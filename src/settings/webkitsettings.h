/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef WEBKITSETTINGS_H
#define WEBKITSETTINGS_H

class KConfig;
class KConfigGroup;

#include <QtGui/QColor>
#include <QtCore/QStringList>
#include <QtCore/QPair>
#include <kdewebkit_export.h>

struct KPerDomainSettings;
class WebKitSettingsPrivate;

/**
 * Settings for the HTML view.
 */
class KDEWEBKIT_EXPORT WebKitSettings
{
public:

    /**
     * This enum specifies whether Java/JavaScript execution is allowed.
     */
    enum KJavaScriptAdvice {
      KJavaScriptDunno=0,
      KJavaScriptAccept,
      KJavaScriptReject
    };

    enum KAnimationAdvice {
      KAnimationDisabled=0,
      KAnimationLoopOnce,
      KAnimationEnabled
    };

    enum KSmoothScrollingMode {
      KSmoothScrollingDisabled=0,
      KSmoothScrollingWhenEfficient,
      KSmoothScrollingEnabled
    };

    /**
     * This enum specifies the policy for window.open
     */
    enum KJSWindowOpenPolicy {
    	KJSWindowOpenAllow=0,
    	KJSWindowOpenAsk,
    	KJSWindowOpenDeny,
    	KJSWindowOpenSmart
    };

    /**
     * This enum specifies the policy for window.status and .defaultStatus
     */
    enum KJSWindowStatusPolicy {
    	KJSWindowStatusAllow=0,
    	KJSWindowStatusIgnore
    };

    /**
     * This enum specifies the policy for window.moveBy and .moveTo
     */
    enum KJSWindowMovePolicy {
    	KJSWindowMoveAllow=0,
    	KJSWindowMoveIgnore
    };

    /**
     * This enum specifies the policy for window.resizeBy and .resizeTo
     */
    enum KJSWindowResizePolicy {
    	KJSWindowResizeAllow=0,
    	KJSWindowResizeIgnore
    };

    /**
     * This enum specifies the policy for window.focus
     */
    enum KJSWindowFocusPolicy {
    	KJSWindowFocusAllow=0,
    	KJSWindowFocusIgnore
    };

    /**
     * Called by constructor and reparseConfiguration
     */
    void init();

    /** Read settings from @p config.
     * @param config is a pointer to KConfig object.
     * @param reset if true, settings are always set; if false,
     *  settings are only set if the config file has a corresponding key.
     */
    void init( KConfig * config, bool reset = true );

    /**
     * Destructor. Don't delete any instance by yourself.
     */
    virtual ~WebKitSettings();

    // Behavior settings
    bool changeCursor() const;
    bool underlineLink() const;
    bool hoverLink() const;
    bool allowTabulation() const;
    bool autoSpellCheck() const;
    KAnimationAdvice showAnimations() const;
    KSmoothScrollingMode smoothScrolling() const;
    bool zoomTextOnly() const;

    // Font settings
    QString stdFontName() const;
    QString fixedFontName() const;
    QString serifFontName() const;
    QString sansSerifFontName() const;
    QString cursiveFontName() const;
    QString fantasyFontName() const;

    // these two can be set. Mainly for historical reasons (the method in KHTMLPart exists...)
    void setStdFontName(const QString &n);
    void setFixedFontName(const QString &n);

    int minFontSize() const;
    int mediumFontSize() const;

    void computeFontSizes(int logicalDpi);

    bool jsErrorsEnabled() const;
    void setJSErrorsEnabled(bool enabled);

    const QString &encoding() const;

    bool followSystemColors() const;

    // Color settings
    const QColor& textColor() const;
    const QColor& baseColor() const;
    const QColor& linkColor() const;
    const QColor& vLinkColor() const;

    // Automatic page reload/refresh...
    bool autoPageRefresh() const;

    // Autoload images
    bool autoLoadImages() const;
    bool unfinishedImageFrame() const;

    bool isOpenMiddleClickEnabled();
    bool isBackRightClickEnabled();

    // Java and JavaScript
    bool isJavaEnabled( const QString& hostname = QString() ) const;
    bool isJavaScriptEnabled( const QString& hostname = QString() ) const;
    bool isJavaScriptDebugEnabled( const QString& hostname = QString() ) const;
    bool isJavaScriptErrorReportingEnabled( const QString& hostname = QString() ) const;
    bool isPluginsEnabled( const QString& hostname = QString() ) const;

    // AdBlocK Filtering
    bool isAdFiltered( const QString &url ) const;
    bool isAdFilterEnabled() const;
    bool isHideAdsEnabled() const;
    void addAdFilter( const QString &url );

    // Access Keys
    bool accessKeysEnabled() const;

    KJSWindowOpenPolicy windowOpenPolicy( const QString& hostname = QString() ) const;
    KJSWindowMovePolicy windowMovePolicy( const QString& hostname = QString() ) const;
    KJSWindowResizePolicy windowResizePolicy( const QString& hostname = QString() ) const;
    KJSWindowStatusPolicy windowStatusPolicy( const QString& hostname = QString() ) const;
    KJSWindowFocusPolicy windowFocusPolicy( const QString& hostname = QString() ) const;

    // helpers for parsing domain-specific configuration, used in KControl module as well
    static KJavaScriptAdvice strToAdvice(const QString& _str);
    static void splitDomainAdvice(const QString& configStr, QString &domain,
				  KJavaScriptAdvice &javaAdvice, KJavaScriptAdvice& javaScriptAdvice);
    static const char* adviceToStr(KJavaScriptAdvice _advice);

    /** reads from @p config's current group, forcing initialization
      * if @p reset is true.
      * @param config is a pointer to KConfig object.
      * @param reset true if initialization is to be forced.
      * @param global true if the global domain is to be read.
      * @param pd_settings will be initialised with the computed (inherited)
      *		settings.
      */
    void readDomainSettings(const KConfigGroup &config, bool reset,
			bool global, KPerDomainSettings &pd_settings);

    QString settingsToCSS() const;
    static const QString &availableFamilies();

    QString userStyleSheet() const;

    // Form completion
    bool isFormCompletionEnabled() const;
    int maxFormCompletionItems() const;

    // Meta refresh/redirect (http-equiv)
    bool isAutoDelayedActionsEnabled () const;

    QList< QPair< QString, QChar > > fallbackAccessKeysAssignments() const;

    // Whether to show passive popup when windows are blocked
    void setJSPopupBlockerPassivePopup(bool enabled);
    bool jsPopupBlockerPassivePopup() const;

    // CookieJar...
    bool isCookieJarEnabled() const;

    // Password storage...
    bool isNonPasswordStorableSite(const QString &host) const;
    void addNonPasswordStorableSite(const QString &host);
    void removeNonPasswordStorableSite(const QString &host);

    // Global config object stuff.
    static WebKitSettings* self();
    /**
     * @internal Constructor
     */
    WebKitSettings();
    WebKitSettings(const WebKitSettings &other);

private:
    QString lookupFont(int i) const;

    WebKitSettingsPrivate* const d;
    static QString *avFamilies;
};

#endif
