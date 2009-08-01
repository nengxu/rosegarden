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

#ifndef _WARNING_WIDGET_H_
#define _WARNING_WIDGET_H_

#include <QWidget>


namespace Rosegarden
{


/** Fed with data from the sequence manager, this widget displays the status of
 * various environmental runtime factors, and provides detailed information to
 * users when sub-optimal runtime conditions are detected.  This mechanism
 * replaces the tedious parade of startup warning dialogs we used to dump in the
 * user's face the first time they tried to run Rosegarden on an ordinary
 * everyday desktop-oriented distro.
 *
 * \author D. Michael Mcintyre
 */
class WarningWidget : public QWidget
{
    Q_OBJECT
public:
    /** Constructs a WarningWidget.  The status of various warning conditions
     * must be determined before creating this object, and no provision is
     * currently made (or envisioned) for changing this status after creation.
     * All of these conditions require restarting Rosegarden, even if it might
     * be possible to correct some of these problems without making changes that
     * involve rebooting the entire system (for example, loading the
     * snd-rtctimer kernel module happens to work)
     *
     * \a showMidiWarning   - the MIDI subsystem is broken
     * \a showAudioWarning  - the audio subsystem is broken (ie. JACK can't
     *                         start)
     * \a showTimerWarning  - user has a stock distro kernel with a busted timer
     */
    WarningWidget(bool showMidiWarning,
                  bool showAudioWarning,
                  bool showTimerWarning
                 );
    ~WarningWidget();

    //virtual QSize sizeHint() const;

protected:
};


}

#endif
