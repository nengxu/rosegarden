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
#include "document/ConfigGroups.h"

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

    // be non-modal so we can get away with just doing everything in a simple
    // linear fashion here in the ctor without making the entire application
    // seem hung while we do our bit here
    //
    // NOTE: doesn't actually work.  Damn.  So we could waitForStarted() on
    // launching the final external app and then continue along, or else we can
    // turn the whole thing into a complicated something or other.  I'll have to
    // think about how to do this series of linear steps in a non-linear
    // signal-driven fashion.  My first thoughts on that involve using goto
    // inside the slots, which is why I tried to avoid all that entirely.
    //
    // Maybe the blocking effect won't happen if it's a non-modal dialog and
    // it's running its scripty bit inside exec() instead of the ctor.
    //
    // <scratches beard>
    //
    // Oh well.  I'll figure it out later.  For now, time to enjoy that it's
    // even working at all, and prepare to slumber.  If I don't bail now, I'll
    // be here until dawn piddling.  A demain.
    this->setModal(false);

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
    //
    /* NOTE TO SELF: This might be a bad idea after all, because the entire app
     * freezes while it blocks on KPDF running further into the process.  Also,
     * the whole thing top to bottom is screaming to be modularized into some
     * runThisThing(with, options) that gets called multiple times, instead of
     * all this block copied code.  I may or may not actually do that.  It would
     * be cleaner to be sure, but it's unlikely there will be much more to add
     * in the future in terms of new iterations of manipulating a QProcess, so
     * if it works, it might make as much sense as anything else to just leave
     * it alone and save refactoring it for some future rewrite.  Especially
     * since only two out of the three block copied snippets really lend
     * themselves to being stuffed into a similar function.
     *
     * Anyway, maybe I really do need some signal trapping when finished
     * nonsense in here, or maybe it's sufficient to make this dialog non-modal
     * so the main window's event loop isn't stuck while this modal dialog's
     * ctor is blocking.  That will probably do the trick for super cheap, and
     * writing this code has been pulling teeth.  I'm not enjoying this at all.
     * Blah.  What a tedious, miserable bit of work.  I can't wait to tackle the
     * project packager and the audio file importer next. Someone please smash
     * my testicles with a brick to curb my enthusiasm.
     */
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

    QString pdfName = filename.replace(".ly", ".pdf");

    // retrieve user preferences from QSettings
    QSettings settings;
    settings.beginGroup(ExternalApplicationsConfigGroup);
    int pdfViewerIndex = settings.value("pdfviewer", 0 /* Okular */ ).toUInt();
    settings.endGroup();

    QString pdfViewer;

    // assumes the PDF viewer is available in the PATH; no provision is made for
    // the user to specify the location of any of these explicitly, and I'd like
    // to avoid having to go to that length if at all possible, in order to
    // reduce complexity both in code and on the user side of the configuration
    // page
    switch (pdfViewerIndex) {
        case 0: pdfViewer = "okular";   break;
        case 1: pdfViewer = "evince";   break;
        case 2: pdfViewer = "acroread"; break;
        case 3: pdfViewer = "kpdf"; 
        default: pdfViewer = "kpdf";
    }

    if (p->exitCode() == 0 && !panic) {
        delete p;
        p = new QProcess;


        switch (mode) {
            case LilyPondProcessor::Print:
                info->setText(tr("Printing %1...").arg(pdfName));
                /* stuff goes here */
                break;

            // just default to preview
            case LilyPondProcessor::Preview:
            default:
                info->setText(tr("Previewing %1...").arg(pdfName));
                p->start(pdfViewer, QStringList() << pdfName);
        }

        p->waitForFinished();
        
    }

    if (p->exitCode() == 0 && !panic) {
        info->setText(tr("Complete!"));
    } else {
        info->setWordWrap(true);
        /* half forgotten idea from yesterday...  let's try to recall it...
         * look at incoming options (are they in QSettings?) to see if we
         * exported with beams || staff group brackets || &c. and use this
         * information to do point for point shit happens messages.  "You have
         * manual beaming turned on, and LilyPond puked on the output.  Certain
         * operations can result in unwanted properties blah blah blah.  Select
         * all, un-beam, then re-beam manually, or fix the bad events or just
         * turn the damn beaming export off blah blah and so on."  Likewise blah
         * blah text about staff brackets, and so on for any other cases that
         * appear in due course
         */
        /* also note that while it's probably not easy to play an audio file
         * inside one of these warning boxes, it would be funny as shit to use
         * harmon.wav for these things, taking some inspiration from that blog
         * about how application developers shouldn't be afraid to make error
         * messages fun.
         */
        info->setText(tr("<qt><p>The LilyPond process terminated with errors.  Unfortunately, our LilyPond export can be fragile at times.</p><p>One of the more likely sources of trouble is the staff group brackets found in Track Parameters.  Double check that the brackets you are using are all terminated and nested properly, and that you are not exporting a subset of segments whose combined brackets do not make sense.  Try exporting with <b>Export track staff brackets</b> turned off.  Another common source of trouble occurs when you export Rosegarden's beaming, but you have beam properties that do not make any sense.  This can happen from time to time when using features like split by pitch, or when cutting and pasting.  Try exporting with <b>Export beamings</b> turned off.  If all else fails, export your work manually with <b>File -> Export</b> and run <b>lilypond</b> on your file by hand.  If you have problems that are not obvious, please see <a href= \"mailto:rosegarden-user@lists.sourceforge.net\">Rosegarden User</a> for more help.  We apologize for the inconvenience.</p></qt>"));
    }

    if (panic) {
        info->setText("<qt><p>There was an error processing output!  No further information is available due to the incomplete nature of this emerging feature.  Oops.</p></qt>");
    }
}

}

#include "LilyPondProcessor.moc"
