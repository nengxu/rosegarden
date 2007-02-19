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


#include "FingeringBox2.h"
#include "Fingering2.h"

#include "misc/Debug.h"

namespace Rosegarden
{

FingeringBox2::FingeringBox2(unsigned int nbFrets, unsigned int nbStrings, bool editable, QWidget *parent, const char* name)
    : QFrame(parent, name),
    m_nbFretsDisplayed(nbFrets),
    m_startFret(0),
    m_nbStrings(nbStrings),
    m_transientFretNb(0),
    m_transientStringNb(0),
    m_editable(editable),
    m_noteSymbols(m_nbStrings, m_nbFretsDisplayed)
{
    init();    
}

FingeringBox2::FingeringBox2(bool editable, QWidget *parent, const char* name)
    : QFrame(parent, name),
    m_nbFretsDisplayed(4),
    m_startFret(0),
    m_nbStrings(Fingering2::DEFAULT_NB_STRINGS),
    m_editable(editable),
    m_noteSymbols(m_nbStrings, m_nbFretsDisplayed)
{
    init();
}

void
FingeringBox2::init()
{
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setFixedSize(IMG_WIDTH, IMG_HEIGHT);
    setBackgroundMode(PaletteBase);
    
}

void
FingeringBox2::drawContents(QPainter* p)
{
    NOTATION_DEBUG << "FingeringBox2::drawContents()" << endl;
    
    // For all strings on guitar
    //   check state of string
    //     If pressed display note
    //     Else display muted or open symbol
    // For all bars
    //   display bar
    // Horizontal separator line

    m_noteSymbols.drawFretNumber(p, m_startFret);
    m_noteSymbols.drawFrets(p);
    m_noteSymbols.drawStrings(p);

    unsigned int stringNb = 0;
    
    for (Fingering2::const_iterator pos = m_fingering.begin();
         pos != m_fingering.end();
         ++pos, ++stringNb) {
                
        switch (*pos) {
        case Fingering2::OPEN:
                NOTATION_DEBUG << "Fingering::drawContents - drawing Open symbol on string " << stringNb << endl;
                m_noteSymbols.drawOpenSymbol(p, stringNb);
                break;

        case Fingering2::MUTED:
                NOTATION_DEBUG << "Fingering::drawContents - drawing Mute symbol on string" << stringNb << endl;
                m_noteSymbols.drawMuteSymbol(p, stringNb);
                break;

        default:
                NOTATION_DEBUG << "Fingering::drawContents - drawing note symbol at " << *pos << " on string " << stringNb << endl;
                m_noteSymbols.drawNoteSymbol(p, stringNb, *pos - (m_startFret - 1), false);
                break;
        }
    }

    // TODO: barres ?
    
    if (m_transientFretNb > 0 && m_transientFretNb <= m_nbFretsDisplayed &&
        m_transientStringNb > 0 && m_transientStringNb <= m_nbStrings) {
        m_noteSymbols.drawNoteSymbol(p, m_nbStrings - m_transientStringNb, m_transientFretNb - m_startFret, true);
    }
    
}

void
FingeringBox2::setFingering(const Fingering2& f) {
    m_fingering = f;
    m_startFret = m_fingering.getStartFret();
    update();
}

unsigned int
FingeringBox2::getStringNumber(const QPoint& pos)
{
    PositionPair result = m_noteSymbols.getStringNumber(maximumHeight(),
                                                        pos.x(),
                                                        m_nbStrings);
    unsigned int stringNum = 0;

    if(result.first){
        stringNum = m_nbStrings - result.second;
    }

    return stringNum;
}

unsigned int
FingeringBox2::getFretNumber(const QPoint& pos)
{
    unsigned int fretNum = 0;

    if(pos.y() > m_noteSymbols.getTopBorder(maximumHeight())) {
        // If fret position is below the top line of the fretboard image.
        PositionPair result = m_noteSymbols.getFretNumber(maximumWidth(),
                                                          pos.y(),
                                                          m_nbFretsDisplayed);

        if(result.first) {
            fretNum = result.second + m_startFret;
        }
    }

    return fretNum;
}

void
FingeringBox2::mousePressEvent(QMouseEvent *event)
{
    if (!m_editable)
        return;
        
    if((event->button() == LeftButton) && m_editable) {

        // Find string position
        m_press_string_num = getStringNumber(event->pos());

        // Find fret position
        m_press_fret_num = getFretNumber(event->pos());
    }
}

void
FingeringBox2::mouseReleaseEvent(QMouseEvent *event)
{
    if(!m_editable)
        return ;

    unsigned int release_string_num = getStringNumber(event->pos());
    unsigned int release_fret_num = getFretNumber(event->pos());

    processMouseRelease(release_string_num, release_fret_num);
}

void
FingeringBox2::processMouseRelease(unsigned int release_string_num,
                                        unsigned int release_fret_num)
{
    if(m_press_fret_num == release_fret_num){
        // If press string & fret pos == release string & fret position, display chord
        if(m_press_string_num == release_string_num){
            // QUESTION: Move check for whether a note pressed is at position 0
            // here or not?
            if((m_press_string_num > 0)&&
               (m_press_string_num <= m_nbStrings)&&
               (m_press_fret_num < (m_startFret + m_nbFretsDisplayed))) {
                /**
                IF m_press_fret_num == 0
                    Get fretStatus
                        status | new status
                        MUTED  | OPEN
                        OPEN   | MUTED
                ELSE m_press_fret_num > 0
                    status  | new status
                    MUTED   | PRESSED
                    OPEN    | PRESSED
                        PRESSED | PRESSED
                */
                unsigned int aVal = m_press_fret_num;
                
                if(m_press_fret_num == 0) {
                    
                    int stringStatus = m_fingering.getStringStatus(m_press_string_num);

                    if (stringStatus == Fingering2::MUTED)
                        aVal = Fingering2::OPEN;
                    else if (stringStatus >= Fingering2::OPEN)
                        aVal = Fingering2::MUTED;

                }
                
                m_fingering.setStringStatus(m_press_string_num, m_press_fret_num);
                
                update();
            }
        }
        // else if press fret pos == release fret pos & press string pos != release string pos, display bar
        else {
            if(((m_press_string_num > 0)&&(release_string_num > 0)) &&
                   (( m_press_string_num <= m_nbStrings)&&
                     (release_string_num <= m_nbStrings)) &&
                   (( m_press_fret_num <(m_startFret + m_nbFretsDisplayed)) &&
                     (release_fret_num <(m_startFret + m_nbFretsDisplayed)))) {

                // TODO deal with barre later on

            }
        }
    }
}


void
FingeringBox2::mouseMoveEvent( QMouseEvent *event )
{
    if (!m_editable)
        return;
        
    unsigned int transientStringNb = getStringNumber(event->pos());
    unsigned int transientFretNb   = getFretNumber(event->pos());
        
    if (transientStringNb != m_transientStringNb ||
        transientFretNb != m_transientFretNb) {

        QRect r1 = m_noteSymbols.getTransientNoteSymbolRect(size(),
                                                            m_nbStrings - m_transientStringNb,
                                                            m_transientFretNb - m_startFret);
        m_transientStringNb = transientStringNb;
        m_transientFretNb   = transientFretNb;
        QRect r2 = m_noteSymbols.getTransientNoteSymbolRect(size(),
                                                            m_nbStrings - m_transientStringNb,
                                                            m_transientFretNb - m_startFret);
    
//    RG_DEBUG << "Fingering::updateTransientPos r1 = " << r1 << " - r2 = " << r2 << endl;
     
        QRect updateRect = r1 | r2;
        update(updateRect);
            
    }    
    
}

}
