/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */


/*
    Rosegarden
    A sequencer and musical notation editor.
    Copyright 2000-2010 the Rosegarden development team.
    See the AUTHORS file for more details.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _LINKEDSEGMENT_H
#define _LINKEDSEGMENT_H

#include "Segment.h"
#include "LinkedSegmentReference.h"

#include <QtCore/QObject>
#include <QtCore/QSharedPointer>

namespace Rosegarden 
{
    
class Command;

class LinkedSegment : public QObject, public Segment 
{
    Q_OBJECT

public:
    LinkedSegment(const LinkedSegment &other);
    LinkedSegment(const Segment &other);
    LinkedSegment(const Segment &other, QSharedPointer<LinkedSegmentReference> refSeg);
    virtual ~LinkedSegment();
    
    void clearSelfRefreshStatus();
    
    int getLinkedReferenceSegmentId() const { return m_referenceSegment->getLinkId(); }

    ///overrides from Segment
    virtual bool isLinked() const { return true; }
    
    ///link transpose interface
    struct TransposeParams
    {
        TransposeParams() : m_changeKey(false), m_steps(0),
            m_semitones(0), m_transposeSegmentBack(false) { }
        TransposeParams(bool chgKey, int steps, int semitones, bool transBack) : 
            m_changeKey(chgKey), m_steps(steps), m_semitones(semitones), 
            m_transposeSegmentBack(transBack) { }
        bool m_changeKey;
        int m_steps;
        int m_semitones;
        bool m_transposeSegmentBack;
    };
    TransposeParams getLinkTransposeParams() const {
                                                return m_linkTransposeParams; }
    void setLinkTransposeParams(TransposeParams params) {
                                              m_linkTransposeParams = params; }

    ///re read the entire set of events from the reference segment
    void refresh();
    
    ///erase all events which aren't ignored for link purposes
    void eraseNonIgnored(const_iterator itrFrom, const_iterator itrTo);

protected:
    virtual LinkedSegment *cloneImpl() const { return new LinkedSegment(*this); }

protected slots:
    void slotUpdateLinkedSegments(Command* command);
    
private:
    ///don't want linked segments made unless created from an existing one
    LinkedSegment() {}

    void init(const Segment& other);

    QSharedPointer<LinkedSegmentReference> m_referenceSegment;
    unsigned int m_refreshStatusId;
    ///current params used when this segment was last transposed
    TransposeParams m_linkTransposeParams;
};

}

#endif // LINKEDSEGMENT_H
