/*****************************************************************************
 * Epg.cpp : Epg Viewer dialog
 ****************************************************************************
 * Copyright © 2010 VideoLAN and AUTHORS
 *
 * Authors:    Jean-Baptiste Kempf <jb@videolan.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "dialogs/epg.hpp"

#include "components/epg/EPGWidget.hpp"

#include <QVBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QTimer>

#include "qt4.hpp"
#include "input_manager.hpp"

EpgDialog::EpgDialog( intf_thread_t *_p_intf ): QVLCFrame( _p_intf )
{
    setWindowTitle( qtr( "Program Guide" ) );

    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setMargin( 0 );
    epg = new EPGWidget( this );

    QGroupBox *descBox = new QGroupBox( qtr( "Description" ), this );

    QVBoxLayout *boxLayout = new QVBoxLayout( descBox );

    description = new QTextEdit( this );
    description->setReadOnly( true );
    description->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    description->setAutoFillBackground( true );
    description->setAlignment( Qt::AlignLeft | Qt::AlignTop );
    description->setFixedHeight( 100 );

    QPalette palette;
    palette.setBrush(QPalette::Active, QPalette::Window, palette.brush( QPalette::Base ) );
    description->setPalette( palette );

    title = new QLabel( qtr( "Title" ), this );
    title->setWordWrap( true );

    boxLayout->addWidget( title );
    boxLayout->addWidget( description );

    layout->addWidget( epg, 10 );
    layout->addWidget( descBox );

    CONNECT( epg, itemSelectionChanged( EPGEvent *), this, showEvent( EPGEvent *) );
    CONNECT( THEMIM->getIM(), epgChanged(), this, updateInfos() );
    CONNECT( THEMIM, inputChanged( input_thread_t * ), this, updateInfos() );

    QDialogButtonBox *buttonsBox = new QDialogButtonBox( this );

#if 0
    QPushButton *update = new QPushButton( qtr( "Update" ) ); // Temporary to test
    buttonsBox->addButton( update, QDialogButtonBox::ActionRole );
    BUTTONACT( update, updateInfos() );
#endif

    buttonsBox->addButton( new QPushButton( qtr( "&Close" ) ),
                           QDialogButtonBox::RejectRole );
    boxLayout->addWidget( buttonsBox );
    CONNECT( buttonsBox, rejected(), this, close() );

    timer = new QTimer( this );
    timer->setSingleShot( true );
    timer->setInterval( 1000 * 60 );
    CONNECT( timer, timeout(), this, updateInfos() );

    updateInfos();
    readSettings( "EPGDialog", QSize( 650, 450 ) );
}

EpgDialog::~EpgDialog()
{
    writeSettings( "EPGDialog" );
}

void EpgDialog::showEvent( EPGEvent *event )
{
    if( !event ) return;

    QString titleDescription, textDescription;
    if( event->description.isEmpty() )
        textDescription = event->shortDescription;
    else
    {
        textDescription = event->description;
        if( !event->shortDescription.isEmpty() )
            titleDescription = " - " + event->shortDescription;
    }

    QDateTime end = event->start.addSecs( event->duration );
    title->setText( event->start.toString( "hh:mm" ) + " - "
                    + end.toString( "hh:mm" ) + " : "
                    + event->name + titleDescription );

    description->setText( textDescription );
}

void EpgDialog::updateInfos()
{
    if( !THEMIM->getInput() ) return;
    timer->stop();
    input_item_t *p_input_item = input_GetItem( THEMIM->getInput() );
    vlc_mutex_lock(  & p_input_item->lock );
    int i_nbitems = p_input_item->i_epg;
    if ( i_nbitems > 0 ) msg_Dbg( p_intf, "Found %i EPG items", i_nbitems );
    epg->updateEPG( p_input_item->pp_epg,
                    p_input_item->i_epg,
                    p_input_item->i_type);
    vlc_mutex_unlock( & p_input_item->lock );
    if ( isVisible() ) timer->start();
}
