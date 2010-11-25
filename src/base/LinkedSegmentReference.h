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

#ifndef _LINKEDSEGMENTREFERENCE_H
#define _LINKEDSEGMENTREFERENCE_H

#include "Segment.h"

#include <QtCore/QWeakPointer>
#include <QtCore/QSharedPointer>

namespace Rosegarden 
{

class LinkedSegment;
    
typedef unsigned int LinkedSegmentReferenceId;

class LinkedSegmentReference : public Segment
{
public:
    LinkedSegmentReference(const Segment &other);    
    LinkedSegmentReference(const Segment &other, int linkId);    
    virtual ~LinkedSegmentReference() {}
    
    void addLinkedSegment(LinkedSegment *linkedSeg) { 
                                       m_linkedSegments.push_back(linkedSeg); }
    void removeLinkedSegment(LinkedSegment *linkedSeg) { 
                                          m_linkedSegments.remove(linkedSeg); }
    void linkedSegmentChanged(const LinkedSegment *linkedSeg, timeT from, 
                                                              timeT to);
    void resetLinkedSegmentRefreshStatuses();
    
    static int m_count;
    LinkedSegmentReferenceId getLinkId() const { return m_id; }
 
    //overrides from segment
    QString getXmlElementName() const { return "linkedsegmentreference"; }

    static void insertMappedEvent(Segment *seg, const Event *e,  timeT t, 
                                                int semitones,  int steps);

private:
    typedef std::list<LinkedSegment *> LinkedSegmentSet;
    LinkedSegmentSet m_linkedSegments;
    LinkedSegmentReferenceId m_id;
};

struct LinkedSegmentReferenceCmp
{
    bool operator()(const QWeakPointer<LinkedSegmentReference> &r1,
                    const QWeakPointer<LinkedSegmentReference> &r2) const {
        QSharedPointer<LinkedSegmentReference> linkedSegRef1=r1.toStrongRef();
        QSharedPointer<LinkedSegmentReference> linkedSegRef2=r2.toStrongRef();
        if(!linkedSegRef1.isNull() && !linkedSegRef2.isNull())
        {
            return linkedSegRef1->getLinkId() < linkedSegRef2->getLinkId();
        }
        else
        {
            return true;
        }
    }
};

}

#endif // _LINKEDSEGMENTREFERENCE_H
