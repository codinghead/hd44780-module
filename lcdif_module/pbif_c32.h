/*******************************************************************************
*
* PARALLEL BUS MODULE FOR C32 COMPILER AND PIC32
*
*******************************************************************************/

/*******************************************************************************
*
* This file provides the necessary information to create a bit-banged GPIO based
* parallel peripheral bus for use on a PIC32 with the C32 compiler. This 
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
* Filename : pbif_c32.h
* Programmer(s) : Stuart Cording aka. CODINGHEAD
* 
********************************************************************************
* Note(s) : 
* 
*
*******************************************************************************/

/*******************************************************************************
*
*                                PBIFC32 MODULE
*
*******************************************************************************/

#ifndef __PBIFC32_MODULE_PRESENT__
#define __PBIFC32_MODULE_PRESENT__

/*******************************************************************************
*                                 INCLUDE FILES
*******************************************************************************/
                                    /******************************************/
                                    /* Microchip C32 compiler                 */
                                    /******************************************/
#if defined(__PIC32MX__)
	// PIC32 processor
	#include <p32xxxx.h>


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
*   This decribes the object for the parallel bus interface
*******************************************************************************/
typedef struct PBIFOBJTYPE {
    volatile unsigned int         * RW_LAT;
    unsigned char				    RW_BIT;
    volatile unsigned int         * RS_LAT;
    unsigned char				    RS_BIT;
    volatile unsigned int         * DATA_LAT;
    volatile unsigned int         * DATA_PORT;
    volatile unsigned int         * DATA_TRIS;
    unsigned char				    DATA_MASK;
    unsigned int                    mutex;
} PBIFOBJ;


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
#ifndef __PIC32MX__
#error This module requires the use of the C32 compiler.
#error If you wish to use this code with another platform, search in the 
#error directory where you found this file for a possible port.
#error This file is currently saved here: __FILE__
#endif


/*******************************************************************************
*
*                              PBIFC32 MODULE END
*
*******************************************************************************/
#endif
