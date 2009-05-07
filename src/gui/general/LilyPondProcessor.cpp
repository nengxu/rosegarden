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
    COPYING included with this distribution for more m_information.
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
        QDialog(parent),
        m_mode(mode),
        m_filename(filename)
{
    // (I'm not sure why RG_DEBUG didn't work from in here.  Having to use
    // iostream is mildly irritating, as QStrings have to be converted, but
    // whatever, I'll figure that out later, or just leave well enough alone)
    std::cerr << "LilyPondProcessor::LilyPondProcessor():  mode: " << mode << " m_filename: " << m_filename.toStdString() << std::endl;

    this->setModal(false);

    setWindowTitle(tr("Processing %1").arg(m_filename));
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

    m_info = new QLabel(this);
    layout->addWidget(m_info, 1, 1);

    m_progress = new QProgressBar(this);
    m_progress->setMinimum(0);
    m_progress->setMaximum(100);
    layout->addWidget(m_progress, 2, 1);

/*    QPushButton *ok = new QPushButton(tr("Ok"), this); //!!! use a QButtonBox as elsewhere
    layout->addWidget(ok, 1, 1); */

   
    // Just run convert.ly without all the logic to figure out if it's needed or
    // not.  This is harmless, and adds little extra processing time if the
    // conversion isn't required.  This is the first link in a spaghetti bowl
    // chain of slots.  We either have to run all of this in a thread apart from
    // the main GUI thread or use this spaghetti bowl chaining technique in
    // order to avoid freezing the entire application while chewing on large
    // processing jobs, as we really must wait for step A to finish before
    // continuing to step B.  I have no experience with threads, so the
    // spaghetti option is the most expedient, if less educational choice.
    runConvertLy();
}

void
LilyPondProcessor::puke(QString error)
{
    m_info->setWordWrap(true);
    m_info->setText(error);
    // probably want to use a QMessageBox::error here to block until the user
    // reads, and then abort the whole shebang afterwards, rather than the
    // little info QLabel in the main dialog, but one thing at a time shall we
//    reject();
}

void
LilyPondProcessor::runConvertLy()
{
    m_info->setText(tr("Running <b>convert-ly</b>..."));
    QProcess *p = new QProcess;
    p->start("convert-ly", QStringList() << "-e" << m_filename);
    connect(p, SIGNAL(processExited(QProcess *)),
            this, SLOT(runLilyPond(QProcess *)));

    // wait up to 10 seconds for process to start
    if (p->waitForStarted(10)) {
        m_info->setText(tr("<b>convert-ly</b> started..."));
    } else {
        puke(tr("<qt><p>Could not run <b>convert-ly</b>!</p><p>Please install LilyPond and ensure that the \"convert-ly\" and \"lilypond\" commands are available on your path.  If you perform a <b>Run Command</b> (typically <b>Alt+F2</b>) and type \"convert-ly\" into the box, you should not get a \"command not found\" error.  If you can do that without getting an error, but still see this error message, please consult <a href=\"mailto:rosegarden-user@lists.sourceforge.net>rosegarden-user@lists.sourceforge.net</a> for additional help.</p><p>Processing terminated due to fatal errors.</p></qt>"));
    }

    m_progress->setValue(25);
}

void
LilyPondProcessor::runLilyPond(QProcess *p)
{
    std::cerr << "LilyPondProcessor::runLilyPond()" << std::endl;

    if (p->exitCode() == 0) {
        m_info->setText(tr("<b>convert-ly</b> finished..."));
        delete p;
    } else {
        puke(tr("<qt><p>Ran <b>convert-ly</b> successfully, but it terminated with errors.</p><p>Processing terminated due to fatal errors.</p></qt>"));
    }

    m_progress->setValue(50);

    p = new QProcess;
    m_info->setText(tr("Running <b>lilypond</b>..."));
    p->start("lilypond", QStringList() << "--pdf" << m_filename);

    switch (m_mode) {
        case LilyPondProcessor::Preview:
                connect(p, SIGNAL(processExited(QProcess *)),
                        this, SLOT(runPdfViewer(QProcess *)));
                break;
        case LilyPondProcessor::Print:
                connect(p, SIGNAL(processExited(QProcess *)),
                        this, SLOT(runFilePrinter(QProcess *)));
    }
            
    if (p->waitForStarted()) {
        m_info->setText(tr("<b>lilypond</b> started..."));
    } else {
        puke(tr("<qt><p>Could not run <b>lilypond</b>!</p><p>Please install LilyPond and ensure that the \"convert-ly\" and \"lilypond\" commands are available on your path.  If you perform a <b>Run Command</b> (typically <b>Alt+F2</b>) and type \"lilypond\" into the box, you should not get a \"command not found\" error.  If you can do that without getting an error, but still see this error message, please consult <a href=\"mailto:rosegarden-user@lists.sourceforge.net>rosegarden-user@lists.sourceforge.net</a> for additional help.</p><p>Processing terminated due to fatal errors.</p></qt>"));
    }

    m_progress->setValue(75);
}

void
LilyPondProcessor::runFinalStage(QProcess *p)
{
    if (p->exitCode() == 0) {
        m_info->setText(tr("<b>lilypond</b> finished..."));
        delete p;
    } else {
        // this is where all the try and try again with different options stuff
        // comes into play eventually, maybe calling this slot recursively or
        // something?
        puke(tr("<qt><p>Ran <b>lilypond</b> successfully, but it terminated with errors.</p><p>Processing terminated due to fatal errors.</p></qt>"));
    }

    QString pdfName = m_filename.replace(".ly", ".pdf");

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
        default: pdfViewer = "kpdf"; // just because I'm still currently on KDE3
    }

    if (p->exitCode() == 0) {
        delete p;
        p = new QProcess;


        switch (m_mode) {
            case LilyPondProcessor::Print:
                m_info->setText(tr("Printing %1...").arg(pdfName));
                /* stuff goes here */
                break;

            // just default to preview
            case LilyPondProcessor::Preview:
            default:
                m_info->setText(tr("Previewing %1...").arg(pdfName));
                p->start(pdfViewer, QStringList() << pdfName);
        }

    }

    /* more didn't start error trapping here, then we can just accept() and go
     * home I think */

/*  if (p->exitCode() == 0 && !panic) {
        m_info->setText(tr("Complete!"));
    } else {
        m_info->setWordWrap(true);
        m_info->setText(tr("<qt><p>The LilyPond process terminated with errors.  Unfortunately, our LilyPond export can be fragile at times.</p><p>One of the more likely sources of trouble is the staff group brackets found in Track Parameters.  Double check that the brackets you are using are all terminated and nested properly, and that you are not exporting a subset of segments whose combined brackets do not make sense.  Try exporting with <b>Export track staff brackets</b> turned off.  Another common source of trouble occurs when you export Rosegarden's beaming, but you have beam properties that do not make any sense.  This can happen from time to time when using features like split by pitch, or when cutting and pasting.  Try exporting with <b>Export beamings</b> turned off.  If all else fails, export your work manually with <b>File -> Export</b> and run <b>lilypond</b> on your file by hand.  If you have problems that are not obvious, please see <a href= \"mailto:rosegarden-user@lists.sourceforge.net\">Rosegarden User</a> for more help.  We apologize for the inconvenience.</p></qt>"));
    }

    if (panic) {
        m_info->setText("<qt><p>There was an error processing output!  No further m_information is available due to the incomplete nature of this emerging feature.  Oops.</p></qt>");
    }*/
}

}

#include "LilyPondProcessor.moc"
