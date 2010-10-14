/*
 * QDigiDocClient
 *
 * Copyright (C) 2009,2010 Jargo Kõster <jargo@innovaatik.ee>
 * Copyright (C) 2009,2010 Raul Metsma <raul@innovaatik.ee>
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

#include "DigiDoc.h"

#include "Application.h"
#include "QMobileSigner.h"
#include "QSigner.h"

#include <common/SslCertificate.h>
#include <common/TokenData.h>

#include <digidocpp/DDoc.h>
#include <digidocpp/Document.h>
#include <digidocpp/SignatureTM.h>
#include <digidocpp/WDoc.h>
#include <digidocpp/crypto/cert/X509Cert.h>
#include <digidocpp/io/ZipSerialize.h>

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QSettings>

#include <stdexcept>


using namespace digidoc;

static std::string to( const QString &str ) { return std::string( str.toUtf8().constData() ); }
static QString from( const std::string &str ) { return QString::fromUtf8( str.c_str() ); }

DigiDocSignature::DigiDocSignature( const digidoc::Signature *signature, DigiDoc *parent )
:	s(signature)
,	m_parent(parent)
{}

QSslCertificate DigiDocSignature::cert() const
{
	QSslCertificate c;
	try
	{
		X509 *x509 = s->getSigningCertificate().getX509();
		c = SslCertificate::fromX509( Qt::HANDLE(x509) );
		X509_free( x509 );
	}
	catch( const Exception & ) {}
	return c;
}

QDateTime DigiDocSignature::dateTime() const
{
	QString dateTime;
	switch( type() )
	{
	case TMType:
		dateTime = from( static_cast<const SignatureTM*>(s)->getProducedAt() );
		break;
	case DDocType:
		dateTime = from( static_cast<const SignatureDDOC*>(s)->getProducedAt() );
		break;
	default: break;
	}

	if( dateTime.isEmpty() )
		dateTime = from( s->getSigningTime() );

	if( dateTime.isEmpty() )
		return QDateTime();

	QDateTime date = QDateTime::fromString( dateTime, "yyyy-MM-dd'T'hh:mm:ss'Z'" );
	date.setTimeSpec( Qt::UTC );
	return date.toLocalTime();
}

QString DigiDocSignature::digestMethod() const
{
	try
	{
		std::vector<unsigned char> data;
		std::string method;
		switch( type() )
		{
		case TMType:
			static_cast<const SignatureTM*>(s)->getRevocationOCSPRef( data, method );
			break;
		case DDocType:
			static_cast<const SignatureDDOC*>(s)->getRevocationOCSPRef( data, method );
			break;
		default: return QString();
		}
		return from( method );
	}
	catch( const Exception & ) {}
	return QString();
}

QByteArray DigiDocSignature::digestValue() const
{
	try
	{
		std::vector<unsigned char> data;
		std::string method;
		switch( type() )
		{
		case TMType:
			static_cast<const SignatureTM*>(s)->getRevocationOCSPRef( data, method );
			break;
		case DDocType:
			static_cast<const SignatureDDOC*>(s)->getRevocationOCSPRef( data, method );
			break;
		default: return QByteArray();
		}
		if( data.size() > 0 )
			return QByteArray( (const char*)&data[0], data.size() );
	}
	catch( const Exception & ) {}
	return QByteArray();
}

QString DigiDocSignature::lastError() const { return m_lastError; }

QString DigiDocSignature::location() const
{
	QStringList l = locations();
	l.removeAll( "" );
	return l.join( ", " );
}

QStringList DigiDocSignature::locations() const
{
	const SignatureProductionPlace p = s->getProductionPlace();
	return QStringList()
		<< from( p.city ).trimmed()
		<< from( p.stateOrProvince ).trimmed()
		<< from( p.postalCode ).trimmed()
		<< from( p.countryName ).trimmed();
}

QString DigiDocSignature::mediaType() const
{ return from( s->getMediaType() ); }

QSslCertificate DigiDocSignature::ocspCert() const
{
	X509 *x = 0;
	try
	{
		switch( type() )
		{
		case TMType:
			x = static_cast<const SignatureTM*>(s)->getOCSPCertificate().getX509();
			break;
		case DDocType:
			x = static_cast<const SignatureDDOC*>(s)->getOCSPCertificate().getX509();
			break;
		default: break;
		}
	}
	catch( const Exception & ) {}

	QSslCertificate c = SslCertificate::fromX509( Qt::HANDLE(x) );
	X509_free( x );
	return c;
}

DigiDoc* DigiDocSignature::parent() const { return m_parent; }

int DigiDocSignature::parseException( const digidoc::Exception &e )
{
	Q_FOREACH( const Exception &c, e.getCauses() )
	{
		int code = parseException( c );
		if( code != Exception::NoException )
			return code;
	}
	return e.code();
}

void DigiDocSignature::parseExceptionStrings( const digidoc::Exception &e, QStringList &causes )
{
	causes << from( e.getMsg() );
	Q_FOREACH( const Exception &c, e.getCauses() )
		parseExceptionStrings( c, causes );
}

QString DigiDocSignature::role() const
{
	QStringList r = roles();
	r.removeAll( "" );
	return r.join( ", " );
}

QStringList DigiDocSignature::roles() const
{
	QStringList list;
	const SignerRole::TRoles roles = s->getSignerRole().claimedRoles;
	SignerRole::TRoles::const_iterator i = roles.begin();
	for( ; i != roles.end(); ++i )
		list << QString::fromUtf8( i->c_str() ).trimmed();
	return list;
}

void DigiDocSignature::setLastError( const Exception &e )
{
	QStringList causes;
	parseExceptionStrings( e, causes );
	m_lastError = causes.join( "<br />" );
}

DigiDocSignature::SignatureType DigiDocSignature::type() const
{
	const std::string ver = s->getMediaType();
	if( ver.compare( "signature/bdoc-1.0/TM" ) == 0 )
		return TMType;
	if( ver.compare( "signature/bdoc-1.0/TS" ) == 0 )
		return TSType;
	if( ver.compare( "signature/bdoc-1.0/BES" ) == 0 )
		return BESType;
	if( ver.compare( 0, 11, "DIGIDOC-XML" ) == 0 ||
		ver.compare( 0, 6, "SK-XML" ) == 0 )
		return DDocType;
	return UnknownType;
}

DigiDocSignature::SignatureStatus DigiDocSignature::validate()
{
	try
	{
		s->validateOffline();
		if( type() == BESType )
		{
			switch( s->validateOnline() )
			{
			case OCSP::GOOD: return Valid;
			case OCSP::REVOKED: return Invalid;
			case OCSP::UNKNOWN: return Unknown;
			}
		}
		else
			return Valid;
	}
	catch( const Exception &e )
	{
		setLastError( e );
		switch( parseException( e ) )
		{
		case Exception::CertificateIssuerMissing:
		case Exception::CertificateUnknown:
		case Exception::OCSPResponderMissing:
		case Exception::OCSPCertMissing: return Unknown;
		default: break;
		}
	}
	return Invalid;
}



DigiDoc::DigiDoc( QObject *parent )
:	QObject( parent )
,	b(0)
{
	connect( this, SIGNAL(error(QString)), qApp, SLOT(showWarning(QString)) );
}

DigiDoc::~DigiDoc() { clear(); }

void DigiDoc::addFile( const QString &file )
{
	if( !checkDoc( b->signatureCount() > 0, tr("Cannot add files to signed container") ) )
		return;
	try { b->addDocument( Document( to(file), "file" ) ); }
	catch( const Exception &e ) { setLastError( e ); }
}

bool DigiDoc::checkDoc( bool status, const QString &msg )
{
	if( isNull() )
		Q_EMIT error( tr("Container is not open") );
	else if( status )
		Q_EMIT error( msg );
	return !isNull() && !status;
}

void DigiDoc::clear()
{
	delete b;
	b = 0;
	m_fileName.clear();
}

void DigiDoc::create( const QString &file )
{
	clear();
	QString type = QFileInfo( file ).suffix().toLower();
	if( type == "bdoc" )
		b = new WDoc( WDoc::BDocType );
	else if( type == "ddoc" )
		b = new WDoc( WDoc::DDocType );
	m_fileName = file;
}

QList<Document> DigiDoc::documents()
{
	QList<Document> list;
	if( !checkDoc() )
		return list;
	try
	{
		unsigned int count = b->documentCount();
		for( unsigned int i = 0; i < count; ++i )
			list << b->getDocument( i );
	}
	catch( const Exception &e ) { setLastError( e ); }

	return list;
}

QString DigiDoc::fileName() const { return m_fileName; }
bool DigiDoc::isNull() const { return b == 0; }

bool DigiDoc::open( const QString &file )
{
	clear();
	m_fileName = file;
	try
	{
		b = new WDoc( to(file) );
		if( b->documentType() == WDoc::DDocType && b->signatureCount() )
		{
			std::string ver = b->getSignature( 0 )->getMediaType();
			if( ver.compare( 0, 6, "SK-XML" ) == 0 ||
				ver.compare( 0, 15, "DIGIDOC-XML/1.1" ) == 0 ||
				ver.compare( 0, 15, "DIGIDOC-XML/1.2" ) == 0 )
			{
				Q_EMIT error( tr(
					"The current file is a DigiDoc container not supported officially any longer.\n"
					"We do not recommend you to add signature to this document.\n"
					"There is an option to re-sign this document in a new container.") );
			}
		}
		return true;
	}
	catch( const Exception &e )
	{
		QStringList causes;
		Exception::ExceptionCode code = Exception::NoException;
		parseException( e, causes, code );
		Q_EMIT error( tr("An error occurred while opening the document.<br />%1").arg( causes.join("\n") ) );
	}
	return false;
}

bool DigiDoc::parseException( const Exception &e, QStringList &causes, Exception::ExceptionCode &code )
{
	switch( e.code() )
	{
	case Exception::CertificateRevoked:
	case Exception::CertificateUnknown:
	case Exception::OCSPTimeSlot:
	case Exception::OCSPRequestUnauthorized:
	case Exception::PINCanceled:
	case Exception::PINFailed:
	case Exception::PINIncorrect:
	case Exception::PINLocked:
		code = e.code(); return false;
	default:
		causes << from( e.getMsg() );
		break;
	}
	Q_FOREACH( const Exception &c, e.getCauses() )
		if( !parseException( c, causes, code ) )
			return false;
	return true;
}

void DigiDoc::removeDocument( unsigned int num )
{
	if( !checkDoc( num >= b->documentCount(), tr("Missing document") ) )
		return;
	try { b->removeDocument( num ); }
	catch( const Exception &e ) { setLastError( e ); }
}

void DigiDoc::removeSignature( unsigned int num )
{
	if( !checkDoc( num >= b->signatureCount(), tr("Missing signature") ) )
		return;
	try { b->removeSignature( num ); }
	catch( const Exception &e ) { setLastError( e ); }
}

void DigiDoc::save()
{
	/*if( !checkDoc() );
		return; */
	try
	{
		std::auto_ptr<ISerialize> s(new ZipSerialize( to(m_fileName) ));
		b->saveTo( s );
	}
	catch( const Exception &e ) { setLastError( e ); }
}

void DigiDoc::setLastError( const Exception &e )
{
	QStringList causes;
	Exception::ExceptionCode code = Exception::NoException;
	parseException( e, causes, code );
	switch( code )
	{
	case Exception::CertificateRevoked:
		Q_EMIT error( tr("Certificate status revoked") ); break;
	case Exception::CertificateUnknown:
		Q_EMIT error( tr("Certificate status unknown") ); break;
	case Exception::OCSPTimeSlot:
		Q_EMIT error( tr("Check your computer time") ); break;
	case Exception::OCSPRequestUnauthorized:
		Q_EMIT error( tr("Server access certificate is required")); break;
	case Exception::PINCanceled:
		break;
	case Exception::PINFailed:
		Q_EMIT error( tr("PIN Login failed") ); break;
	case Exception::PINIncorrect:
		Q_EMIT error( tr("PIN Incorrect") ); break;
	case Exception::PINLocked:
		Q_EMIT error( tr("PIN Locked") ); break;
	default:
		Q_EMIT error( causes.join( "\n" ) ); break;
	}
}

bool DigiDoc::sign( const QString &city, const QString &state, const QString &zip,
	const QString &country, const QString &role, const QString &role2 )
{
	if( !checkDoc( b->documentCount() == 0, tr("Cannot add signature to empty container") ) )
		return false;

	bool result = false;
	try
	{
		qApp->signer()->setSignatureProductionPlace(
			SignatureProductionPlace( to(city), to(state), to(zip), to(country) ) );
		SignerRole sRole( to(role) );
		if ( !role2.isEmpty() )
			sRole.claimedRoles.push_back( to(role2) );
		qApp->signer()->setSignerRole( sRole );
		b->sign( qApp->signer(), Signature::TM );
		result = true;
	}
	catch( const Exception &e )
	{
		QStringList causes;
		Exception::ExceptionCode code = Exception::NoException;
		parseException( e, causes, code );
		if( code == Exception::PINIncorrect )
		{
			Q_EMIT error( tr("PIN Incorrect") );
			if( !(qApp->tokenData().flags() & TokenData::PinLocked) )
				return sign( city, state, zip, country, role, role2 );
		}
		else
			setLastError( e );
	}
	return result;
}

bool DigiDoc::signMobile( const QString &fName )
{
	if( !checkDoc( b->documentCount() == 0, tr("Cannot add signature to empty container") ) )
		return false;

	bool result = false;
	try
	{
		b->sign( new digidoc::QMobileSigner( fName ), Signature::MOBILE );
		result = true;
	}
	catch( const Exception &e ) { setLastError( e ); }
	return result;
}

QList<DigiDocSignature> DigiDoc::signatures()
{
	QList<DigiDocSignature> list;
	if( !checkDoc() )
		return list;
	try
	{
		unsigned int count = b->signatureCount();
		for( unsigned int i = 0; i < count; ++i )
			list << DigiDocSignature( b->getSignature( i ), this );
	}
	catch( const Exception &e ) { setLastError( e ); }
	return list;
}

WDoc::DocumentType DigiDoc::documentType()
{ return checkDoc() ? b->documentType() : WDoc::BDocType; }

QByteArray DigiDoc::getFileDigest( unsigned int i )
{
	QByteArray result;
	if( !checkDoc() )
		return result;
	result.resize(20);
	b->getFileDigest( i, (unsigned char*)result.data() );
	return result;
}
