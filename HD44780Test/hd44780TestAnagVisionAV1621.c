/*******************************************************************************
*
* HD44780 MODULE TEST PROGRAM
*
*******************************************************************************/

/*******************************************************************************
*
* Tests the HD44780 Interface module's functionality
*
* Filename : hd44780Test.c
* Version : V0.01
* Programmer(s) : Stuart Cording aka CODINGHEAD 
* 
********************************************************************************
* Note(s) : 
* V0.01 -   First cut
*******************************************************************************/

/*******************************************************************************
* Commenting notes
* ???? Question(s) regarding implementation or design specification.
* $$$$ Future function that needs to be implemented.
* @@@@ Old code to leave as-is because ....
* #### Technical issue not (satisfactorily) resolved.
*******************************************************************************/

/*******************************************************************************
*
*                           HD44780 MODULE TEST PROGRAM
*
*******************************************************************************/


/*******************************************************************************
*                                 INCLUDE FILES
*******************************************************************************/

                                    /******************************************/
                                    /* HI-TECH PICC / PICC-Lite compiler      */
                                    /******************************************/
#if	defined(__PICC__) || defined(__PICCLITE__)
    // PIC16 processor
    #include <pic.h>
                                    /******************************************/
                                    /* HI-TECH PICC-18 compiler               */
                                    /******************************************/
#elif	defined(__PICC18__)         
    // PIC18 processor
    #include <pic18.h>
                                    /******************************************/
                                    /* HI-TECH dsPICC compiler                */
                                    /******************************************/
#elif	defined(__DSPICC__) 
    // dsPIC or PIC24 processor
    #include <dspic.h>
                                    /******************************************/
                                    /* HI-TECH C for PIC32                    */
                                    /******************************************/
#elif defined(__PICC32__)   
    #include <pic32.h>
                                    /******************************************/
                                    /* Microchip C18 compiler                 */
                                    /******************************************/
#elif defined(__18CXX)      
    // PIC18 processor
    #include <p18cxxx.h>
    #include "..\lcdif_module\lcdif_c18.h"
    #include "..\lcdif_module\pbif_c18.h"
    #if defined(__18F4520)
        #include "p18f4520configbits.h"
    #elif defined(__18F4620)
        #include "p18f4620configbits.h"
    #else
        #error Chosen PIC18 device is not supported
    #endif     
                                    /******************************************/
                                    /* Microchip C30 compiler                 */
                                    /******************************************/
#elif defined(__PIC24F__)
	// PIC24F processor
	#include <p24Fxxxx.h>
	#include "..\lcdif_module\lcdif_c30.h"
    #include "..\lcdif_module\pbif_c30.h"
                                    /******************************************/
                                    /* Microchip C30 compiler                 */
                                    /******************************************/
#elif defined(__PIC24H__)
	// PIC24H processor
	#include <p24Hxxxx.h>
	#include "..\lcdif_module\lcdif_c30.h"
    #include "..\lcdif_module\pbif_c30.h"
                                    /******************************************/
                                    /* Microchip C30 compiler                 */
                                    /******************************************/
#elif defined(__dsPIC33F__)
	// dsPIC33F processor
	#include <p33Fxxxx.h>
	#include "..\lcdif_module\lcdif_c30.h"
    #include "..\lcdif_module\pbif_c30.h"
                                    /******************************************/
                                    /* Microchip C30 compiler                 */
                                    /******************************************/
#elif defined(__dsPIC30F__)
	// dsPIC30F processor
	#include <p30fxxxx.h>
	#include "..\lcdif_module\lcdif_c30.h"
    #include "..\lcdif_module\pbif_c30.h"
                                    /******************************************/
                                    /* Microchip C32 compiler                 */
                                    /******************************************/
#elif defined(__PIC32MX__)
	#include <p32xxxx.h>
	#include <plib.h>
	#pragma config POSCMOD=XT, FNOSC=PRIPLL
	#pragma config FPLLIDIV=DIV_2, FPLLMUL=MUL_20, FPLLODIV=DIV_1
	#pragma config FPBDIV=DIV_2, FWDTEN=OFF, CP=OFF, BWP=OFF

#else
	#error Unknown processor or compiler.
#endif

#include "..\hd44780_module\hd44780.h"

/*******************************************************************************
*                                 LOCAL DEFINES
*******************************************************************************/
#define WAITMS      91

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
unsigned char testString[] = "2010 Arizona";
unsigned char character1[] = { 0b10000000, 
                               0b00011111,
                               0b10000000, 
                               0b00011111,
                               0b10000000, 
                               0b00011111,
                               0b10000000, 
                               0b00011111,
                               0
};
unsigned int        timerValue;

/*******************************************************************************
*                             LOCAL FUNCTION PROTOTYPES
*******************************************************************************/
static void wait(unsigned int count);

/*******************************************************************************
*                            LOCAL CONFIGURATION ERRORS
*******************************************************************************/
#if !defined(PICDEM2PLUS_RED) && !defined(PICDEM2PLUS_GREEN)
#error No board specified! Please add one of the following preprocessor macros:\
- PICDEM2PLUS_RED - for old PICDEM 2 Plus Red Board \
- PICDEM2PLUS_GREEN - for newer PICDEM 2 Plus Green ROHS Board \
~\
For C18 select Project->Build Options...->Project, "MPLAB C18" tab and\
 then in "Proprocessor Macros" click "Add..." button.\
~
#endif

/*******************************************************************************
* main()
*
* Description: 
*   Main application code
*
* See also:
*
* Arguments: 
*   void
*
* Returns: 
*   void
*
* Callers: C start-up code
*
* Notes : 
*
*******************************************************************************/
#if	defined(__18CXX)
void main (void)
#else
int main(void)
#endif
{
    HD44780OBJ          hd44780ObjOne;
    HD44780OBJ          hd44780ObjTwo;
    HD44780NUM          hd44780One;
    HD44780NUM          hd44780Two;
    HHD44780            hHd44780One;
    HHD44780            hHd44780Two;
    LCDIFFP             lcdIfFuncPointers;
    LCDIFNUM            lcdIfOne;
    LCDIFNUM            lcdIfTwo;
    HLCDIF              hLcdIfOne;
    HLCDIF              hLcdIfTwo;
    PBIFOBJ             pbIf;
    LCDIFOBJ            lcdIfOneObj;
    LCDIFOBJ            lcdIfTwoObj;
    unsigned char       readData=0;
    unsigned char       readAddress=0;
    unsigned char       returnValue;
    unsigned char     * pString;
    
                                        /* Init modules                       */
    lcdifInit();
    
    hd44780Init();
                                        /* Make RW, RS and E lines outputs    */
#if	defined(__18CXX)
                                        /* Clear LATD and LATA registers      */
                                        /* PORTA is data port; PORTD control  */
    LATD = 0;
    LATA = 0;
                                        /* For AV1621 control pins:           */
                                        /*  RD5 = RW                          */
                                        /*  RD4 = RS                          */
                                        /*  RD3 = E                           */
    TRISDbits.TRISD5 = 0;
    TRISDbits.TRISD4 = 0;
    TRISDbits.TRISD3 = 0;
                                        /* Turn on the on-board LCD display   */
                                        /* (uses RD7)                         */
    TRISDbits.TRISD7 = 0;
    LATDbits.LATD7 = 1;

#endif                                  
    
                                        /* Fill parallel bus interface struct */
    pbIf.RW_LAT     = &LATD;
    pbIf.RW_BIT     = (1 << 5);
    pbIf.RS_LAT     = &LATD;
    pbIf.RS_BIT     = (1 << 4);
    pbIf.DATA_LAT   = &LATA;
    pbIf.DATA_PORT  = &PORTA;
    pbIf.DATA_TRIS  = &TRISA;
    pbIf.DATA_MASK  = 0xFF;

                                        /* Fill lcd interface struct          */
    lcdIfOneObj.E_LAT      = &LATD;
    lcdIfOneObj.E_BIT      = (1 << 3);


    lcdIfOneObj.pbIfObject = &pbIf;
                                        /* Create an LCD interface            */
    lcdIfOne = lcdifCreate(&lcdIfOneObj);
                                        /* Open the interface                 */
    hLcdIfOne = lcdifOpen(lcdIfOne);

                                        /* Fill an LCD function pointers      */
                                        /* struct                             */
    lcdIfFuncPointers.pGetBus = lcdifGetPb;
    lcdIfFuncPointers.pReturnBus = lcdifReturnPb;
    lcdIfFuncPointers.pWriteData = lcdifWriteData;
    lcdIfFuncPointers.pReadData = lcdifReadData;
    lcdIfFuncPointers.pWriteInstr = lcdifWriteInstruction;
    lcdIfFuncPointers.pReadAddr = lcdifReadAddress;
    lcdIfFuncPointers.p4BitFunctionSet = lcdif4BitFunctionSet;

                                        /* Create an HD44780 display object   */
    hd44780One = hd44780Create(hLcdIfOne, &lcdIfFuncPointers, &hd44780ObjOne);
    
                                        /* Open an HD44780 object             */
    hHd44780One = hd44780Open(hd44780One);
    
    do
    {
        returnValue = hd44780InstructionInit(hHd44780One, KS0066U, 
                                             FS_5X8DOTS & FS_2LINE,
                                             DOFC_BLINKINGOFF & DOFC_CURSOROFF & 
                                             DOFC_DISPLAYON,
                                             EMS_CURSORMOVE & EMS_INCREMENT);
        if (returnValue > 1)
        {
            wait(returnValue);
        }    
    }    
    while (returnValue != 0);    
    
    //hd44780DisplayControl(hHd44780One,
    //                      DOFC_BLINKINGOFF & DOFC_CURSOROFF & DOFC_DISPLAYON);
    
    hd44780WriteChar(hHd44780One, '1');
    hd44780WriteChar(hHd44780One, '4');
    hd44780WriteChar(hHd44780One, '2');
    hd44780WriteChar(hHd44780One, '2');
    hd44780WriteChar(hHd44780One, 'G');
    hd44780WriteChar(hHd44780One, 'S');
    hd44780WriteChar(hHd44780One, 'M');
    hd44780WriteChar(hHd44780One, ' ');
    hd44780WriteChar(hHd44780One, 'M');
    hd44780WriteChar(hHd44780One, 'A');
    hd44780WriteChar(hHd44780One, 'S');
    hd44780WriteChar(hHd44780One, 'T');
    hd44780WriteChar(hHd44780One, 'E');
    hd44780WriteChar(hHd44780One, 'R');
    hd44780WriteChar(hHd44780One, 's');
    
#if 0    
    hd44780EntryModeSet(hHd44780One, EMS_DISPLAYSHIFT & EMS_INCREMENT);
    

    hd44780WriteChar(hHd44780One, 'B');
    hd44780WriteChar(hHd44780One, 'e');
    hd44780WriteChar(hHd44780One, 'l');
    hd44780WriteChar(hHd44780One, 'l');
    hd44780WriteChar(hHd44780One, 'o');
    hd44780WriteChar(hHd44780One, '!');
    
    do
    {
        returnValue = hd44780EntryModeSet(hHd44780One, EMS_CURSORMOVE & EMS_DECREMENT);
    } while (returnValue != 1);
    
    do
    {
        returnValue = hd44780WriteChar(hHd44780One, 'e');
    } while (returnValue != 1);
    
    hd44780WriteChar(hHd44780One, 'e');
    hd44780WriteChar(hHd44780One, 'c');
    hd44780WriteChar(hHd44780One, 'i');
    hd44780WriteChar(hHd44780One, 'p');
    hd44780WriteChar(hHd44780One, 'S');
    hd44780WriteChar(hHd44780One, ':');
    
    hd44780EntryModeSet(hHd44780One, EMS_CURSORMOVE & EMS_INCREMENT);
#endif
    hd44780SetCursorAddr(hHd44780One, 0x40);
    //hd44780WriteString(hHd44780One, "This is a test ;o)");
    pString = testString;
    do
    {
        pString = hd44780WriteRAMString(hHd44780One, pString);
    } while (pString != (unsigned char *) 0);
    
    
#if 0    
    hd44780ReturnHome(hHd44780One);
    hd44780ClearDisplay(hHd44780One);
    hd44780WriteChar(hHd44780One, 0);
    hd44780WriteChar(hHd44780One, 1);
    hd44780WriteChar(hHd44780One, 2);
    hd44780WriteChar(hHd44780One, 3);
    hd44780WriteChar(hHd44780One, 4);
    hd44780WriteChar(hHd44780One, 5);
    hd44780SetCGRAMAddr(hHd44780One,0x00);
    hd44780WriteCGRAM(hHd44780One, character1, CGRAMFONT_5X8);
    hd44780SetCursorAddr(hHd44780One, 0x40);
    hd44780WriteChar(hHd44780One, 0);
    hd44780WriteChar(hHd44780One, 0);
    hd44780WriteChar(hHd44780One, 0);
    hd44780WriteChar(hHd44780One, 0);
    
#endif   
    
    TRISBbits.TRISB0 = 0;
    /* Congratulations! If we stop here all tests passed */
    while(1)
    {
        volatile unsigned int x;
        
        for(x=0; x<0x0FFF; x++)
        {
        }
        LATBbits.LATB0 = !LATBbits.LATB0;
    }    
}

void wait(unsigned int count)
{
    /* count of 1 is approx 10.5us */
    volatile unsigned int x;
    
    for(x = 0; x < count; x++)
    {
        Nop();
    }    
}    
/*******************************************************************************
*
*                     LCD INTERFACE MODULE TEST PROGRAM END
*
*******************************************************************************/
