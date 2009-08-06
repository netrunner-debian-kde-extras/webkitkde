/*
 * This file is part of the KDE project.
 *
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
#ifndef KWEBPAGESSLINFO_H
#define KWEBPAGESSLINFO_H

#include <kdemacros.h>

#include <QtCore/QUrl>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QSslCertificate>

class WebSslInfo
{
public:
  WebSslInfo();
  virtual ~WebSslInfo();

  QUrl url() const;
  QHostAddress peerAddress() const;
  QHostAddress parentAddress() const;
  QString ciphers() const;
  QString protocol() const;
  QString certificateErrors() const;
  int supportedChiperBits () const;
  int usedChiperBits () const;
  QList<QSslCertificate> certificateChain() const;
  bool isValid() const;

protected:
  void reset();
  void setUrl (const QUrl& url);
  void setUrl (const QString& url);

  void setCiphers(const QString& ciphers);
  void setProtocol(const QString& protocol);
  void setPeerAddress(const QString& address);
  void setParentAddress(const QString& address);
  void setCertificateChain(const QByteArray& chain);
  void setCertificateErrors(const QString& certErrors);

  void setUsedCipherBits(const QString& bits);
  void setSupportedCipherBits(const QString& bits);

private:
  class WebSslInfoPrivate;
  WebSslInfoPrivate* d;
};

#endif 
