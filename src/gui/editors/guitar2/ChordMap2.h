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

#ifndef _RG_CHORDMAP2_H_
#define _RG_CHORDMAP2_H_

#include "Chord2.h"

#include <qstringlist.h>
#include <set>

namespace Rosegarden
{

class ChordMap2
{
    typedef std::set<Chord2, Chord2::ChordCmp> chordset; 

public:
    typedef std::vector<Chord2> chordarray;

    typedef chordset::iterator iterator;
    typedef chordset::const_iterator const_iterator;

    static int FILE_FORMAT_VERSION_MAJOR;
    static int FILE_FORMAT_VERSION_MINOR;
    static int FILE_FORMAT_VERSION_POINT;
    
	ChordMap2();
    
    void insert(const Chord2&);
    void substitute(const Chord2& oldChord, const Chord2& newChord);
    void remove(const Chord2&);
    
    chordarray getChords(const QString& root, const QString& ext) const;
    
    QStringList getRootList() const;
    QStringList getExtList(const QString& root) const;
    
    void debugDump() const;
    
    bool needSave() const { return m_needSave; }
    void clearNeedSave() { m_needSave = false; }

    bool saveDocument(const QString& filename, QString& errMsg);

    iterator begin() { return m_map.begin(); }
    iterator end()   { return m_map.end();   }
    const_iterator begin() const { return m_map.begin(); }
    const_iterator end()   const { return m_map.end(); }
    
protected:

    chordset m_map;
    
    bool m_needSave;
};

}

#endif /*_RG_CHORDMAP2_H_*/
