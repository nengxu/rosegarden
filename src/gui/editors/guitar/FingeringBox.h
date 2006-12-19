#ifndef FINGERS_H
#define FINGERS_H

#include <qframe.h>
#include <qspinbox.h>

class QScrollBar;

/*
        KGuitar interface was used as a guideline for design the FingeringBox. Where the code
        from the project was used is defined by comments.
*/

namespace Rosegarden
{

namespace Guitar
{

class Fingering;
class GuitarNeck;
class NoteSymbols;
class FC_Mode;
class FC_InsertMode;
class FC_DeleteMode;

class FingeringBox : public QFrame
{
    Q_OBJECT

public:

    static const unsigned int MAX_FRET_DISPLAYED = 5;

    //! Constant values obtained from KGuitar project
    static const unsigned int IMG_WIDTH = 200;
    static const unsigned int IMG_HEIGHT = 200;
    static const unsigned int SCROLL = 15;


    //! Constructor
    FingeringBox ( GuitarNeck *instr,
                           QWidget *parent = 0,
                           bool editable = true,
                           const char *name = 0 );

    virtual ~FingeringBox() {}

public slots:

    //! Clear the chord display of Bar and Note objects
    void clear();

    /**
     * Set fingering object to be displayed
     * @param  arrange Fingering object pointer
     */
    void setFingering( Guitar::Fingering* arrange );

    //! Get the displayed fingering object
    Fingering* getFingering ( void ) const;

    //! Set the base fret for fingering
    void setFirstFret( int );


signals:

    //! Alert this object that a chordChange has occurred
    void chordChange();

protected:

    virtual void drawContents ( QPainter * );

    virtual void mousePressEvent ( QMouseEvent * );

    virtual void mouseReleaseEvent ( QMouseEvent * );

    virtual void mouseMoveEvent ( QMouseEvent * );

    void processMouseRelease( unsigned int release_string_num, unsigned int release_fret_num );

    typedef std::pair<bool, unsigned int> PositionPair;

    unsigned int getStringNumber ( QMouseEvent const* event );

    unsigned int getFretNumber ( QMouseEvent const* event );

    //! Spin Box to manage setting the base fret for fingering (KGuitar)
    QSpinBox *m_fret_spinbox;

    //! Handle to the guitar associated with present fingering
    GuitarNeck *m_instr;

    //! Present mode
    bool m_editable;

    //! Maximum number of frets displayed by FingeringBox
    unsigned int m_frets_displayed;

    //! Handle to the present fingering
    Fingering* m_fingering;

    //! String number where a mouse press event was located
    unsigned int m_press_string_num;

    //! Fret number where a mouse press event was located
    unsigned int m_press_fret_num;

};

} /* namespace */

}

#endif
