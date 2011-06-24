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
* Version : V0.01
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
*   These defines are used by the user to define the settings they wish to use
* in conjunction with the available HD44780 instructions
*******************************************************************************/
                                        /* Entry Mode Set settings            */
#define EMS_CURSORMOVE      0xFE
#define EMS_DISPLAYSHIFT    0xFF
#define EMS_DECREMENT       0xFD
#define EMS_INCREMENT       0xFF
                                        /* Display On/Off Control settings    */
#define DOFC_BLINKINGOFF    0xFE
#define DOFC_BLINKINGON     0xFF
#define DOFC_CURSOROFF      0xFD
#define DOFC_CURSORON       0xFF
#define DOFC_DISPLAYOFF     0xFB
#define DOFC_DISPLAYON      0xFF
                                        /* Cursor or Display Shift settings   */
#define CODS_SHIFTLEFT      0xFB
#define CODS_SHIFTRIGHT     0xFF
#define CODS_CURSORMOVE     0xF7
#define CODS_DISPLAYSHIFT   0xFF
                                        /* Function Set settings              */
#define FS_5X8DOTS          0xFD
#define FS_5X10DOTS         0xFF
#define FS_1LINE            0xF7
#define FS_2LINE            0xFF
#define FS_4BITBUS          0xEF
#define FS_8BITBUS          0xFF

/*******************************************************************************
* Summary:
*   These defines are used by the user to define the CGRAM font size when 
* creating new fonts
*******************************************************************************/
#define CGRAMFONT_5X8       8
#define CGRAMFONT_5X10      10

/*******************************************************************************
* Summary:
*   Signals that hd44780Destroy() failed to deallocate requested buffer object
*******************************************************************************/
#define HD44780_DESTROY_FAIL         0x00

/*******************************************************************************
* Summary:
*   Signals that hd44780Destroy() successfully deallocated requested object
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
* New data type LCDIFFP                                                    
* Description:
*   This structure allows the end user to store pointers to the functions
* implemented in the LCD interface layer which abstracts this module from the
* hardware interface being used
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
* which is currently in use
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
* instruction initialisation depending on the chip set being used
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
