/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.

    This program is Copyright 2000-2007
        Guillaume Laurent   <glaurent@telegraph-road.org>,
        Chris Cannam        <cannam@all-day-breakfast.com>,
        Richard Bown        <richard.bown@ferventsoftware.com>

    The moral rights of Guillaume Laurent, Chris Cannam, and Richard
    Bown to claim authorship of this work have been asserted.

    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/



#ifndef _RG_FINGERINGBOX2_H_
#define _RG_FINGERINGBOX2_H_

#include <qframe.h>

#include "gui/editors/guitar/NoteSymbols.h"
#include "Fingering2.h"

namespace Rosegarden
{

class Fingering2;

class FingeringBox2 : public QFrame
{
    static const unsigned int IMG_WIDTH  = 200;
    static const unsigned int IMG_HEIGHT = 200;
    
public:
	FingeringBox2(unsigned int nbFrets, unsigned int nbStrings, bool editable, QWidget *parent, const char* name = 0);
    FingeringBox2(bool editable, QWidget *parent, const char* name = 0);

    void setStartFret(unsigned int f) { m_startFret = f; }
    unsigned int getStartFret() const { return m_startFret; }
    
    void setFingering(const Fingering2&);
    const Fingering2& getFingering() { return m_fingering; }
    
    const Guitar::NoteSymbols& getNoteSymbols() const { return m_noteSymbols; }
    
protected:
    void init();

    virtual void drawContents(QPainter*);

    virtual void mousePressEvent(QMouseEvent*);
    virtual void mouseReleaseEvent(QMouseEvent*);
    virtual void mouseMoveEvent(QMouseEvent*);

    void processMouseRelease( unsigned int release_string_num, unsigned int release_fret_num);

    typedef std::pair<bool, unsigned int> PositionPair;

    unsigned int getStringNumber(const QPoint&);

    unsigned int getFretNumber(const QPoint&);

    //! Maximum number of frets displayed by FingeringBox
    unsigned int m_nbFretsDisplayed;

    unsigned int m_startFret;
    
    unsigned int m_nbStrings;
    
    unsigned int m_transientFretNb;
    unsigned int m_transientStringNb;
    
    //! Present mode
    bool m_editable;

    //! Handle to the present fingering
    Fingering2 m_fingering;

    //! String number where a mouse press event was located
    unsigned int m_press_string_num;

    //! Fret number where a mouse press event was located
    unsigned int m_press_fret_num;

    Guitar::NoteSymbols m_noteSymbols;

};

}

#endif /*_RG_FINGERINGBOX2_H_*/
