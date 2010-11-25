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
#include "document/Command.h"
#include "Composition.h"
#include "BaseProperties.h"
#include "base/SegmentNotationHelper.h"

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
    m_referenceSegment(other.m_referenceSegment),
    m_linkTransposeParams(other.m_linkTransposeParams)
{
    init(other);
}

/**
 * Construction from existing ordinary segment, so a new reference segment
 * needs to be made
 */
LinkedSegment::LinkedSegment(const Segment& other) :
    Segment(other)
{
    //need to handle implied C Major in segments without a key change event
    //at the start of them
    
    //rather than keep having to write "handle implied C Major key in segments
    //with no key change event at segment start" code, i'm just going to stick
    //a null-op C Major key at the start (if no key already exists there)

    bool foundKey = false;
    
    timeT segFrom = getStartTime();
    timeT segTo = segFrom + 1;
    iterator itrFrom = findTime(segFrom);
    iterator itrTo = findTime(segTo);
    
    for(const_iterator itr = itrFrom; itr != itrTo; ++itr) {
        if ((*itr)->isa(Rosegarden::Key::EventType)) {
            foundKey = true;
            break;
        }
    }

    if (!foundKey) {
        Rosegarden::Key key;
        SegmentNotationHelper helper(*this);
        helper.insertKey(getStartTime(),key);
    }
    
    m_referenceSegment = 
     QSharedPointer<LinkedSegmentReference>(new LinkedSegmentReference(other));
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
    connect(CommandHistory::getInstance(), SIGNAL(updateLinkedSegments(Command *)),
        this, SLOT(slotUpdateLinkedSegments(Command *)));
}

void LinkedSegment::init(const Segment& other)
{
    m_refreshStatusId = getNewRefreshStatusId();
    m_referenceSegment->addLinkedSegment(this);
    connect(CommandHistory::getInstance(), SIGNAL(updateLinkedSegments(Command *)),
        this, SLOT(slotUpdateLinkedSegments(Command *)));
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

void LinkedSegment::slotUpdateLinkedSegments(Command *command)
{
    SegmentRefreshStatus &rs = getRefreshStatus(m_refreshStatusId);
        
    if (command->getUpdateLinks() && rs.needsRefresh()) {
        m_referenceSegment->linkedSegmentChanged(this,rs.from(),rs.to());
    }
    
    rs.setNeedsRefresh(false);
}

void LinkedSegment::refresh()
{
    timeT startTime = getStartTime();
    eraseNonIgnored(begin(),end());
    LinkedSegmentReference::const_iterator itr;
    for(itr=m_referenceSegment->begin(); itr!=m_referenceSegment->end(); ++itr) {
        const Event *refEvent = *itr;

        timeT refEventTime = refEvent->getAbsoluteTime();
        timeT freshEventTime = refEventTime+startTime;
        
        LinkedSegmentReference::insertMappedEvent(this,refEvent,freshEventTime,
               m_linkTransposeParams.m_semitones,m_linkTransposeParams.m_steps);
    }
}

void LinkedSegment::eraseNonIgnored(const_iterator itrFrom, const_iterator itrTo)
{
    //only erase items which aren't ignored for link purposes
    LinkedSegment::iterator eraseItr;
    for(eraseItr=itrFrom; eraseItr!=end() && eraseItr!=itrTo; ) {
        bool ignore = false;
        (*eraseItr)->get<Bool>(BaseProperties::LINKED_SEGMENT_IGNORE_UPDATE,
                                ignore);
        if (!ignore) {
            erase(eraseItr++);
        } else {
            ++eraseItr;
        }
    }
} 

}

#include "LinkedSegment.moc"
