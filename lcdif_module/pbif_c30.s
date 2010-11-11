
/*******************************************************************************
*
* PARALLEL BUS MODULE FOR C30 COMPILER AND PIC24/dsPIC
*
*******************************************************************************/

/*******************************************************************************
*
* This module is used to provide the interface for an HD44780 LCD display, based
* upon the bit-banged GPIO based parallel bus provided the "pbif_<compiler>.h"
* module
*
* Filename : pbif_c30.s
* Version : V0.01
* Programmer(s) : Stuart Cording aka. CODINGHEAD
* 
********************************************************************************
* Note(s) : 
* V0.01 -   First cut
*
*******************************************************************************/

/*******************************************************************************
*
*                                PBIFC30 MODULE
*
*******************************************************************************/

/*******************************************************************************
*                                 INCLUDE FILES
*******************************************************************************/
                                    /******************************************/
                                    /* Microchip C30 compiler                 */
                                    /******************************************/
    .ifdef __PIC24F
	; PIC24F processor
	.include "p24Fxxxx.inc"
                                    /******************************************/
                                    /* Microchip C30 compiler                 */
                                    /******************************************/
    .elseif __PIC24H
	; PIC24H processor
	.include "p24Hxxxx.inc"
                                    /******************************************/
                                    /* Microchip C30 compiler                 */
                                    /******************************************/
    .elseif __dsPIC33F
	; dsPIC33F processor
	.include "p33Fxxxx.inc"
                                    /******************************************/
                                    /* Microchip C30 compiler                 */
                                    /******************************************/
    .elseif __dsPIC30F
	; dsPIC30F processor
	.include "p30fxxxx.inc"
    .endif


/*******************************************************************************
*                                 LOCAL DEFINES
*******************************************************************************/

/*******************************************************************************
* Summary:
*   Used to indicate that the LCD interface object is open and in use
*******************************************************************************/
; #define LCDIF_OPEN      (0x01 << 7)


/*******************************************************************************
*                                LOCAL CONSTANTS
*******************************************************************************/


/*******************************************************************************
*                                LOCAL DATA TYPES
*******************************************************************************/


/*******************************************************************************
*                                  LOCAL TABLES
*******************************************************************************/


/*******************************************************************************
*                             LOCAL GLOBAL VARIABLES
*******************************************************************************/


/*******************************************************************************
*                             LOCAL FUNCTION PROTOTYPES
*******************************************************************************/


/*******************************************************************************
*                            LOCAL CONFIGURATION ERRORS
*******************************************************************************/

                                    ; #### ToDo: is it possible to find out if 
                                    ; MPASM was called to build code for a 
                                    ; PIC18?
    .ifndef __C30COFF
    .error This module requires the use of the C30 compiler.
    .error If you wish to use this code with another platform, search in the 
    .error directory where you found this file for a possible port.
    .error This file is currently saved here: __FILE__
    .endif

/*******************************************************************************
*                             GLOBAL FUNCTIONS
*******************************************************************************/
    .text
    
    .global _pbifGetBusMutex
    .global _pbifReturnBusMutex
    
/*******************************************************************************
* pbifGetBusMutex()
*
* Summary: 
*   This function provides a mutex for the parallel bus interface. This ensures
* that only one caller at a time can use the resource. Unfortunately there is 
* no atomic test-and-set instruction in the PIC18 instruction set for the case
* where we only have a pointer to the variable we want to test. Additionally
* there are no indirect file pointers available for free use as they are all 
* used by the compiler. An indirect file pointer (FSRx) is needed because we
* receive the address of the variable we need to use for our mutex here, rather
* than the content of that variable. To ensure that this function cannot be 
* interrupted, we switch of interrupts (if enabled), check the mutex, note the
* result to return to the caller, and re-enable interrupts (if enabled). This 
* ensures that, for the C environment, the use of the FSR registers goes 
* unnoticed.
*
* See also:
*   pbIfReturnMutex
*
* Arguments: 
*   pPbIfFlag       - 16-bit address pointer to flag variable where parallel bus
*                     interface flag is stored
*
* Returns: 
*   1               - if we got the mutex
*   0               - if we did not get the mutex
*
* Callers: 
*   lcdifWriteData(), lcdifReadData(), lcdifWriteInstruction(), 
*   lcdifReadAddress()
*
* Notes : 
*   1 - This function uses two bytes of stack space to store and restore the
*       value in FSR0
*******************************************************************************/
_pbifGetBusMutex:
    btsts.z [w0], #0
    bra     nz,didntGetMutex
gotMutex:
    retlw   #1, w0
didntGetMutex:
    retlw   #0, w0

/*******************************************************************************
* pbifReturnBusMutex()
*
* Summary: 
*   This function returns the mutex that was taken with pbIfGetMutex
*
* See also:
*   pbIfGetMutex
*
* Arguments: 
*   pPbIfFlag       - 16-bit address pointer to flag variable where parallel bus
*                     interface flag is stored
*
* Returns: 
*   None
*
* Callers: 
*   lcdifWriteData(), lcdifReadData(), lcdifWriteInstruction(), 
*   lcdifReadAddress()
*
* Notes : 
*   1 - This function uses three bytes of stack space to store and restore the
*       value in FSR0 and store the status of the INTCON.GIE bit
*   2 - This function actually returns 0x01 in WREG, but the caller doesn't 
*       use it
*******************************************************************************/
_pbifReturnBusMutex:
    bclr    [w0], #0
    return
    .end
    
    
/*******************************************************************************
*
*                             PBIFC18 MODULE END
*
*******************************************************************************/
