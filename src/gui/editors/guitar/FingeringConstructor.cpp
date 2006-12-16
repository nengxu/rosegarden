#include "Fingering.h"
#include "NoteSymbols.h"
#include "misc/Debug.h"

#include <qpainter.h>
#include <qdrawutil.h>
#include <qevent.h>
#include <qbrush.h>
#include <qstring.h>
#include <qscrollbar.h>

#include <kcursor.h>

#include <iostream>

namespace Rosegarden
{

namespace Guitar
{

/*------------------------------------------
     Parent: Fingering Constructor
-------------------------------------------*/
FingeringConstructor::FingeringConstructor
( GuitarNeck *instr,
  QWidget *parent,
  bool editable,
  const char *name )
        : QFrame( parent, name ),
        m_instr ( instr ),
        m_editable ( editable ),
        m_frets_displayed ( MAX_FRET_DISPLAYED ),
        m_fingering ( new Fingering ( instr, m_frets_displayed ) ),
        m_press_string_num ( 0 ),
        m_press_fret_num ( 0 )
{
    setFixedSize( IMG_WIDTH, IMG_HEIGHT );
    setFrameStyle( Panel | Sunken );
    setBackgroundMode( PaletteBase );

    if ( m_editable ) {
        m_fret_spinbox =
            new QSpinBox ( 1,
                           m_instr->getFretNumber() - m_frets_displayed + 1,
                           1,
                           this );
        m_fret_spinbox->setGeometry( width() - SCROLL, 0, SCROLL, height() );

        connect( m_fret_spinbox,
                 SIGNAL( valueChanged( int ) ),
                 SLOT( setFirstFret( int ) ) );

        setMouseTracking( true );
        
    } else {
        setEnabled(false); // no need of events if widget is not editable
    }

//    clear();
}

void FingeringConstructor::clear()
{
    m_instr->clear();
    delete m_fingering;
    m_fingering = new Fingering ( m_instr, m_frets_displayed );
    update();
    emit chordChange();
}

void FingeringConstructor::setFingering ( Fingering* arrange )
{
    delete m_fingering;
    m_fingering = new Fingering( *arrange );
    update();
    emit chordChange();
}

void FingeringConstructor::setFirstFret( int fret )
{
    m_fingering->setFirstFret ( fret );
    update();
    emit chordChange();
}

void FingeringConstructor::drawContents( QPainter *p )
{
    /*
            // Horizontal separator line
            NoteSymbols ns;
            ns.drawSeperator(p, SCALE, m_instr->getStringNumber());
            ns.drawFretHorizontalLines (p, SCALE, m_frets_displayed, m_instr->getStringNumber());
            ns.drawFretVerticalLines (p, SCALE, m_frets_displayed, m_instr->getStringNumber());
    */

    if ( m_editable ) {
        // Set beginning fret number and scroll bar
        m_fret_spinbox->setValue ( m_fingering->getFirstFret() );
    }

    // Draw Fingering
    m_fingering->drawContents ( p );

}

Fingering*
FingeringConstructor::getFingering ( void ) const
{
    return m_fingering;
}

unsigned int
FingeringConstructor::getStringNumber ( QMouseEvent const* event )
{
    const QPoint pos = event->pos();
    NoteSymbols ns;
    PositionPair result = ns.getStringNumber ( maximumHeight(),
                          pos.x(),
                          m_instr->getStringNumber() );
    unsigned int string_num = 0;

    if ( result.first ) {
        string_num = m_instr->getStringNumber() - result.second;
    }

    return string_num;
}

unsigned int
FingeringConstructor::getFretNumber ( QMouseEvent const* event )
{
    NoteSymbols ns;
    const QPoint pos = event->pos();
    unsigned int y_pos = pos.y();
    unsigned int fret_num = 0;

    if ( y_pos > ns.getTopBorder( maximumHeight() ) ) {
        // If fret position is below the top line of the fretboard image.
        PositionPair result = ns.getFretNumber ( maximumWidth(),
                              y_pos,
                              m_frets_displayed );

        if ( result.first ) {
            fret_num = result.second + m_fret_spinbox->value();
        }
    }

    return fret_num;
}

void FingeringConstructor::mousePressEvent( QMouseEvent *event )
{
    if ( ( event->button() == LeftButton ) && ( m_editable ) ) {

        // Find string position
        m_press_string_num = getStringNumber ( event );

        // Find fret position
        m_press_fret_num = getFretNumber ( event );
    }
}

void FingeringConstructor::mouseReleaseEvent( QMouseEvent *event )
{
    // If we are display only then we will not updated on a mouse event
    if ( !m_editable ) {
        return ;
    }

    unsigned int release_string_num = getStringNumber( event );
    unsigned int release_fret_num = getFretNumber( event );

    processMouseRelease ( release_string_num, release_fret_num );
}

void FingeringConstructor::processMouseRelease( unsigned int release_string_num,
                                                unsigned int release_fret_num )
{
    if ( m_press_fret_num == release_fret_num ) {
        // If press string & fret pos == release string & fret position, display chord
        if ( m_press_string_num == release_string_num ) {
            // QUESTION: Move check for whether a note pressed is at position 0
            // here or not?
            if ( ( m_press_string_num > 0 ) &&
                    ( m_press_string_num <= m_instr->getStringNumber() ) &&
                    ( m_press_fret_num < ( m_fret_spinbox->value() + m_frets_displayed ) )
               ) {
                /**
                IF m_press_fret_num == 0
                    Get fretStatus
                        status | new status
                        MUTED  | OPEN
                        OPEN   | MUTED
                ELSE m_press_fret_num > 0
                    status  | new status
                    MUTED   | PRESSED
                    OPEN    | PRESSED
                        PRESSED | PRESSED
                */
                GuitarString::Action aVal = GuitarString::PRESSED;
                if ( m_press_fret_num == 0 ) {
                    switch ( m_fingering->getStringStatus ( m_press_string_num ) ) {
                    case GuitarString::MUTED: {
                            aVal = GuitarString::OPEN;
                            break;
                        }
                    case GuitarString::OPEN:
                    default: {
                            aVal = GuitarString::MUTED;
                            break;
                        }
                    }
                } else {

                    if ( m_fingering->hasNote ( m_press_string_num ) ) {
                        
                        Note * note_ptr = m_fingering->getNote ( m_press_string_num );
                        
                        if ( ( note_ptr->getFret() == m_press_fret_num ) &&
                                ( note_ptr->getStringNumber() == m_press_string_num ) ) {
                            // Remove note if there was one already at this place
                            m_fingering->removeNote ( m_press_string_num );
                        } else { // there was a note on this string, but not at this fret, so move it to the new fret                                                
                            note_ptr->setNote ( m_press_string_num, m_press_fret_num );
                            m_fingering->setStringStatus( m_press_string_num, aVal );
                        }
                    } else {
                        Note* note_ptr = new Note ( m_press_string_num, m_press_fret_num );
                        m_fingering->addNote( note_ptr );
                        m_fingering->setStringStatus( m_press_string_num, aVal );
                    }
                }
                update();
            }
        }
        // else if press fret pos == release fret pos & press string pos != release string pos, display bar
        else {
            if ( ( ( m_press_string_num > 0 ) && ( release_string_num > 0 ) ) &&
                    ( ( m_press_string_num <= m_instr->getStringNumber() ) &&
                      ( release_string_num <= m_instr->getStringNumber() ) ) &&
                    ( ( m_press_fret_num < ( m_fret_spinbox->value() + m_frets_displayed ) ) &&
                      ( release_fret_num < ( m_fret_spinbox->value () + m_frets_displayed ) ) ) ) {
                /*
                                std::cout << "FingeringConsturctor::mouseHandle - Adding a bar from string #"
                                << event->press_string_num << ", fret #" << event->press_fret_num
                                << " to string #" << release_string_num << ", fret #" << release_fret_num << std::endl;
                */

                if ( m_fingering->hasBarre ( m_press_fret_num ) ) {
                    Barre * b_ptr = m_fingering->getBarre( m_press_fret_num );
                    if (b_ptr->getFret() == m_press_fret_num) {
                        m_fingering->removeBarre ( m_press_fret_num );
                    } else {
                        b_ptr->setBarre( m_press_fret_num, m_press_string_num, release_string_num );
                    }
                } else {
                    Barre* b_ptr = new Barre ( m_press_fret_num,
                                               m_press_string_num,
                                               release_string_num );
                    m_fingering->addBarre( b_ptr );
                }
                update();
            }
        }
    }
}


void FingeringConstructor::mouseMoveEvent( QMouseEvent *event )
{
    if (m_fingering) {
        unsigned int transientStringNb = getStringNumber(event);
        unsigned int transientFretNb = getFretNumber(event);
        
        if (transientStringNb != m_fingering->getTransientStringNb() ||
            transientFretNb != m_fingering->getTransientFretNb()) {
            
            QRect updateRect = m_fingering->updateTransientPos(size(), transientStringNb, transientFretNb);
            
            update(updateRect);
            
        }
    }
    
    
}

} /* namespace Guitar */

}

#include "FingeringConstructor.moc"

