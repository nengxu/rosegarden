// -*- c-basic-offset: 4 -*-

/*
    Rosegarden-4 v0.1
    A sequencer and musical notation editor.

    This program is Copyright 2000-2002
        Guillaume Laurent   <glaurent@telegraph-road.org>,
        Chris Cannam        <cannam@all-day-breakfast.com>,
        Richard Bown        <bownie@bownie.com>

    The moral right of the authors to claim authorship of this work
    has been asserted.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _INSTRUMENT_H_
#define _INSTRUMENT_H_

#include "XmlExportable.h"
#include <string>

// An Instrument connects a Track (which itself contains
// a list of Segments) to a device that can play that
// Track.  
//
// An Instrument is either MIDI or Audio (or whatever else
// we decide to implement).
//
//

namespace Rosegarden
{

typedef unsigned int InstrumentId;
typedef unsigned char MidiByte;

struct MidiBank
{
    MidiByte msb;
    MidiByte lsb;
    std::string name;
};

struct MidiProgram : public MidiBank
{
    MidiByte program;
};

// A mapped MIDI instrument - a drum track click for example
//
struct MidiMetronome : public MidiProgram
{
    MidiByte pitch;
    MidiByte channel;
};


// Instrument number groups
//
const InstrumentId SystemInstrumentBase = 0;
const InstrumentId AudioInstrumentBase  = 1000;
const InstrumentId MidiInstrumentBase   = 2000;

const MidiByte MidiMaxValue = 127;
const MidiByte MidiMidValue = 64;
const MidiByte MidiMinValue = 0;

// Predeclare Device
//
class Device;

class Instrument : public XmlExportable
{
public:
    enum InstrumentType { Midi, Audio };

    Instrument(InstrumentId id,
               InstrumentType it,
               const std::string &name,
               Device *device);

    Instrument(InstrumentId id,
               InstrumentType it,
               const std::string &name,
               MidiByte channel,
               Device *device);

    ~Instrument();

    std::string getName() { return m_name; }

    void setID(int id) { m_id = id; }
    InstrumentId getID() const { return m_id; }

    void setName(const std::string &name) { m_name = name; }
    InstrumentType getType() const { return m_type; }

    void setType(InstrumentType type) { m_type = type; }
    InstrumentType getInstrumentType() { return m_type; }

    void setMidiChannel(MidiByte mC) { m_channel = mC; }
    MidiByte getMidiChannel() const { return m_channel; }

    void setMidiTranspose(MidiByte mT) { m_transpose = mT; }
    MidiByte getMidiTranspose() const { return m_transpose; }

    void setPan(MidiByte pan) { m_pan = pan; }
    MidiByte getPan() const { return m_pan; }

    void setVolume(MidiByte volume) { m_volume = volume; }
    MidiByte getVolume() const { return m_volume; }

    void setProgramChange(MidiByte program);
    MidiByte getProgramChange() const { return m_programChange; }

    void setSendBankSelect(bool value) { m_sendBankSelect = value; }
    bool sendsBankSelect() const { return m_sendBankSelect; }

    void setSendProgramChange(bool value) { m_sendProgramChange = value; }
    bool sendsProgramChange() const { return m_sendProgramChange; }

    void setSendPan(bool value) { m_sendPan = value; }
    bool sendsPan() const { return m_pan; }

    void setSendsVelocity(bool value) { m_sendVelocity = value; }
    bool sendsVelocity() const { return m_sendVelocity; } 

    // Implementation of virtual function
    //
    virtual std::string toXmlString();

    // Return the device pointer
    //
    Device* getDevice() { return m_device; }

private:
    InstrumentId    m_id;
    std::string     m_name;
    InstrumentType  m_type;
    
    // Mainly MIDI elements that we might be interested in
    MidiByte        m_channel;
    MidiByte        m_programChange;
    MidiByte        m_msb;
    MidiByte        m_lsb;
    MidiByte        m_transpose;
    MidiByte        m_pan;
    MidiByte        m_volume;

    Device         *m_device;

    // Do we send at this intrument or do we leave these
    // things up to the parent device and God?  These are
    // directly relatable to GUI elements
    // 
    bool         m_sendBankSelect;
    bool         m_sendProgramChange;
    bool         m_sendPan;
    bool         m_sendVelocity;

};

}

#endif // _INSTRUMENT_H_
