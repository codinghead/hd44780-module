/*******************************************************************************
*
* LCD INTERFACE MODULE FOR C32 COMPILER AND PIC32
*
*******************************************************************************/

/*******************************************************************************
*
* This file provides the necessary information required to create an LCD
* interface for use with the HD44780 module. Since this module is hardware
* dependent, it is written for use with the C18 compiler, targetted for PIC18
* microcontrollers. If you wish to use the HD44780 module with another
* microcontroller, see if there is a port of this file for your hardware or use
* this file as the basis for a port.
* All contents within this file are 'public' and to be used by end user
*
* Filename : lcdif_c32.h
* Version : V0.01
* Programmer(s) : Stuart Cording aka. CODINGHEAD 
* 
********************************************************************************
* Note(s) : 
* See the lcdif_<compiler>.c file for the version changes and notes for this 
* module
*
*******************************************************************************/

/*******************************************************************************
*
*                                LCDIFC32 MODULE
*
*******************************************************************************/

#ifndef __LCDIFC32_MODULE_PRESENT__
#define __LCDIFC32_MODULE_PRESENT__

/*******************************************************************************
*                                 INCLUDE FILES
*******************************************************************************/
                                    /******************************************/
                                    /* Microchip C32 compiler                 */
                                    /******************************************/
#if defined(__PIC32MX__)
	// PIC32 processor
	#include <p32xxxx.h>
#endif
#include "pbif_c32.h"

/*******************************************************************************
*                                    EXTERNS
*******************************************************************************/


/*******************************************************************************
*                             DEFAULT CONFIGURATION
*******************************************************************************/


/*******************************************************************************
*                                    DEFINES
*******************************************************************************/

/*******************************************************************************
* Summary:
*   This is used by the lcdifGetBusWidth function to tell upper layer the width
* of the data bus. This is required for the "Initialising by Instruction" 
* process
*******************************************************************************/
#define     BUS4BITSWIDE    0
#define     BUS8BITSWIDE    1

/*******************************************************************************
*                                   DATA TYPES
*******************************************************************************/

/*******************************************************************************
* New data type LCDIFNUM                                                    
* Description:
*   Used to hold the LCD interface number issued by the LCD IF Module
*******************************************************************************/
typedef unsigned int LCDIFNUM;

/*******************************************************************************
* New data type LCDIFOBJTYPE                                                     
* Description:
*   Holds the object information for each LCD interface object created
*******************************************************************************/
typedef struct LCDIFOBJTYPE {
    volatile unsigned int         * E_LAT;
    unsigned char                   E_BIT;
    PBIFOBJ                       * pbIfObject;
    LCDIFNUM                        lcdIfNum;
    unsigned char                   lcdIfFlags;
    struct LCDIFOBJTYPE           * nextLcdIfObj;
} LCDIFOBJ;

/*******************************************************************************
* New data type HLCDIF
* Description:
*   Holds a pointer to an LCDIF object
*******************************************************************************/
typedef LCDIFOBJ * HLCDIF;


/*******************************************************************************
*                                GLOBAL VARIABLES
*******************************************************************************/


/*******************************************************************************
*                                    MACROS
*******************************************************************************/


/*******************************************************************************
*                              FUNCTION PROTOTYPES
*******************************************************************************/
void            lcdifInit(void);
void            lcdifDeinit(void);

LCDIFNUM        lcdifCreate(LCDIFOBJ            * const lcdIfObj);
unsigned char   lcdifDestroy(LCDIFNUM                   lcdIfNumber);

HLCDIF          lcdifOpen(LCDIFNUM                      lcdIfNumber);
LCDIFNUM        lcdifClose(HLCDIF                 const hLcdIf);

unsigned char   lcdifGetPb(HLCDIF                 const hLcdIf);
void            lcdifReturnPb(HLCDIF              const hLcdIf);

unsigned char   lcdifWriteData(HLCDIF             const hLcdIf,
                               unsigned char            data);
unsigned char   lcdifReadData(HLCDIF              const hLcdIf,
                              unsigned char     * const data);
                               
unsigned char   lcdifWriteInstruction(HLCDIF      const hLcdIf,
                                      unsigned char     instruction);
unsigned char   lcdifReadAddress(HLCDIF           const hLcdIf,
                                 unsigned char  * const address);

unsigned char   lcdif4BitFunctionSet(HLCDIF       const hLcdIf,
                                      unsigned char     instruction);

unsigned char   lcdifGetPbBusWidth(HLCDIF         const hLcdIf);

/*******************************************************************************
*                              CONFIGURATION ERRORS
*******************************************************************************/
#ifndef __PIC32MX__
#error This module requires the use of the C32 compiler.
#error If you wish to use this code with another platform, search in the 
#error directory where you found this file for a possible port.
#error This file is currently saved here: __FILE__
#endif


/*******************************************************************************
*
*                               LCDIFC18 MODULE END
*
*******************************************************************************/
#endif
