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

WarningWidget::WarningWidget(bool showMidiWarning,
                             bool showAudioWarning,
                             bool showtimerWarning
                            ) :
        QWidget()
{
    setStyleSheet("background: red");
    /*
//    setFixedHeight(30);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    QHBoxLayout *layout = new QHBoxLayout();
    setLayout(layout);

    QLabel *midiCheck = new QLabel();
    layout->addWidget(midiCheck);
    midiCheck->setPixmap(IconLoader().loadPixmap("midi-ok")); 

    QLabel *audioCheck = new QLabel();
    layout->addWidget(audioCheck);
    audioCheck->setPixmap(IconLoader().loadPixmap("audio-ok")); 

    QLabel *timerCheck = new QLabel();
    layout->addWidget(timerCheck);
    timerCheck->setPixmap(IconLoader().loadPixmap("timer-ok")); 

    QLabel *importerCheck = new QLabel();
    layout->addWidget(importerCheck);
    importerCheck->setPixmap(IconLoader().loadPixmap("midi-ok")); 

    // It might be nice to put something in here to either display the audio
    // file path (in a tooltip, perhaps) or even offer a shortcut for changing
    // this well buried but important piece of information.  Come back to that
    // thought later.
    std::cerr << "WARNING WIDGET HEIGHT " << this->height() << std::endl;*/

}

WarningWidget::~WarningWidget()
{
}


}
#include "WarningWidget.moc"
