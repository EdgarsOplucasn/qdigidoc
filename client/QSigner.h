/*
 * QDigiDocClient
 *
 * Copyright (C) 2009-2013 Jargo Kõster <jargo@innovaatik.ee>
 * Copyright (C) 2009-2013 Raul Metsma <raul@innovaatik.ee>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#pragma once

#include <QtCore/QThread>
#include <digidocpp/crypto/Signer.h>

class QMutex;
class QSignerPrivate;
class TokenData;

class QSigner: public QThread, public digidoc::Signer
{
	Q_OBJECT

public:
	enum ApiType
	{
		PKCS11,
		CAPI,
		CNG
	};
	explicit QSigner( ApiType api, QObject *parent = 0 );
	~QSigner();

	ApiType apiType() const;
	digidoc::X509Cert cert() const;
	Qt::HANDLE handle() const;
	QMutex* mutex() const;
	void lock();
	void sign( const std::string &method, const std::vector<unsigned char> &digest,
		std::vector<unsigned char>& signature );
	TokenData token() const;
	void unlock();

Q_SIGNALS:
	void dataChanged();
	void error( const QString &msg );

private Q_SLOTS:
	void selectCard( const QString &card );
	void showWarning( const QString &msg );

private:
	void reload();
	void run();
	void throwException( const QString &msg, digidoc::Exception::ExceptionCode code, int line );

	QSignerPrivate *d;
};
