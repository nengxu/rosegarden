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

#include "ProjectPackager.h"

#include "document/RosegardenDocument.h"
#include "base/Composition.h"
#include "base/Track.h"
#include "gui/general/IconLoader.h"
#include "misc/ConfigGroups.h"
#include "misc/Strings.h"
#include "sound/AudioFile.h"
#include "sound/AudioFileManager.h"

#include <QDialog>
#include <QProcess>
#include <QGridLayout>
#include <QPushButton>
#include <QSettings>
#include <QLabel>
#include <QProgressBar>
#include <QMessageBox>
#include <QDir>
#include <QFile>

#include <iostream>

// NOTE: we're using std::cout everywhere in here for the moment.  It's easy to
// swap later to std::cerr, and for the time being this is convenient, because
// we can ./rosegarden > /dev/null to ignore everything except these messages
// we're generating in here.

namespace Rosegarden
{

ProjectPackager::ProjectPackager(QWidget *parent, RosegardenDocument *document,  int mode, QString filename) :
        QDialog(parent),
        m_doc(document),
        m_mode(mode),
        m_filename(filename)
{
    // (I'm not sure why RG_DEBUG didn't work from in here.  Having to use
    // iostream is mildly irritating, as QStrings have to be converted, but
    // whatever, I'll figure that out later, or just leave well enough alone)
    std::cout << "ProjectPackager::ProjectPackager():  mode: " << mode << " m_filename: " << m_filename.toStdString() << std::endl;

    this->setModal(false);

    setIcon(IconLoader().loadPixmap("window-packager"));

    QGridLayout *layout = new QGridLayout;
    this->setLayout(layout);

    QLabel *icon = new QLabel(this);
    icon->setPixmap(IconLoader().loadPixmap("rosegarden-packager"));
    layout->addWidget(icon, 0, 0);

    QString modeStr;
    switch (mode) {
        case ProjectPackager::Unpack:  modeStr = tr("Unpack"); break;
        case ProjectPackager::Pack:    modeStr = tr("Pack");   break;
    }
    this->setWindowTitle(tr("Rosegarden - %1 Project Package...").arg(modeStr));

    m_info = new QLabel(this);
    m_info->setWordWrap(true);
    layout->addWidget(m_info, 0, 1);

    m_progress = new QProgressBar(this);
    m_progress->setMinimum(0);
    m_progress->setMaximum(100);
    layout->addWidget(m_progress, 1, 1);

    QPushButton *ok = new QPushButton(tr("Cancel"), this);
    connect(ok, SIGNAL(clicked()), this, SLOT(reject()));
    layout->addWidget(ok, 3, 1);

    sanityCheck();
}

void
ProjectPackager::puke(QString error)
{
    m_progress->setMaximum(100);
    m_progress->hide();

    m_info->setText(tr("Fatal error.  Processing aborted."));
    QMessageBox::critical(this, tr("Rosegarden - Fatal processing error!"), error, QMessageBox::Ok, QMessageBox::Ok);

    // abort processing after a fatal error, so calls to puke() abort the whole
    // process in its tracks
    reject();

    // Well, that was the theory.  In practice it apparently isn't so easy to do
    // the bash equivalent of a spontaneous "exit 1" inside a QDialog.  Hrm.
}

QStringList
ProjectPackager::getAudioFiles()
{
    QStringList list;

    // get the Composition from the document, so we can iterate through it
    Composition *comp = &m_doc->getComposition();

    // We don't particularly care about tracks here, so just iterate through the
    // entire Composition to find the audio segments and get the associated
    // file IDs from which to obtain a list of actual files.  This could
    // conceivably pick up audio segments that are residing on MIDI tracks and
    // wouldn't otherwise be functional, but the important thing is to never
    // miss a single file that has any chance of being worth preserving.
    for (Composition::iterator i = comp->begin(); i != comp->end(); ++i) {
        if ((*i)->getType() == Segment::Audio) {

            AudioFileManager *manager = &m_doc->getAudioFileManager();

            unsigned int id = (*i)->getAudioFileId();

            AudioFile *file = manager->getAudioFile(id);

            // some polite sanity checking to avoid possible crashes
            if (!file) continue;

            list << strtoqstr(file->getName());
        }
    }

    // This requires Qt 4.5 or later to work, and it really seems worth it.  All
    // the hand wringing I did about if or whether or how, shazam man, it's this
    // simple to settle all of those questions.  It's irresistable.
    list.removeDuplicates();

    return list;
}

// to avoid problems, we check for flac, which is an integral part of the process.
// we also use tar, but we can safely assume that tar exists.
void
ProjectPackager::sanityCheck() {
    m_process = new QProcess;
    m_process->start("flac", QStringList() << "--help");
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(runPackUnpack(int, QProcess::ExitStatus)));

    // wait up to 30 seconds for process to start
    m_info->setText(tr("Checking for flac..."));
    if( !m_process->waitForStarted()) {
        puke(tr("Couldn't start sanity check."));
        return;
    }

    m_progress->setValue(10);
}

void
ProjectPackager::runPackUnpack(int exitCode, QProcess::ExitStatus) {
    if( exitCode == 0) {
       delete m_process;
    } else {
        puke(tr("Flac was not found. Flac is necessary for Import and Export."));
        return;
    }

    switch (m_mode) {
        case ProjectPackager::Unpack:  runUnpack();    break;
        case ProjectPackager::Pack:    runPack();  break;
    }
}

///////////////////////////
//                       //
//  PACKING OPERATIONS   //
//                       //
///////////////////////////
void
ProjectPackager::runPack()
{
    m_info->setText(tr("Packing project..."));

    QStringList audioFiles = getAudioFiles();

    // get the audio path from the Document via the AudioFileManager (eg.
    // "/home/jsmith/rosegarden" )  (note that Rosegarden stores such things
    // internally as std::strings for obscure legacy reasons)
    AudioFileManager *manager = &m_doc->getAudioFileManager();
    std::string audioPath = manager->getAudioPath();

//  QDir::homePath() and so on

//      Stitch together QDirs?
//
//      Qt method for copying files?  (Probably is one.  QFile?  QDir?)

    QString path0("rosegarden-project-packager-tmp");
    QString path1 = path0 + "/test";
    int start1, stop1;
    start1 = m_filename.findRev('/');
    stop1 = m_filename.findRev('.');
    path1 = m_filename.mid(start1, stop1);
    std::string t123 = path1.toStdString();
    std::string t122 = audioFiles[0].toStdString();
 
    QDir tmpDir;
    if (tmpDir.exists(path0)) {
        // If the directory exists, it's left over from an aborted previous run.
        // Should we clean it up silently, or warn and abort?  At this stage in
        // development, let's ignore it and carry on
        tmpDir.remove(path0);
        std::cout << "Removing path: " << path0.toStdString() << std::endl;
    }

    // make the temporary working directory
    if (tmpDir.mkdir(path0)) {
        QFile::copy("README", path1);
        // copy m_filename
    } else {
        puke(tr("<qt>Could not create temporary working directory.<br>Processing aborted!</qt>"));
        return;
    }


    /* 1. find suitable place to write a tmp directory (should it be /tmp or
     * under ~ somewhere, eg. ~/tmp or maybe even a Qt class can figure it out
     * so we don't have to)
     *
     * THOUGHTS: let's use ~/rosegarden-project-packager-tmp and warn if it
     * isn't empty when we start, etc.  We'll want to do this in userland in
     * case they have a partitioning scheme that limits system disk usage.  Most
     * users don't bother with this kind of thing these days, but I do, and I
     * can't be alone. (I also do this on my system. Ilan)
     *
     * What we'll want to do is take the full path/to/filename coming into this
     * thing for later use, and in the interim we work with the "basename" part
     * of the filename, relative to this temporary working location.  Let's
     * build it like
     *
     * ~/rosegarden-project-packager-tmp/m_filename.rg
     * ~/rosegarden-project-packager-tmp/m_filename/[included files]
     *
     * Then once the whole shebang is rolled up, copy it back to the fully
     * qualified original filename, in the originally specified location.
     *
     * The whole scheme as currently laid out may not yet have enough QProcess
     * chain links to get every operation done, but if so, that's why we have
     * text editors and fingers, right?
     *
     *
     *
     * 3. save/cp m_filename.rg to eg. /tmp/$m_filename.rg (NOTE: don't assume
     * the code that called us in RosegardenMainWindow is at all sacred.)
     * 2. mkdir m_filename there (eg. /tmp/$m_filename)
     * 4. cp extracted audioFiles from $audioPath/$audioFiles to /tmp/$m_filename/$audioFiles (use iterator as in sample code below)
     * 5. run external flac utility on /tmp/$m_filename/$audioFiles
     * 6. prompt for additional files, and add them under /$m_filename directory if any
     * 7. run tar czf command that includes ./$m_filename.rg and ./m_filename directory
     * &c.
     * (at some stage in the overall process we need to change the audio path,
     * manager->setAudioPath() to point to...  Hrm.  Not sure how to handle
     * this, actually, as audio paths stored internally are absolute except for
     * the ~ but we probably can't know where this will actually be extracted,
     * so we're probably going to get that wrong.  I think this is something the
     * original script never got right either.  So we probably want to do that
     * when UN-packing, after we KNOW where the files are.  Instead of feeding
     * the user a "can't find audio file rg-123.wav, use this file dialog to
     * point me at it because I'm Rosegarden and I'm brain damaged"
     *
     * save this for one of the last things to refine later and don't worry
     * overmuch going in)
     */

    std::cout << "Audio files test:" << std::endl;
    QStringList::const_iterator si;
    for (si = audioFiles.constBegin(); si != audioFiles.constEnd(); ++si) {
        std::string o = (*si).toLocal8Bit().constData();
        std::cout << audioPath << " " << o << std::endl;
    }

    // for testing only.  we actually begin with assembleFiles()
    startFlacEncoder(strtoqstr(audioPath), audioFiles);


    /*
    m_process = new QProcess;
    m_process->start("convert-ly", QStringList() << "-e" << m_filename);
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(runUnpack(int, QProcess::ExitStatus)));

    // wait up to 30 seconds for process to start
    if (m_process->waitForStarted()) {
        m_info->setText(tr("<b>convert-ly</b> started..."));
    } else {
        puke(tr("<qt><p>Could not run <b>convert-ly</b>!</p><p>Please install LilyPond and ensure that the \"convert-ly\" and \"lilypond\" commands are available on your path.  If you perform a <b>Run Command</b> (typically <b>Alt+F2</b>) and type \"convert-ly\" into the box, you should not get a \"command not found\" error.  If you can do that without getting an error, but still see this error message, please consult <a href=\"mailto:rosegarden-user@lists.sourceforge.net\">rosegarden-user@lists.sourceforge.net</a> for additional help.</p><p>Processing terminated due to fatal errors.</p></qt>"));
    }

    m_progress->setValue(25);
    * */
}

void
ProjectPackager::assembleFiles(QString path, QStringList files)
{
    // We have the document audio path and a list of necessary audio files put
    // together in the calling code, assembled into the incoming path and files
    // variables
}

void
ProjectPackager::startFlacEncoder(QString path, QStringList files)
{
    // we can't do a oneliner bash script straight out of a QProcess command
    // line, so we'll have to create a purpose built script and run that
    QString scriptName("/tmp/rosegarden-flac-encoder-backend");
    m_script.setName(scriptName);

    // remove any lingering copy from a previous run
    if (m_script.exists()) m_script.remove();

    if (!m_script.open(QIODevice::WriteOnly | QIODevice::Text)) {
        puke(tr("<qt>Unable to write to temporary backend processing script %1.<br>Processing aborted.</qt>"));
        return;
    }

    QTextStream out(&m_script);
    out << "# This script was generated by Rosegarden to combine multiple external processing"      << endl
        << "# operations so they could be managed by a single QProcess.  If you find this script"   << endl
        << "# it is likely that something has gone terribly wrong. See http://rosegardenmusic.com" << endl;

    //TODO we don't yet have a /tmp/m_filename directory with copies of files to
    // work out of and so forth.  We will need to ensure this flac processing
    // script runs out of the correct location and works on the correct files,
    // but none of that is hooked up for this proof of concept
    QStringList::const_iterator si;
    int errorPoint = 1;
    for (si = files.constBegin(); si != files.constEnd(); ++si) {
        std::string o = (*si).toLocal8Bit().constData();
        // default flac behavior is to encode
        //
        // we'll eschew anything fancy or pretty in this disposable script and
        // just write a command on each line, terminating with an || exit n
        // which can be used to figure out at which point processing broke, for
        // cheap and easy error reporting without a lot of fancy stream wiring
        out << "#flac " <<  o << " || exit " << errorPoint << endl;
        out << "sleep 1m" << endl; // just testing to make sure the process doesn't block
        errorPoint++;
    }

    m_script.close();

    // run the assembled script
    m_process = new QProcess;
    m_process->setWorkingDirectory(path);
    m_process->start("bash", QStringList() << scriptName);
//    m_process->waitForFinished();  (just testing to make sure the script
//    actually ate up time as intended for proof of concept purposes)
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(runTar(int, QProcess::ExitStatus)));
// in runTar kill m_script
}

void
ProjectPackager::promptAdditionalFiles()
{
    // Users need to be prompted "Is there anything else you want to add?" and
    // given a chance to keep adding stuff until they're bored.  Keep looping
    // and assembling files into a QStringList until they're bored.  That part
    // of the processing won't block anything.   Then once that list is
    // assembled, do the QProcess bit all in a single QProcess as seems most
    // befitting when actual code exists here
}

void
ProjectPackager::compressPackage()
{
    // Once all the ducks are in a row, we need to run final QProcess to
    // complete the process of turning all of this into a .tar.gz file that has
    // an .rgp extension.  Come to think of it, the way the original script
    // worked was always pretty screwy with respect to where it created the
    // temporary directory to work out of.  What we want to do is pick one place
    // (probably ~/rosegarden-project-packager-tmp let's just say) and want to
    // have done all the processing up to here in that location.  When it's all
    // said and done, we'll mv the resulting .rgp file to the filename
    // originally specified.
}

///////////////////////////
//                       //
// UNPACKING OPERATIONS  //
//                       //
///////////////////////////
void
ProjectPackager::runUnpack()
{
    std::cout << "ProjectPackager::runUnpack()" << std::endl;
    m_info->setText(tr("Unpacking project..."));

    /* As far as I can see the process should be:
     * 1) get the user working diretory
     * 2) use equivalent of tar -xf xxx.rgp to that directory
     * 3) build a list of flac files with path to files
     * 4) run startFlacDecoder - this works!!!
     * 5) find the code to update the audio path in the rg file and run it.
     * 6) return to RosegardenMainWindow and correctly continue the processing
     */

    m_process = new QProcess;
    m_process->start("tar", QStringList() << "-xf" << m_filename);
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(unpackStep2(int, QProcess::ExitStatus)));

    // wait up to 30 seconds for process to start
    m_info->setText(tr("Running tar..."));
    if( !m_process->waitForStarted()) {
        puke(tr("Couldn't start tar."));
        return;
    }

    m_progress->setValue(50);
}

void
ProjectPackager::unpackStep2(int exitCode, QProcess::ExitStatus)
{
     if( exitCode == 0) {
       delete m_process;
    } else {
        puke(tr("Unable to extract files using tar."));
        return;
    }
    std::cout << "test 123" << std::endl;

    m_process = new QProcess;
    m_process->start("tar", QStringList() << "-tf " << m_filename << " > /tmp/rosegarden-filelist");
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(unpackStep3(int, QProcess::ExitStatus)));

    // wait up to 30 seconds for process to start
    m_info->setText(tr("Running tar again..."));
    if( !m_process->waitForStarted()) {
        puke(tr("Couldn't start tar."));
        return;
    }

    m_progress->setValue(60);
}

void
ProjectPackager::unpackStep3(int exitCode, QProcess::ExitStatus)
{
    if( exitCode == 0) {
       delete m_process;
    } else {
        puke(tr("Unable to obtain list of files using tar."));
        return;
    }

      QFile contents("/tmp/rosegarden-filelist");

    if (!contents.open(QIODevice::ReadOnly | QIODevice::Text)) {
        puke(tr("<qt>Unable to read to temporary file list.<br>Processing aborted.</qt>"));
        return;
    }

    QTextStream in1(&contents);
    QString line;
    QStringList files;
    while(true) {
        line = in1.readLine(1000);
        if( line.isEmpty()) break;
        if( line.find(".flac", 0) > 0) {
            files << line;
            std::cout << line.toStdString() << std::endl;
        }
    }
    contents.close();
//    contents.remove();
//    files << "zynfidel/rg-20050622-003944-7.wav.rgp.flac";
//    files << "zynfidel/C-chord.wav.rgp.flac";
//    files << "zynfidel/F-chord.wav.rgp.flac";
    startFlacDecoder( files);
}


void
ProjectPackager::startFlacDecoder(QStringList files)
{
    // we can't do a oneliner bash script straight out of a QProcess command
    // line, so we'll have to create a purpose built script and run that
    QString scriptName("/tmp/rosegarden-flac-decoder-backend");
    m_script.setName(scriptName);

    // remove any lingering copy from a previous run
    if (m_script.exists()) m_script.remove();

    if (!m_script.open(QIODevice::WriteOnly | QIODevice::Text)) {
        puke(tr("<qt>Unable to write to temporary backend processing script %1.<br>Processing aborted.</qt>"));
        return;
    }

    QTextStream out(&m_script);
    out << "# This script was generated by Rosegarden to combine multiple external processing"      << endl
        << "# operations so they could be managed by a single QProcess.  If you find this script"   << endl
        << "# it is likely that something has gone terribly wrong. See http://rosegardenmusic.com" << endl;

    QStringList::const_iterator si;
    int errorPoint = 1;
    int len;
    for (si = files.constBegin(); si != files.constEnd(); ++si) {
        std::string o1 = (*si).toLocal8Bit().constData();
        std::string o2;

        // the file strings are things like xxx.wav.rgp.flac
        // without specifying the output file they will turn into xxx.wav.rgp.wav
        // thus it is best to specify the output as xxx.wav
        len = o1.find(".wav");
        o2 = o1.substr(0,len+4);

        // we'll eschew anything fancy or pretty in this disposable script and
        // just write a command on each line, terminating with an || exit n
        // which can be used to figure out at which point processing broke, for
        // cheap and easy error reporting without a lot of fancy stream wiring
        out << "flac -d " <<  o1 << " -o " << o2 << " || exit " << errorPoint << endl;
//      out << "flac -d " <<  o1 << " -o " << o2 << endl;   // simple version
        // I would like to delete the flac file after conversion and add this to the scipt
        out << "rm " << o1 << endl;
        errorPoint++;
    }

    m_script.close();

    // run the assembled script
    m_process = new QProcess;
    // if setWorkingDirectory is needed at all, it should point to the
    // directory where the rgp file is located. So far, I don't see the need.
//    m_process->setWorkingDirectory(path);
    m_process->start("bash", QStringList() << scriptName);
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(updateAudioPath(int, QProcess::ExitStatus)));
}


// After checking, there is no need to update the audio path.
// It works perfectly well as is. Just remove the script.
void
ProjectPackager::updateAudioPath(int exitCode, QProcess::ExitStatus) {
    std::cout << "update Audio path " << exitCode << std::endl;
    if( exitCode == 0) {
        delete m_process;
    } else {
        puke(tr("Wasn't able to run flac"));
        return;
    }

    m_script.remove();
    accept();
    exitCode++; // break point
}
}

#include "ProjectPackager.moc"
