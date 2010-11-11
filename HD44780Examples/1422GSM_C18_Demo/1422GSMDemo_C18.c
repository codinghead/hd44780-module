/*******************************************************************************
*
* 1422GSM Multiple LCD Module Support Demo
*
*******************************************************************************/

/*******************************************************************************
*
* Demo of multiple LCD module support for the 1422GSM Class at MASTERs 2010
*
* Filename : 1422GSMDemo_C18.c
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
unsigned char testString1[] = "Welcome to";
unsigned char testString2[] = "Arizona!";


/*******************************************************************************
*                             LOCAL FUNCTION PROTOTYPES
*******************************************************************************/
static void wait(unsigned int count);

/*******************************************************************************
*                            LOCAL CONFIGURATION ERRORS
*******************************************************************************/


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
void main (void)
{
    HD44780OBJ          hd44780ObjOnBoard;
    HD44780OBJ          hd44780ObjOffBoard;
    HD44780NUM          hd44780OnBoardNum;
    HD44780NUM          hd44780OffBoardNum;
    HHD44780            hHd44780OnBoard;
    HHD44780            hHd44780OffBoard;
    LCDIFFP             lcdIfFuncPointers;
    LCDIFNUM            lcdIfOnBoardNum;
    LCDIFNUM            lcdIfOffBoardNum;
    HLCDIF              hLcdIfOnBoard;
    HLCDIF              hLcdIfOffBoard;
    PBIFOBJ             pbIf;
    LCDIFOBJ            lcdIfOnBoardObj;
    LCDIFOBJ            lcdIfOffBoardObj;
    unsigned char       readData=0;
    unsigned char       readAddress=0;
    unsigned char       returnValue;
    unsigned char     * pString;
    
                                        /* Init modules                       */
    lcdifInit();
    
    hd44780Init();
                                        
                                        /* Clear LATD register. Will be used  */
                                        /* as control and data lines for both */
                                        /* displays                           */
    LATD = 0;
                                        /* Clear LATA register. Will be used  */
                                        /* as E control for off-baord LCD     */
                                        /* Also make digital                  */
    LATA = 0;
    ADCON1 = 0x0F;
                                        /* Make RW, RS and E lines outputs    */
                                        /* On GREEN PICDEM 2 Plus these are:  */
                                        /*  RD5 = RW                          */
                                        /*  RD4 = RS                          */
                                        /*  RD6 = E                           */
    TRISDbits.TRISD5 = 0;
    TRISDbits.TRISD4 = 0;
    TRISDbits.TRISD6 = 0;
                                        /* Off-board E output is on RA1       */
    TRISAbits.TRISA1 = 0;
                                        /* Turn on the LCD on-board display's */
                                        /* power supply (uses RD7)            */
    TRISDbits.TRISD7 = 0;
    LATDbits.LATD7 = 1;

                                        /* Fill parallel bus interface struct */
    pbIf.RW_LAT     = &LATD;
    pbIf.RW_BIT     = (1 << 5);
    pbIf.RS_LAT     = &LATD;
    pbIf.RS_BIT     = (1 << 4);
    pbIf.DATA_LAT   = &LATD;
    pbIf.DATA_PORT  = &PORTD;
    pbIf.DATA_TRIS  = &TRISD;
    pbIf.DATA_MASK  = 0x0F;

                                        /* Fill on-board lcd interface struct */
    lcdIfOnBoardObj.E_LAT      = &LATD;
    lcdIfOnBoardObj.E_BIT      = (1 << 6);
    lcdIfOnBoardObj.pbIfObject = &pbIf;
                                        /* Fill off-board lcd interface       */
                                        /* struct                             */
    lcdIfOffBoardObj.E_LAT      = &LATA;
    lcdIfOffBoardObj.E_BIT      = (1 << 1);
    lcdIfOffBoardObj.pbIfObject = &pbIf;

                                        /* Create an on-board LCD interface   */
    lcdIfOnBoardNum = lcdifCreate(&lcdIfOnBoardObj);
                                        /* Create an off-board LCD interface  */
    lcdIfOffBoardNum = lcdifCreate(&lcdIfOffBoardObj);

                                        /* Open the interface to on-board LCD */
    hLcdIfOnBoard = lcdifOpen(lcdIfOnBoardNum);
                                        /* Open the interface to off-board LCD*/
    hLcdIfOffBoard = lcdifOpen(lcdIfOffBoardNum);
    
                                        /* Fix nibble swap on address reads   */
                                        /* for on-board LCD module            */
    lcdifFixNibbleSwap(hLcdIfOnBoard);
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
                                        /* for the on-board LCD module        */
    hd44780OnBoardNum = hd44780Create(hLcdIfOnBoard, &lcdIfFuncPointers, 
                                                           &hd44780ObjOnBoard);
                                        /* Open the on-board HD44780 object   */
    hHd44780OnBoard = hd44780Open(hd44780OnBoardNum);
    
                                        /* Create an HD44780 display object   */
                                        /* for the off-board LCD module       */
    hd44780OffBoardNum = hd44780Create(hLcdIfOffBoard, &lcdIfFuncPointers, 
                                                           &hd44780ObjOffBoard);
                                        /* Open the off-board HD44780 object  */
    hHd44780OffBoard = hd44780Open(hd44780OffBoardNum);
    
                                        /* Perform software init of on-board  */
                                        /* LCD display                        */
    do
    {
        returnValue = hd44780InstructionInit(hHd44780OnBoard, HD44780U, 
                                             FS_5X8DOTS & FS_2LINE,
                                             DOFC_BLINKINGOFF & DOFC_CURSOROFF & 
                                             DOFC_DISPLAYOFF,
                                             EMS_CURSORMOVE & EMS_INCREMENT);
        if (returnValue > 1)
        {
            wait(returnValue);
        }    
    }    
    while (returnValue != 0);    

                                        /* Perform software init of off-board */
                                        /* LCD display                        */
    do
    {
        returnValue = hd44780InstructionInit(hHd44780OffBoard, KS0066U, 
                                             FS_5X8DOTS & FS_2LINE,
                                             DOFC_BLINKINGOFF & DOFC_CURSOROFF & 
                                             DOFC_DISPLAYOFF,
                                             EMS_CURSORMOVE & EMS_INCREMENT);
        if (returnValue > 1)
        {
            wait(returnValue);
        }    
    }    
    while (returnValue != 0);
    
    hd44780DisplayControl(hHd44780OnBoard,
                          DOFC_BLINKINGON & DOFC_CURSOROFF & DOFC_DISPLAYON);

    hd44780DisplayControl(hHd44780OffBoard,
                          DOFC_BLINKINGON & DOFC_CURSOROFF & DOFC_DISPLAYON);

    hd44780WriteChar(hHd44780OnBoard, '1');
    hd44780WriteChar(hHd44780OnBoard, '4');
    hd44780WriteChar(hHd44780OnBoard, '2');
    hd44780WriteChar(hHd44780OnBoard, '2');
    hd44780WriteChar(hHd44780OnBoard, 'G');
    hd44780WriteChar(hHd44780OnBoard, 'S');
    hd44780WriteChar(hHd44780OnBoard, 'M');
    hd44780WriteChar(hHd44780OnBoard, ' ');
    hd44780WriteChar(hHd44780OnBoard, 'M');
    hd44780WriteChar(hHd44780OnBoard, 'A');
    hd44780WriteChar(hHd44780OnBoard, 'S');
    hd44780WriteChar(hHd44780OnBoard, 'T');
    hd44780WriteChar(hHd44780OnBoard, 'E');
    hd44780WriteChar(hHd44780OnBoard, 'R');
    hd44780WriteChar(hHd44780OnBoard, 'S');   

    pString = testString1;
    do
    {
        pString = hd44780WriteRAMString(hHd44780OffBoard, pString);
    } while (pString != (unsigned char *) 0);
    
    hd44780SetCursorAddr(hHd44780OffBoard, 0x40);
    
    pString = testString2;
    do
    {
        pString = hd44780WriteRAMString(hHd44780OffBoard, pString);
    } while (pString != (unsigned char *) 0);

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
