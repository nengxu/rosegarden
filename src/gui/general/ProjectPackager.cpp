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
#include "gui/widgets/FileDialog.h"
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
#include <QFileInfo>
#include <QDirIterator>

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
        m_filename(filename),
        m_trueFilename(filename)
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

QString
ProjectPackager::getTrueFilename()
{
    // get the path from the original m_filename, which is wherever the unpacked
    // .rgp file sat on disk, eg. /home/melvin/Documents/
    QFileInfo origFI(m_filename);
    QString dirname = origFI.path();

    std::cout << "ProjectPackager::getTrueFilename() - directory component is: " << dirname.toStdString() << std::endl;

    // get the filename component from the true m_trueFilename discovered while
    // unpacking the .rgp + extension (eg. foo.rgp yields bar.rg here)
    QFileInfo trueFI(m_trueFilename);
    QString basename = QString("%1.%2").arg(trueFI.baseName()).arg(trueFI.completeSuffix());

    std::cout << "                                          name component is: " << basename.toStdString() << std::endl;

    return QString("%1/%2").arg(dirname).arg(basename);
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

void
ProjectPackager::rmdir(const QDir dir, const QString &tmpDirName)
{
    if (dir.exists()) {
        QProcess rm;
        rm.start("rm", QStringList() << "-rf" << tmpDirName);
        rm.waitForStarted();
        std::cout << "process started: rm -rf " << qstrtostr(tmpDirName) << std::endl;
        rm.waitForFinished();
    }

// Bleargh, bollocks to this!  Using a QDirIterator is the right way to handle
// the possibility of there being extra unexpected subdirectories full of files,
// but there are sort order issues and all manner of other ills.  While we live
// on Linux, let's just say the hell with it and do an rm -rf
//
//    if (dir.exists()) {
//        // first find and remove all the files
//        QDirIterator fi(dir.path(), QDir::Files, QDirIterator::Subdirectories);
//        while (fi.hasNext()) {
//            std::cout << "rm " << fi.next().toStdString() << (QFile::remove(fi.next()) ? "OK" : "FAILED") << std::endl;
//        }
//
//        // then clean up the empty directories
//        QDirIterator di(dir.path(), QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
//        while (di.hasNext()) {
//            QDir d;
//            std::cout << "rmdir: " << di.next().toStdString() << (d.remove(di.next()) ? "OK" : "FAILED") << std::endl;
//        }
//    }
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
    if (!m_process->waitForStarted()) {
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
        puke(tr("<qt><p>The <b>flac</b> command was not found.</p><p>FLAC is a lossless audio compression format used to reduce the size of Rosegarden project packages with no loss of audio quality.  Please install FLAC and try again.  This utility is typically available to most distros as a package called \"flac\".</p>"));
        return;
    }

    switch (m_mode) {
        case ProjectPackager::Unpack:  runUnpack(); break;
        case ProjectPackager::Pack:    runPack();   break;
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
    QString audioPath = strtoqstr(manager->getAudioPath());

    // the base tmp directory where we'll assemble all the files
    QString tmpDirName = QString("%1/rosegarden-project-packager-tmp").arg(QDir::homePath());

    // the data directory where audio and other files will go
    QFileInfo fi(m_filename);
    QString dataDirName = fi.baseName();

    std::cout << "using tmp data directory: " << tmpDirName.toStdString() << "/" << dataDirName.toStdString() << std::endl;

    QDir tmpDir(tmpDirName);

    // if the directory already exists, just hose it
    rmdir(tmpDir, tmpDirName);

    // make the temporary working directory
    if (tmpDir.mkdir(tmpDirName)) {
        QFileInfo fi(m_filename);
        // QFileInfo::baseName() given /tmp/foo/bar/rat.rgp returns rat
        //
        // m_filename comes in already having an .rgp extension, but the file
        // was saved .rg
        QString oldName = QString("%1/%2.rg").arg(fi.path()).arg(fi.baseName());
        QString newName = QString("%1/%2.rg").arg(tmpDirName).arg(fi.baseName());
        std::cout << "cp " << oldName.toStdString() << " " << newName.toStdString() << std::endl;

        // copy m_filename(.rgp) as $tmp/m_filename.rg
        QFile::copy(oldName, newName);

        // remove the original file
        //
        // (well no, let's save that for final cleanup in the very end stage)
//        QFile::remove(m_filename);
    } else {
        puke(tr("<qt>Could not create temporary working directory.<br>Processing aborted!</qt>"));
        return;
    }

    // make the data subdir
    tmpDir.mkdir(dataDirName);    


    // copy the audio files (do not remove the originals!)
    QStringList::const_iterator si;
    for (si = audioFiles.constBegin(); si != audioFiles.constEnd(); ++si) {
    
        QString srcFile = QString("%1/%2").arg(audioPath).arg(*si);
        QString dstFile = QString("%1/%2/%3").arg(tmpDirName).arg(dataDirName).arg(*si);
        QString dstFilePk = QString("%1.pk").arg(dstFile);

        std::cout << "cp " << srcFile.toStdString() << " " << dstFile.toStdString() << std::endl;
        std::cout << "cp " << srcFile.toStdString() << " " << dstFilePk.toStdString() << std::endl;
        QFile::copy(srcFile, dstFile);
        QFile::copy(srcFile, dstFilePk);

        // we should update the progress bar in some pleasant way here based on
        // total files, but I don't feel like sorting that out
    }

    // deal with adding any extra files
    QStringList extraFiles;

    QMessageBox::StandardButton reply = QMessageBox::information(this,
            tr("Rosegarden"),
            tr("<qt><p>Rosegarden can add any number of extra files you may desire to a project package.  For example, you may wish to include an explanatory text file, a bank definition for ZynAddSubFX, or perhaps some cover art.  It's all up to you!</p><p>Would you like to include any additional files?</p></qt>"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    while (reply == QMessageBox::Yes) {

        // it would take some trouble to make the last used paths thing work
        // here, where we're building a list of files from potentially anywhere,
        // so we'll just use the open_file path as it was last set elsewhere,
        // and leave it at that until somebody complains
        QSettings settings;
        settings.beginGroup(LastUsedPathsConfigGroup);
        QString directory = settings.value("open_file", QDir::homePath()).toString();
        settings.endGroup();

        extraFiles <<  FileDialog::getOpenFileNames(this, "Open File", directory, tr("All files") + " (*)", 0, 0);
        
        reply =  QMessageBox::information(this,
                tr("Rosegarden"),
                tr("<qt><p>Would you like to include any additional files?</p></qt>"),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    }

    // copy the extra files (do not remove the originals!)
    // (iterator previously declared)
    for (si = extraFiles.constBegin(); si != extraFiles.constEnd(); ++si) {
    
        // each QStringList item from the FileDialog will include the full path
        QString srcFile = (*si);

        // so we cut it up to swap the source dir for the dest dir while leaving
        // the complete filename stuck on the end
        QFileInfo efi(*si);
        QString basename = QString("%1.%2").arg(efi.baseName()).arg(efi.completeSuffix());
        QString dstFile = QString("%1/%2/%3").arg(tmpDirName).arg(dataDirName).arg(basename);

        std::cout << "cp " << srcFile.toStdString() << " " << dstFile.toStdString() << std::endl;
        QFile::copy(srcFile, dstFile);

        // we should update the progress bar in some pleasant way here based on
        // total files, but I don't feel like sorting that out
    }

    // and now we have everything discovered, uncovered, added, smothered,
    // scattered and splattered, and we're ready to pack the flac files and
    // get the hell out of here!
    startFlacEncoder(audioPath, audioFiles, tmpDirName, dataDirName);
}

void
ProjectPackager::startFlacEncoder(QString path, QStringList files, const QString &tmpDirName, const QString &dataDirName)
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
    //foo42
    // The working directory must be the key to why tar is not failing, but
    // failing to do anything detectable.  Let's cut apart m_filename...
    QFileInfo fi(m_filename);
    QString dirname = fi.path();
    QString basename = QString("%1.%2").arg(fi.baseName()).arg(fi.completeSuffix());


    QTextStream out(&m_script);
    out << "# This script was generated by Rosegarden to combine multiple external processing"      << endl
        << "# operations so they could be managed by a single QProcess.  If you find this script"   << endl
        << "# it is likely that something has gone terribly wrong. See http://rosegardenmusic.com" << endl;

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
        out << "flac " << dataDirName.toLocal8Bit() << "/" <<  o << " || exit " << errorPoint << endl;
        errorPoint++;
    }

    // Throw tar on the ass end of this script and save an extra processing step
    //
    // first cheap trick, dataDirName.rg is our boy and we know it
    QString rgFile = QString("%1.rg").arg(dataDirName);

    // second cheap trick, don't make a tarball in tmpdir and move it, just
    // write it at m_filename and shazam, nuke the tmpdir behind us and peace out
    out << "tar czf \"" << m_filename << "\" " << rgFile.toLocal8Bit() << " " <<  dataDirName.toLocal8Bit() <<  "/ || exit " << errorPoint++ << endl;

    m_script.close();
return;
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


///////////////////////////
//                       //
// UNPACKING OPERATIONS  //
//                       //
///////////////////////////
void
ProjectPackager::runUnpack()
{
    std::cout << "ProjectPackager::runUnpack() - unpacking " << qstrtostr(m_filename) << std::endl;
    m_info->setText(tr("Unpacking project..."));

    // The first chain of the operation running tar xf m_filename always
    // executed successfully and returned exit status 0, but never produced any
    // files on disk for some reason.  I don't know why.  I never could find
    // where they had ever been written anywhere.  So I give up.  We'll defer
    // unpacking the files and shove that into the backend decoder script.  We
    // don't need to unpack the thing to list its contents anyway.

    m_process = new QProcess;

    // We can't assume foo.rgp actually contains foo.rg, it could
    // contain bar.rg and bar/ if the user was evil, and users tend to be.
    //
    // So while there are other ways to get here, Ilan had already written all
    // of this code to process a text file, and we'll just go that route.
    QString ofile("/tmp/rosegarden-project-package-filelist");

    // merge stdout and sterr for laziness of debugging (any errors in here mean
    // bad news)
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    // equivalent of [command] > ofile
    m_process->setStandardOutputFile(ofile, QIODevice::Truncate);

    // This is a very fast operation, just listing the files in a tarball
    // straight to a file on disk without involving a terminal, so we
    // will do this one as a waitForFinished() and risk blocking here
    //
    // (note that QProcess apparently handles escaping any spaces &c. in
    // m_filename here)
    m_process->start("tar", QStringList() << "tf" << m_filename);
    m_process->waitForStarted();
    std::cout << "process started: tar tf " << qstrtostr(m_filename) << std::endl;
    m_process->waitForFinished();

    if (m_process->exitCode() == 0) {
       delete m_process;
    } else {
        puke(tr("<qt>Unable to obtain list of files using tar.  Process exited with status code %1</qt>").arg(m_process->exitCode()));
        return;
    }

    QFile contents(ofile);

    if (!contents.open(QIODevice::ReadOnly | QIODevice::Text)) {
        puke(tr("<qt>Unable to read to temporary file list.<br>Processing aborted.</qt>"));
        return;
    }

    QTextStream in1(&contents);
    QString line;
    QStringList files;
    while (true) {
        line = in1.readLine(1000);
        if (line.isEmpty()) break;
        if (line.find(".flac", 0) > 0) {
            files << line;
            std::cout << "Discovered for decoding: " <<  line.toStdString() << std::endl;
        } else if (line.find(".rg", 0) > 0) {
            m_trueFilename = line;
            std::cout << "Discovered true filename: " << m_trueFilename.toStdString() << std::endl;
        }

    }
    contents.remove();

    QString completeTrueFilename = getTrueFilename();

    QFileInfo fi(completeTrueFilename);
    if (fi.exists()) {
        QMessageBox::StandardButton reply =  QMessageBox::critical(this,
                tr("Rosegarden - Fatal processing error!"),
                tr("<qt><p>It appears that you have already unpacked this project package.</p><p>Would you like to load %1 now?</p></qt>").arg(completeTrueFilename),
                QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);

        if (reply == QMessageBox::Ok) {
            // If they choose Ok, we'll accept() here to abort processing and
            // tell RosegardenMainWindow to load m_trueFilename
            accept();
        } else {
            reject();
        }
     } else {
         startFlacDecoder(files);
     }
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
        puke(tr("<qt>Unable to write to temporary backend processing script %1.<br>Processing aborted.</qt>").arg(scriptName));
        return;
    }

    QTextStream out(&m_script);
    out << "# This script was generated by Rosegarden to combine multiple external processing"      << endl
        << "# operations so they could be managed by a single QProcess.  If you find this script"   << endl
        << "# it is likely that something has gone terribly wrong. See http://rosegardenmusic.com" << endl;

    int errorPoint = 1;

    // The working directory must be the key to why tar is not failing, but
    // failing to do anything detectable.  Let's cut apart m_filename...
    QFileInfo fi(m_filename);
    QString dirname = fi.path();
    QString basename = QString("%1.%2").arg(fi.baseName()).arg(fi.completeSuffix());

    // There were mysterious stupid problems running tar xf in a separate
    // QProcess step, so screw it, let's just throw it into this script!
    out << "tar xzf \"" << basename << "\" || exit " << errorPoint++ << endl;

    QStringList::const_iterator si;
    int len;
    for (si = files.constBegin(); si != files.constEnd(); ++si) {
        std::string o1 = (*si).toLocal8Bit().constData();
        std::string o2;

        // the file strings are things like xxx.wav.rgp.flac
        // without specifying the output file they will turn into xxx.wav.rgp.wav
        // thus it is best to specify the output as xxx.wav
        len = o1.find(".wav");
        o2 = o1.substr(0, len + 4);

        // we'll eschew anything fancy or pretty in this disposable script and
        // just write a command on each line, terminating with an || exit n
        // which can be used to figure out at which point processing broke, for
        // cheap and easy error reporting without a lot of fancy stream wiring
        //
        // (let's just try escaping spaces &c. with surrounding " and see if
        // that is good enough)
        out << "flac -d \"" <<  o1 << "\" -o \"" << o2 << "\" && rm \"" << o1 <<  "\" || exit " << errorPoint << endl;
        errorPoint++;
    }

    m_script.close();

    // run the assembled script
    m_process = new QProcess;

    // set to the working directory extracted from m_filename above, as this is
    // was apparently the reason why tar always failed to do anything
    m_process->setWorkingDirectory(dirname);
    m_process->start("bash", QStringList() << scriptName);
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(finishUnpack(int, QProcess::ExitStatus)));

    // wait up to 30 seconds for process to start
    m_info->setText(tr("Decoding audio files..."));
    if (!m_process->waitForStarted()) {
        puke(tr("<qt>Could not start backend processing script %1.</qt>").arg(scriptName));
        return;
    }
}


// After checking, there is no need to update the audio path.
// It works perfectly well as is. Just remove the script.
//
// (Confirmed here.  Import /tmp/foo.rgp to zynfidel.rg and live audio file path
// is /tmp/zynfidel so this test passes and the extra hackery has been removed)
void
ProjectPackager::finishUnpack(int exitCode, QProcess::ExitStatus) {
    std::cout << "ProjectPackager::finishUnpack - exit code: " << exitCode << std::endl;

    if (exitCode == 0) {
        delete m_process;
    } else {
        puke(tr("<qt>Extracting and decoding files failed with exit status %1. Checking %2 for the line that ends with \"exit %1\" may be useful for diagnostic purposes.<br>Processing aborted.</qt>").arg(exitCode).arg(m_script.fileName()));
        return;
    }

    m_script.remove();
    accept();
    exitCode++; // break point
}


}

#include "ProjectPackager.moc"
