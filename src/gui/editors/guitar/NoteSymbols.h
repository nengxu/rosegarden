#ifndef SYMBOLS_H_
#define SYMBOLS_H_

#include <qbrush.h>
#include <qpainter.h>

#include <iostream>

#include "GuitarNeck.h"
#include "Fingering.h"
#include "FingeringBox.h"

namespace Rosegarden
{

/**
 *----------------------------------------
 * Finding X position on fretboard pixmap
 *----------------------------------------
 *
 * Originally x = position * scale + FC::BORDER + FC::CIRCBORD + FC::FRETTEXT
 *
 * The last three can be condense into on term called XBorder
 *      XBorder = FC::BORDER + FC::CIRCBORD + FC::FRETTEXT
 *             = 5 + 2 + 10 (see fingers.h)
 *             = 17
 *
 * The drawable fretboard space on the x-axis:
 *      XFretboard = pixmap width - XBorder
 *                = width - 17
 *
 * The fretboard x-axis is broken up into colums which represent the drawable
 * space for a fretboard component (e.g. note, barre)
 *      Column Width = XFretboard / number of strings
 *
 * Therefore a new x can be calculated from the position and the column width
 *      x = (position * Column Width) + XBorder
 *
 *-------------------------------------------
 * Finding Y position on fretboard pixmap
 *-------------------------------------------
 *
 * Originally y = (FC::BORDER * scale) + (2 * FC::SPACER) + (fret * scale) + FC::CIRCBORD
 *
 * As with the x-axis the equation can be separated into the position plus the border. In
 * this case YBorder
 *      YBorder = (FC::BORDER*scale) + (2*FC::SPACER) + FC::CIRCBORD
 *              = 17 (If we want to use the same border as the x-axis)
 *
 * The drawable fretboard space on the y-axis:
 *      YFretboard = pixmap height - YBorder
 *
 * The fretboard y-axis is broken up into rows which represent the drawable
 * space for a fretboard component (e.g. note, barre)
 *      Row Height = YFretboard / number of frets
 *
 * Therefore a new y can be calculated from the fret position and the row height
 *      y = fret * Row Height
 **/

namespace Guitar
{

class NoteSymbols
{
private:
    typedef FingeringBox FC;
    typedef std::pair<unsigned int, unsigned int> posPair;

    static float const LEFT_BORDER_PERCENTAGE;
    static float const RIGHT_BORDER_PERCENTAGE;
    static float const FRETBOARD_WIDTH_PERCENTAGE;
    static float const TOP_BORDER_PERCENTAGE;
    static float const BOTTOM_BORDER_PERCENTAGE;
    static float const FRETBOARD_HEIGHT_PERCENTAGE;
    static int   const TOP_FRETBOARD_MARGIN;
    static int   const FRET_PEN_WIDTH;
    static int   const STRING_PEN_WIDTH;
    
public:

    NoteSymbols(unsigned int nbOfStrings, unsigned int nbOfFrets) :
        m_nbOfStrings(nbOfStrings), 
        m_nbOfFrets(nbOfFrets) {};

    //! Display a mute symbol in the QPainter object
    void
    drawMuteSymbol ( QPainter* p,
                     unsigned int position ) const;

    /* This code borrowed from KGuitar 0.5 */
    //! Display a open symbol in the QPainter object (KGuitar)
    void drawOpenSymbol ( QPainter* p,
                          unsigned int position ) const;

    /* This code borrowed from KGuitar 0.5 */
    //! Display a note symbol in the QPainter object (KGuitar)
    void drawNoteSymbol ( QPainter* p,
                          unsigned int stringNb,
                          int fretNb,
                          bool transient = false ) const;

    /* This code borrowed from KGuitar 0.5 */
    /**
     * Display a bar symbol in the QPainter object (KGuitar)
     * The code from the KGuitar project was modified to display a bar. This feature was not
     * available in that project
     */
    void drawBarreSymbol ( QPainter* p,
                           int fretNb,
                           unsigned int start,
                           unsigned int end ) const;

    void drawFretNumber ( QPainter* p,
                          unsigned int fret_num ) const;

    void drawFrets ( QPainter* p ) const;

    void drawStrings ( QPainter* p ) const;

    unsigned int getTopBorder ( unsigned int imgHeight ) const;

    unsigned int getBottomBorder ( unsigned int imgHeight ) const;

    unsigned int getLeftBorder ( unsigned int imgWidth ) const;

    unsigned int getRightBorder ( unsigned int imgWidth ) const;

    unsigned int getFretboardWidth ( int imgWidth ) const;

    unsigned int getFretboardHeight ( int imgHeight ) const;

    std::pair<bool, unsigned int>
    getStringNumber ( int imgWidth,
                      unsigned int x_pos,
                      unsigned int string_num ) const;

    std::pair<bool, unsigned int>
    getFretNumber ( int imgHeight,
                    unsigned int y_pos,
                    unsigned int maxFretNum ) const;

    QRect getTransientNoteSymbolRect(QSize fretboardSize,
                                     unsigned int stringNb,
                                     int fretNb) const;
    
private:

    posPair
    getX ( int imgWidth, unsigned int stringNb, unsigned int nbOfStrings ) const;

    posPair
    getY ( int imgHeight, unsigned int fretNb, unsigned int nbOfFrets ) const;


    unsigned int m_nbOfStrings;
    unsigned int m_nbOfFrets;

};

} /* namespace Guitar */

}

#endif /* SYMBOLS_H_ */

