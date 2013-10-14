/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
    Copyright 2000-2013 the Rosegarden development team.
 
    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.
 
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "SelectDialog.h"

// local includes
#include "misc/Debug.h"
#include "misc/ConfigGroups.h"

#include "gui/general/IconLoader.h"

#include "gui/widgets/CheckButton.h"

// Qt includes
#include <QDialog>
#include <QUrl>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QToolButton>


namespace Rosegarden
{


SelectDialog::SelectDialog(QWidget *parent) :
    QDialog(parent)
{
    setModal(true);
    setWindowTitle(tr("Search and Select"));
//    setStyleSheet("background: #FFFFFF");

    // master layout
    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);

    // create tab widget
    m_tabWidget = new QTabWidget();
    layout->addWidget(m_tabWidget);

    // create duration tab
    makeDurationTab();
    m_tabWidget->addTab(m_durationTab, tr("Duration"));
    m_tabWidget->addTab(m_durationTab, "Twice");

    // primary buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help);
    layout->addWidget(buttonBox);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(buttonBox, SIGNAL(helpRequested()), this, SLOT(help()));

}

SelectDialog::~SelectDialog()
{
}

void
SelectDialog::help()
{
    // TRANSLATORS: if the manual is translated into your language, you can
    // change the two-letter language code in this URL to point to your language
    // version, eg. "http://rosegardenmusic.com/wiki/doc:manual-es" for the
    // Spanish version. If your language doesn't yet have a translation, feel
    // free to create one.
    QString helpURL = tr("http://rosegardenmusic.com/wiki/doc:manual-search-and-select-en");
    QDesktopServices::openUrl(QUrl(helpURL));
}

void
SelectDialog::makeDurationTab()
{
    // button grid
    QGridLayout *grid = new QGridLayout();
    m_durationTab = new QWidget();
    m_durationTab->setLayout(grid);

    // breve buttons
    m_use_duration_breve = new CheckButton("breve");
    grid->addWidget(m_use_duration_breve, 0, 0);

    m_use_duration_breve_dotted = new CheckButton("dotted-breve");
    grid->addWidget(m_use_duration_breve_dotted, 0, 1);

    m_use_duration_breve_double_dotted = new CheckButton("double-dotted-breve");
    grid->addWidget(m_use_duration_breve_double_dotted, 0, 2);

    m_use_duration_breve_tuplet = new CheckButton("tuplet-breve");
    grid->addWidget(m_use_duration_breve_tuplet, 0, 3);

    m_use_duration_breve_rest = new CheckButton("rest-breve");
    grid->addWidget(m_use_duration_breve_rest, 0, 4);

    m_use_duration_breve_dotted_rest = new CheckButton("dotted-rest-breve");
    grid->addWidget(m_use_duration_breve_dotted_rest, 0, 5);

    m_use_duration_breve_double_dotted_rest = new CheckButton("double-dotted-rest-breve");
    grid->addWidget(m_use_duration_breve_double_dotted_rest, 0, 6);

    m_use_duration_breve_rest_tuplet = new CheckButton("rest-tuplet-breve");
    grid->addWidget(m_use_duration_breve_rest_tuplet, 0, 7);

    m_use_all_breves = new CheckButton("golden-arrow-left", false);
    grid->addWidget(m_use_all_breves, 0, 8);


    // semibreve buttons
    m_use_duration_semibreve = new CheckButton("semibreve");
    grid->addWidget(m_use_duration_semibreve, 1, 0);

    m_use_duration_semibreve_dotted = new CheckButton("dotted-semibreve");
    grid->addWidget(m_use_duration_semibreve_dotted, 1, 1);

    m_use_duration_semibreve_double_dotted = new CheckButton("double-dotted-semibreve");
    grid->addWidget(m_use_duration_semibreve_double_dotted, 1, 2);

    m_use_duration_semibreve_tuplet = new CheckButton("tuplet-semibreve");
    grid->addWidget(m_use_duration_semibreve_tuplet, 1, 3);

    m_use_duration_semibreve_rest = new CheckButton("rest-semibreve");
    grid->addWidget(m_use_duration_semibreve_rest, 1, 4);

    m_use_duration_semibreve_dotted_rest = new CheckButton("dotted-rest-semibreve");
    grid->addWidget(m_use_duration_semibreve_dotted_rest, 1, 5);

    m_use_duration_semibreve_double_dotted_rest = new CheckButton("double-dotted-rest-semibreve");
    grid->addWidget(m_use_duration_semibreve_double_dotted_rest, 1, 6);

    m_use_duration_semibreve_rest_tuplet = new CheckButton("rest-tuplet-semibreve");
    grid->addWidget(m_use_duration_semibreve_rest_tuplet, 1, 7);

    m_use_all_semibreves = new CheckButton("golden-arrow-left", false);
    grid->addWidget(m_use_all_semibreves, 1, 8);


    // minim buttons
    m_use_duration_minim = new CheckButton("minim");
    grid->addWidget(m_use_duration_minim, 2, 0);

    m_use_duration_minim_dotted = new CheckButton("dotted-minim");
    grid->addWidget(m_use_duration_minim_dotted, 2, 1);

    m_use_duration_minim_double_dotted = new CheckButton("double-dotted-minim");
    grid->addWidget(m_use_duration_minim_double_dotted, 2, 2);

    m_use_duration_minim_tuplet = new CheckButton("tuplet-minim");
    grid->addWidget(m_use_duration_minim_tuplet, 2, 3);

    m_use_duration_minim_rest = new CheckButton("rest-minim");
    grid->addWidget(m_use_duration_minim_rest, 2, 4);

    m_use_duration_minim_dotted_rest = new CheckButton("dotted-rest-minim");
    grid->addWidget(m_use_duration_minim_dotted_rest, 2, 5);

    m_use_duration_minim_double_dotted_rest = new CheckButton("double-dotted-rest-minim");
    grid->addWidget(m_use_duration_minim_double_dotted_rest, 2, 6);

    m_use_duration_minim_rest_tuplet = new CheckButton("rest-tuplet-minim");
    grid->addWidget(m_use_duration_minim_rest_tuplet, 2, 7);

    m_use_all_minims = new CheckButton("golden-arrow-left", false);
    grid->addWidget(m_use_all_minims, 2, 8);


    // crotchet buttons
    m_use_duration_crotchet = new CheckButton("crotchet");
    grid->addWidget(m_use_duration_crotchet, 3, 0);

    m_use_duration_crotchet_dotted = new CheckButton("dotted-crotchet");
    grid->addWidget(m_use_duration_crotchet_dotted, 3, 1);

    m_use_duration_crotchet_double_dotted = new CheckButton("double-dotted-crotchet");
    grid->addWidget(m_use_duration_crotchet_double_dotted, 3, 2);

    m_use_duration_crotchet_tuplet = new CheckButton("tuplet-crotchet");
    grid->addWidget(m_use_duration_crotchet_tuplet, 3, 3);

    m_use_duration_crotchet_rest = new CheckButton("rest-crotchet");
    grid->addWidget(m_use_duration_crotchet_rest, 3, 4);

    m_use_duration_crotchet_dotted_rest = new CheckButton("dotted-rest-crotchet");
    grid->addWidget(m_use_duration_crotchet_dotted_rest, 3, 5);

    m_use_duration_crotchet_double_dotted_rest = new CheckButton("double-dotted-rest-crotchet");
    grid->addWidget(m_use_duration_crotchet_double_dotted_rest, 3, 6);

    m_use_duration_crotchet_rest_tuplet = new CheckButton("rest-tuplet-crotchet");
    grid->addWidget(m_use_duration_crotchet_rest_tuplet, 3, 7);

    m_use_all_crotchets = new CheckButton("golden-arrow-left", false);
    grid->addWidget(m_use_all_crotchets, 3, 8);


    // quaver buttons
    m_use_duration_quaver = new CheckButton("quaver");
    grid->addWidget(m_use_duration_quaver, 4, 0);

    m_use_duration_quaver_dotted = new CheckButton("dotted-quaver");
    grid->addWidget(m_use_duration_quaver_dotted, 4, 1);

    m_use_duration_quaver_double_dotted = new CheckButton("double-dotted-quaver");
    grid->addWidget(m_use_duration_quaver_double_dotted, 4, 2);

    m_use_duration_quaver_tuplet = new CheckButton("tuplet-quaver");
    grid->addWidget(m_use_duration_quaver_tuplet, 4, 3);

    m_use_duration_quaver_rest = new CheckButton("rest-quaver");
    grid->addWidget(m_use_duration_quaver_rest, 4, 4);

    m_use_duration_quaver_dotted_rest = new CheckButton("dotted-rest-quaver");
    grid->addWidget(m_use_duration_quaver_dotted_rest, 4, 5);

    m_use_duration_quaver_double_dotted_rest = new CheckButton("double-dotted-rest-quaver");
    grid->addWidget(m_use_duration_quaver_double_dotted_rest, 4, 6);

    m_use_duration_quaver_rest_tuplet = new CheckButton("rest-tuplet-quaver");
    grid->addWidget(m_use_duration_quaver_rest_tuplet, 4, 7);

    m_use_all_quavers = new CheckButton("golden-arrow-left", false);
    grid->addWidget(m_use_all_quavers, 4, 8);


    // semiquaver buttons
    m_use_duration_semiquaver = new CheckButton("semiquaver");
    grid->addWidget(m_use_duration_semiquaver, 5, 0);

    m_use_duration_semiquaver_dotted = new CheckButton("dotted-semiquaver");
    grid->addWidget(m_use_duration_semiquaver_dotted, 5, 1);

    m_use_duration_semiquaver_double_dotted = new CheckButton("double-dotted-semiquaver");
    grid->addWidget(m_use_duration_semiquaver_double_dotted, 5, 2);

    m_use_duration_semiquaver_tuplet = new CheckButton("tuplet-semiquaver");
    grid->addWidget(m_use_duration_semiquaver_tuplet, 5, 3);

    m_use_duration_semiquaver_rest = new CheckButton("rest-semiquaver");
    grid->addWidget(m_use_duration_semiquaver_rest, 5, 4);

    m_use_duration_semiquaver_dotted_rest = new CheckButton("dotted-rest-semiquaver");
    grid->addWidget(m_use_duration_semiquaver_dotted_rest, 5, 5);

    m_use_duration_semiquaver_double_dotted_rest = new CheckButton("double-dotted-rest-semiquaver");
    grid->addWidget(m_use_duration_semiquaver_double_dotted_rest, 5, 6);

    m_use_duration_semiquaver_rest_tuplet = new CheckButton("rest-tuplet-semiquaver");
    grid->addWidget(m_use_duration_semiquaver_rest_tuplet, 5, 7);

    m_use_all_semiquavers = new CheckButton("golden-arrow-left", false);
    grid->addWidget(m_use_all_semiquavers, 5, 8);


    // demisemi buttons
    m_use_duration_demisemi = new CheckButton("demisemi");
    grid->addWidget(m_use_duration_demisemi, 6, 0);

    m_use_duration_demisemi_dotted = new CheckButton("dotted-demisemi");
    grid->addWidget(m_use_duration_demisemi_dotted, 6, 1);

    m_use_duration_demisemi_double_dotted = new CheckButton("double-dotted-demisemi");
    grid->addWidget(m_use_duration_demisemi_double_dotted, 6, 2);

    m_use_duration_demisemi_tuplet = new CheckButton("tuplet-demisemi");
    grid->addWidget(m_use_duration_demisemi_tuplet, 6, 3);

    m_use_duration_demisemi_rest = new CheckButton("rest-demisemi");
    grid->addWidget(m_use_duration_demisemi_rest, 6, 4);

    m_use_duration_demisemi_dotted_rest = new CheckButton("dotted-rest-demisemi");
    grid->addWidget(m_use_duration_demisemi_dotted_rest, 6, 5);

    m_use_duration_demisemi_double_dotted_rest = new CheckButton("double-dotted-rest-demisemi");
    grid->addWidget(m_use_duration_demisemi_double_dotted_rest, 6, 6);

    m_use_duration_demisemi_rest_tuplet = new CheckButton("rest-tuplet-demisemi");
    grid->addWidget(m_use_duration_demisemi_rest_tuplet, 6, 7);

    m_use_all_demisemis = new CheckButton("golden-arrow-left", false);
    grid->addWidget(m_use_all_demisemis, 6, 8);


    // hemidemisemi buttons
    m_use_duration_hemidemisemi = new CheckButton("hemidemisemi");
    grid->addWidget(m_use_duration_hemidemisemi, 7, 0);

    m_use_duration_hemidemisemi_dotted = new CheckButton("dotted-hemidemisemi");
    grid->addWidget(m_use_duration_hemidemisemi_dotted, 7, 1);

    m_use_duration_hemidemisemi_double_dotted = new CheckButton("double-dotted-hemidemisemi");
    grid->addWidget(m_use_duration_hemidemisemi_double_dotted, 7, 2);

    m_use_duration_hemidemisemi_tuplet = new CheckButton("tuplet-hemidemisemi");
    grid->addWidget(m_use_duration_hemidemisemi_tuplet, 7, 3);

    m_use_duration_hemidemisemi_rest = new CheckButton("rest-hemidemisemi");
    grid->addWidget(m_use_duration_hemidemisemi_rest, 7, 4);

    m_use_duration_hemidemisemi_dotted_rest = new CheckButton("dotted-rest-hemidemisemi");
    grid->addWidget(m_use_duration_hemidemisemi_dotted_rest, 7, 5);

    m_use_duration_hemidemisemi_double_dotted_rest = new CheckButton("double-dotted-rest-hemidemisemi");
    grid->addWidget(m_use_duration_hemidemisemi_double_dotted_rest, 7, 6);

    m_use_duration_hemidemisemi_rest_tuplet = new CheckButton("rest-tuplet-hemidemisemi");
    grid->addWidget(m_use_duration_hemidemisemi_rest_tuplet, 7, 7);

    m_use_all_hemidemisemis = new CheckButton("golden-arrow-left", false);
    grid->addWidget(m_use_all_hemidemisemis, 7, 8);


    // column use all buttons
    m_use_all_normals = new CheckButton("golden-arrow-up", false);
    grid->addWidget(m_use_all_normals, 8, 0);

    m_use_all_dotteds = new CheckButton("golden-arrow-up", false);
    grid->addWidget(m_use_all_dotteds, 8, 1);

    m_use_all_double_dotteds = new CheckButton("golden-arrow-up", false);
    grid->addWidget(m_use_all_double_dotteds, 8, 2);

    m_use_all_tuplets = new CheckButton("golden-arrow-up", false);
    grid->addWidget(m_use_all_tuplets, 8, 3);

    m_use_all_rests = new CheckButton("golden-arrow-up", false);
    grid->addWidget(m_use_all_rests, 8, 4);

    m_use_all_dotted_rests = new CheckButton("golden-arrow-up", false);
    grid->addWidget(m_use_all_dotted_rests, 8, 5);

    m_use_all_double_dotted_rests = new CheckButton("golden-arrow-up", false);
    grid->addWidget(m_use_all_double_dotted_rests, 8, 6);

    m_use_all_rest_tuplets = new CheckButton("golden-arrow-up", false);
    grid->addWidget(m_use_all_rest_tuplets, 8, 7);

    m_use_everything = new CheckButton("golden-arrow-left-up", false);
    grid->addWidget(m_use_everything, 8, 8);

    // performance durations are [longer|shorter] combo &c

  
}


}

#include "SelectDialog.moc"
