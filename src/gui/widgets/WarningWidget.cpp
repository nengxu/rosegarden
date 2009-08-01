/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
    Copyright 2000-2009 the Rosegarden development team.
 
    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.
 
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "WarningWidget.h"

#include "gui/general/IconLoader.h"

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>

#include <iostream>
namespace Rosegarden
{

WarningWidget::WarningWidget() :
        QWidget()
{
    //setStyleSheet("{ background: red; border 0px transparent; padding 0;");
//    setStyleSheet("{ background: red;");
    setContentsMargins(0, 0, 0, 0);
    
//    setFixedHeight(30);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    QHBoxLayout *layout = new QHBoxLayout();
    setLayout(layout);

    m_midiIcon = new QLabel();
    layout->addWidget(m_midiIcon);
    m_midiIcon->setPixmap(IconLoader().loadPixmap("midi-ok")); 

    m_audioIcon = new QLabel();
    layout->addWidget(m_audioIcon);
    m_audioIcon->setPixmap(IconLoader().loadPixmap("audio-ok")); 

    m_warningIcon = new QLabel();
    layout->addWidget(m_warningIcon);
    m_warningIcon->setPixmap(IconLoader().loadPixmap("warning"));
    m_warningIcon->hide();

    // It might be nice to put something in here to either display the audio
    // file path (in a tooltip, perhaps) or even offer a shortcut for changing
    // this well buried but important piece of information.  Come back to that
    // thought later.
    std::cerr << "WARNING WIDGET HEIGHT " << this->height() << std::endl;

}

void
WarningWidget::setMidiWarning(const bool status)
{
    m_midiIcon->setPixmap(IconLoader().loadPixmap("midi-broken"));
    m_warningIcon->show();
}

void
WarningWidget::setAudioWarning(const bool status)
{
    m_audioIcon->setPixmap(IconLoader().loadPixmap("audio-broken"));
    m_warningIcon->show();
}

void
WarningWidget::setTimerWarning(const bool status)
{
    m_midiIcon->setPixmap(IconLoader().loadPixmap("midi-half-broken"));
    m_audioIcon->setPixmap(IconLoader().loadPixmap("audio-broken"));
    m_warningIcon->show();
}

WarningWidget::~WarningWidget()
{
}


}
#include "WarningWidget.moc"
