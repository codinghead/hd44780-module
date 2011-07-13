/*******************************************************************************
*
* PARALLEL BUS MODULE FOR C18 COMPILER AND PIC18
*
*******************************************************************************/

/*******************************************************************************
*
* This file provides the necessary information to create a bit-banged GPIO based
* parallel peripheral bus for use on a PIC18 with the C18 compiler. This 
* parallel bus can then be used by other modules to create a shared resource 
* parallel bus. The bus itself allows the user to define a bus with the desired
* number of data lines, a R/W (read/write) line and an RS (register select) 
* line. The naming used here comes from the interface on an HD44780 alphanumeric
* LCD display, but could equally apply to other parallel bus based peripherals.
* If you wish to use this software with another compiler and microcontroller,
* please look to see if there is a port available and, if not, use this file
* as a template
* All contents within this file are 'public' and to be used by end user
*
* Filename : pbif_c18.h
* Programmer(s) : Stuart Cording aka. CODINGHEAD
* 
********************************************************************************
* Note(s) : 
*
*******************************************************************************/

/*******************************************************************************
*
*                                PBIFC18 MODULE
*
*******************************************************************************/

#ifndef __PBIFC18_MODULE_PRESENT__
#define __PBIFC18_MODULE_PRESENT__

/*******************************************************************************
*                                 INCLUDE FILES
*******************************************************************************/
#ifdef __18CXX
#include <p18cxxx.h>
#endif


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
*                                   DATA TYPES
*******************************************************************************/

/*******************************************************************************
* New data type PBIFOBJ                                                    
* Description:
*   This decribes the object for the parallel bus interface. It requires the
* following elements:
* - The LAT register to which the R/W pin is connected
* - The bit number to which the R/W pin is connected (counting up from 0)
* - The LAT register to which the RS pin is connected
* - The bit number to which the RS pin is connected (counting up from 0)
* - The LAT register to which the data pins are connected
* - The PORT register to which the data pins are connected
* - The TRIS register to which the data pins are connected
* - A mask of 4 or 8 bits to define which of the data pins from the GPIO port
*   are connected to the LCD interface (must be consecutive)
* - A mutex variable used by the LCDIF module only
*******************************************************************************/
typedef struct PBIFOBJTYPE {
    volatile near unsigned char   * RW_LAT;
    unsigned char                   RW_BIT;
    volatile near unsigned char   * RS_LAT;
    unsigned char                   RS_BIT;
    volatile near unsigned char   * DATA_LAT;
    volatile near unsigned char   * DATA_PORT;
    volatile near unsigned char   * DATA_TRIS;
    unsigned char                   DATA_MASK;
    char                            mutex;
} PBIFOBJ;

/*******************************************************************************
* New data type PBIFLCDENOBJ
* Description:
*   This decribes the object for the enable pin for an individual LCD interdace
* It requires the following elements:
* - The LAT register to which the E pin is connected
* - The bit number to which the E pin is connected (counting up from 0)
*******************************************************************************/
typedef struct PBIFLCDENOBJTYPE {
    volatile near unsigned char   * E_LAT;
    unsigned char                   E_BIT;
} PBIFLCDENOBJ;

/*******************************************************************************
*                                GLOBAL VARIABLES
*******************************************************************************/


/*******************************************************************************
*                                    MACROS
*******************************************************************************/


/*******************************************************************************
*                              FUNCTION PROTOTYPES
*******************************************************************************/


/*******************************************************************************
*                              CONFIGURATION ERRORS
*******************************************************************************/
#ifndef __18CXX
#error This module requires the use of the C18 compiler.
#error If you wish to use this code with another platform, search in the 
#error directory where you found this file for a possible port.
#error This file is currently saved here: __FILE__
#endif


/*******************************************************************************
*
*                              PBIFC18 MODULE END
*
*******************************************************************************/
#endif
