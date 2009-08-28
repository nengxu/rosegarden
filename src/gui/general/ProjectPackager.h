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

protected:
    RosegardenDocument *m_doc;
    int                 m_mode;
    QString             m_filename;
    QProgressBar       *m_progress;
    QLabel             *m_info;
    QProcess           *m_process;

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

protected slots:
    /**
     * Display an explanatory failure message and terminate processing
     */
    void puke(QString error);

    /**
     * Try to unpack an existing .rgd file, which will entail:
     *
     *   1. QProcess a tar xzf command
     *
     *   2. decompress the included FLAC files back to .wav
     *
     *   3. ???
     *
     */
    void runUnpack();

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
    void runPack();
};


}

#endif
