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

#include "GuitarChordEditorDialog.h"
#include "FingeringBox2.h"

#include <klineedit.h>
#include <qcombobox.h>

namespace Rosegarden
{

GuitarChordEditorDialog::GuitarChordEditorDialog(QWidget *parent)
    : KDialogBase(parent, "GuitarChordEditor", true, i18n("Guitar Chord Editor"), Ok|Cancel)    
{
    QWidget *page = new QWidget(this);
    setMainWidget(page);
    QGridLayout *topLayout = new QGridLayout(page, 5, 2, spacingHint());
    
    topLayout->addWidget(new QLabel(i18n("Root"), page), 0, 1);
    m_rootNotesList = new QComboBox(page);
    topLayout->addWidget(m_rootNotesList, 1, 1);
    
    topLayout->addWidget(new QLabel(i18n("Extension"), page), 2, 1);
    m_ext = new KLineEdit(page);
    topLayout->addWidget(m_ext, 3, 1);

    topLayout->addItem(new QSpacerItem(1, 1), 4, 1);

    m_fingeringBox = new FingeringBox2(true, page);
    topLayout->addMultiCellWidget(m_fingeringBox, 0, 5, 0, 0);

}


}
