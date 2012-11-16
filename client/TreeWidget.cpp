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

#include "TreeWidget.h"

#include "Application.h"
#include "DigiDoc.h"

#include <common/FileDialog.h>

#include <QtGui/QDesktopServices>
#include <QtGui/QHeaderView>
#include <QtGui/QKeyEvent>
#include <QtGui/QMessageBox>

TreeWidget::TreeWidget( QWidget *parent )
:	QTreeView( parent )
{}

void TreeWidget::clicked( const QModelIndex &index )
{
	switch( index.column() )
	{
	case DocumentModel::Save:
	{
		QString dest;
		while( true )
		{
			dest = FileDialog::getSaveFileName( qApp->activeWindow(),
				tr("Save file"), QString( "%1/%2" )
					.arg( QDesktopServices::storageLocation( QDesktopServices::DocumentsLocation ) )
					.arg( m->index( index.row(), 0 ).data().toString() ) );
			if( !dest.isEmpty() && !FileDialog::fileIsWritable( dest ) )
			{
				QMessageBox::warning( qApp->activeWindow(), tr("DigiDoc3 client"),
					tr( "You don't have sufficient privileges to write this file into folder %1" ).arg( dest ) );
			}
			else
				break;
		}
		QString src = m->index( index.row(), 0 ).data( Qt::UserRole ).toString();
		if( !dest.isEmpty() && !src.isEmpty() && dest != src )
		{
			if( QFile::exists( dest ) )
				QFile::remove( dest );
			QFile::copy( src, dest );
		}
		break;
	}
	case DocumentModel::Remove: model()->removeRow( index.row() ); break;
	default: break;
	}
}

void TreeWidget::keyPressEvent( QKeyEvent *e )
{
	QModelIndexList i = selectionModel()->selectedRows();
	if( hasFocus() && !i.isEmpty() && i[0].isValid() )
	{
		switch( e->key() )
		{
		case Qt::Key_Delete:
			model()->removeRow( i[0].row() );
			e->accept();
			break;
		case Qt::Key_Return:
			m->open( i[0] );
			e->accept();
			break;
		default: break;
		}
	}
	QTreeView::keyPressEvent( e );
}

void TreeWidget::setDocumentModel( DocumentModel *model )
{
	setModel( m = model );
	header()->setStretchLastSection( false );
	header()->setResizeMode( QHeaderView::ResizeToContents );
	header()->setResizeMode( DocumentModel::Name, QHeaderView::Stretch );
	setColumnHidden( DocumentModel::Mime, true );
	connect( this, SIGNAL(clicked(QModelIndex)), SLOT(clicked(QModelIndex)) );
	connect( this, SIGNAL(doubleClicked(QModelIndex)), m, SLOT(open(QModelIndex)) );
}
