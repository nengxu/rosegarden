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


    switch (mode) {
        case ProjectPackager::Unpack:  runUnpack();    break;
        case ProjectPackager::Pack:    runPack();  break;
    }
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

#ifdef COME_BACK_LATER
    QString tmpDir("~/rosegarden-project-packager-tmp");
    QString dataDir("");
    if (tmpDir.exists()) {
        // If the directory exists, it's left over from an aborted previous run.
        // Should we clean it up silently, or warn and abort?  At this stage in
        // development, let's ignore it and carry on
    } // else

    // make the temporary working directory
    if (tmpDir.mkdir()) {
        // copy m_filename
    } else {
        puke(tr("<qt>Could not create temporary working directory %1.<br>Processing aborted!</qt>"));
        return;
    }

#endif


    /* 1. find suitable place to write a tmp directory (should it be /tmp or
     * under ~ somewhere, eg. ~/tmp or maybe even a Qt class can figure it out
     * so we don't have to)
     *
     * THOUGHTS: let's use ~/rosegarden-project-packager-tmp and warn if it
     * isn't empty when we start, etc.  We'll want to do this in userland in
     * case they have a partitioning scheme that limits system disk usage.  Most
     * users don't bother with this kind of thing these days, but I do, and I
     * can't be alone.
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
    QFile script(scriptName);

    // remove any lingering copy from a previous run
    if (script.exists()) script.remove();

    if (!script.open(QIODevice::WriteOnly | QIODevice::Text)) {
        puke(tr("<qt>Unable to write to temporary backend processing script %1.<br>Processing aborted.</qt>"));
        return;
    }

    QTextStream out(&script);
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

    script.close();

    // run the assembled script
    m_process = new QProcess;
    m_process->setWorkingDirectory(path);
    m_process->start("bash", QStringList() << scriptName);
//    m_process->waitForFinished();  (just testing to make sure the script
//    actually ate up time as intended for proof of concept purposes)
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(runTar(int, QProcess::ExitStatus)));
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
    return;

    /* As far as I can see the process should be:
     * 1) get the user working diretory
     * 2) use equivalent of tar -xf xxx.rgp to that directory
     * 3) build a list of flac files with path to files
     * 4) run startFlacDecoder - this works!!!
     * 5) find the code to update the audio path in the rg file and run it.
     * 6) return to RosegardenMainWindow and correctly continue the processing
     */
    // I'll generate some test strings here
    QString path = "/home/ilan/project_packager_rewrite/zynfidel/temp/";
    QStringList files;
    files << "rg-20050622-003944-7.wav.rgp.flac";
    files << "C-chord.wav.rgp.flac";
    files << "F-chord.wav.rgp.flac";
    startFlacDecoder(path, files);

/*    if (m_process->exitCode() == 0) {
        m_info->setText(tr("<b>convert-ly</b> finished..."));
        delete m_process;
    } else {
        puke(tr("<qt><p>Ran <b>convert-ly</b> successfully, but it terminated with errors.</p><p>Processing terminated due to fatal errors.</p></qt>"));
    }

    m_progress->setValue(50);

    m_process = new QProcess;
    m_info->setText(tr("Running <b>lilypond</b>..."));
    m_process->start("lilypond", QStringList() << "--pdf" << m_filename);
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(runFinalStage(int, QProcess::ExitStatus)));
            

    if (m_process->waitForStarted()) {
        m_info->setText(tr("<b>lilypond</b> started..."));
    } else {
        puke(tr("<qt><p>Could not run <b>lilypond</b>!</p><p>Please install LilyPond and ensure that the \"convert-ly\" and \"lilypond\" commands are available on your path.  If you perform a <b>Run Command</b> (typically <b>Alt+F2</b>) and type \"lilypond\" into the box, you should not get a \"command not found\" error.  If you can do that without getting an error, but still see this error message, please consult <a href=\"mailto:rosegarden-user@lists.sourceforge.net>rosegarden-user@lists.sourceforge.net</a> for additional help.</p><p>Processing terminated due to fatal errors.</p></qt>"));
    }

    // go into Knight Rider mode when chewing on LilyPond, because it can take
    // an eternity, but I don't really want to re-create all the text stream
    // monitoring and guessing code that's easy to do in a script and hell to do
    // in real code
    m_progress->setMaximum(0);*/
}
/*
void
ProjectPackager::runFinalStage(int exitCode, QProcess::ExitStatus)
{
    if (exitCode == 0) {
        m_info->setText(tr("<b>lilypond</b> finished..."));
        delete m_process;
    } else {

        // read preferences from last export from QSettings to offer clues what
        // failed
        QSettings settings;
        settings.beginGroup(LilyPondExportConfigGroup);
        bool exportedBeams = settings.value("lilyexportbeamings", false).toBool();
        bool exportedBrackets = settings.value("lilyexportstaffbrackets", false).toBool();
        settings.endGroup();

        std::cout << "  finalStage: exportedBeams == " << (exportedBeams ? "true" : "false") << std::endl
                  << " exportedBrackets == " << (exportedBrackets ? "true" : "false") << std::endl;

        QString vomitus = QString(tr("<qt><p>Ran <b>lilypond</b> successfully, but it terminated with errors.</p>"));

        if (exportedBeams) {
            vomitus += QString(tr("<p>You opted to export Rosegarden's beaming, and LilyPond could not process the file.  It is likely that you performed certain actions in the course of editing your file that resulted in hidden beaming properties being attached to events where they did not belong, and this probably caused LilyPond to fail.  The recommended solution is to either leave beaming to LilyPond (whose automatic beaming is far better than Rosegarden's) and un-check this option, or to un-beam everything and then re-beam it all manually inside Rosgarden.  Leaving the beaming up to LilyPond is probaby the best solution.</p>"));
        }

        if (exportedBrackets) {
            vomitus += QString(tr("<p>You opted to export staff group brackets, and LilyPond could not process the file.  Unfortunately, this useful feature can be very fragile.  Please go back and ensure that all the brackets you've selected make logical sense, paying particular attention to nesting.  Also, please check that if you are working with a subset of the total number of tracks, the brackets on that subset make sense together when taken out of the context of the whole.  If you have any doubts, please try turning off the export of staff group brackets to see whether LilyPond can then successfully render the result.</p>"));
        }

        vomitus += QString(tr("<p>Processing terminated due to fatal errors.</p></qt>"));

        puke(vomitus);

        // puke doesn't actually work, so we have to return in order to avoid
        // further processing
        return;
    }

    QString pdfName = m_filename.replace(".ly", ".pdf");

    // retrieve user preferences from QSettings
    QSettings settings;
    settings.beginGroup(ExternalApplicationsConfigGroup);
    int pdfViewerIndex = settings.value("pdfviewer", 0).toUInt();
    int filePrinterIndex = settings.value("fileprinter", 0).toUInt();
    settings.endGroup();

    QString pdfViewer, filePrinter;

    // assumes the PDF viewer is available in the PATH; no provision is made for
    // the user to specify the location of any of these explicitly, and I'd like
    // to avoid having to go to that length if at all possible, in order to
    // reduce complexity both in code and on the user side of the configuration
    // page (I guess arguably the configuration page shouldn't exist, and we
    // should just try things sequentially until something works, but it gets
    // into real headaches trying to guess what someone would prefer based on
    // what desktop they're running, and anyway specifying explicitly avoids the
    // reason why my copy of acroread is normally chmod -x so the script
    // ancestor of this class wouldn't pick it up against my wishes)
    switch (pdfViewerIndex) {
        case 0: pdfViewer = "okular";   break;
        case 1: pdfViewer = "evince";   break;
        case 2: pdfViewer = "acroread"; break;
        case 3: pdfViewer = "kpdf"; 
        default: pdfViewer = "kpdf"; // just because I'm still currently on KDE3
    }

    switch (filePrinterIndex) {
        case 0: filePrinter = "kprinter"; break;
        case 1: filePrinter = "gtklp";    break;
        case 2: filePrinter = "lpr";      break;
        case 3: filePrinter = "lp";       break;
        case 4: filePrinter = "hp-print"; break;
        default: filePrinter = "lpr";     break;
    }

    // So why didn't I just manipulate finalProcessor in the first place?
    // Because I just thought of that, but don't feel like refactoring all of
    // this yet again.  Oh well.
    QString finalProcessor;

    m_process = new QProcess;

    switch (m_mode) {
        case ProjectPackager::Print:
            m_info->setText(tr("Printing %1...").arg(pdfName));
            finalProcessor = filePrinter;
            break;

        // just default to preview (I always use preview anyway, as I never
        // trust the results for a direct print without previewing them first,
        // and in fact the direct print option seems somewhat dubious to me)
        case ProjectPackager::Preview:
        default:
            m_info->setText(tr("Previewing %1...").arg(pdfName));
            finalProcessor = pdfViewer;
    }

    m_process->start(finalProcessor, QStringList() << pdfName);
    if (m_process->waitForStarted()) {
        QString t = QString(tr("<b>%1</b> started...").arg(finalProcessor));
    } else {
        QString t = QString(tr("<qt><p>LilyPond processed the file successfully, but <b>%1</b> did not run!</p><p>Please configure a valid %2 under <b>Settings -> Configure Rosegarden -> General -> External Applications</b> and try again.</p><p>Processing terminated due to fatal errors.</p></qt>")).arg(finalProcessor).arg(
                (m_mode == ProjectPackager::Print ? tr("file printer") : tr("PDF viewer")));
        puke(t);
    }

    m_progress->setMaximum(100);
    m_progress->setValue(100);

    accept();
} */

void
ProjectPackager::runFlacDecoder()
{
}

void
ProjectPackager::startFlacDecoder(QString path, QStringList files)
{
    // we can't do a oneliner bash script straight out of a QProcess command
    // line, so we'll have to create a purpose built script and run that
    QString scriptName("/tmp/rosegarden-flac-decoder-backend");
    QFile script(scriptName);

    // remove any lingering copy from a previous run
    if (script.exists()) script.remove();

    if (!script.open(QIODevice::WriteOnly | QIODevice::Text)) {
        puke(tr("<qt>Unable to write to temporary backend processing script %1.<br>Processing aborted.</qt>"));
        return;
    }

    QTextStream out(&script);
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
//        out << "flac -d " <<  o1 << " -o " << o2 << endl;
        // I would like to delete the flac file after conversion and add this to the
        // script, but I don't know the command, something like
        // out << "delete " << o1 << endl;
        // would you agree to doing this?
        errorPoint++;
    }

    script.close();

    // run the assembled script
    m_process = new QProcess;
    m_process->setWorkingDirectory(path);
    m_process->start("bash", QStringList() << scriptName);
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(updateAudioPath(int, QProcess::ExitStatus)));
}


void
ProjectPackager::updateAudioPath(int exitCode, QProcess::ExitStatus) {
    std::cout << "update Audio path " << exitCode << std::endl;
    if( exitCode == 0) {
        delete m_process;
    } else {
        puke(tr("Wasn't able to run flac"));
        return;
    }
    exitCode++; // break point
}
}

#include "ProjectPackager.moc"
