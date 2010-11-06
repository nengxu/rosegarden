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

#include "LinkedSegment.h"

#include "document/CommandHistory.h"
#include "Composition.h"

#include <cassert>
#include <QtGlobal>

namespace Rosegarden
{

/**
 * Construction from existing linked segment, just need to attach
 * to its reference segment
 */
LinkedSegment::LinkedSegment(const LinkedSegment& other) :
    Segment(other),
    m_referenceSegment(other.m_referenceSegment)
{
    init(other);
}

/**
 * Construction from existing ordinary segment, so a new reference segment
 * needs to be made
 */
LinkedSegment::LinkedSegment(const Segment& other) :
    Segment(other),
    m_referenceSegment(new LinkedSegmentReference(other))
{
    m_referenceSegment->setStartTime(0);
    
    init(other);
}

/**
 * Called during construction from serialisation, where the refSeg already
 * exists and just needs attatching to
 */
LinkedSegment::LinkedSegment(const Segment& other,
                             QSharedPointer<LinkedSegmentReference> refSeg) :
    Segment(other),
    m_referenceSegment(refSeg)
{
    m_refreshStatusId = getNewRefreshStatusId();
    m_referenceSegment->addLinkedSegment(this);
    connect(CommandHistory::getInstance(), SIGNAL(updateLinkedSegments()),
        this, SLOT(slotUpdateLinkedSegments()));
}

void LinkedSegment::init(const Segment& other)
{
    m_refreshStatusId = getNewRefreshStatusId();
    m_referenceSegment->addLinkedSegment(this);
    connect(CommandHistory::getInstance(), SIGNAL(updateLinkedSegments()),
        this, SLOT(slotUpdateLinkedSegments()));
    Composition* comp = other.getComposition();
    if(comp) {
        comp->addLinkedSegmentReference(m_referenceSegment);
    }
}

LinkedSegment::~LinkedSegment()
{
    m_referenceSegment->removeLinkedSegment(this);
}

void LinkedSegment::clearSelfRefreshStatus()
{
    SegmentRefreshStatus &rs = getRefreshStatus(m_refreshStatusId);
    rs.setNeedsRefresh(false);
}

void LinkedSegment::slotUpdateLinkedSegments()
{
    SegmentRefreshStatus &rs = getRefreshStatus(m_refreshStatusId);
        
    if (rs.needsRefresh()) { // && !(rs.from()==0 && rs.to()==0)) {
        m_referenceSegment->linkedSegmentChanged(this,rs.from(),rs.to());
    }
    
    rs.setNeedsRefresh(false);
}

}

#include "LinkedSegment.moc"
