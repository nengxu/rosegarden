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

#ifndef _PROJECT_PACKAGER_H_
#define _PROJECT_PACKAGER_H_

#include "document/RosegardenDocument.h"

#include <QDialog>
#include <QProgressBar>
#include <QLabel>
#include <QProcess>
#include <QStringList>


namespace Rosegarden
{

/** Implement functionality equivalent to the old external
 *  rosegarden-project-package script.  The script used the external dcop and
 *  kdialog command line utlities to provide a user interface.  We'll do the
 *  user interface in real code, though we still have to use external helper
 *  applications for various purposes to make the thing run.
 *
 *  \author D. Michael McIntyre
 */

class ProjectPackager : public QDialog
{
    Q_OBJECT

public:
    /** The old command line arguments are replaced with an int passed into the
     * ctor, using the following named constants to replace them, and avoid a
     * bunch of string parsing nonsense.  We no longer need a startup ConfTest
     * target.  We'll do the conftest every time we run instead, and only
     * complain if there is a problem.  We no longer need a version target,
     * since the version is tied to Rosegarden itself.
     *
     * The filename parameter should be a temporary file set elsewhere and
     * passed in.
     */
    static const int ConfTest  = 0;
    static const int Pack      = 1;
    static const int Unpack    = 2;

    ProjectPackager(QWidget *parent,
                    RosegardenDocument *document,
                    int mode,
                    QString filename);
    ~ProjectPackager() { };

    /** Return the true filename as discovered when analyzing the contents of
     * the .rgp file.  foo.rgp might contain bar.rg and directory bar/
     */
    QString getTrueFilename();

protected:
    RosegardenDocument *m_doc;
    int                 m_mode;
    QString             m_filename;
    QProgressBar       *m_progress;
    QLabel             *m_info;
    QProcess           *m_process;
    
    /// The backend script has to be accessed from multiple locations
    QFile               m_script;

    /** The real filename contained within the project package.  It is necessary
     * to discover and transmit this because foo.rgp might really contain bar.rg
     * and data files in bar/ if the user ever renamed the file for some reason
     */
    QString             m_trueFilename;


    /** Returns a QStringList containing a sorted|uniqed list of audio files
     * used by m_doc
     *
     * Problems to solve: how do we |sort|uniq a QStringList?  The same audio
     * file might be used by hundreds of audio segments (eg. Emergence) and
     * while we could pull it out and overwrite the same file 100 times to
     * result in only one final copy, that's a big waste.
     */
    QStringList getAudioFiles();

/* General questions not resolved yet:
 *
 * When do we change the audio file path from whatever it was to the new one
 * we're creating?  At pack time or unpack time?  Did the old script even do
 * this?
 *
 * I suppose we could do it in both places.  Harmless enough isn't it?  No,
 * scratch that, do it when we pack, because we're packing with a live document,
 * but doing it on an unpack requires...  Well either code hooks (signals and
 * slots?) or some XML hacking.
 *
 */

    // to avoid troubles, check that flac is available. It is fast and could
    // possibly avoid troubles.
    void sanityCheck();


protected slots:
    /**
     * Display an explanatory failure message and terminate processing
     */
    void puke(QString error);

    /**
     * Begin the packing process, which will entail:
     *
     *   1. discover audio files used by the composition
     *
     *   2. pull out copies to a tmp directory
     *
     *   3. compress audio files with FLAC (preferably using the library,
     *   although that is quite hopeless for the moment, due to the assert()
     *   conflict)
     *
     *   4. prompt user for additional files
     *
     *   5. add them
     *
     *   6. final directory structure looks like:
     *
     *       ./export_filename.rg
     *       ./export_filename/[wav files compressed with FLAC]
     *       ./export_filename/[misc files]
     *
     *   7. tarball this (tar czf)
     *
     *   8. rename it to .rgp from .tar.gz
     *
     *   9. ???
     */
     // run this after the sanity check
    void runPackUnpack(int exitCode, QProcess::ExitStatus);

    void runPack();

    /** Create a temporary working directory and assemble copied files to that
     * location for further processing.  The copy operations could take some
     * time, so we probably need a QProcess slot for this, and assemble the
     * various copy operations into one backend script to execute from that
     * QProcess.
     *
     * When complete, trigger startFlacEncoder()
     */
    void assembleFiles(QString path, QStringList files);

    /** Assemble the various flac encoding operations into one backend script to
     * operate from a single QProcess, so this chewing can take place without
     * blocking the GUI.  Since it's possible to track the total number of
     * files, and what file out of n we're on, we should work out some nice way
     * to hook this up to m_status and give some indication of progress,
     * instead of just leaving it in "busy" mode.
     *
     * When complete, trigger promptAdditionalFiles()
     */
    void startFlacEncoder(QString path, QStringList files);

    // flac may leave the original .wav files intact, in which case we need to
    // clean them up, but we can probably build that into the purpose-built
    // script created by the preceding chain link without having to insert
    // another one here.  Probably.

    /** Prompt the user for any additional files they wish to include in the
     * project package.  This will require a bit of thought.  Once the list is
     * assembled, copy the files to the temporary working location.
     *
     * When complete, trigger compressPackage()
     */
    void promptAdditionalFiles();

    /** Turn the whole assembled shebang into a completed .rgp file.  If this
     * would require more than one QProcess, perhaps do another temporary
     * backend script, since we will have had so many of them by then, what the
     * hell.
     *
     * We can probably arrange it so this is the final link in the chain, and
     * signals the end of the pack operation.
     */
    void compressPackage();

    /** The first stage of unpacking an .rgp file:
     *
     * 1. QProcess out a tar tf to obtain a list of files from the .rgp tarball
     *
     * 2. Comb this assembled list for .flac files
     *
     * 3. Pass this list to the decoder backend
     */
    void runUnpack();

    /** Assemble a script to:
      *
      * 1. Actually unpack the tarball (tar xzf)
      *
      * 2. Decode the .flac files and remove them
      */
    void startFlacDecoder(QStringList files);

    /** Final unpack stage
     */
    void finishUnpack(int exitCode, QProcess::ExitStatus);
};


}

#endif
