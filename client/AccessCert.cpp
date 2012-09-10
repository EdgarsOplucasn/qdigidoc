/*
 * QDigiDocClient
 *
 * Copyright (C) 2009-2012 Jargo Kõster <jargo@innovaatik.ee>
 * Copyright (C) 2009-2012 Raul Metsma <raul@innovaatik.ee>
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

#include "AccessCert.h"

#include "Application.h"
#include "QSigner.h"

#ifdef Q_OS_WIN
#include <common/QCNG.h>
#endif
#include <common/QPKCS11.h>
#include <common/SslCertificate.h>
#include <common/sslConnect.h>
#include <common/TokenData.h>

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QScopedPointer>
#include <QtCore/QUrl>
#include <QtCore/QXmlStreamReader>
#include <QtGui/QDesktopServices>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>

#ifdef Q_OS_MAC
#include <Security/Security.h>
#endif

AccessCert::AccessCert( QWidget *parent )
:	QMessageBox( parent )
{
	setWindowTitle( tr("Server access certificate") );
	if( QLabel *label = findChild<QLabel*>() )
		label->setOpenExternalLinks( true );
#ifndef Q_OS_MAC
	m_cert = Application::confValue( Application::PKCS12Cert ).toString();
	m_pass = Application::confValue( Application::PKCS12Pass ).toString();
#endif
}

AccessCert::~AccessCert()
{
#ifndef Q_OS_MAC
	Application::setConfValue( Application::PKCS12Cert, m_cert );
	Application::setConfValue( Application::PKCS12Pass, m_pass );
#endif
}

bool AccessCert::download( bool noCard )
{
	if( noCard )
	{
		QDesktopServices::openUrl( QUrl( tr("http://www.sk.ee/toend/") ) );
		return false;
	}

	SslCertificate tempel( qApp->signer()->token().cert() );
	if( tempel.isTempel() )
	{
		setIcon( Information );
		setText( tr("For getting server access certificate to Tempel contact <a href=\"mailto:sales@sk.ee\">sales@sk.ee</a>") );
		return false;
	}

	setIcon( Information );
	setText(
		tr("Hereby I agree to terms and conditions of validity confirmation service and "
		   "will use the service in extent of 10 signatures per month. If you going to "
		   "exceed the limit of 10 signatures per month or/and will use the service for "
		   "commercial purposes, please refer to IT support of your company. Additional "
		   "information is available from <a href=\"%1\">%1</a> or phone 1777")
			.arg( tr("http://www.id.ee/kehtivuskinnitus") ) );
	setStandardButtons( Help );
	QPushButton *agree = addButton( tr("Agree"), AcceptRole );
	if( exec() == Help )
	{
		QDesktopServices::openUrl( QUrl( tr("http://www.id.ee/kehtivuskinnitus") ) );
		return false;
	}
	removeButton( agree );

	QSigner *s = qApp->signer();
	QPKCS11 *p = qobject_cast<QPKCS11*>(reinterpret_cast<QObject*>(s->handle()));
#ifdef Q_OS_WIN
	QCNG *c = qobject_cast<QCNG*>(reinterpret_cast<QObject*>(s->handle()));
	if( !p && !s )
		return false;
#endif

	s->lock();
	Qt::HANDLE key = 0;
	TokenData token;
	if( p )
	{
		bool retry = false;
		do
		{
			retry = false;
			token = p->selectSlot( s->token().card(), SslCertificate::KeyUsageNone, SslCertificate::ClientAuth );
			QPKCS11::PinStatus status = p->login( token );
			switch( status )
			{
			case QPKCS11::PinOK: break;
			case QPKCS11::PinCanceled:
				s->unlock();
				return false;
			case QPKCS11::PinIncorrect:
				showWarning( QPKCS11::errorString( status ) );
				retry = true;
				break;
			default:
				showWarning( tr("Error downloading server access certificate!") + "\n" + QPKCS11::errorString( status ) );
				s->unlock();
				return false;
			}
		}
		while( retry );
		key = p->key();
	}
	else
	{
#ifdef Q_OS_WIN
		foreach( const SslCertificate &cert, c->certs() )
		{
			if( cert.isValid() && cert.enhancedKeyUsage().contains( SslCertificate::ClientAuth ) )
			{
				token = c->selectCert( cert );
				break;
			}
		}
		key = c->key();
#else
		s->unlock();
		return false;
#endif
	}

	QScopedPointer<SSLConnect> ssl( new SSLConnect );
	ssl->setToken( token.cert(), key );
	QByteArray result = ssl->getUrl( SSLConnect::AccessCert );
	s->unlock();
	if( !ssl->errorString().isEmpty() )
	{
		showWarning( tr("Error downloading server access certificate!") + "\n" + ssl->errorString() );
		return false;
	}

	if( result.isEmpty() )
	{
		showWarning( tr("Empty result!") );
		return false;
	}

	QString status, cert, pass, message;
	QXmlStreamReader xml( result );
	while( xml.readNext() != QXmlStreamReader::Invalid )
	{
		if( !xml.isStartElement() )
			continue;
		if( xml.name() == "StatusCode" )
			status = xml.readElementText();
		else if( xml.name() == "MessageToDisplay" )
			message = xml.readElementText();
		else if( xml.name() == "TokenData" )
			cert = xml.readElementText();
		else if( xml.name() == "TokenPassword" )
			pass = xml.readElementText();
	}

	if( status.isEmpty() )
	{
		showWarning( tr("Error parsing server access certificate result!") );
		return false;
	}

	switch( status.toInt() )
	{
	case 1: //need to order cert manually from SK web
		QDesktopServices::openUrl( QUrl( tr("http://www.sk.ee/toend/") ) );
		return false;
	case 2: //got error, show message from MessageToDisplay element
		showWarning( tr("Error downloading server access certificate!\n%1").arg( message ) );
		return false;
	default: break; //ok
	}

	if( cert.isEmpty() )
	{
		showWarning( tr("Error reading server access certificate - empty content!") );
		return false;
	}

#ifdef Q_OS_MAC
	QByteArray data = QByteArray::fromBase64( cert.toUtf8() );
	CFDataRef pkcs12data = CFDataCreate( 0, (const UInt8*)data.constData(), data.size() );
	CFStringRef password = CFStringCreateWithCharacters( 0,
		reinterpret_cast<const UniChar *>(pass.unicode()), pass.length() );

	SecExternalFormat format = kSecFormatPKCS12;
	SecExternalItemType type = kSecItemTypeAggregate;

	SecKeyImportExportParameters params;
	memset( &params, 0, sizeof(params) );
	params.version = SEC_KEY_IMPORT_EXPORT_PARAMS_VERSION;
	params.flags = kSecKeyImportOnlyOne;
	params.keyAttributes = CSSM_KEYATTR_PERMANENT|CSSM_KEYATTR_EXTRACTABLE;
	params.passphrase = password;

	SecKeychainRef keychain;
	SecKeychainCopyDefault( &keychain );
	CFArrayRef items = 0;
	OSStatus err = SecKeychainItemImport( pkcs12data, 0, &format, &type, 0, &params, keychain, &items );
	CFRelease( password );

	if( err != errSecSuccess )
	{
		showWarning( tr("Failed to save server access certificate file to KeyChain!") );
		return false;
	}

	SecIdentityRef identity = 0;
	for( CFIndex i = 0; i < CFArrayGetCount( items ); ++i )
	{
		CFTypeRef item = CFTypeRef(CFArrayGetValueAtIndex( items, i ));
		if( CFGetTypeID( item ) == SecIdentityGetTypeID() )
			identity = SecIdentityRef(item);
	}

	err = SecIdentitySetPreference( identity, CFSTR("ocsp.sk.ee"), 0 );
	CFRelease( items );
	if( err != errSecSuccess )
	{
		showWarning( tr("Failed to save server access certificate file to KeyChain!") );
		return false;
	}
#else
	QString path = QDesktopServices::storageLocation( QDesktopServices::DataLocation );
	if ( !QDir( path ).exists() )
		QDir().mkpath( path );

	QFile f( QString( "%1/%2.p12" ).arg( path,
		SslCertificate( qApp->signer()->token().cert() ).subjectInfo( "serialNumber" ) ) );
	if ( !f.open( QIODevice::WriteOnly|QIODevice::Truncate ) )
	{
		showWarning( tr("Failed to save server access certificate file to %1!\n%2")
			.arg( f.fileName() )
			.arg( f.errorString() ) );
		return false;
	}
	f.write( QByteArray::fromBase64( cert.toUtf8() ) );
	f.close();

	Application::setConfValue( Application::PKCS12Cert, m_cert = QDir::toNativeSeparators( f.fileName() ) );
	Application::setConfValue( Application::PKCS12Pass, m_pass = pass );
#endif

	setIcon( Information );
	setText( tr("Server access certificate has been installed") );
	setStandardButtons( Cancel );
	setDefaultButton( addButton( tr("Continue signing"), AcceptRole ) );
	return exec() == AcceptRole;
}

void AccessCert::showWarning( const QString &msg )
{
	setIcon( Warning );
	setText( msg );
	setStandardButtons( Ok );
	exec();
}

bool AccessCert::showWarning2( const QString &msg )
{
	setIcon( Warning );
	setText( msg );
	setStandardButtons( Yes | No );
	setDefaultButton( Yes );
	exec();
	return standardButton(clickedButton()) == No;
}

bool AccessCert::validate()
{
	if( Application::confValue( Application::PKCS12Disable, false ).toBool() )
		return true;
#ifdef Q_OS_MAC
	SecIdentityRef identity = 0;
	OSStatus err = SecIdentityCopyPreference(CFSTR("ocsp.sk.ee"), 0, 0, &identity);
	if( !identity )
		return false;

	SecCertificateRef certref = 0;
	err = SecIdentityCopyCertificate(identity, &certref);
	CFRelease(identity);
	if( !certref )
		return false;

	CFDataRef certdata = SecCertificateCopyData(certref);
	CFRelease(certref);
	if( !certdata )
		return false;

	QSslCertificate cert(
		QByteArray( (const char*)CFDataGetBytePtr(certdata), CFDataGetLength(certdata) ), QSsl::Der );
	CFRelease(certdata);

	if( !cert.isValid() &&
		showWarning2( tr("Server access certificate is not valid!\nStart downloading?") ) )
		return true;
	if( cert.expiryDate() < QDateTime::currentDateTime().addDays( 8 ) &&
		!showWarning2( tr("Server access certificate is about to expire!\nStart downloading?") ) )
		return false;
	return true;
#else
	m_cert = Application::confValue( Application::PKCS12Cert ).toString();
	m_pass = Application::confValue( Application::PKCS12Pass ).toString();

	QFile f( m_cert );
	if( !f.exists() )
	{
		if( showWarning2( tr("Did not find any server access certificate!\nStart downloading?") ) )
		{
			Application::setConfValue( Application::PKCS12Cert, QVariant() );
			Application::setConfValue( Application::PKCS12Pass, QVariant() );
			return true;
		}
	}
	else if( !f.open( QIODevice::ReadOnly ) )
	{
		if( showWarning2( tr("Failed to read server access certificate!\nStart downloading?") ) )
		{
			Application::setConfValue( Application::PKCS12Cert, QVariant() );
			Application::setConfValue( Application::PKCS12Pass, QVariant() );
			return true;
		}
	}
	else
	{
		PKCS12Certificate p12Cert( &f, m_pass );

		if( p12Cert.error() == PKCS12Certificate::InvalidPasswordError )
		{
			if( showWarning2( tr("Server access certificate password is not valid!\nStart downloading?") ) )
			{
				Application::setConfValue( Application::PKCS12Cert, QVariant() );
				Application::setConfValue( Application::PKCS12Pass, QVariant() );
				return true;
			}
		}
		else if( !p12Cert.certificate().isValid() )
		{
			if( showWarning2( tr("Server access certificate is not valid!\nStart downloading?") ) )
			{
				Application::setConfValue( Application::PKCS12Cert, QVariant() );
				Application::setConfValue( Application::PKCS12Pass, QVariant() );
				return true;
			}
		}
		else if( p12Cert.certificate().expiryDate() < QDateTime::currentDateTime().addDays( 8 ) &&
			!showWarning2( tr("Server access certificate is about to expire!\nStart downloading?") ) )
			return false;
		else
			return true;
	}
	return false;
#endif
}
