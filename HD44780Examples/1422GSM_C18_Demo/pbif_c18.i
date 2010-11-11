#line 1 "C:\MyMPLABWork\Personal\CODINGHEAD\MASTERS2010-1422GSM\HD44780I\lcdif_module\pbif_c18.asm"
#line 1 "C:\MyMPLABWork\Personal\CODINGHEAD\MASTERS2010-1422GSM\HD44780I\lcdif_module\pbif_c18.asm"
;*******************************************************************************
;
; PARALLEL BUS MODULE FOR C18 COMPILER AND PIC18
;
;******************************************************************************/

;*******************************************************************************
;
; This module is used to provide the interface for an HD44780 LCD display, based
; upon the bit-banged GPIO based parallel bus provided the "pbif_<compiler>.h"
; module
;
; Filename : pbif_c18.c
; Version : V0.01
; Programmer(s) : Stuart Cording aka. CODINGHEAD
; 
;*******************************************************************************
; Note(s) : 
; V0.01 -   First cut
;
;******************************************************************************/

;*******************************************************************************
;
;                                PBIFC18 MODULE
;
;******************************************************************************/


;*******************************************************************************
;                                 INCLUDE FILES
;******************************************************************************/


;*******************************************************************************
;                                 LOCAL DEFINES
;******************************************************************************/

;*******************************************************************************
; Summary:
;   Used to indicate that the LCD interface object is open and in use
;******************************************************************************/
;#define LCDIF_OPEN      (0x01 << 7)


;*******************************************************************************
;                                LOCAL CONSTANTS
;******************************************************************************/


;*******************************************************************************
;                                LOCAL DATA TYPES
;******************************************************************************/


;*******************************************************************************
;                                  LOCAL TABLES
;******************************************************************************/


;*******************************************************************************
;                             LOCAL GLOBAL VARIABLES
;******************************************************************************/


;*******************************************************************************
;                             LOCAL FUNCTION PROTOTYPES
;******************************************************************************/


;*******************************************************************************
;                            LOCAL CONFIGURATION ERRORS
;******************************************************************************/

                                    ; #### ToDo: is it possible to find out if 
                                    ; MPASM was called to build code for a 
                                    ; PIC18?
    if 0
    ifndef 1
    error This module requires the use of the C18 compiler.
    error If you wish to use this code with another platform, search in the 
    error directory where you found this file for a possible port.
    error This file is currently saved here: "C:\MyMPLABWork\Personal\CODINGHEAD\MASTERS2010-1422GSM\HD44780I\lcdif_module\pbif_c18.asm"
    endif
    endif

;*******************************************************************************
; pbifGetBusMutex()
;
; Summary: 
;   This function provides a mutex for the parallel bus interface. This ensures
; that only one caller at a time can use the resource. Unfortunately there is 
; no atomic test-and-set instruction in the PIC18 instruction set for the case
; where we only have a pointer to the variable we want to test. Additionally
; there are no indirect file pointers available for free use as they are all 
; used by the compiler. An indirect file pointer (FSRx) is needed because we
; receive the address of the variable we need to use for our mutex here, rather
; than the content of that variable. To ensure that this function cannot be 
; interrupted, we switch of interrupts (if enabled), check the mutex, note the
; result to return to the caller, and re-enable interrupts (if enabled). This 
; ensures that, for the C environment, the use of the FSR registers goes 
; unnoticed.
;
; See also:
;   pbIfReturnMutex
;
; Arguments: 
;   pPbIfFlag       - 16-bit address pointer to flag variable where parallel bus
;                     interface flag is stored
;
; Returns: 
;   1               - if we got the mutex
;   0               - if we did not get the mutex
;
; Callers: 
;   lcdifWriteData(), lcdifReadData(), lcdifWriteInstruction(), 
;   lcdifReadAddress()
;
; Notes : 
;   1 - This function uses two bytes of stack space to store and restore the
;       value in FSR0
;******************************************************************************/
    code
pbifGetBusMutex:
    global  pbifGetBusMutex
                                    ; Following the C18 calling convention, this
                                    ; function, when called by a C program,
                                    ; will have the:
                                    ; argument pPbIfFlag: in FSR1 - 1 (high 
                                    ;                     address byte)
                                    ;                        FSR1 - 2 (low
                                    ;                     address byte)
                                    ; returns           : in WREG (8-bit value)
                                    
                                    ; First, if interrupts are enabled, disable
                                    ; them noting current state first by saving
                                    ; current value on stack (SP = FSR1)
                                    ; Clear the location, then set bit 0 if
                                    ; interrupts are active or simply increment
                                    ; FSR1 if they are not
    clrf    INDF1
    btfsc   INTCON, GIE, A
    goto    interruptsOn1
    goto    interruptsOff1
                                    ; If interrupts are on, save status on 
                                    ; stack and turn interrupts off
interruptsOn1:
    bsf     POSTINC1, 0, A
    bcf     INTCON, GIE, A
    goto    getMutex
                                    ; Else just increment stack pointer
interruptsOff1:
    movf    POSTINC1, F

getMutex:
                                    ; Now save the FSR0 register pair on the 
                                    ; stack (SP = FSR1)
    movff   FSR0L, POSTINC1
    movff   FSR0H, POSTINC1
                                    ; Copy address in FSR1 - 4 and FSR1 - 5 into
                                    ; FSR0L and FSR0H
    movlw   -.4
    movff   PLUSW1, FSR0H
    movlw   -.5
    movff   PLUSW1, FSR0L
                                    ; Now get the mutex - we are pointing to
                                    ; variable with FSR0
                                    ; 1 is subtracted from the mutex variable
                                    ; If the result is "ZERO", the resource
                                    ; belongs to us, if not, we add 1 back to
                                    ; the value and the resource is not ours
    movlw   .1
    subwf   INDF0, F, A
    btfsc   STATUS, Z, A
                                    ; 1 is still in WREG, so we can just clear
                                    ; up and return
gotMutex:
    goto    reinstateStatus
                                    ; Otherwise add 1 back to our mutex var.
                                    ; add clear WREG so that caller knows that
                                    ; resource is busy
noMutex:
    addwf   INDF0, F, A
    movlw   0x00
    
reinstateStatus:
                                    ; Reinstate the FSR0 register pair
    movf    POSTDEC1, F
    movff   POSTDEC1, FSR0H
    movff   POSTDEC1, FSR0L
                                    ; If interrupts were enabled, re-enable them
                                    ; status is on the stack
    btfsc   INDF1, 0, A
    bsf     INTCON, GIE, A
    
    return

;*******************************************************************************
; pbifReturnBusMutex()
;
; Summary: 
;   This function returns the mutex that was taken with pbIfGetMutex
;
; See also:
;   pbIfGetMutex
;
; Arguments: 
;   pPbIfFlag       - 16-bit address pointer to flag variable where parallel bus
;                     interface flag is stored
;
; Returns: 
;   None
;
; Callers: 
;   lcdifWriteData(), lcdifReadData(), lcdifWriteInstruction(), 
;   lcdifReadAddress()
;
; Notes : 
;   1 - This function uses three bytes of stack space to store and restore the
;       value in FSR0 and store the status of the INTCON.GIE bit
;   2 - This function actually returns 0x01 in WREG, but the caller doesn't 
;       use it
;******************************************************************************/
pbifReturnBusMutex:
    global  pbifReturnBusMutex
                                    ; Following the C18 calling convention, this
                                    ; function, when called by a C program,
                                    ; will have the:
                                    ; argument pPbIfFlag: in FSR1 - 1 (high 
                                    ;                     address byte)
                                    ;                        FSR1 - 2 (low
                                    ;                     address byte)
                                    ; returns           : in WREG (8-bit value)
                                    
                                    ; First, if interrupts are enabled, disable
                                    ; them noting current state first by saving
                                    ; current value on stack (SP = FSR1)
                                    ; Clear the location, then set bit 0 if
                                    ; interrupts are active or simply increment
                                    ; FSR1 if they are not
    clrf    INDF1
    btfsc   INTCON, GIE, A
    goto    interruptsOn2
    goto    interruptsOff2
                                    ; If interrupts are on, save status on 
                                    ; stack and turn interrupts off
interruptsOn2:
    bsf     POSTINC1, 0, A
    bcf     INTCON, GIE, A
    goto    returnMutex
                                    ; Else just increment stack pointer
interruptsOff2:
    movf    POSTINC1, F

returnMutex:
                                    ; Now save the FSR0 register pair on the 
                                    ; stack (SP = FSR1)
    movff   FSR0L, POSTINC1
    movff   FSR0H, POSTINC1
                                    ; Copy address in FSR1 - 4 and FSR1 - 5 into
                                    ; FSR0L and FSR0H
    movlw   -.4
    movff   PLUSW1, FSR0H
    movlw   -.5
    movff   PLUSW1, FSR0L
    
    movlw   .1
                                    ; Check the mutex is actually 0x00
    movf    INDF0, F, A
                                    ; If not, don't modify it just in case
                                    ; caller is trying to return a mutex he
                                    ; doesn't really own
    btfsc   STATUS, Z
                                    ; Now return the mutex - we are pointing to
                                    ; variable with FSR0 - if it was 0x00
    addwf   INDF0, F, A
                                    ; Reinstate the status back to what is was
                                    ; before we were called (implemented in 
                                    ; pbIfGetMutex)
    goto reinstateStatus
    end
    
    
;*******************************************************************************
;
;                             PBIFC18 MODULE END
;
;******************************************************************************/
