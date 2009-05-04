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

#include "LilyPondProcessor.h"

#include "gui/general/IconLoader.h"

#include <QDialog>
#include <QProcess>
#include <QGridLayout>
#include <QPushButton>
#include <QSettings>
#include <QLabel>
#include <QProgressBar>

#include <iostream>

namespace Rosegarden
{

LilyPondProcessor::LilyPondProcessor(QWidget *parent, int mode, QString filename) :
        QDialog(parent)
{
    // (I'm not sure why RG_DEBUG didn't work from in here.  Having to use
    // iostream is mildly irritating, as QStrings have to be converted, but
    // whatever, I'll figure that out later, or just leave well enough alone)
    std::cerr << "LilyPondProcessor::LilyPondProcessor():  mode: " << mode << " filename: " << filename.toStdString() << std::endl;

    setWindowTitle(tr("Processing %1").arg(filename));
    setIcon(IconLoader().loadPixmap("window-lilypond"));

    QGridLayout *layout = new QGridLayout;
    this->setLayout(layout);

    QLabel *icon = new QLabel(this);
    icon->setPixmap(IconLoader().loadPixmap("rosegarden-lilypond"));
    layout->addWidget(icon, 0, 0);

    QString modeStr;
    switch (mode) {
        case LilyPondProcessor::Preview: modeStr = tr("Preview"); break;
        case LilyPondProcessor::Print:   modeStr = tr("Print");   break;
        default:                         modeStr = tr("INTERNAL ERROR"); break;
    }
    QLabel *greeting = new QLabel(tr("<qt><p>%1 with LilyPond...</p><br></qt>").arg(modeStr), this);
    layout->addWidget(greeting, 0, 1);

    QLabel *info = new QLabel(this);
    layout->addWidget(info, 1, 1);

    QProgressBar *progress = new QProgressBar(this);
    // Set minimum and maximum to 0 to turn the progress bar into a busy
    // indicator, and thus avoid having to work out some system of steps
    progress->setMinimum(0);
    progress->setMaximum(0);
    layout->addWidget(progress, 2, 1);

/*    QPushButton *ok = new QPushButton(tr("Ok"), this); //!!! use a QButtonBox as elsewhere
    layout->addWidget(ok, 1, 1); */

    bool panic = false;

    // Just run convert.ly without all the logic to figure out if it's needed or
    // not.  This is harmless, and adds little extra processing time if the
    // conversion isn't required.
    info->setText(tr("Running <b>convert-ly</b>..."));
    QProcess *p = new QProcess;
    p->start("convert-ly", QStringList() << "-e" << filename);

    // We have to QProcess several times to re-create what used to be done in a
    // script, so let's just block until this completes, instead of creating a
    // pile of process-finished signal handler slots
    if (p->waitForStarted() && !panic) {
        info->setText(tr("<b>convert-ly</b> started..."));
    } else {
        panic = true;
    }
    
    p->waitForFinished();

    if (p->exitCode() == 0 && !panic) {
        info->setText(tr("<b>convert-ly</b> finished..."));
        delete p;
    } else {
        panic = true;
    }

    p = new QProcess;
    info->setText(tr("Running <b>lilypond</b>..."));
    p->start("lilypond", QStringList() << "--pdf" << filename);
    if (p->waitForStarted() && !panic) {
        info->setText(tr("<b>lilypond</b> started..."));
    } else {
        panic = true;
    }

    p->waitForFinished();

    if (p->exitCode() == 0 && !panic) {
        info->setText(tr("Complete!"));
    } else {
        info->setWordWrap(true);
        info->setText(tr("<qt><p>The LilyPond process terminated with errors.  Unfortunately, our LilyPond export can be fragile at times.</p><p>One of the more likely sources of trouble is the staff group brackets found in Track Parameters.  Double check that the brackets you are using are all terminated and nested properly, and that you are not exporting a subset of segments whose combined brackets do not make sense.  Try exporting with <b>Export track staff brackets</b> turned off.  Another common source of trouble occurs when you export Rosegarden's beaming, but you have beam properties that do not make any sense.  This can happen from time to time when using features like split by pitch, or when cutting and pasting.  Try exporting with <b>Export beamings</b> turned off.  If all else fails, export your work manually with <b>File -> Export</b> and run <b>lilypond</b> on your file by hand.  If you have problems that are not obvious, please see <a href= \"mailto:rosegarden-user@lists.sourceforge.net\">Rosegarden User</a> for more help.  We apologize for the inconvenience.</p></qt>"));
    }

    if (panic) {
        info->setText("<qt><p>There was an error processing output!  No further information is available due to the incomplete nature of this emerging feature.  Oops.</p></qt>");
    }
}

}

#include "LilyPondProcessor.moc"
