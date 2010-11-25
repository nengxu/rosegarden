
/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
    Copyright 2000-2010 the Rosegarden development team.

    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _RG_SEGMENTLINKTRANSPOSECOMMAND_H_
#define _RG_SEGMENTLINKTRANSPOSECOMMAND_H_

#include "document/Command.h"
#include "document/BasicCommand.h"
#include "document/BasicSelectionCommand.h"
#include "base/LinkedSegment.h"
#include "base/Selection.h"

#include <QCoreApplication>

namespace Rosegarden
{

class LinkedSegment;


class SegmentLinkTransposeCommand : public MacroCommand
{
    Q_DECLARE_TR_FUNCTIONS(Rosegarden::SegmentLinkTransposeCommand)

public:
    /// Set transpose on segments.
    SegmentLinkTransposeCommand(std::vector<LinkedSegment *> linkedSegs,
        bool changeKey, int steps, int semitones, bool transposeSegmentBack);
    virtual ~SegmentLinkTransposeCommand();

    virtual void execute();
    virtual void unexecute();

    static QString getGlobalName() { return tr("Transpose Linked Segments"); }

private:
    std::vector<LinkedSegment *> m_linkedSegs;

    ///new parameters
    LinkedSegment::TransposeParams m_linkTransposeParams;
    
    ///old parameters
    std::vector<LinkedSegment::TransposeParams> m_oldLinkTransposeParams;
};

class SegmentLinkResetTransposeCommand : public MacroCommand
{
    Q_DECLARE_TR_FUNCTIONS(Rosegarden::SegmentLinkResetTransposeCommand)

public:
    SegmentLinkResetTransposeCommand(std::vector<LinkedSegment *> &linkedSegs);

    static QString getGlobalName() {
        return tr("Reset Transpose on Linked Segments"); }
};

class SingleSegmentLinkResetTransposeCommand : public BasicSelectionCommand
{
    Q_DECLARE_TR_FUNCTIONS(Rosegarden::SingleSegmentLinkResetTransposeCommand)

public:
    SingleSegmentLinkResetTransposeCommand(LinkedSegment &linkedSeg) :
        BasicSelectionCommand(getGlobalName(), linkedSeg, true),
        m_linkedSeg(&linkedSeg),
        m_linkTransposeParams(m_linkedSeg->getLinkTransposeParams()) { }

    static QString getGlobalName() {
        return tr("Reset Transpose on Linked Segment"); }

    virtual void execute();
    virtual void unexecute();

protected:
    virtual void modifySegment();

private:
    LinkedSegment *m_linkedSeg;
    LinkedSegment::TransposeParams m_linkTransposeParams;
};

}

#endif
