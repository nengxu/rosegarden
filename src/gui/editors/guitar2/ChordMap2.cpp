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

#include "misc/Debug.h"
#include "ChordMap2.h"

namespace Rosegarden
{

ChordMap2::ChordMap2()
    : m_needSave(false)
{
}

void ChordMap2::insert(const Chord2& c)
{
    m_map.insert(c);
    m_needSave = true;
}


ChordMap2::chordarray
ChordMap2::getChords(const QString& root, const QString& ext) const
{
    chordarray res;
    
    Chord2 tmp(root, ext);
    
    for (chordset::const_iterator i = m_map.lower_bound(tmp); i != m_map.end(); ++i) {
        NOTATION_DEBUG << "ChordMap2::getChords : checking chord " << *i << endl;
        
        if (i->getRoot() != root)
            break;

        if (/* ext.isNull() || */ i->getExt() == ext) {
            NOTATION_DEBUG << "ChordMap2::getChords : adding chord " << *i << endl;
            res.push_back(*i);
        } else {
            break;
        }
    }
        
    return res;
}
    
QStringList
ChordMap2::getRootList() const
{
    QStringList rootNotes;
    QString currentRoot;
    
    for(chordset::const_iterator i = m_map.begin(); i != m_map.end(); ++i) {
        const Chord2& chord = *i;
        if (chord.getRoot() != currentRoot) {
            rootNotes.push_back(chord.getRoot());
            currentRoot = chord.getRoot();
        } 
    }

    return rootNotes;
}

QStringList
ChordMap2::getExtList(const QString& root) const
{
    QStringList extList;
    QString currentExt = "ZZ";
    
    Chord2 tmp(root);
    
    for(chordset::const_iterator i = m_map.lower_bound(tmp); i != m_map.end(); ++i) {
        const Chord2& chord = *i;
//        NOTATION_DEBUG << "ChordMap2::getExtList : chord = " << chord << endl;
         
        if (chord.getRoot() != root)
            break;
            
        if (chord.getExt() != currentExt) {
//            NOTATION_DEBUG << "ChordMap2::getExtList : adding ext " << chord.getExt() << " for root " << root << endl;
            extList.push_back(chord.getExt());
            currentExt = chord.getExt();
        }        
    }

    return extList;
}

void
ChordMap2::substitute(const Chord2& oldChord, const Chord2& newChord)
{
    remove(oldChord);
    insert(newChord);
}

void
ChordMap2::remove(const Chord2& c)
{
    m_map.erase(c);
    m_needSave = true;    
}


void
ChordMap2::debugDump() const
{
    for(chordset::const_iterator i = m_map.begin(); i != m_map.end(); ++i) {
        Chord2 chord = *i;
        NOTATION_DEBUG << "ChordMap2::debugDump " << chord << endl;
    } 
}

}
