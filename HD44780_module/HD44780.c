/*******************************************************************************
*
* HD44780 MODULE
*
*******************************************************************************/

/*******************************************************************************
*
* This module provides an API for controlling HD44780 based alphanumeric LCD
* displays. Due to slight differences between such controllers,a degree of
* customisation has been implemented for the following chipsets:
*
* Hitachi HD44780U
* Sitronix ST7066U
* Samsung S6A0069
* Samsung KS0066U
* Novatek NT7603
*
* Filename : HD44780.c
* Version : V0.01
* Programmer(s) : Stuart Cording aka CODINGHEAD
* 
********************************************************************************
* Note(s) : 
* 4th July 2010 - V0.01 - First cut
*
*******************************************************************************/

/*******************************************************************************
*
*                                 HD44780 MODULE
*
*******************************************************************************/

/*******************************************************************************
*                                 INCLUDE FILES
*******************************************************************************/
#include "HD44780.h"
#if defined (__18CXX)
    #include "lcdif_c18.h"
    #include "pbif_c18.h"
#elif defined (__PIC32MX__)
    //#include "lcdif_c32.h"
    //#include "pbif_c32.h"
#endif
/*******************************************************************************
*                                 LOCAL DEFINES
*******************************************************************************/

/*******************************************************************************
* Summary:
*   Used to indicate that the chosen HD44780 object is open and in use
*******************************************************************************/
#define HD44780_OPEN                (0x01 << 7)

/*******************************************************************************
* Summary:
*   Used to mask the lower bits of the hd44780Flags element for use in the
* instruction initialisation routine's state-machine
*******************************************************************************/
#define HD44780_INSTRINITSTATE      (0x07)

/*******************************************************************************
* Summary:
*   The following define the instructions for the HD44780. All bits which can
* be used by the instruction can be set. The user clear the bits they wish to 
* clear when calling the appropriate functions, except for instructions which 
* require an address
*******************************************************************************/
#define HD44780_CLEARDISPLAY            0x01
#define HD44780_RETURNHOME              0x02
#define HD44780_ENTRYMODESET            0x07
#define HD44780_DISPLAYONOFFCONTROL     0x0F
#define HD44780_CURSORORDISPLAYSHIFT    0x1C
#define HD44780_FUNCTIONSET             0x3C
#define HD44780_SETCGRAMADDRESS         0x7F
#define HD44780_SETDDRAMADDRESS         0xFF


/*******************************************************************************
*                                LOCAL CONSTANTS
*******************************************************************************/

//<Add all local constants here>
//const <Data Type> <Variable>          /* Constant description               */

/*******************************************************************************
*                                LOCAL DATA TYPES
*******************************************************************************/

/*******************************************************************************
* New data type HD44780INSTRINITSTATE
* Description:
*   Holds a list of states to be stepped through when executing the instruction
* initialisation routine
*******************************************************************************/
typedef enum HD44780INSTRINITSTATETYPE
{
                                        /* Step 0: Initialisation started     */
    STARTINIT,
                                        /* Step 1: Function set               */
    FUNCTIONSET1,
                                        /* Step 2: Function set (optional)    */
    FUNCTIONSET2,
                                        /* Step 3: Function set (optional)    */
    FUNCTIONSET3,
                                        /* Step 4: Function set (optional)    */
    FUNCTIONSET4,                       
                                        /* Step 5: Display on/off control     */
    DISPLAYONOFFCONTROL,
                                        /* Step 6: Display clear              */
    DISPLAYCLEAR,
                                        /* Step 7: Entry mode set             */
    ENTRYMODESET
} HD44780INSTRINITSTATE;


/*******************************************************************************
*                                  LOCAL TABLES
*******************************************************************************/

//<Add all locally created tables of data here>


/*******************************************************************************
*                             LOCAL GLOBAL VARIABLES
*******************************************************************************/

/*******************************************************************************
* Summary:
*   Local pointer to a linked list of HD44780 LCD objects
*******************************************************************************/
static HD44780OBJ * startOfHD44780Objs;

/*******************************************************************************
* Summary:
*   Used to note how many HD44780 objects are active. Each bit in this
* variable relates to one active display object.
*******************************************************************************/
static unsigned int activeHD44780Objects;


/*******************************************************************************
*#X#                          LOCAL FUNCTION PROTOTYPES
*******************************************************************************/

static unsigned char isHD44780Busy(HHD44780 const hHd44780);


/*******************************************************************************
*                            LOCAL CONFIGURATION ERRORS
*******************************************************************************/

//<Add all pre-processor error messages here>
//#ifndef <Important Define>
//#error Important header file is missing!
//#endif


/*******************************************************************************
* hd44780Init()
*
* Summary: 
*   Initialises the HD44780 module for first use
*
* See also:
*   hd44780Deinit()
*
* Arguments: 
*   None
*
* Returns: 
*   void
*
* Callers: 
*   Main application code
*
* Notes : 
*   None
*******************************************************************************/
void hd44780Init(void)
{
                                        /* Initialise the local pointer to    */
                                        /* the list of HD44780 LCD objects    */
    startOfHD44780Objs = (HD44780OBJ *) 0;
                                        /* Currently no active HD44780 LCD    */
                                        /* objects                            */
    activeHD44780Objects = 0;
}

/*******************************************************************************
* hd44780Deinit()
*
* Summary: 
*   Deinitialises the HD44780 module
*
* See also:
*   hd44780Init()
*
* Arguments: 
*   None
*
* Returns: 
*   void
*
* Callers: 
*   Main application code
*
* Notes : 
*   None
*******************************************************************************/
void hd44780Deinit(void)
{
                                        /* Initialise the local pointer to    */
                                        /* the list of HD44780 LCD objects    */
    startOfHD44780Objs = (HD44780OBJ *) 0;
                                        /* Currently no active HD44780 LCD    */
                                        /* objects                            */
    activeHD44780Objects = 0;
}

/*******************************************************************************
* hd44780Create()
*
* Summary: 
*   Creates an HD44780 LCD for use by this module
*
* See also:
*   hd44780Destroy()
*
* Arguments: 
*   hd44780Obj              - HD44780 object to insert in a list of HD44780 
*                             objects 
*
* Returns: 
*   - 1 to MAX_HD44780IFS  	- number the LCD interface has been assigned if it 
*                             was possible to allocate it
*   - 0		                - if the HD44780 LCD allocation failed
*
* Callers: 
*   Main application code
*
* Notes : 
*   1. hd44780Create() must have been called prior to calling this function
*******************************************************************************/
HD44780NUM hd44780Create(HLCDIF const           hLcdIf,
                         LCDIFFP * const        lcdIfFunctionPointers,
                         HD44780OBJ * const     hd44780Obj)
{
    HD44780OBJ * localHd44780Obj;       /* Stores local copy of pointer to    */
                                        /* linked list so we can insert next  */
                                        /* object in at the top               */
    unsigned int  hd44780Number = 0x0001;      
                                        /* Used to calculate the interface    */
                                        /* number to return to caller         */

                                        /* Check we got an object to point to */
    if(hd44780Obj != (HD44780OBJ *) 0)
    {
                                        /* Check we got an LCD interface      */
        if (hLcdIf == (HLCDIF) 0)
        {
            goto cannot_create_HD44780;
        }
                                        /* #### Probably better to check that */
                                        /* each function is present - don't   */
                                        /* need to demand 4bitFunctionSet     */
                                        /* since it is only needed in 4bit    */
                                        /* bus mode                           */
                                        /* Check we got some functions to     */
                                        /* point to                           */
        if (lcdIfFunctionPointers == 0)
        {
            goto cannot_create_HD44780;
        }
                                        /* If we got here we have             */
                                        /* valid data we can work with        */

                                        /* If there are no active objects in  */
                                        /* the list put this object at the    */
                                        /* top                                */
        if (activeHD44780Objects == 0 && startOfHD44780Objs == (HD44780OBJ *) 0)
        {
            startOfHD44780Objs = hd44780Obj;
            startOfHD44780Objs->nextHD44780Obj = (HD44780OBJ *) 0;
                                        /* Clear the object's flags           */
            startOfHD44780Objs->hd44780Flags = 0;
                                        /* Store the function pointers        */
            startOfHD44780Objs->lcdIfFunctionPointers = lcdIfFunctionPointers;
                                        /* Store the handle to the LCD        */
                                        /* interface                          */
            startOfHD44780Objs->hLcdIf = hLcdIf;
                                        /* Assign the interface number        */
            activeHD44780Objects |= hd44780Number;
            startOfHD44780Objs->hd44780Num = hd44780Number;
                                        /* Return the interface number        */
            return startOfHD44780Objs->hd44780Num;
        }
                                        /* Otherwise, if we haven't allocated */
                                        /* all the HD44780 objects we         */
                                        /* can support, insert another        */
        else if (activeHD44780Objects != 0xFFFF)
        {
                                        /* Insert this object at start of the */
                                        /* list                               */
            localHd44780Obj = startOfHD44780Objs;
            startOfHD44780Objs = hd44780Obj;
            startOfHD44780Objs->nextHD44780Obj = localHd44780Obj;
                                        /* Clear the object's flags           */
            startOfHD44780Objs->hd44780Flags = 0;
                                        /* Store the function pointers        */
            startOfHD44780Objs->lcdIfFunctionPointers = lcdIfFunctionPointers;
                                        /* Store the handle to the LCD        */
                                        /* interface                          */
            startOfHD44780Objs->hLcdIf = hLcdIf;
                                        /* Find a free LCD interface number   */
                                        /* for this interface                 */
                                        /* HD44780 Number '1' is assigned, so */
                                        /* start looking from '2' for a free  */
                                        /* number                             */
            hd44780Number = 0x0002;
            
            do
            {
                if (!(activeHD44780Objects & hd44780Number))
                {
                                        /* Assign the interface number        */
                    activeHD44780Objects |= hd44780Number;
                    startOfHD44780Objs->hd44780Num = hd44780Number;
                    return startOfHD44780Objs->hd44780Num;
                }
                                        /* That wasn't free; try next bit     */
                hd44780Number <<= 1;
                                        /* Mask just in case unsigned int is  */
                                        /* bigger than 16-bits                */
                hd44780Number &= 0xFFFF;
            } while(hd44780Number != 0x0000);
        }
    }
cannot_create_HD44780:
                                        /* Couldn't create interface          */
    return 0;
}

/*******************************************************************************
* hd44780Destroy()
*
* Summary: 
*   Destroys a previously created HD44780 object by removing it from the
*   linked list. The HD44780 object must have already been closed, otherwise
*   this call will fail.
*
* See also:
*   hd44780Create(), hd44780Close()
*
* Arguments: 
*   hd44780Number - number of the HD44780 interface object to destroy
*
* Returns: 
*   - HD44780_DESTROY_OK    - HD44780 interface was successfully destroyed
*   - HD44780_DESTROY_FAIL  - couldn't detroy requested HD44780 interface -
*                             reason is that it is probably still open
*
* Callers: 
*   Main application code
*
* Notes : 
*   1. hd44780Create() must have been called prior to calling this function
*   2. Destroying this object does not free the memory associated with the
*      associated HD44780OBJ object
*   3. An open HD44780 object cannot be destroyed. Such a case returns a FAIL
*******************************************************************************/
unsigned char hd44780Destroy(HD44780NUM hd44780Number)
{
    HD44780OBJ * localHd44780Obj;       /* Stores local copy of pointer to    */
                                        /* linked list so we can insert next  */
                                        /* object in at the top               */
    HD44780OBJ * previousHd44780Obj;    /* Temporary object pointer so we can */
                                        /* remove an object from the list     */

                                        /* Check that there are created       */
                                        /* objects                            */
    if(startOfHD44780Objs != (HD44780OBJ *) 0)
    {
                                        /* Check if the first object in the   */
                                        /* list is the one we are looking for */
        if (startOfHD44780Objs->hd44780Num == hd44780Number)
        {
                                        /* Simply copy where this object is   */
                                        /* pointing to into the               */
                                        /* startOfHD44780Objs                 */
            startOfHD44780Objs = startOfHD44780Objs->nextHD44780Obj;
                                        /* Clear this object's bit in the     */
                                        /* active HD44780 objects variable    */
            activeHD44780Objects &= ~hd44780Number;
                                        /* Destroyed the desired object       */
            return HD44780_DESTROY_OK;
        }
                                        /* Otherwise loop through all the     */
                                        /* objects until we find the right    */
                                        /* one, if it is to be found at all   */
        else if (startOfHD44780Objs->nextHD44780Obj != (HD44780OBJ *) 0)
        {
                                        /* Keep a copy of the pointer to this */
                                        /* object                             */
            previousHd44780Obj = startOfHD44780Objs;
                                        /* Get next object in the list        */
            localHd44780Obj = startOfHD44780Objs->nextHD44780Obj;
                                        /* Loop through each item and check   */
                                        /* until we find the one we are       */
                                        /* searching for                      */
            do
            {
                if (localHd44780Obj->hd44780Num == hd44780Number)
                {
                                        /* Remove this buffer from the list   */
                    previousHd44780Obj->nextHD44780Obj =
                                                localHd44780Obj->nextHD44780Obj;
                                        /* Clear this object's bit in the     */
                                        /* active HD44780 objects variable    */
                    activeHD44780Objects &= ~hd44780Number;
                                        /* Destroyed the desired object       */
                    return HD44780_DESTROY_OK;
                }
                else
                {
                                        /* That wasn't the one we were        */
                                        /* looking for. Move to the next one  */
                    previousHd44780Obj = localHd44780Obj;
                    localHd44780Obj = localHd44780Obj->nextHD44780Obj;
                }
            } while (localHd44780Obj != (HD44780OBJ *) 0);
        }
    }
                                        /* Couldn't destroy object            */
        return HD44780_DESTROY_FAIL;
}

/*******************************************************************************
* lcdifOpen()
*
* Summary: 
*   Opens an LCD interface for use by caller and initialises an HLCDIF
*   handle to it
*
* See also:
*   lcdifClose()
*
* Arguments: 
*   lcdIfNumber     - number of an existing LCD interface object to use
*
* Returns: 
*   - NULL	        - if LCD interface couldn't be opened
*   - handle        - if LCD interface was opened properly
*
* Callers: 
*   User application
*
* Notes : 
* 1. Caller must have created (lcdifCreate) at least one LCD interface object 
*    before calling this function
*******************************************************************************/
HHD44780 hd44780Open(HD44780NUM hd44780Num)
{
	HD44780OBJ * localHD44780Obj;       /* Stores local copy of pointer to    */
                                        /* linked list so we can insert next  */
                                        /* object in at the top               */
	
                                        /* Check that there are created       */
                                        /* objects                            */
    if(startOfHD44780Objs != (HD44780OBJ *) 0)
    {
                                        /* Check that a LCDIFNUM object       */
                                        /* exists with lcdIfNumber            */

        localHD44780Obj = startOfHD44780Objs;
        
        do
        {
    	    if (localHD44780Obj->hd44780Num == hd44780Num)
    	    {
        	                            /* Check LCD is not already open      */
                if (!(localHD44780Obj->hd44780Flags & HD44780_OPEN))
                {
                                        /* Note that it is now in use         */
                    localHD44780Obj->hd44780Flags |= HD44780_OPEN;
        	                            /* Return handle to it                */
        	        return localHD44780Obj;
        	    }
        	                            /* If buffer is already open, return  */
        	                            /* NULL handle pointer                */
        	    else
        	    {
            	    return (HD44780OBJ *) 0;
            	}    
        	}    
        	else
        	{
            	localHD44780Obj = localHD44780Obj->nextHD44780Obj;
            }   	
    	} while (localHD44780Obj != (HD44780OBJ *) 0);
    }
    	                                /* Return handle to NULL otherwise    */
    return (HD44780OBJ *) 0;
}

/*******************************************************************************
* hd44780Close()
*
* Summary: 
*   Closes an HD44780 LCD and releases the handle to it
*
* See also:
*   hd44780Open()
*
* Arguments: 
*   hHd44780        - handle to the open HD44780
*
* Returns: 
*   - >0  	        - number of HD44780 object if it was was open
*   - 0             - if the HD44780 interface was not open
*
* Callers: 
*   User application
*
* Notes : 
* 1. Caller must have 'created' at least one HD44780 object before
*    calling this function
*
*******************************************************************************/
HD44780NUM hd44780Close(HHD44780 const hHd44780)
{
                                        /* Check that there are created       */
                                        /* objects                            */
    if(startOfHD44780Objs != (HD44780OBJ *) 0)
    {
    	                                /* Check LCD interface is actually    */
    	                                /* open                               */
        if (hHd44780->hd44780Flags & HD44780_OPEN)
        {
        	                            /* Note that this LCD interface       */
        	                            /* object is closed                   */
            hHd44780->hd44780Flags &= ~HD44780_OPEN;
                                        /* Return LCD interface object's      */
                                        /* interface number                   */
            return hHd44780->hd44780Num;
        }
                                        /* Otherwise return 0 to say that     */
                                        /* buffer object wasn't open          */
        else
        {
            return (HD44780NUM) 0;
        }
    }
    return 0;
}

/*******************************************************************************
* hd44780ClearDisplay()
*
* Summary: 
*   Clear the display's content
*
* See also:
*   
*
* Arguments: 
*   hHd44780            - handle to the open HD44780
*
* Returns: 
*   - 1  	        - command was completed successfully
*   - 0             - command couldn't complete (most likely bus busy)
*
* Callers: 
*   User application
*
* Notes : 
* 1. Caller must have 'created' at least one HD44780 object before
*    calling this function
*
*******************************************************************************/
unsigned char hd44780ClearDisplay(HHD44780 const hHd44780)
{
    unsigned char returnValue = 0;
                                        /* Check LCD interface is actually    */
    	                                /* open                               */
    if (hHd44780->hd44780Flags & HD44780_OPEN)
    {
                                        /* First get the bus                  */
        if (hHd44780->lcdIfFunctionPointers->pGetBus(hHd44780->hLcdIf))
        {
                                        /* Check busy bit                     */
            if(!isHD44780Busy(hHd44780))
            {
                hHd44780->lcdIfFunctionPointers->pWriteInstr(hHd44780->hLcdIf,
                                                          HD44780_CLEARDISPLAY);
                returnValue = 1;
            }
                                        /* Return the bus                     */
            hHd44780->lcdIfFunctionPointers->pReturnBus(hHd44780->hLcdIf);
        }    
    }
    
    return returnValue;
}

/*******************************************************************************
* hd44780ReturnHome()
*
* Summary: 
*   Return cursor to home position
*
* See also:
*   
*
* Arguments: 
*   hHd44780            - handle to the open HD44780
*
* Returns: 
*   - 1  	        - command was completed successfully
*   - 0             - command couldn't complete (most likely bus busy)
*
* Callers: 
*   User application
*
* Notes : 
* 1. Caller must have 'created' at least one HD44780 object before
*    calling this function
*
*******************************************************************************/
unsigned char hd44780ReturnHome(HHD44780 const hHd44780)
{
    unsigned char returnValue = 0;
                                        /* Check LCD interface is actually    */
    	                                /* open                               */
    if (hHd44780->hd44780Flags & HD44780_OPEN)
    {
                                        /* First get the bus                  */
        if (hHd44780->lcdIfFunctionPointers->pGetBus(hHd44780->hLcdIf))
        {
                                        /* Check busy bit                     */
            if(!isHD44780Busy(hHd44780))
            {
                hHd44780->lcdIfFunctionPointers->pWriteInstr(hHd44780->hLcdIf,
                                                            HD44780_RETURNHOME);
                                        /* Return the bus                     */
                hHd44780->lcdIfFunctionPointers->pReturnBus(hHd44780->hLcdIf);
                returnValue = 1;
            }
                                        /* Return the bus                     */
            hHd44780->lcdIfFunctionPointers->pReturnBus(hHd44780->hLcdIf);
        }    
    }
    
    return returnValue;
}    

/*******************************************************************************
* hd44780EntryModeSet()
*
* Summary: 
*   Send the Entry Mode command to LCD
*
* See also:
*   
*
* Arguments: 
*   hHd44780            - handle to the open HD44780
*   entryMode           - bit mask of options desired for this command
*
* Returns: 
*   - 1  	        - command was completed successfully
*   - 0             - command couldn't complete (most likely bus busy)
*
* Callers: 
*   User application
*
* Notes : 
* 1. Caller must have 'created' at least one HD44780 object before
*    calling this function
*
*******************************************************************************/
unsigned char hd44780EntryModeSet(HHD44780 const     hHd44780,
                                  unsigned char      entryMode)
{
    unsigned char returnValue = 0;
                                        /* Check LCD interface is actually    */
    	                                /* open                               */
    if (hHd44780->hd44780Flags & HD44780_OPEN)
    {
                                        /* First get the bus                  */
        if (hHd44780->lcdIfFunctionPointers->pGetBus(hHd44780->hLcdIf))
        {
                                        /* Check busy bit                     */
            if(!isHD44780Busy(hHd44780))
            {
                hHd44780->lcdIfFunctionPointers->pWriteInstr(hHd44780->hLcdIf,
                                              HD44780_ENTRYMODESET & entryMode);
                                        
                returnValue = 1;
            }
                                        /* Return the bus                     */
            hHd44780->lcdIfFunctionPointers->pReturnBus(hHd44780->hLcdIf);
        }    
    }
    
    return returnValue;
}

/*******************************************************************************
* hd44780DisplayControl()
*
* Summary: 
*   Send the Display On/Off Control command to LCD
*
* See also:
*   
*
* Arguments: 
*   hHd44780            - handle to the open HD44780
*   displayOnOffControl - bit mask of options desired for this command
*
* Returns: 
*   - 1  	        - command was completed successfully
*   - 0             - command couldn't complete (most likely bus busy)
*
* Callers: 
*   User application
*
* Notes : 
* 1. Caller must have 'created' at least one HD44780 object before
*    calling this function
*
*******************************************************************************/
unsigned char hd44780DisplayControl(HHD44780 const   hHd44780,
                                    unsigned char    displayOnOffControl)
{
    unsigned char returnValue = 0;
                                        /* Check LCD interface is actually    */
    	                                /* open                               */
    if (hHd44780->hd44780Flags & HD44780_OPEN)
    {
                                        /* First get the bus                  */
        if (hHd44780->lcdIfFunctionPointers->pGetBus(hHd44780->hLcdIf))
        {
                                        /* Check busy bit                     */
            if(!isHD44780Busy(hHd44780))
            {
                hHd44780->lcdIfFunctionPointers->pWriteInstr(hHd44780->hLcdIf,
                             HD44780_DISPLAYONOFFCONTROL & displayOnOffControl);
                                        
                returnValue = 1;
            }
                                        /* Return the bus                     */
            hHd44780->lcdIfFunctionPointers->pReturnBus(hHd44780->hLcdIf);
        }    
    }
    
    return returnValue;
}

/*******************************************************************************
* hd44780ShiftControl()
*
* Summary: 
*   Send the Shift Control command to LCD
*
* See also:
*   
*
* Arguments: 
*   hHd44780            - handle to the open HD44780
*   displayOnOffControl - bit mask of options desired for this command
*
* Returns: 
*   - 1  	        - command was completed successfully
*   - 0             - command couldn't complete (most likely bus busy)
*
* Callers: 
*   User application
*
* Notes : 
* 1. Caller must have 'created' at least one HD44780 object before
*    calling this function
*
*******************************************************************************/
unsigned char hd44780ShiftControl(HHD44780 const hHd44780, 
                                   unsigned char shiftControl)
{
    unsigned char returnValue = 0;
                                        /* Check LCD interface is actually    */
    	                                /* open                               */
    if (hHd44780->hd44780Flags & HD44780_OPEN)
    {
                                        /* First get the bus                  */
        if (hHd44780->lcdIfFunctionPointers->pGetBus(hHd44780->hLcdIf))
        {
                                        /* Check busy bit                     */
            if(!isHD44780Busy(hHd44780))
            {
                hHd44780->lcdIfFunctionPointers->pWriteInstr(hHd44780->hLcdIf,
                             HD44780_CURSORORDISPLAYSHIFT & shiftControl);
                                        
                returnValue = 1;
            }
                                        /* Return the bus                     */
            hHd44780->lcdIfFunctionPointers->pReturnBus(hHd44780->hLcdIf);
        }    
    }
    
    return returnValue;
}    

/*******************************************************************************
* hd44780FunctionSet()
*
* Summary: 
*   Send the Function Set command to LCD
*
* See also:
*   
*
* Arguments: 
*   hHd44780            - handle to the open HD44780
*   displayOnOffControl - bit mask of options desired for this command
*
* Returns: 
*   - 1  	        - command was completed successfully
*   - 0             - command couldn't complete (most likely bus busy)
*
* Callers: 
*   User application
*
* Notes : 
* 1. Caller must have 'created' at least one HD44780 object before
*    calling this function
*
*******************************************************************************/
unsigned char hd44780FunctionSet(HHD44780 const hHd44780, 
                                  unsigned char functionSet)
{
    unsigned char returnValue = 0;
                                        /* Check LCD interface is actually    */
    	                                /* open                               */
    if (hHd44780->hd44780Flags & HD44780_OPEN)
    {
                                        /* First get the bus                  */
        if (hHd44780->lcdIfFunctionPointers->pGetBus(hHd44780->hLcdIf))
        {
                                        /* Check busy bit                     */
            if(!isHD44780Busy(hHd44780))
            {
                hHd44780->lcdIfFunctionPointers->pWriteInstr(hHd44780->hLcdIf,
                             HD44780_FUNCTIONSET & functionSet);
                                        
                returnValue = 1;
            }
                                        /* Return the bus                     */
            hHd44780->lcdIfFunctionPointers->pReturnBus(hHd44780->hLcdIf);
        }    
    }
    
    return returnValue;

}    

/*******************************************************************************
* hd44780SetCGRAMAddr()
*
* Summary: 
*   Set the address to write the next Character Graphics RAM (CGRAM) data
*
* See also:
*   
*
* Arguments: 
*   hHd44780            - handle to the open HD44780
*   address             - 6-bit address to be used; each character starts eight
*                         addresses away from previous in 5x8 dot mode, giving a
*                         total of 8 characters; in 5x10 dot mode, each
*                         character start 16 addresses from previous, giving a 
*                         total of 4 characters
*
* Returns: 
*   - 1  	        - command was completed successfully
*   - 0             - command couldn't complete (most likely bus busy)
*
* Callers: 
*   User application
*
* Notes : 
* 1. Caller must have 'created' at least one HD44780 object before
*    calling this function
*
* #### Possible improvement - firstly, replace address with character position
*                           - secondly, get rid of this function all together
*                             and simply set the address when writing new 
*                             characters to the CGRAM in hd44780CGRAMWrite()
*******************************************************************************/
unsigned char hd44780SetCGRAMAddr(HHD44780 const hHd44780, 
                                   unsigned char address)
{
    unsigned char returnValue = 0;
                                        /* Check LCD interface is actually    */
    	                                /* open                               */
    if (hHd44780->hd44780Flags & HD44780_OPEN)
    {
                                        /* First get the bus                  */
        if (hHd44780->lcdIfFunctionPointers->pGetBus(hHd44780->hLcdIf))
        {
                                        /* Check busy bit                     */
            if(!isHD44780Busy(hHd44780))
            {
                                        /* Set 6th bit of address, otherwise  */
                                        /* the commands 6th bit gets cleared  */
                address = address | 0x40;
                hHd44780->lcdIfFunctionPointers->pWriteInstr(hHd44780->hLcdIf,
                             HD44780_SETCGRAMADDRESS & address);
                                        
                returnValue = 1;
            }
                                        /* Return the bus                     */
            hHd44780->lcdIfFunctionPointers->pReturnBus(hHd44780->hLcdIf);
        }    
    }
    
    return returnValue;
}    

/*******************************************************************************
* hd44780SetCursorAddr()
*
* Summary: 
*   Set the address of the cursor to where the next character will be written to
*   the LCD; also known as setting the Display Data RAM (DDRAM) address
*
* See also:
*   
*
* Arguments: 
*   hHd44780            - handle to the open HD44780
*   address             - 7-bit address to be used; the first line starts at 
*                         address 0x00 through 0x27 giving 40 characters on the
*                         line; the second line starts at address 0x40 through
*                         0x67 giving a further 40 characters 
*
* Returns: 
*   - 1  	        - command was completed successfully
*   - 0             - command couldn't complete (most likely bus busy)
*
* Callers: 
*   User application
*
* Notes : 
* 1. Caller must have 'created' at least one HD44780 object before
*    calling this function
*
*******************************************************************************/
unsigned char hd44780SetCursorAddr(HHD44780 const hHd44780,
                                        unsigned char address)
{
    unsigned char returnValue = 0;
                                        /* Check LCD interface is actually    */
    	                                /* open                               */
    if (hHd44780->hd44780Flags & HD44780_OPEN)
    {
                                        /* First get the bus                  */
        if (hHd44780->lcdIfFunctionPointers->pGetBus(hHd44780->hLcdIf))
        {
                                        /* Check busy bit                     */
            if(!isHD44780Busy(hHd44780))
            {
                                        /* Set MSb of address, otherwise the  */
                                        /* commands MSb gets cleared          */
                address = address | 0x80;
                hHd44780->lcdIfFunctionPointers->pWriteInstr(hHd44780->hLcdIf,
                             HD44780_SETDDRAMADDRESS & address);
                                        
                returnValue = 1;
            }
                                        /* Return the bus                     */
            hHd44780->lcdIfFunctionPointers->pReturnBus(hHd44780->hLcdIf);
        }    
    }
    
    return returnValue;    
}

/*******************************************************************************
* hd44780ReadAddr()
*
* Summary: 
*   Reads the current address being pointed to. If the most recent "Set Address"
*   command set a CGRAM address, the CGRAM address will be returned. If the most 
*   recent "Set Address" command set a DDRAM (cursor) address, the DDRAM
*   (cursor) address will be returned. It is not possible to determine which
*   address is being returned.
*
* See also:
*   
*
* Arguments: 
*   hHd44780            - handle to the open HD44780
*   address             - pointer to return the address read 
*
* Returns: 
*   - 1  	        - command was completed successfully
*   - 0             - command couldn't complete (most likely bus busy)
*
* Callers: 
*   User application
*
* Notes : 
* 1. Caller must have 'created' at least one HD44780 object before
*    calling this function
*
*******************************************************************************/
unsigned char hd44780ReadAddr(HHD44780 const hHd44780, unsigned char * address)
{
    unsigned char returnValue = 0;
                                        /* Check LCD interface is actually    */
    	                                /* open                               */
    if (hHd44780->hd44780Flags & HD44780_OPEN)
    {
                                        /* First get the bus                  */
        if (hHd44780->lcdIfFunctionPointers->pGetBus(hHd44780->hLcdIf))
        {
                                        /* Check busy bit                     */
            if(!isHD44780Busy(hHd44780))
            {
                hHd44780->lcdIfFunctionPointers->pReadAddr(hHd44780->hLcdIf,
                                                                       address);
                                        
                returnValue = 1;
            }
                                        /* Return the bus                     */
            hHd44780->lcdIfFunctionPointers->pReturnBus(hHd44780->hLcdIf);
        }    
    }
    
    return returnValue;   
}
    
/*******************************************************************************
* hd44780WriteChar()
*
* Summary: 
*   Send a single character to the HD44780
*
* See also:
*   hd44780WriteString()
*
* Arguments: 
*   hHd44780            - handle to the open HD44780
*   data                - data to be written
*
* Returns: 
*   - 1  	        - command was completed successfully
*   - 0             - command couldn't complete (most likely bus busy)
*
* Callers: 
*   User application
*
* Notes : 
* 1. Caller must have 'created' at least one HD44780 object before
*    calling this function
*
*******************************************************************************/
unsigned char hd44780WriteChar(HHD44780 const hHd44780, unsigned char data)
{
    unsigned char returnValue = 0;
                                        /* Check LCD interface is actually    */
    	                                /* open                               */
    if (hHd44780->hd44780Flags & HD44780_OPEN)
    {
                                        /* First get the bus                  */
        if (hHd44780->lcdIfFunctionPointers->pGetBus(hHd44780->hLcdIf))
        {
                                        /* Check busy bit                     */
            if(!isHD44780Busy(hHd44780))
            {
                hHd44780->lcdIfFunctionPointers->pWriteData(hHd44780->hLcdIf,
                                                            data);
                                        
                returnValue = 1;
            }
                                        /* Return the bus                     */
            hHd44780->lcdIfFunctionPointers->pReturnBus(hHd44780->hLcdIf);
        }    
    }
    
    return returnValue;
}    

/*******************************************************************************
* hd44780ReadChar()
*
* Summary: 
*   Read a single character from the HD44780. Assumes that the most recent 
*   "Set Address" command was to a DDRAM (cursor) address
*
* See also:
*   hd44780WriteString()
*
* Arguments: 
*   hHd44780            - handle to the open HD44780
*   data                - place to store data read
*
* Returns: 
*   - 1  	        - command was completed successfully
*   - 0             - command couldn't complete (most likely bus busy)
*
* Callers: 
*   User application
*
* Notes : 
* 1. Caller must have 'created' at least one HD44780 object before
*    calling this function
*
*******************************************************************************/
unsigned char hd44780ReadChar(HHD44780 const hHd44780, unsigned char * data)
{
    unsigned char returnValue = 0;
                                        /* Check LCD interface is actually    */
    	                                /* open                               */
    if (hHd44780->hd44780Flags & HD44780_OPEN)
    {
                                        /* First get the bus                  */
        if (hHd44780->lcdIfFunctionPointers->pGetBus(hHd44780->hLcdIf))
        {
                                        /* Check busy bit                     */
            if(!isHD44780Busy(hHd44780))
            {
                hHd44780->lcdIfFunctionPointers->pReadData(hHd44780->hLcdIf,
                                                                          data);
                returnValue = 1;
            }
                                        /* Return the bus                     */
            hHd44780->lcdIfFunctionPointers->pReturnBus(hHd44780->hLcdIf);
        }    
    }
    
    return returnValue;    
}

/*******************************************************************************
* hd44780WriteRAMString()
*
* Summary: 
*   Write a string of data to the display. The string itself must lie in SRAM
*   on a Harvard architecture PIC controller (everything but PIC32)
*
* See also:
*   None
*
* Arguments: 
*   hHd44780            - handle to the open HD44780
*   string              - string where the data is stored
*
* Returns: 
*   - !NULL         - command not complete (most likely bus busy); returned 
*                     value indicates where in string we stopped
*   - NULL          - command complete
*
* Callers: 
*   User application
*
* Notes : 
* 1. Caller must have 'created' at least one HD44780 object before
*    calling this function
*
*******************************************************************************/
const unsigned char * hd44780WriteRAMString(HHD44780 const   hHd44780,
                                 const unsigned char * string)
{
                                        /* Check LCD interface is actually    */
    	                                /* open                               */
    if (hHd44780->hd44780Flags & HD44780_OPEN)
    {
                                        /* First get the bus                  */
        if (hHd44780->lcdIfFunctionPointers->pGetBus(hHd44780->hLcdIf))
        {
                                        /* Check busy bit                     */
            while(!isHD44780Busy(hHd44780))
            {
                hHd44780->lcdIfFunctionPointers->pWriteData(hHd44780->hLcdIf,
                                                                       *string);
                                        /* Increment string pointer           */
                string++;
                                        /* If no more data, return NULL       */
                if (*string == 0)
                {
                                        /* Return the bus                     */
                    hHd44780->lcdIfFunctionPointers->pReturnBus(hHd44780->hLcdIf);

                    return (unsigned char *) 0;
                }    
                                        
            }
                                        /* Return the bus                     */
            hHd44780->lcdIfFunctionPointers->pReturnBus(hHd44780->hLcdIf);
            return string;
        }    
    }
                                        /* Return string if we couldn't do    */
                                        /* anything                           */
    return string;
}    

/*******************************************************************************
* hd44780WriteCGRAM()
*
* Summary: 
*   Write a string of data to the display. The string itself must lie in SRAM
*   on a Harvard architecture PIC controller (everything but PIC32)
*
* See also:
*   None
*
* Arguments: 
*   hHd44780            - handle to the open HD44780
*   character           - pointer to where the character data is stored
*   font                - the font size (5x8 or 5x10 dots)
*
* Returns: 
*   - !NULL         - command not complete (most likely bus busy); returned 
*                     value indicates where in charcter we stopped
*   - NULL          - command complete
*
* Callers: 
*   User application
*
* Notes : 
* 1. Caller must have 'created' at least one HD44780 object before
*    calling this function
* 2. Caller must have set a CGRAM address before using this function
*
*******************************************************************************/
const unsigned char * hd44780WriteCGRAM(HHD44780 const hHd44780,
                         const unsigned char * character,
                                 unsigned char font)
{
    unsigned char counter;
                                        /* Check LCD interface is actually    */
    	                                /* open                               */
    if (hHd44780->hd44780Flags & HD44780_OPEN)
    {
                                        /* First get the bus                  */
        if (hHd44780->lcdIfFunctionPointers->pGetBus(hHd44780->hLcdIf))
        {
                                        /* Check busy bit                     */
            while(!isHD44780Busy(hHd44780))
            {
                hHd44780->lcdIfFunctionPointers->pWriteData(hHd44780->hLcdIf,
                                                                    *character);
                                        /* Increment character pointer        */
                character++;
                                        /* If no more data, return NULL       */
                if (*character == 0)
                {
                                        /* Return the bus                     */
                    hHd44780->lcdIfFunctionPointers->pReturnBus(hHd44780->hLcdIf);

                    return (unsigned char *) 0;
                }    
                                        
            }
                                        /* Return the bus                     */
            hHd44780->lcdIfFunctionPointers->pReturnBus(hHd44780->hLcdIf);
            return character;
        }    
    }
                                        /* Return string if we couldn't do    */
                                        /* anything                           */
    return character;
}    

/*******************************************************************************
* hd44780InstructionInit(()
*
* Summary: 
*   Closes an LCD interface and releases the handle to it
*
* See also:
*
* Arguments: 
*   hHd44780            - handle to the open HD44780
*   hd44780Clone        - chipset if not exactly an Hitachi HD44780U
*   displayOnOffControl - desired settings for the Display On/Off Control 
*                         command which will be called as part of this procedure
*   entryModeSet        - desired settings for the Entry Mode Set command which
*                         will be called as part of this procedure
*
* Returns: 
*   - >1  	        - number of microseconds (us) to wait until calling this
*                     function again
*   - 1             - call this function repeatedly until 0 is returned
*   - 0             - this function has completed the initialisation procedure
*                     and the LCD is ready to be used
*
* Callers: 
*   User application
*
* Notes : 
* 1. Caller must have 'created' at least one LCD interface object before
*    calling this function
* 2. For a true HD44780U chip set, this function will leave the display in the 
*    "display off" state as required by the Instruction Initialisation routine
*    in the datasheet
*
*******************************************************************************/
unsigned int hd44780InstructionInit(HHD44780 const  hHd44780,
                                    HD44780CLONE    hd44780Clone,
                                    unsigned char   functionSet,
                                    unsigned char   displayOnOffControl,
                                    unsigned char   entryModeSet)
{
    unsigned int returnValue;
                                        /* Check LCD interface is actually    */
    	                                /* open                               */
    if (hHd44780->hd44780Flags & HD44780_OPEN)
    {
                                        /* Check what state we are currently  */
                                        /* in                                 */
        switch(hHd44780->hd44780Flags & HD44780_INSTRINITSTATE)
        {
/******************************************************************************/
/*                        START OF INITIALISATION                             */
/******************************************************************************/
                                        /* On starting initialisation, we     */
                                        /* always wait a set time before      */
                                        /* doing anything                     */
            case STARTINIT:
                if (hd44780Clone == HD44780U)
                {
                                        /* Wait >15ms                         */
                    returnValue = 15000;
                }
                else if (hd44780Clone == KS0066U || hd44780Clone == NT7603)
                {
                                        /* Wait >30ms                         */
                    returnValue = 30000;   
                }
                else
                {
                                        /* Otherwise wait >40ms               */
                    returnValue = 40000;   
                }
                                        /* Set up next state                  */
                hHd44780->hd44780Flags &= ~HD44780_INSTRINITSTATE;
                hHd44780->hd44780Flags |= FUNCTIONSET1;
                break;
                
/******************************************************************************/
/*                        FIRST FUNCTION SET CALL                             */
/******************************************************************************/
                                        /* This implements the first Function */
                                        /* set command. This is a single      */
                                        /* 4-bit access in 4-bit bus mode.    */
                                        /* Some chipsets allow an immediate   */
                                        /* second 8-bit access                */
            case FUNCTIONSET1:
                                        /* First get the bus                  */
                if (hHd44780->lcdIfFunctionPointers->pGetBus(hHd44780->hLcdIf))
                {
                                        /* Is it a 4- or 8-bit bus?           */
                    if (lcdifGetPbBusWidth(hHd44780->hLcdIf) == BUS4BITSWIDE)
                    {
/*4444444444444444444444444444444444444444444444444444444444444444444444444444*/
/*                                   4-BIT BUS                                */
/*4444444444444444444444444444444444444444444444444444444444444444444444444444*/
                                        /* For a 4-bit bus...                 */
                        if (hd44780Clone == ST7066U)
                        {   
                                        /* Send first 4 bit FuncSet command   */
                            hHd44780->lcdIfFunctionPointers->p4BitFunctionSet(
                                                               hHd44780->hLcdIf,
                                                               0x03);
                                        /* Wait further 37us                  */
                            returnValue = 37;
                                        /* Set up next state                  */
                            hHd44780->hd44780Flags &= ~HD44780_INSTRINITSTATE;
                            hHd44780->hd44780Flags |= FUNCTIONSET2;
                        }
                        else if ((hd44780Clone == KS0066U) || 
                                 (hd44780Clone == S6A0069) ||
                                 (hd44780Clone == NT7603))
                        {
                                        /* Send two function set commands     */
                                        /* first 4-bit, second 8-bit          */
                            hHd44780->lcdIfFunctionPointers->p4BitFunctionSet(
                                                               hHd44780->hLcdIf,
                                                               0x02);
                            hHd44780->lcdIfFunctionPointers->pWriteInstr(
                                              hHd44780->hLcdIf, 
                                              HD44780_FUNCTIONSET & FS_4BITBUS &
                                              functionSet);
                            if (hd44780Clone == NT7603)
                            {
                                        /* Wait further 40us                  */
                                returnValue = 40;
                            }
                            else
                            {
                                        /* Wait further 39us                  */
                                returnValue = 39;
                            }
                                        /* Set up next state                  */
                            hHd44780->hd44780Flags &= ~HD44780_INSTRINITSTATE;
                            hHd44780->hd44780Flags |= DISPLAYONOFFCONTROL;
                        }
                        else if (hd44780Clone == HD44780U)
                        {
                                        /* Send single 4-bit function set     */
                                        /* command                            */
                            hHd44780->lcdIfFunctionPointers->p4BitFunctionSet(
                                                               hHd44780->hLcdIf,
                                                               0x03);
                                        /* Wait further 4.1ms                 */
                            returnValue = 4100;
                                        /* Set up next state                  */
                            hHd44780->hd44780Flags &= ~HD44780_INSTRINITSTATE;
                            hHd44780->hd44780Flags |= FUNCTIONSET2;
                        }   
                    }
/*8888888888888888888888888888888888888888888888888888888888888888888888888888*/
/*                                   8-BIT BUS                                */
/*8888888888888888888888888888888888888888888888888888888888888888888888888888*/
                    else                /* Bus is 8-bits                      */
                    {
                        if (hd44780Clone == ST7066U)
                        {
                                        /* Send a function set command        */
                            hHd44780->lcdIfFunctionPointers->pWriteInstr(
                                             hHd44780->hLcdIf, 
                                             HD44780_FUNCTIONSET & functionSet);
                                        /* Wait further 37us */
                            returnValue = 37;
                                        /* Set up next state                  */
                            hHd44780->hd44780Flags &= ~HD44780_INSTRINITSTATE;
                            hHd44780->hd44780Flags |= FUNCTIONSET2;
                        }
                        else if ((hd44780Clone == KS0066U) ||
                                 (hd44780Clone == S6A0069) ||
                                 (hd44780Clone == NT7603))
                        {
                                        /* Send a function set command        */
                            hHd44780->lcdIfFunctionPointers->pWriteInstr(
                                             hHd44780->hLcdIf, 
                                             HD44780_FUNCTIONSET & functionSet);
                                        
                            if (hd44780Clone == NT7603)
                            {
                                        /* Wait further 40us                  */
                                returnValue = 40;
                            }
                            else
                            {
                                        /* Wait further 39us                  */
                                returnValue = 39;
                            }
                                        /* Set up next state                  */
                            hHd44780->hd44780Flags &= ~HD44780_INSTRINITSTATE;
                            hHd44780->hd44780Flags |= DISPLAYONOFFCONTROL;
                        }    
                        else if (hd44780Clone == HD44780U)
                        {
                                        /* Send a function set command        */
                            hHd44780->lcdIfFunctionPointers->pWriteInstr(
                                             hHd44780->hLcdIf, 
                                             HD44780_FUNCTIONSET & functionSet);
                                        /* Wait further 4.1ms */
                            returnValue = 4100;
                                        /* Set up next state                  */
                            hHd44780->hd44780Flags &= ~HD44780_INSTRINITSTATE;
                            hHd44780->hd44780Flags |= FUNCTIONSET2;
                        }    
                    }
                                        /* Return the bus                     */
                    hHd44780->lcdIfFunctionPointers->pReturnBus(
                                                              hHd44780->hLcdIf);    
                }
                                        /* If we couldn't get the bus, return */
                                        /* 1 so we get called again           */
                else
                {
                    returnValue = 1;
                }
                break;
/******************************************************************************/
/*                        SECOND FUNCTION SET CALL                            */
/******************************************************************************/
                                        /* This implements the second         */
                                        /* Function set command.              */            
            case FUNCTIONSET2:
                                        /* First get the bus                  */
                if (lcdifGetPb(hHd44780->hLcdIf))
                {
                                        /* Is it a 4- or 8-bit bus?           */
                    if (lcdifGetPbBusWidth(hHd44780->hLcdIf) == BUS4BITSWIDE)
                    {
/*4444444444444444444444444444444444444444444444444444444444444444444444444444*/
/*                                   4-BIT BUS                                */
/*4444444444444444444444444444444444444444444444444444444444444444444444444444*/                        
                        if (hd44780Clone == ST7066U)
                        {
                            hHd44780->lcdIfFunctionPointers->pWriteInstr(
                                              hHd44780->hLcdIf, 
                                              HD44780_FUNCTIONSET & FS_4BITBUS &
                                              functionSet);
                                        /* Wait further 37us                  */
                            returnValue = 37;
                                        /* Set up next state                  */
                            hHd44780->hd44780Flags &= ~HD44780_INSTRINITSTATE;
                            hHd44780->hd44780Flags |= FUNCTIONSET3;
                        }
                        else if (hd44780Clone == HD44780U)
                        {
                                        /* Send 4 bit FuncSet command         */
                            hHd44780->lcdIfFunctionPointers->p4BitFunctionSet(
                                                               hHd44780->hLcdIf,
                                                               0x03);
                                        /* Wait further 100us                 */
                            returnValue = 100;
                                        /* Set up next state                  */
                            hHd44780->hd44780Flags &= ~HD44780_INSTRINITSTATE;
                            hHd44780->hd44780Flags |= FUNCTIONSET3;
                        }     
                    }
/*8888888888888888888888888888888888888888888888888888888888888888888888888888*/
/*                                   8-BIT BUS                                */
/*8888888888888888888888888888888888888888888888888888888888888888888888888888*/
                    else                /* Bus is 8-bits wide                 */
                    {
                        if (hd44780Clone == ST7066U)
                        {
                                        /* Send a function set command        */
                            hHd44780->lcdIfFunctionPointers->pWriteInstr(
                                             hHd44780->hLcdIf, 
                                             HD44780_FUNCTIONSET & functionSet);
                                        /* Wait further 37us */
                            returnValue = 37;
                                        /* Set up next state                  */
                            hHd44780->hd44780Flags &= ~HD44780_INSTRINITSTATE;
                            hHd44780->hd44780Flags |= DISPLAYONOFFCONTROL;
                        }
                        else if (hd44780Clone == HD44780U)
                        {
                                        /* Send a function set command        */
                            hHd44780->lcdIfFunctionPointers->pWriteInstr(
                                             hHd44780->hLcdIf, 
                                             HD44780_FUNCTIONSET & functionSet);
                                        /* Wait further 100us */
                            returnValue = 100;
                                        /* Set up next state                  */
                            hHd44780->hd44780Flags &= ~HD44780_INSTRINITSTATE;
                            hHd44780->hd44780Flags |= FUNCTIONSET3;
                        }    

                    }
                                        /* Return the bus                     */
                    hHd44780->lcdIfFunctionPointers->pReturnBus(
                                                              hHd44780->hLcdIf);  
                }
                                        /* If we couldn't get the bus, return */
                                        /* 1 so we get called again           */
                else
                {
                    returnValue = 1;
                }
                break;

/******************************************************************************/
/*                        THIRD FUNCTION SET CALL                             */
/******************************************************************************/
                                        /* This implements the third          */
                                        /* Function set command.              */            
            case FUNCTIONSET3:
                                        /* First get the bus                  */
                if (lcdifGetPb(hHd44780->hLcdIf))
                {
                                        /* Is it a 4- or 8-bit bus?           */
                    if (lcdifGetPbBusWidth(hHd44780->hLcdIf) == BUS4BITSWIDE)
                    {
/*4444444444444444444444444444444444444444444444444444444444444444444444444444*/
/*                                   4-BIT BUS                                */
/*4444444444444444444444444444444444444444444444444444444444444444444444444444*/
                        if (hd44780Clone == ST7066U)
                        {
                            hHd44780->lcdIfFunctionPointers->pWriteInstr(
                                              hHd44780->hLcdIf, 
                                              HD44780_FUNCTIONSET & FS_4BITBUS &
                                              functionSet);
                                        /* Return 1 so that we are called     */
                                        /* again                              */
                            returnValue = 1;
                                        /* Set up next state                  */
                            hHd44780->hd44780Flags &= ~HD44780_INSTRINITSTATE;
                            hHd44780->hd44780Flags |= DISPLAYONOFFCONTROL;
                        }
                        else if (hd44780Clone == HD44780U)
                        {
                                        /* Send 4 bit FuncSet command         */
                            hHd44780->lcdIfFunctionPointers->p4BitFunctionSet(
                                                               hHd44780->hLcdIf,
                                                               0x03);
                                        /* Send further 4 bit FuncSet command */
                                        /* This time setting 4-bit bus        */
                            hHd44780->lcdIfFunctionPointers->p4BitFunctionSet(
                                                               hHd44780->hLcdIf,
                                                               0x02);
                                        /* Return 1 so we get called again    */
                            returnValue = 1;
                                        /* Set up next state                  */
                            hHd44780->hd44780Flags &= ~HD44780_INSTRINITSTATE;
                            hHd44780->hd44780Flags |= FUNCTIONSET4;
                        }    
                    }
/*8888888888888888888888888888888888888888888888888888888888888888888888888888*/
/*                                   8-BIT BUS                                */
/*8888888888888888888888888888888888888888888888888888888888888888888888888888*/
                                        /* If bus is 8-bits wide              */
                    else
                    {
                        if (hd44780Clone == HD44780U)
                        {
                                        /* Send a function set command        */
                            hHd44780->lcdIfFunctionPointers->pWriteInstr(
                                             hHd44780->hLcdIf, 
                                             HD44780_FUNCTIONSET & functionSet);
                                        /* Return 1 so we get called again    */
                            returnValue = 1;
                                        /* Set up next state                  */
                            hHd44780->hd44780Flags &= ~HD44780_INSTRINITSTATE;
                            hHd44780->hd44780Flags |= FUNCTIONSET4;
                        } 
                    }    
                                        /* Return the bus                     */
                    hHd44780->lcdIfFunctionPointers->pReturnBus(
                                                              hHd44780->hLcdIf);
                }            
                                        /* If we couldn't get the bus, return */
                                        /* 1 so we get called again           */
                else
                {
                    returnValue = 1;
                }
                break;

/******************************************************************************/
/*                        FOURTH FUNCTION SET CALL                            */
/******************************************************************************/
                                        /* This implements the fourth         */
                                        /* Function set command.              */            
            case FUNCTIONSET4:
                                        /* First get the bus                  */
                if (lcdifGetPb(hHd44780->hLcdIf))
                {
                                        /* Is it a 4- or 8-bit bus?           */
                    if (lcdifGetPbBusWidth(hHd44780->hLcdIf) == BUS4BITSWIDE)
                    {
/*4444444444444444444444444444444444444444444444444444444444444444444444444444*/
/*                                   4-BIT BUS                                */
/*4444444444444444444444444444444444444444444444444444444444444444444444444444*/
                        if (hd44780Clone == HD44780U)
                        {
                                        /* Send full FuncSet command          */
                            hHd44780->lcdIfFunctionPointers->pWriteInstr(
                                                           hHd44780->hLcdIf,
                                                           HD44780_FUNCTIONSET &
                                                           FS_4BITBUS &
                                                           functionSet);
                                        /* Return 1 so we get called again    */
                            returnValue = 1;
                                        /* Set up next state                  */
                            hHd44780->hd44780Flags &= ~HD44780_INSTRINITSTATE;
                            hHd44780->hd44780Flags |= DISPLAYONOFFCONTROL;
                        }    
                    }
/*8888888888888888888888888888888888888888888888888888888888888888888888888888*/
/*                                   8-BIT BUS                                */
/*8888888888888888888888888888888888888888888888888888888888888888888888888888*/
                    /* If bus is 8-bits wide              */
                    else
                    {
                        if (hd44780Clone == HD44780U)
                        {
                                        /* Send a function set command        */
                            hHd44780->lcdIfFunctionPointers->pWriteInstr(
                                             hHd44780->hLcdIf, 
                                             HD44780_FUNCTIONSET & functionSet);
                                        /* Return 1 so we get called again    */
                            returnValue = 1;
                                        /* Set up next state                  */
                            hHd44780->hd44780Flags &= ~HD44780_INSTRINITSTATE;
                            hHd44780->hd44780Flags |= DISPLAYONOFFCONTROL;
                        } 
                    }
                                        /* Return the bus                     */
                    hHd44780->lcdIfFunctionPointers->pReturnBus(
                                                              hHd44780->hLcdIf);
                }            
                                        /* If we couldn't get the bus, return */
                                        /* 1 so we get called again           */
                else
                {
                    returnValue = 1;
                }
                break;
                
/******************************************************************************/
/*                        DISPLAY ON/OFF CALL                                 */
/******************************************************************************/
                                        /* This implements the display on/off */
                                        /* command                            */
            case DISPLAYONOFFCONTROL:
                if (lcdifGetPb(hHd44780->hLcdIf))
                {
                    unsigned char address;
                                        /* No longer need to check the bus    */
                                        /* width as the HAL layer deals with  */
                                        /* this issue for us                  */

                                        /* Reading busy bit only works for    */
                                        /* ST7066U and HD44780U at this point */
                                        /* Other controllers still need to    */
                                        /* wait a set time                    */
                    if (hd44780Clone == ST7066U || hd44780Clone == HD44780U)
                    {
                                        /* Read address to get the busy-bit   */
                        hHd44780->lcdIfFunctionPointers->pReadAddr(
                                                    hHd44780->hLcdIf, &address);
                        if (!(address & 0x80))
                        {
                            if (hd44780Clone == ST7066U)
                            {
                                hHd44780->lcdIfFunctionPointers->pWriteInstr(
                                                   hHd44780->hLcdIf,
                                                   HD44780_DISPLAYONOFFCONTROL &
                                                   displayOnOffControl);
                                        /* Set up next state                  */
                                hHd44780->hd44780Flags &= 
                                                        ~HD44780_INSTRINITSTATE;
                                hHd44780->hd44780Flags |= DISPLAYCLEAR;
                            }
                            else if (hd44780Clone == HD44780U)
                            {
                                hHd44780->lcdIfFunctionPointers->pWriteInstr(
                                                   hHd44780->hLcdIf,
                                                   HD44780_DISPLAYONOFFCONTROL &
                                                   DOFC_DISPLAYOFF & 
                                                   DOFC_CURSOROFF &
                                                   DOFC_BLINKINGOFF);
                                        /* Set up next state                  */
                                hHd44780->hd44780Flags &= 
                                                        ~HD44780_INSTRINITSTATE;
                                hHd44780->hd44780Flags |= DISPLAYCLEAR;
                            }
                                        /* Return 1 so we get called again    */
                            returnValue = 1;    
                        }
                                        /* If busy bit was set, note that we  */
                                        /* should be called again             */
                        else
                        {
                            returnValue = 1;   
                        }    
                    }
                    else if ((hd44780Clone == KS0066U) || 
                             (hd44780Clone == S6A0069) ||
                             (hd44780Clone == NT7603))
                    {
                        hHd44780->lcdIfFunctionPointers->pWriteInstr(
                                                   hHd44780->hLcdIf,
                                                   HD44780_DISPLAYONOFFCONTROL &
                                                   displayOnOffControl);
                                        /* Set up next state                  */
                        hHd44780->hd44780Flags &= ~HD44780_INSTRINITSTATE;
                        hHd44780->hd44780Flags |= DISPLAYCLEAR;

                        if (hd44780Clone == NT7603)
                        {
                                        /* Wait further 40us                  */
                            returnValue = 40;
                        }
                        else
                        {
                                        /* Wait further 39us                  */
                            returnValue = 39;
                        }
                                        /* Return the bus                     */
                        hHd44780->lcdIfFunctionPointers->pReturnBus(
                                                              hHd44780->hLcdIf);
                    }
                }
                                        /* If we couldn't get the bus, return */
                                        /* 1 so we get called again           */
                else
                {
                    returnValue = 1;
                }
                break;

/******************************************************************************/
/*                        DISPLAY CLEAR                                       */
/******************************************************************************/
                                        /* This implements the display on/off */
                                        /* command                            */
            case DISPLAYCLEAR:
                if (lcdifGetPb(hHd44780->hLcdIf))
                {
                    unsigned char address;
                                        /* No longer need to check the bus    */
                                        /* width as the HAL layer deals with  */
                                        /* this issue for us                  */

                                        /* Reading busy bit only works for    */
                                        /* ST7066U and HD44780U at this point */
                                        /* Other controllers still need to    */
                                        /* wait a set time                    */
                    if (hd44780Clone == ST7066U || hd44780Clone == HD44780U)
                    {
                                        /* Read address to get the busy-bit   */
                        hHd44780->lcdIfFunctionPointers->pReadAddr(
                                                    hHd44780->hLcdIf, &address);
                        if (!(address & 0x80))
                        {
                            hHd44780->lcdIfFunctionPointers->pWriteInstr(
                                                          hHd44780->hLcdIf,
                                                          HD44780_CLEARDISPLAY);
                                        /* Set up next state                  */
                                hHd44780->hd44780Flags &= 
                                                        ~HD44780_INSTRINITSTATE;
                                hHd44780->hd44780Flags |= ENTRYMODESET;
                                        /* Return 1 so we get called again    */
                            returnValue = 1;    
                        }
                                        /* If busy bit was set, note that we  */
                                        /* should be called again             */
                        else
                        {
                            returnValue = 1;   
                        }
                    }
                    else if (hd44780Clone == KS0066U || 
                             hd44780Clone == S6A0069 ||
                             hd44780Clone == NT7603)
                    {
                        hHd44780->lcdIfFunctionPointers->pWriteInstr(
                                                          hHd44780->hLcdIf,
                                                          HD44780_CLEARDISPLAY);
                                        /* Set up next state                  */
                        hHd44780->hd44780Flags &= ~HD44780_INSTRINITSTATE;
                        hHd44780->hd44780Flags |= ENTRYMODESET;                       
                                        
                        if (hd44780Clone == NT7603)
                        {
                                        /* Call again in 1.64ms or more       */
                            returnValue = 1640;
                        }
                        else
                        {
                                        /* Call again in 1.53ms or more       */
                            returnValue = 1530;
                        }
                    }    

                                        /* Return the bus                     */
                    hHd44780->lcdIfFunctionPointers->pReturnBus(
                                                              hHd44780->hLcdIf);
                }
                                        /* If we couldn't get the bus, return */
                                        /* 1 so we get called again           */
                else
                {
                    returnValue = 1;
                }
                break;
                
/******************************************************************************/
/*                        ENTRY MODE SET                                      */
/******************************************************************************/
                                        /* This implements the display on/off */
                                        /* command                            */
            case ENTRYMODESET:
                if (lcdifGetPb(hHd44780->hLcdIf))
                {
                    unsigned char address;
                                        /* No longer need to check the bus    */
                                        /* width as the HAL layer deals with  */
                                        /* this issue for us                  */

                                        /* Reading busy bit only works for    */
                                        /* ST7066U and HD44780U at this point */
                                        /* Other controllers still need to    */
                                        /* wait a set time                    */
                    if (hd44780Clone == ST7066U || hd44780Clone == HD44780U)
                    {
                                        /* Read address to get the busy-bit   */
                        hHd44780->lcdIfFunctionPointers->pReadAddr(
                                                    hHd44780->hLcdIf, &address);
                        if (!(address & 0x80))
                        {
                            hHd44780->lcdIfFunctionPointers->pWriteInstr( 
                                                          hHd44780->hLcdIf,
                                                          HD44780_ENTRYMODESET &
                                                          entryModeSet);
                                        /* Set up start state in case this    */
                                        /* function is called again           */
                                hHd44780->hd44780Flags &= 
                                                        ~HD44780_INSTRINITSTATE;
                                hHd44780->hd44780Flags |= STARTINIT;
                                        /* Return 0 as initialisation is      */
                                        /* complete                           */
                            returnValue = 0;    
                        }
                                        /* If busy bit was set, note that we  */
                                        /* should be called again             */
                        else
                        {
                            returnValue = 1;   
                        }
                    }
                    else if (hd44780Clone == KS0066U || 
                             hd44780Clone == S6A0069 ||
                             hd44780Clone == NT7603)
                    {
                        hHd44780->lcdIfFunctionPointers->pWriteInstr(
                                                          hHd44780->hLcdIf,
                                                          HD44780_ENTRYMODESET &
                                                          entryModeSet);
                                        /* Set up start state in case this    */
                                        /* function is called again           */
                        hHd44780->hd44780Flags &= ~HD44780_INSTRINITSTATE;
                        hHd44780->hd44780Flags |= STARTINIT;                       
                                        /* Return 0 as initialisation is      */
                                        /* complete                           */
                            returnValue = 0;   
                    }    
   
                                        /* Return the bus                     */
                    hHd44780->lcdIfFunctionPointers->pReturnBus(
                                                              hHd44780->hLcdIf);
                }
                                        /* If we couldn't get the bus, return */
                                        /* 1 so we get called again           */
                else
                {
                    returnValue = 1;
                }
                break;
        }    
    } 
    
    return returnValue;   
}    

/*******************************************************************************
* isHD44780Busy() --PRIVATE FUNCTION--
*
* Summary: 
*   Checks the busy bit of the LCD chip set. This function is private to the 
* HD44780 Module.
*
* See also:
*   None
*
* Arguments: 
*   hHd44780        - handle to valid HD44780 object
*
* Returns: 
*   0               - device not busy
*   1               - device is busy
*
* Callers: 
*   hd44780ClearDisplay(), hd44780ReturnHome(), hd44780EntryModeSet(), 
*   hd44780DisplayControl(), hd44780ShiftControl(), hd44780FunctionSet(), 
*   hd44780SetCGRAMAddr(), hd44780SetCursorAddr(), hd44780ReadAddr(), 
*   hd44780WriteChar(), hd44780ReadChar(), hd44780WriteRAMString(), 
*   hd44780WriteCGRAM()
*
* Notes : 
* 1. You must own the pbIf bus before calling this function, i.e. 
*    hHd44780->lcdIfFunctionPointers->pGetBus(hHd44780->hLcdIf) *must* have
*    returned true. If you don't, this call will fail and return 1
*******************************************************************************/
unsigned char isHD44780Busy(HHD44780 const hHd44780)
{
    unsigned char address;              /* Storage for return value of        */
                                        /* pReadAddr                          */
    hHd44780->lcdIfFunctionPointers->pReadAddr(hHd44780->hLcdIf, &address);
    if (!(address & 0x80))
    {
        return 0;
    }    
    
    return 1;        
}

    
/*******************************************************************************
*
*                                   HD44780 MODULE END
*
*******************************************************************************/
