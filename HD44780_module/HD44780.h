/*******************************************************************************
*
* HD44780 ALPHANUMERIC LCD DRIVER MODULE
*
*******************************************************************************/

/*******************************************************************************
*
* This module is designed to control HD44780 alphanumeric LCD chipsets, or 
* clones thereof, providing a universal driver module for this well known and 
* often used LCD driver.
*
* Filename : HD44780.h
* Programmer(s) : Stuart Cording a.k.a. CODINGHEAD
* 
********************************************************************************
* Note(s) : 
* See the HD44780.c file for the version changes and notes for this module
*
*******************************************************************************/

/*******************************************************************************
*
*                     HD44780 ALPHANUMERIC LCD DRIVER MODULE
*
*******************************************************************************/
#ifndef __HD44780_MODULE_PRESENT__
#define __HD44780_MODULE_PRESENT__

/*******************************************************************************
*                                 INCLUDE FILES
*******************************************************************************/
#if defined(__18CXX)
    #include "lcdif_c18.h"
#elif defined (__PIC32MX__)
    #include "lcdif_c32.h"
#else
    #error This processor family or toolchain is not currently supported
#endif


/*******************************************************************************
*                                    EXTERNS
*******************************************************************************/
//<List any required extern variables here>


/*******************************************************************************
*                             DEFAULT CONFIGURATION
*******************************************************************************/
//<List any default configuration information here>


/*******************************************************************************
*                                    DEFINES
*******************************************************************************/

/*******************************************************************************
* Summary:
*   Entry Mode Set HD44780 Setting "CURSORMOVE" - cursor moves after each
*   character is written
* See also:
*   <link hd44780EntryModeSet>
*******************************************************************************/
#define EMS_CURSORMOVE      0xFE

/*******************************************************************************
* Summary:
*   Entry Mode Set HD44780 Setting "DISPLAYSHIFT" - displayed text moves after
*   each character is written and cursor stays in current position
* See also:
*   <link hd44780EntryModeSet>
*******************************************************************************/
#define EMS_DISPLAYSHIFT    0xFF

/*******************************************************************************
* Summary:
*   Entry Mode Set HD44780 Setting "DECREMENT" - cursor moves to the left after
*   each character is written
* See also:
*   <link hd44780EntryModeSet>
*******************************************************************************/
#define EMS_DECREMENT       0xFD

/*******************************************************************************
* Summary:
*   Entry Mode Set HD44780 Setting "INCREMENT" - cursor moves to the right after
*   each character is written
* See also:
*   <link hd44780EntryModeSet>
*******************************************************************************/
#define EMS_INCREMENT       0xFF

/*******************************************************************************
* Summary:
*   Display On/Off Control HD44780 Setting "BLINKINGOFF" - cursor blinking
*   is switched off
* See also:
*   <link hd44780DisplayControl>
*******************************************************************************/
#define DOFC_BLINKINGOFF    0xFE

/*******************************************************************************
* Summary:
*   Display On/Off Control HD44780 Setting "BLINKINGON" - cursor blinking
*   is switched on
* See also:
*   <link hd44780DisplayControl>
*******************************************************************************/
#define DOFC_BLINKINGON     0xFF

/*******************************************************************************
* Summary:
*   Display On/Off Control HD44780 Setting "CURSOROFF" - cursor is switched off
*   making it invisible
* See also:
*   <link hd44780DisplayControl>
*******************************************************************************/
#define DOFC_CURSOROFF      0xFD

/*******************************************************************************
* Summary:
*   Display On/Off Control HD44780 Setting "CURSORON" - cursor is switched on
*   making it visible
* See also:
*   <link hd44780DisplayControl>
*******************************************************************************/
#define DOFC_CURSORON       0xFF

/*******************************************************************************
* Summary:
*   Display On/Off Control HD44780 Setting "DISPLAYOFF" - entire display is
*   turned off making its content invisible - text remains in SRAM
* See also:
*   <link hd44780DisplayControl>
*******************************************************************************/
#define DOFC_DISPLAYOFF     0xFB

/*******************************************************************************
* Summary:
*   Display On/Off Control HD44780 Setting "DISPLAYON" - entire display is
*   turned on making its content visible. Any text in SRAM will be redisplayed
* See also:
*   <link hd44780DisplayControl>
*******************************************************************************/
#define DOFC_DISPLAYON      0xFF

/*******************************************************************************
* Summary:
*   Cursor or Display Shift HD44780 Setting "SHIFTLEFT" - moves the cursor to
*   the left, decrementing the Address Counter (AC). Does not change the content
*   of the display
* See also:
*   <link hd44780ShiftControl>
*******************************************************************************/
#define CODS_SHIFTLEFT      0xFB

/*******************************************************************************
* Summary:
*   Cursor or Display Shift HD44780 Setting "SHIFTRIGHT" - moves the cursor to
*   the right, incrementing the Address Counter (AC). Does not change the
*   content of the display
* See also:
*   <link hd44780ShiftControl>
*******************************************************************************/
#define CODS_SHIFTRIGHT     0xFF

/*******************************************************************************
* Summary:
*   Cursor or Display Shift HD44780 Setting "CURSORMOVE" - only the cursor moves
*   in the required direction
* See also:
*   <link hd44780ShiftControl>
*******************************************************************************/
#define CODS_CURSORMOVE     0xF7

/*******************************************************************************
* Summary:
*   Cursor or Display Shift HD44780 Setting "DISPLAYSHIFT" - content of display
*   move in the required direction
* See also:
*   <link hd44780ShiftControl>
*******************************************************************************/
#define CODS_DISPLAYSHIFT   0xFF

/*******************************************************************************
* Summary:
*   Function Set HD44780 Setting "5X8DOTS" - defines character RAM dimension to
*   5x8 pixels when creating own characters
* See also:
*   <link hd44780FunctionSet>
*******************************************************************************/
#define FS_5X8DOTS          0xFD

/*******************************************************************************
* Summary:
*   Function Set HD44780 Setting "5X10DOTS" - defines character RAM dimension to
*   5x10 pixels when creating own characters
* See also:
*   <link hd44780FunctionSet>
*******************************************************************************/
#define FS_5X10DOTS         0xFF

/*******************************************************************************
* Summary:
*   Function Set HD44780 Setting "1LINE" - defines that the display has only a
*   single line
* See also:
*   <link hd44780FunctionSet>
*******************************************************************************/
#define FS_1LINE            0xF7

/*******************************************************************************
* Summary:
*   Function Set HD44780 Setting "2LINE" - defines that the display has two
*   lines, or that the LCD controller is reponsible for 2 lines of a 4-line
*   LCD display
* See also:
*   <link hd44780FunctionSet>
*******************************************************************************/
#define FS_2LINE            0xFF

/*******************************************************************************
* Summary:
*   Function Set HD44780 Setting "4BITBUS" - defines that the display is
*   connected to the host CPU/MCU via a 4-bit data bus
* See also:
*   <link hd44780FunctionSet>
*******************************************************************************/
#define FS_4BITBUS          0xEF

/*******************************************************************************
* Summary:
*   Function Set HD44780 Setting "8BITBUS" - defines that the display is
*   connected to the host CPU/MCU via an 8-bit data bus
* See also:
*   <link hd44780FunctionSet>
*******************************************************************************/
#define FS_8BITBUS          0xFF

/*******************************************************************************
* Summary:
*   This defines that the CGRAM font has 8 lines of data when creating new fonts
* See also:
*   <link hd44780WriteCGRAM>,  <link hd44780ReadCGRAM>
*******************************************************************************/
#define CGRAMFONT_5X8       8

/*******************************************************************************
* Summary:
*   This defines that the CGRAM font has 10 lines of data when creating new
*   fonts
* See also:
*   <link hd44780WriteCGRAM>, <link hd44780ReadCGRAM>
*******************************************************************************/
#define CGRAMFONT_5X10      10

/*******************************************************************************
* Summary:
*   Signals that hd44780Destroy() failed to deallocate requested buffer object
* See also:
*   <link hd44780Deinit>
*******************************************************************************/
#define HD44780_DESTROY_FAIL         0x00

/*******************************************************************************
* Summary:
*   Signals that hd44780Destroy() successfully deallocated requested object
* See also:
*   <link hd44780Deinit>
*******************************************************************************/
#define HD44780_DESTROY_OK           0x01


/*******************************************************************************
*                                   DATA TYPES
*******************************************************************************/

/*******************************************************************************
* New data type HD44780NUM                                                    
* Description:
*   Used to hold the HD44780 LCD number issued by the HD44780 Module
*******************************************************************************/
typedef unsigned int HD44780NUM;

/*******************************************************************************
* New data type LCDIFFP - LCD InterFace Function Pointers
* Description:
*   This structure allows the end user to store pointers to the functions
*   implemented in the LCD interface layer which abstracts this module from the
*   hardware interface being used. Members are:
*   - *pGetBus          - Pointer to a function that aquires the bus mutex, if
*                         available
*   - *pReturnBus       - Pointer to function that returns the bus mutex when
*                         finshed with the bus
*   - *pWriteData       - Pointer to function that writes the desired data to
*                         the data bus
*   - *pReadData        - Pointer to function that reads data from the data bus
*   - *pWriteIntr       - Pointer to function that writes an LCD controller
*                         instruction to the data bus
*   - *pReadAddr        - Pointer to function that reads the LCD controllers
*                         address  register
*   - *p4BitFunctionSet - Pointer to function that handles the extra single
*                         4-bit write needed during the instruction
*                         initialisation process (only required in 4-bit data
*                         bus mode)
*******************************************************************************/
typedef struct LCDIFFPTYPE {
    unsigned char   (*pGetBus)          (HLCDIF const           hLcdIf);
    void            (*pReturnBus)       (HLCDIF const           hLcdIf);
    unsigned char   (*pWriteData)       (HLCDIF const           hLcdIf, 
                                         unsigned char          data);
    unsigned char   (*pReadData)        (HLCDIF const           hLcdIf, 
                                         unsigned char * const  data);
    unsigned char   (*pWriteInstr)      (HLCDIF const           hLcdIf, 
                                         unsigned char          instr);
    unsigned char   (*pReadAddr)        (HLCDIF const           hLcdIf, 
                                         unsigned char * const  addr);
    unsigned char   (*p4BitFunctionSet) (HLCDIF const hLcdIf, 
                                         unsigned char          instruction);
} LCDIFFP ;

/*******************************************************************************
* New data type HD44780OBJ                                                    
* Description:
*   This is the main object for storing the context of the HD44780 LCD chipset
* which is currently in use. Members are:
*   - hLcdIf                    - Handle to the LCD interface to which this
*                                 HD44780 controller is connected
*   - * lcdIfFunctionPointers   - Pointer to the functions that implement the
*                                 low level access to the LCD data bus
*   - hd44780Num                - The number assigned to this HD44780 controller
*   - hd44780Flags              - Flags used by the module to keep track of the
*                                 status of this HD44780 controller (private to
*                                 this module)
*   - *nextHD44780Obj           - Pointer to the next HD44780 object in the
*                                 linked list
*******************************************************************************/
typedef struct HD44780OBJTYPE {
  HLCDIF                    hLcdIf;
  LCDIFFP                 * lcdIfFunctionPointers;
  HD44780NUM                hd44780Num;
  unsigned char             hd44780Flags;
  struct HD44780OBJTYPE   * nextHD44780Obj;
} HD44780OBJ;


/*******************************************************************************
* New data type HHD44780
* Description:
*   Holds a pointer to an HD44780 object
*******************************************************************************/
typedef HD44780OBJ * HHD44780;

/*******************************************************************************
* New data type HD44780CLONE
* Description:
*   Holds a list of HD44780 clone chip sets so that we can perform the 
*   instruction initialisation depending on the chip set being used
*******************************************************************************/
typedef enum HD44780CLONETYPE
{
                                        /* Hitachi HD44780U                   */
    HD44780U,
                                        /* Sitronix ST7066U                   */
    ST7066U,
                                        /* Samsung S6A0069                    */
    S6A0069,
                                        /* Samsung KS0066U                    */
    KS0066U,
                                        /* Novatek NT7603                     */
    NT7603
} HD44780CLONE;    


/*******************************************************************************
*                                GLOBAL VARIABLES
*******************************************************************************/
//<Add all variable which are global here>
/*******************************************************************************
* Global variable <My Global Variable>                                                     
* Summary:
*   <Global variable description>
*******************************************************************************/


/*******************************************************************************
*                                    MACROS
*******************************************************************************/
//<Add all macros here>
//#define <My Special Macro> a < b : 0 ? 1


/*******************************************************************************
*#X#                           FUNCTION PROTOTYPES
*******************************************************************************/

void                hd44780Init(void);
void                hd44780Deinit(void);

HD44780NUM          hd44780Create(HLCDIF const          hLcdIf,
                                  LCDIFFP * const       lcdIfFunctionPointers,
                                  HD44780OBJ * const    hd44780Obj);
unsigned char       hd44780Destroy(HD44780NUM           hd44780Num);

HHD44780            hd44780Open(HD44780NUM              hd44780Num);
HD44780NUM          hd44780Close(HHD44780 const         hHd44780);

unsigned char       hd44780ClearDisplay(HHD44780 const  hHd44780);
unsigned char       hd44780ReturnHome(HHD44780 const    hHd44780);
unsigned char       hd44780EntryModeSet(HHD44780 const     hHd44780,
                                     unsigned char      entryMode);
unsigned char       hd44780DisplayControl(HHD44780 const    hHd44780,
                                         unsigned char     displayOnOffControl);
unsigned char       hd44780ShiftControl(HHD44780 const  hHd44780,
                                        unsigned char   shiftControl);
unsigned char       hd44780FunctionSet(HHD44780 const   hHd44780,
                                        unsigned char   functionSet);
unsigned char       hd44780SetCGRAMAddr(HHD44780 const  hHd44780,
                                        unsigned char   address);
unsigned char       hd44780SetCursorAddr(HHD44780 const hHd44780,
                                        unsigned char   address);
unsigned char       hd44780ReadAddr(HHD44780 const      hHd44780,
                                         unsigned char *    address);
unsigned char       hd44780WriteChar(HHD44780 const        hHd44780,
                                          unsigned char    data);
unsigned char       hd44780ReadChar(HHD44780 const        hHd44780,
                                          unsigned char *   data);
const unsigned char *     hd44780WriteRAMString(HHD44780 const        hHd44780,
                                          const unsigned char * string);
const unsigned char *     hd44780WriteCGRAM(HHD44780 const        hHd44780,
                                          const unsigned char *   character,
                                          unsigned char     font);
unsigned char       hd44780ReadCGRAM(HHD44780 const        hHd44780,
                                          const unsigned char *   character,
                                          unsigned char     font);
unsigned int        hd44780InstructionInit(HHD44780 const   hHd44780,
                                           HD44780CLONE     hd44780Clone,
                                           unsigned char    functionSet,
                                           unsigned char    displayOnOffControl,
                                           unsigned char    entryModeSet);

/*******************************************************************************
*                              CONFIGURATION ERRORS
*******************************************************************************/
//<Add all pre-processor error messages here>
//#ifndef <Important Define>
//#error Important header file is missing!
//#endif


/*******************************************************************************
*
*                HD44780 ALPHANUMERIC LCD DRIVER MODULE MODULE END
*
*******************************************************************************/
#endif
