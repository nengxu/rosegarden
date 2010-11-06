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

protected:
    virtual LinkedSegment *cloneImpl() const { return new LinkedSegment(*this); }

protected slots:
    void slotUpdateLinkedSegments();
    
private:
    ///don't want linked segments made unless they're created from an existing one
    LinkedSegment() {}

    void init(const Segment& other);

    QSharedPointer<LinkedSegmentReference> m_referenceSegment;
    unsigned int m_refreshStatusId;
};

}

#endif // LINKEDSEGMENT_H
