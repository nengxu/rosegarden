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

#include "GuitarChordSelectorDialog.h"
#include "ChordXmlHandler.h"
#include "FingeringBox2.h"

#include "misc/Debug.h"
#include <qlistbox.h>
#include <qlayout.h>

namespace Rosegarden
{

GuitarChordSelectorDialog::GuitarChordSelectorDialog(QWidget *parent)
    : KDialogBase(parent, "GuitarChordSelector", true, i18n("Guitar Chord Selector"), Ok|Cancel)
{
    QWidget *page = new QWidget(this);
    setMainWidget(page);
    QGridLayout *topLayout = new QGridLayout(page, 2, 3, spacingHint());
    
    m_rootNotesList = new QListBox(page);
    topLayout->addWidget(m_rootNotesList, 0, 0);
    
    m_chordExtList = new QListBox(page);
    topLayout->addWidget(m_chordExtList, 0, 1);
    
    m_fingeringsList = new QListBox(page);
    topLayout->addMultiCellWidget(m_fingeringsList, 0, 1, 2, 2);
    
    m_fingeringBox = new FingeringBox2(false, page);
    topLayout->addMultiCellWidget(m_fingeringBox, 1, 1, 0, 1);
    
    connect(m_rootNotesList, SIGNAL(highlighted(int)),
            this, SLOT(slotRootHighlighted(int)));
    connect(m_chordExtList, SIGNAL(highlighted(int)),
            this, SLOT(slotChordExtHighlighted(int)));
    connect(m_fingeringsList, SIGNAL(highlighted(int)),
            this, SLOT(slotFingeringHighlighted(int)));
}

void
GuitarChordSelectorDialog::init()
{
    // populate the listboxes
    //
    std::vector<QString> chordFiles = getAvailableChordFiles();
    
    parseChordFiles(chordFiles);

    m_chordMap.debugDump();
    
    QStringList rootList = m_chordMap.getRootList();
    if (rootList.count() > 0) {
        m_rootNotesList->insertStringList(rootList);

        QStringList extList = m_chordMap.getExtList(rootList.first());
        populateExtensions(extList);
        
        ChordMap2::chordarray chords = m_chordMap.getChords(rootList.first(), extList.first());
        populateFingerings(chords);

        m_chord.setRoot(rootList.first());
        m_chord.setExt(extList.first());
    }
}

void
GuitarChordSelectorDialog::slotRootHighlighted(int i)
{
    NOTATION_DEBUG << "GuitarChordSelectorDialog::slotRootHighlighted " << i << endl;

    m_chord.setRoot(m_rootNotesList->text(i));
}

void
GuitarChordSelectorDialog::slotChordExtHighlighted(int i)
{
    NOTATION_DEBUG << "GuitarChordSelectorDialog::slotChordExtHighlighted " << i << endl;

    m_chord.setExt(m_chordExtList->text(i));
    ChordMap2::chordarray chords = m_chordMap.getChords(m_chord.getRoot(), m_chord.getExt());
    populateFingerings(chords);    
}

void
GuitarChordSelectorDialog::slotFingeringHighlighted(int i)
{
    NOTATION_DEBUG << "GuitarChordSelectorDialog::slotFingeringHighlighted " << i << endl;
    
    QString f = m_fingeringsList->text(i);
    QString errString;
    Fingering2 fingering = Fingering2::parseFingering(f, errString);
    if (m_chord.getNbFingerings() > 0)
        m_chord.setFingering(0, fingering);
    else
        m_chord.addFingering(fingering);

    m_chord.setSelectedFingeringIdx(0);
}


void
GuitarChordSelectorDialog::setChord(const Chord2& chord)
{
    m_chord = chord;
    
    m_chordExtList->clear();
    QStringList extList = m_chordMap.getExtList(chord.getRoot());
    m_chordExtList->insertStringList(extList);
        
    ChordMap2::chordarray chords = m_chordMap.getChords(chord.getRoot(), extList.first());
    populateFingerings(chords);
}

void
GuitarChordSelectorDialog::populateFingerings(const ChordMap2::chordarray& chords)
{
    m_fingeringsList->clear();
    
    for(ChordMap2::chordarray::const_iterator i = chords.begin(); i != chords.end(); ++i) {
        const Chord2& chord = *i;
        for(unsigned int j = 0; j < chord.getNbFingerings(); ++j) {
            QString fingeringString = chord.getFingering(j).toString();
            NOTATION_DEBUG << "GuitarChordSelectorDialog::populateFingerings " << chord << " - fingering : " << fingeringString << endl;
            // TODO add pixmap
            m_fingeringsList->insertItem(fingeringString);
        }
    }
}

void
GuitarChordSelectorDialog::populateExtensions(const QStringList& extList)
{
    m_chordExtList->clear();
    m_chordExtList->insertStringList(extList);
} 

void
GuitarChordSelectorDialog::parseChordFiles(const std::vector<QString>& chordFiles)
{
    for(std::vector<QString>::const_iterator i = chordFiles.begin(); i != chordFiles.end(); ++i) {
        parseChordFile(*i);
    }
}

void
GuitarChordSelectorDialog::parseChordFile(const QString& chordFileName)
{
    ChordXmlHandler handler(m_chordMap);
    QFile chordFile(chordFileName);
    bool ok = chordFile.open(IO_ReadOnly);    
    if (!ok)
        KMessageBox::error(0, i18n("couldn't open file '%1'").arg(handler.errorString()));

    QXmlInputSource source(chordFile);
    QXmlSimpleReader reader;
    reader.setContentHandler(&handler);
    reader.setErrorHandler(&handler);
    NOTATION_DEBUG << "GuitarChordSelectorDialog::parseChordFile() parsing " << chordFileName << endl;
    reader.parse(source);
    if (!ok)
        KMessageBox::error(0, i18n("couldn't parse chord dictionnary : %1").arg(handler.errorString()));
    
}

std::vector<QString>
GuitarChordSelectorDialog::getAvailableChordFiles()
{
    std::vector<QString> names;

    // Read config for default directory
    QString chordDir = KGlobal::dirs()->findResource("appdata", "default_chords/");

    // Read config for user directory
    QString userDir = KGlobal::dirs()->findResource("appdata", "user_chords/");

    if (!chordDir.isEmpty()) {
        readDirectory(chordDir, names);
    }

    if (!userDir.isEmpty()) {
        readDirectory (userDir, names);
    }

    return names;
}

void
GuitarChordSelectorDialog::readDirectory(QString chordDir, std::vector<QString>& names)
{
    QDir dir( chordDir );

    dir.setFilter(QDir::Files | QDir::Readable);
    dir.setNameFilter("*.xml");
    
    QStringList files = dir.entryList();

    for (QStringList::Iterator i = files.begin(); i != files.end(); ++i ) {
        
        // TODO - temporary hack until I remove Stephen's old files
        if ((*i) != "chords.xml") {
//            NOTATION_DEBUG << "GuitarChordSelectorDialog::readDirectory : skip file " << *i << endl;
            continue;
        }
        
        QFileInfo fileInfo(QString("%1/%2").arg(chordDir).arg(*i) );
        names.push_back(fileInfo.filePath());
    }
}


}


#include "GuitarChordSelectorDialog.moc"
