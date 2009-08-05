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

#include "WarningWidget.h"

#include "gui/general/IconLoader.h"
#include "misc/Strings.h"

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QToolButton>

#include <iostream>
namespace Rosegarden
{

WarningWidget::WarningWidget() :
        QWidget(),
        m_text(""),
        m_informativeText("")
{
    std::cerr << "WarningWidget()" << std::endl;

    setContentsMargins(0, 0, 0, 0);
    
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    QHBoxLayout *layout = new QHBoxLayout();
    setLayout(layout);

    m_midiIcon = new QLabel();
    layout->addWidget(m_midiIcon);
    m_midiIcon->hide();

    m_audioIcon = new QLabel();
    layout->addWidget(m_audioIcon);
    m_audioIcon->hide();

    m_timerIcon = new QLabel();
    layout->addWidget(m_timerIcon);
    m_timerIcon->hide();

    m_warningButton = new QToolButton();
    layout->addWidget(m_warningButton);
    m_warningButton->setIconSize(QSize(16, 16));
    m_warningButton->setIcon(IconLoader().loadPixmap("warning"));
//    m_warningButton->hide();

    // It might be nice to put something in here to either display the audio
    // file path (in a tooltip, perhaps) or even offer a shortcut for changing
    // this well buried but important piece of information.  Come back to that
    // thought later.
    std::cerr << "WARNING WIDGET HEIGHT " << this->height() << std::endl;

}

void
WarningWidget::setMidiWarning(const bool status)
{
    if (status) {
        m_midiIcon->hide();
//        m_midiIcon->setToolTip(tr("No MIDI driver!"));
        m_warningButton->show();
    } else {
        m_midiIcon->setPixmap(IconLoader().loadPixmap("midi-ok"));
        m_midiIcon->show();
        m_midiIcon->setToolTip(tr("MIDI OK"));
//        m_warningButton->hide();
    }
}

void
WarningWidget::setAudioWarning(const bool status)
{
    if (status) {
        m_audioIcon->hide();
//        m_audioIcon->setToolTip(tr("No audio driver!"));
        m_warningButton->show();
    } else {
        m_audioIcon->setPixmap(IconLoader().loadPixmap("audio-ok"));
        m_audioIcon->show();
        m_audioIcon->setToolTip(tr("audio OK"));
//        m_warningButton->hide();
    }
}

void
WarningWidget::setTimerWarning(const bool status)
{
    if (status) {
        m_timerIcon->hide();
//        m_timerIcon->setToolTip(tr("Insufficient timer resolution!"));
        m_warningButton->show();
    } else {
        m_timerIcon->setPixmap(IconLoader().loadPixmap("timer-ok"));
        m_timerIcon->show();
        m_timerIcon->setToolTip(tr("timer OK"));
//        m_warningButton->hide();
    }
}

void
WarningWidget::setMessage(const QString text, const QString informativeText)
{
    std::cerr << "WarningWidget::setMessage(" << qstrtostr(text)
              << ", " << qstrtostr(informativeText)
              << ")" << std::endl;
    m_warningButton->show();
}

WarningWidget::~WarningWidget()
{
}

// NOTES:
//
// I just had the idea to do a little QProcess paint by numbers deal like the
// LilyPondProcessor as the backend for two new features.  Instead of having all
// that garbage about sudo modprobe blah blah blah, have something like
//
// "I can try to fix this for you.  Blah blah blah.  Rosegarden must restart
// now."
//
// Probably keep track of attempted_to_fix_midi and attempted_to_fix_audio so we
// don't encourage the user to keep hitting the fix it button repeatedly to see
// if maybe it will eventually work.  In practice, on Jaunty, there is no more
// alsaconf and that recommendation is pointless.  About all you can try to do
// is modprobe snd-seq-midi.  The snd-rtctimer whatever does not work either.
//
// FOLLOWUP TO SELF:
//
// No, we wouldn't want to prevent trying it again, because it could be going
// from boot to boot, not just run to run.
//
// QProcessing out sudo seems like a tall enough order that when mixed with the
// low probability of any of these attempts actually working, it's probably time
// to dump this idea and not get carried away.
//
// Likewise the problem of "rebooting" Rosegarden, although this is less tricky
// to get right.
//
// Sleep on it though.  Who knows.
//
// Also, additional thought, on my default out of the box, JACK won't start with
// -R, so if I do all of the above, another nugget to try is writing a different
// jackdrc without -R 
//
// This leads to thoughts about having different kinds of MIDI and audio
// warnings, and keeping up with them.  Probably leave the simple yes/no for
// MIDI and audio warnings, but have some kind of function to attach different
// messages to the /!\ warning icon, and some things to add to the list are:
//
// * Chris's newly refactored autoconnect logic couldn't find a plausible looking
// thing to talk to, so you probably need to run QSynth now &c.
//
// * ??
//
//
// Wrap-up thoughts for today 8-1-2009
//
// The hasGoodMidi() etc. stuff is a mixed bag.  It's redundant and there's
// already working, more verbose code to do two of the three jobs, so I should
// probably just pull all of that out.
//
// SeqMan::checkDriverStatus(true) or whatever is largely if not completely
// redundant, and should probably be cleaned up all the way out of existence
//
// The timer stuff isn't as cut and dried, because this condition is not
// detected in a static way.  There will need to be a mechanism for catching
// this and setting the warning at that time.
//
// I suppose the easiest way to do that is to just do it where it was originally
// done, using the original mechanism where AlsaDriver threw a
// SendMessage::YourShitIsWhackDawg and SeqMan caught it and puked up a big
// blather of text.
//
// Maybe pass SequenceManager a pointer to the WarningWidget so SequenceManager
// can manipulate it directly, and save a translation layer.  That could work.
// That leaves all the error texts back in SequenceManager where they started,
// and probably makes translators lives easier too, since those won't be marked
// as obsolete strings and removed
//
// What happens if you just change part of some long text.  How smart is Qt
// anyway?  I have the general impression it's not really that smart about that
// kind of thing, and it's prone to losing translations that would have only
// needed minor tweaking.  Should investigate that, I suppose, by starting off
// with some of the original dialog texts verbatim, and then seeing what happens
// if I change them around a bit.  Ultimately I want the warnings reworded, and
// a lot less wordy.  See top of this brainstorm block for thoughts about how to
// manage that.

}
#include "WarningWidget.moc"
