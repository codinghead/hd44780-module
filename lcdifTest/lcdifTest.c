/*******************************************************************************
*
* LCD INTERFACE MODULE TEST PROGRAM
*
*******************************************************************************/

/*******************************************************************************
*
* Tests the LCD Interface module's functionality
*
* Filename : lcdifTest.c
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
*                         LCD INTERFACE MODULE TEST PROGRAM
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
    #else
        #error Chosen PIC18 device is not supported
    #endif     
                                    /******************************************/
                                    /* Microchip C30 compiler                 */
                                    /******************************************/
#elif defined(__PIC24F__)
	// PIC24F processor
	#include <p24Fxxxx.h>
                                    /******************************************/
                                    /* Microchip C30 compiler                 */
                                    /******************************************/
#elif defined(__PIC24H__)
	// PIC24H processor
	#include <p24Hxxxx.h>
                                    /******************************************/
                                    /* Microchip C30 compiler                 */
                                    /******************************************/
#elif defined(__dsPIC33F__)
	// dsPIC33F processor
	#include <p33Fxxxx.h>
                                    /******************************************/
                                    /* Microchip C30 compiler                 */
                                    /******************************************/
#elif defined(__dsPIC30F__)
	// dsPIC30F processor
	#include <p30fxxxx.h>
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

//#include "..\hd44780_module\hd44780.h"
                                        /* #### Note: these defines need to   */
                                        /* later moved into the compiler      */
                                        /* specific defines above             */
                                        /* #### Note: need config bits!       */
                                        /* later moved into the compiler      */
                                        /* specific defines above             */

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
    LCDIFNUM            lcdIfOneNum;
    LCDIFNUM            lcdIfTwoNum;
    HLCDIF              hLcdIfOne;
    HLCDIF              hLcdIfTwo;
    PBIFOBJ             pbIf;
    LCDIFOBJ            lcdIfOneObj;
    LCDIFOBJ            lcdIfTwoObj;
    unsigned char       readData=0;
    unsigned char       readAddress=0;
    
                                        /* Make RW, RS and E lines outputs    */
#if	defined(__18CXX)
                                        /* Clear LATD register (functions as  */
                                        /* data lines on both boards too)     */
    LATD = 0;
    #if defined (PICDEM2PLUS_GREEN)
                                        /* On GREEN PICDEM 2 Plus these are:  */
                                        /*  RD5 = RW                          */
                                        /*  RD4 = RS                          */
                                        /*  RD6 = E                           */
    TRISDbits.TRISD5 = 0;
    TRISDbits.TRISD4 = 0;
    TRISDbits.TRISD6 = 0;
    #elif defined(PICDEM2PLUS_RED)
                                        /* On RED PICDEM 2 Plus these are:    */
                                        /*  RA2 = RW                          */
                                        /*  RA3 = RS                          */
                                        /*  RA1 = E                           */
                                        /* Make PORTA bits digital too and    */
                                        /* clear LATA register                */
    ADCON1 = 0x0F;
    LATA = 0;
    TRISAbits.TRISA2 = 0;
    TRISAbits.TRISA3 = 0;
    TRISAbits.TRISA1 = 0;
    #endif    
#endif                                  
                                        /* Init module                        */
    lcdifInit();
    
                                        /* Fill parallel bus interface struct */
#if defined(PICDEM2PLUS_GREEN)
    pbIf.RW_LAT     = &LATD;
    pbIf.RW_BIT     = (1 << 5);
    pbIf.RS_LAT     = &LATD;
    pbIf.RS_BIT     = (1 << 4);
#elif defined(PICDEM2PLUS_RED)
    pbIf.RW_LAT     = &LATA;
    pbIf.RW_BIT     = (1 << 2);
    pbIf.RS_LAT     = &LATA;
    pbIf.RS_BIT     = (1 << 3);
#endif
    pbIf.DATA_LAT   = &LATD;
    pbIf.DATA_PORT  = &PORTD;
    pbIf.DATA_TRIS  = &TRISD;
    pbIf.DATA_MASK  = 0x0F;


                                        /* Fill lcd interface struct          */
#if	defined(__18CXX)
    #if defined(PICDEM2PLUS_GREEN)
    lcdIfOneObj.E_LAT      = &LATD;
    lcdIfOneObj.E_BIT      = (1 << 6);
    #elif defined(PICDEM2PLUS_RED)
    lcdIfOneObj.E_LAT      = &LATA;
    lcdIfOneObj.E_BIT      = (1 << 1);
    #endif
#endif

    lcdIfOneObj.pbIfObject = &pbIf;
    
    lcdIfOneNum = lcdifCreate(&lcdIfOneObj);
    
    hLcdIfOne = lcdifOpen(lcdIfOneNum);

#if	defined(__18CXX)
                                        /* Turn on the LCD display (uses RD7) */
#if defined(PICDEM2PLUS_GREEN)
    TRISDbits.TRISD7 = 0;
    LATDbits.LATD7 = 1;
#endif

#endif
    
    lcdifGetPb(hLcdIfOne);
    
    wait(3640);                         /* About 40ms                         */
    lcdif4BitFunctionSet(hLcdIfOne, 0x02);
    wait(374);
    lcdif4BitFunctionSet(hLcdIfOne, 0x02);
    wait(10);
    lcdif4BitFunctionSet(hLcdIfOne, 0x02);
    wait(4);
    
                                        /* Set up 4-bit bus, 2 line display   */
                                        /* and 5 x 10 dot font                */
    lcdifWriteInstruction(hLcdIfOne, 0x2C);
    wait(4);                            /* Clear display                      */
    lcdifWriteInstruction(hLcdIfOne, 0x01);
    wait(139);                          /* Set Display ON, Cursor ON, Blink   */
                                        /* ON                                 */
    lcdifWriteInstruction(hLcdIfOne, 0x0F);
    wait(4);                            /* Write some text                    */
    
    lcdifWriteData(hLcdIfOne, 'H');
    wait(4);  
    lcdifWriteData(hLcdIfOne, 'e');
    wait(4);  
    lcdifWriteData(hLcdIfOne, 'l');
    wait(4);  
    lcdifWriteData(hLcdIfOne, 'l');
    wait(4);  
    lcdifWriteData(hLcdIfOne, 'o');
    wait(4);  
    lcdifWriteData(hLcdIfOne, '!');
    wait(4);  
    lcdifWriteData(hLcdIfOne, ' ');
    wait(4);
    lcdifWriteData(hLcdIfOne, 0xC0);
    wait(4);
    lcdifWriteData(hLcdIfOne, 0xC1);
    wait(4);
    lcdifWriteData(hLcdIfOne, 0x00);
    wait(4);
    lcdifWriteInstruction(hLcdIfOne, 0x80);
    wait(4);
    lcdifReadData(hLcdIfOne, &readData);
    wait(4);
    lcdifReadData(hLcdIfOne, &readData);
    wait(4);
    lcdifWriteInstruction(hLcdIfOne, 0x40);
    wait(4);
    lcdifWriteData(hLcdIfOne, 0x01);
    wait(4);
    lcdifWriteData(hLcdIfOne, 0x03);
    wait(4);
    lcdifWriteData(hLcdIfOne, 0x07);
    wait(4);
    lcdifWriteData(hLcdIfOne, 0x0F);
    wait(4);
    lcdifWriteData(hLcdIfOne, 0x1F);
    wait(4);
    lcdifWriteData(hLcdIfOne, 0x3F);
    wait(4);
    lcdifReadAddress(hLcdIfOne, &readAddress);
    wait(4);
    lcdifWriteInstruction(hLcdIfOne, 0x80);
    wait(4);
    lcdifWriteData(hLcdIfOne, 'B');
    wait(4);
    lcdifReadAddress(hLcdIfOne, &readAddress);
    wait(4);
    lcdifWriteInstruction(hLcdIfOne, 0xE4);
    wait(4);
    lcdifReadAddress(hLcdIfOne, &readAddress);
    wait(4);
    lcdifWriteData(hLcdIfOne, 'H');
    wait(4); 
    lcdifWriteInstruction(hLcdIfOne, 0xC6);
    wait(4);
    lcdifReadAddress(hLcdIfOne, &readAddress);
    wait(4);
    lcdifWriteData(hLcdIfOne, 'H');
    wait(4); 
    
    lcdIfOneNum = lcdifClose(hLcdIfOne);
    
    lcdifDestroy(lcdIfOneNum);
    
    lcdifDeinit();
    
    /* Congratulations! If we stop here all tests passed */
    while(1);
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
