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


#ifndef _RG_GUITARCHORDSELECTORDIALOG_H_
#define _RG_GUITARCHORDSELECTORDIALOG_H_

#include "Chord2.h"
#include "ChordMap2.h"

#include <kdialogbase.h>
#include <qstring.h>
#include <vector>

class QListBox;

namespace Rosegarden
{

class FingeringBox2;

class GuitarChordSelectorDialog : public KDialogBase
{
     Q_OBJECT
    
public:
	GuitarChordSelectorDialog(QWidget *parent=0);

    void init();

    const Chord2& getChord() const { return m_chord; }

    void setChord(const Chord2&);

protected slots:
    void slotRootHighlighted(int);
    void slotChordExtHighlighted(int);
    void slotFingeringHighlighted(int);
    
protected:

    void parseChordFiles(const std::vector<QString>& chordFiles);
    void parseChordFile(const QString& chordFileName);
    void populateFingerings(const ChordMap2::chordarray&);
    void populateExtensions(const QStringList& extList);

    QPixmap getFingeringPixmap(const Fingering2& fingering) const;
         
    //! Find all chord list files on the system
    std::vector<QString> getAvailableChordFiles();

    //! Read directory for guitar chord files.
    void readDirectory(QString chordDir, std::vector<QString>&);

    //! List of Chords
    ChordMap2 m_chordMap;

    /// selected chord
    Chord2 m_chord;
    
    // Chord data
    QListBox* m_rootNotesList;
    QListBox* m_chordExtList;
    QListBox* m_fingeringsList;

    //! Fingering constructor object
    FingeringBox2* m_fingeringBox;

};

}

#endif /*_RG_GUITARCHORDSELECTORDIALOG_H_*/
