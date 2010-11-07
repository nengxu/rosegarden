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

#include "LinkedSegmentReference.h"

#include "LinkedSegment.h"
#include "document/CommandHistory.h"

#include <cassert>
#include <stdlib.h>

namespace Rosegarden
{

int LinkedSegmentReference::m_count = 0;

LinkedSegmentReference::LinkedSegmentReference(const Segment &other) :
    Segment(other)
{
    m_id = m_count;
    m_count++;
}
    
LinkedSegmentReference::LinkedSegmentReference(const Segment &other, int linkId) : 
    Segment(other)
{
    m_id = linkId;
    m_count = std::max(m_count,linkId+1);
}
  
/**
 * 1. Clear out this from srcFrom to srcTo
 * 2. Update this in srcFrom-srcTo from linkedSeg, 
 *    accounting for time and pitch shifts
 * 3. Clear out all other segs linked to this in srcFrom-srcTo
 * 4. Update other segs linked to this with the new content in this 
 *    in srcFrom-srcTo (accounting for time and pitch shifts)
 */
void LinkedSegmentReference::linkedSegmentChanged(const LinkedSegment *linkedSeg, 
                                                  timeT srcFrom, timeT srcTo)
{
    //transform from and to into the relative time of the reference segment,
    //taking into account starting time and (eventually) time squash/stretch
    timeT srcSegStartTime = linkedSeg->getStartTime();
    timeT refFrom = srcFrom - srcSegStartTime;
    timeT refTo = srcTo - srcSegStartTime;
    
    //erase our contents in the region to be updated, and refresh it from the
    //linked seg
    erase(findTime(refFrom),findTime(refTo));
    for(Segment::const_iterator itr = linkedSeg->findTime(srcFrom);
                                itr != linkedSeg->findTime(srcTo); ++itr) {
        const Event *e = *itr;
        
        timeT absEventTime = e->getAbsoluteTime();
        timeT relEventTime = absEventTime - srcSegStartTime;
        
        Event *refSegEvent = new Event(*e,relEventTime);
        
        //correct for temporal (and pitch shift??) here eventually...
        
        insert(refSegEvent);
    }
    
    //update all our observers with the new content
    for(LinkedSegmentSet::iterator itr = m_linkedSegments.begin();
                                   itr != m_linkedSegments.end(); ++itr) {
        LinkedSegment *linkedSegToUpdate = *itr;
        
        if(linkedSegToUpdate != linkedSeg) {
            timeT segStartTime = linkedSegToUpdate->getStartTime();
            //these will need correcting for time stretch/squash
            timeT segFrom = segStartTime + refFrom;
            timeT segTo = segStartTime + refTo;
            LinkedSegment::iterator itrFrom = linkedSegToUpdate->findTime(segFrom);
            LinkedSegment::iterator itrTo = linkedSegToUpdate->findTime(segTo);
            linkedSegToUpdate->erase(itrFrom,itrTo);
            
            for(Segment::const_iterator itr = findTime(refFrom);
                                        itr != findTime(refTo); ++itr) {
                const Event *e = *itr;
            
                //correct this time and event for temporal (and pitch shift??)
                //here eventually...
                timeT eventT = e->getAbsoluteTime() + segStartTime;
                
                Event *linkedSegEvent = new Event(*e,eventT);
                
                linkedSegToUpdate->insert(linkedSegEvent);
            }
            
            linkedSegToUpdate->clearSelfRefreshStatus();
        }
    }
}

}

