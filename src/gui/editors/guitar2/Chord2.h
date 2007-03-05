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

#ifndef _RG_CHORD2_H_
#define _RG_CHORD2_H_

#include "Fingering2.h"
#include "base/Event.h"
#include "misc/Debug.h"

#include <vector>
#include <qstring.h>

namespace Rosegarden
{

class Event;

class Chord2
{
    friend bool operator<(const Chord2&, const Chord2&);
    
public:
    static const std::string EventType;
    static const short EventSubOrdering;
    static const PropertyName RootPropertyName;
    static const PropertyName ExtPropertyName;
    static const PropertyName FingeringPropertyName;

	Chord2();
    Chord2(const QString& root, const QString& ext = QString::null);
    Chord2(const Event&);

    Event* getAsEvent(timeT absoluteTime) const;
        
    bool isEmpty() const   { return m_root.isEmpty(); }
    bool operator!() const { return !m_root; }
    
    QString getRoot() const { return m_root; }
    void setRoot(QString r) { m_root = r; } 

    QString getExt() const { return m_ext; }
    void setExt(QString r) { m_ext = r; } 
    
    bool hasAltBass() const { return m_ext.contains('/'); } 

    Fingering2 getFingering(unsigned int idx) const { return m_fingerings[idx]; }
    Fingering2 getSelectedFingering() const { return m_selectedFingeringIdx >= 0 ? m_fingerings[m_selectedFingeringIdx] : Fingering2(); }
    void setFingering(unsigned int idx, Fingering2 f);
    void addFingering(Fingering2 f);
    void removeFingering(unsigned int idx);
    unsigned int getNbFingerings() const { return m_fingerings.size(); }

    int getSelectedFingeringIdx() const { return m_selectedFingeringIdx; } 
    void setSelectedFingeringIdx(int i); 

    struct ChordCmp
    {
        bool operator()(const Chord2 &e1, const Chord2 &e2) const {
            return e1 < e2;
        }
        bool operator()(const Chord2 *e1, const Chord2 *e2) const {
            return *e1 < *e2;
        }
    };
        
protected:

    QString m_root;
    QString m_ext;
    
    std::vector<Fingering2> m_fingerings;
    int m_selectedFingeringIdx;
};

bool operator<(const Chord2&, const Chord2&);    


}

#endif /*_RG_CHORD2_H_*/
