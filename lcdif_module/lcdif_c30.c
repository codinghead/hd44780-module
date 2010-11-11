/*******************************************************************************
*
* LCD INTERFACE MODULE FOR C30 COMPILER AND PIC24/dsPIC
*
*******************************************************************************/

/*******************************************************************************
*
* This module is used to provide the interface for an HD44780 LCD display, based
* upon the bit-banged GPIO based parallel bus provided the "pbif_<compiler>.h"
* module
*
* Filename : lcdif_c30.c
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
*                                LCDIFC18 MODULE
*
*******************************************************************************/

/*******************************************************************************
*                                 INCLUDE FILES
*******************************************************************************/
#include "lcdif_c30.h"
#include "pbif_c30.h"

/*******************************************************************************
*                                 LOCAL DEFINES
*******************************************************************************/

/*******************************************************************************
* Summary:
*   Used to indicate that the LCD interface object is open and in use
*******************************************************************************/
#define LCDIF_OPEN          (0x01 << 7)

/*******************************************************************************
* Summary:
*   Used to indicate the parallel bus width is 4 bits
*******************************************************************************/
#define LCDIF_PBWIDTH4BITS  (0x01 << 6)

/*******************************************************************************
* Summary:
*   Used to indicate that this LCD interface currently owns the peripheral bus
*******************************************************************************/
#define LCDIF_OWNPB         (0x01 << 5)

/*******************************************************************************
* Summary:
*   Used to mask the bitShiftData information in flags (lowest three bits)
*******************************************************************************/
#define LCDIF_SHIFTDATAMASK (0x07)

/*******************************************************************************
* Summary:
*   Used to indicate that the parallel bus is not busy when the LCD interface
* object is initialised in lcdifCreate()
* #### ToDo: If this module becomes a generic module for all controllers,
*            PBIF_NOT_BUSY should be 0x00 for PIC24, dsPIC and PIC32 as they
*            will use an atomic bit test & set to indicate the bus is in use and
*            not a subtraction as is used for the PIC18
*******************************************************************************/
#if defined(__18CXX)
#define PBIF_NOT_BUSY       0x01;
#else
#define PBIF_NOT_BUSY       0x00;
#endif

/*******************************************************************************
* Summary:
*   Used to indicate that the LCD interface is in use from another task
*******************************************************************************/
#define LCDIF_BUSY          0;

/*******************************************************************************
* Summary:
*   Used to indicate that the LCD interface call was successful
*******************************************************************************/
#define LCDIF_SUCCESS       1;

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
* Summary:
*   Local pointer to a linked list of LCD intrface objects
*******************************************************************************/
static LCDIFOBJ * startOfLcdIfObjs;

/*******************************************************************************
* Summary:
*   Used to note how many LCD interface objects are active. Each bit in this
* variable relates to one active object.
*******************************************************************************/
static unsigned int activeLcdIfObjects;

/*******************************************************************************
*                             LOCAL FUNCTION PROTOTYPES
*******************************************************************************/
                                        /* These functions are implemented in */
                                        /* assembly code and hence are        */
                                        /* defined as extern                  */
extern unsigned char    pbifGetBusMutex(unsigned int * pbIfFlag);
extern void             pbifReturnBusMutex(unsigned int * pbIfFlag);


/*******************************************************************************
*                            LOCAL CONFIGURATION ERRORS
*******************************************************************************/

#if !defined(__C30)
#error This module requires the use of the C30 compiler.
#error If you wish to use this code with another platform, search in the 
#error directory where you found this file for a possible port.
#error This file is currently saved here: __FILE__
#endif


/*******************************************************************************
* lcdifInit()
*
* Summary: 
*   Initialises the LCDIFC18 module for first use
*
* See also:
*   lcdifDeinit()
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
void lcdifInit(void)
{
                                        /* Initialise the local pointer to    */
                                        /* the list of LCDIF objects          */
    startOfLcdIfObjs = (LCDIFOBJ *) 0;
                                        /* Currently no active LCDIF objects  */
    activeLcdIfObjects = 0;
}

/*******************************************************************************
* lcdifDeinit()
*
* Summary: 
*   Deinitialises the LCDIFC18 module for first use
*
* See also:
*   lcdifInit()
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
void lcdifDeinit(void)
{
                                        /* Initialise the local pointer to    */
                                        /* the list of LCDIF objects          */
    startOfLcdIfObjs = (LCDIFOBJ *) 0;
                                        /* Currently no active LCDIF objects  */
    activeLcdIfObjects = 0;

}

/*******************************************************************************
* lcdifCreate()
*
* Summary: 
*   Creates an LCD interface for use by this module
*
* See also:
*   lcdifDestroy()
*
* Arguments: 
*   lcdIfObj    - lcdif object to insert in a list of lcdif objects 
*
* Returns: 
*   - 1 to MAX_LCFIFS  	- number the LCD interface has been assigned if it was 
*                         possible to allocate it
*   - 0		            - if the LCD interface allocation failed
*
* Callers: 
*   Main application code
*
* Notes : 
*   1. lcdifCreate() must have been called prior to calling this function
*******************************************************************************/
LCDIFNUM lcdifCreate(LCDIFOBJ * const lcdIfObj)
{
    LCDIFOBJ * localLcdIfObj;           /* Stores local copy of pointer to    */
                                        /* linked list so we can insert next  */
                                        /* object in at the top               */
    unsigned int  interfaceNumber = 0x0001;      
                                        /* Used to calculate the interface    */
                                        /* number to return to caller         */
    unsigned char bitTest;
    unsigned char bitCount;
                                        /* Used to note bus width             */
    unsigned char busWidth;
                                        /* Used to note how many bits to      */
                                        /* shift bus data                     */
    unsigned char busDataShift;
                                        /* Check we got an object to point to */
    if(lcdIfObj != (LCDIFOBJ *) 0)
    {
                                        /* Check that there is some useful    */
                                        /* information in the object that was */
                                        /* passed                             */
                                        /* First, do we have a E_LAT?       */
        if (lcdIfObj->E_LAT == (volatile unsigned int *) 0)
        {
            goto cannot_create_if;
        }
                                        /* Do we have only one E_BIT?        */
        for (bitTest = 0x01, bitCount = 0; bitTest != 0x00; bitTest <<= 1)
        {
            if (lcdIfObj->E_BIT & bitTest)
            {
                bitCount++;
            }    
        }
        if (bitCount != 1)
        {
            goto cannot_create_if;
        }    
                                        /* Do we have a PBIFOBJ object?       */
        if (lcdIfObj->pbIfObject == (PBIFOBJ *) 0)
        {
            goto cannot_create_if;
        }    
                                        /* Do we have anything in the         */
                                        /* PBIFOBJ object?                    */
                                        /* Do we have only one RW_BIT?        */
        for (bitTest = 0x01, bitCount = 0; bitTest != 0x00; bitTest <<= 1)
        {
            if (lcdIfObj->pbIfObject->RW_BIT & bitTest)
            {
                bitCount++;
            }    
        }
        if (bitCount != 1)
        {
            goto cannot_create_if;
        }    
                                        /* Do we have only one RS_BIT?        */
        for (bitTest = 0x01, bitCount = 0; bitTest != 0x00; bitTest <<= 1)
        {
            if (lcdIfObj->pbIfObject->RW_BIT & bitTest)
            {
                bitCount++;
            }    
        }
        if (bitCount != 1)
        {
            goto cannot_create_if;
        }    
                                        /* Do we have only four or eight      */
                                        /* DATA_MASK bits                     */
        for (bitTest = 0x01, bitCount = 0; bitTest != 0x00; bitTest <<= 1)
        {
            if (lcdIfObj->pbIfObject->DATA_MASK & bitTest)
            {
                bitCount++;
            }    
        }
        if (bitCount == 4)
        {
            busWidth = bitCount;
                                        /* Calculate number of bits to shift  */
                                        /* data for 4-bit bus                 */
            for (bitTest = 0x01, busDataShift = 0; bitTest != 0x00; 
                                                        bitTest <<= 1)
            {
                if (!(lcdIfObj->pbIfObject->DATA_MASK & bitTest))
                {
                    busDataShift++;
                }
                else
                {
                    break;
                }    
            }    
        }    
        else if (bitCount == 8)
        {
            busWidth = bitCount;
        }    
        else
        {
            goto cannot_create_if;
        }    
                                        /* Do we have a RW_LAT?              */
        if (lcdIfObj->pbIfObject->RW_LAT == (volatile unsigned int *) 0)
        {
            goto cannot_create_if;
        }
                                        /* Do we have a RS_LAT?              */
        if (lcdIfObj->pbIfObject->RS_LAT == (volatile unsigned int *) 0)
        {
            goto cannot_create_if;
        }
                                        /* Do we have a DATA_PORT?            */
        if (lcdIfObj->pbIfObject->DATA_PORT == (volatile unsigned int *) 0)
        {
            goto cannot_create_if;
        }
                                        /* Do we have a DATA_LAT?             */
        if (lcdIfObj->pbIfObject->DATA_LAT == (volatile unsigned int *) 0)
        {
            goto cannot_create_if;
        }
                                        /* Do we have a DATA_TRIS?            */
        if (lcdIfObj->pbIfObject->DATA_TRIS == (volatile unsigned int *) 0)
        {
            goto cannot_create_if;
        }
                                        /* If we got here the object contains */
                                        /* valid data we can work with        */

                                        /* If there are no active objects in  */
                                        /* the list put this object at the    */
                                        /* top                                */
        if (activeLcdIfObjects == 0 && startOfLcdIfObjs == (LCDIFOBJ *) 0)
        {
            startOfLcdIfObjs = lcdIfObj;
            startOfLcdIfObjs->nextLcdIfObj = (LCDIFOBJ *) 0;
                                        /* Assign the interface number        */
            activeLcdIfObjects |= interfaceNumber;
            startOfLcdIfObjs->lcdIfNum = interfaceNumber;
                                        /* Clear the object's flags           */
            startOfLcdIfObjs->lcdIfFlags = 0;
                                        /* Note parallel bus width - if it is */
                                        /* not 4 it must be 8                 */
                                        /* Also note amount to shift data to  */
                                        /* use on the bus                     */
            if (busWidth == 4)
            {
                startOfLcdIfObjs->lcdIfFlags |= LCDIF_PBWIDTH4BITS;
                startOfLcdIfObjs->lcdIfFlags |= (busDataShift & 
                                                        LCDIF_SHIFTDATAMASK);
            }
                                        /* Note that the parallel bus is not  */
                                        /* in use                             */
            startOfLcdIfObjs->pbIfObject->mutex = PBIF_NOT_BUSY;
            
            return startOfLcdIfObjs->lcdIfNum;
        }
                                        /* Otherwise, if we haven't allocated */
                                        /* all the LCD interface objects we   */
                                        /* can support, insert another        */
                                        /* #### Need to mask this to 16-bits  */
        else if (activeLcdIfObjects != 0xFFFF)
        {
                                        /* Insert this object at start of the */
                                        /* list                               */
            localLcdIfObj = startOfLcdIfObjs;
            startOfLcdIfObjs = lcdIfObj;
            startOfLcdIfObjs->nextLcdIfObj = localLcdIfObj;
                                        /* Find a free LCD interface number   */
                                        /* for this interface                 */
            for (interfaceNumber = 0x0002; interfaceNumber == 0x0000; 
                    interfaceNumber <<= 1)
            {
                if (!activeLcdIfObjects & interfaceNumber)
                {
                                        /* Assign the interface number        */
                    activeLcdIfObjects |= interfaceNumber;
                    startOfLcdIfObjs->lcdIfNum = interfaceNumber;
                                        /* Clear the object's flags           */
                    startOfLcdIfObjs->lcdIfFlags = 0;
                    return startOfLcdIfObjs->lcdIfNum;
                }    
            }    
        }
    }
cannot_create_if:
                                        /* Couldn't create interface          */
    return 0;
}

/*******************************************************************************
* lcdifDestroy()
*
* Summary: 
*   Destroys a previously created LCD interface object
*
* See also:
*   lcdifCreate()
*
* Arguments: 
*   lcdIfNumber - number of the LCD interface object to destroy 
*
* Returns: 
*   - 1   - LCD interface was successfully destroyed
*   - 0	  - couldn't detroy requested LCD interface - probably still open
*
* Callers: 
*   Main application code
*
* Notes : 
*   1. lcdifCreate() must have been called prior to calling this function
*******************************************************************************/
unsigned char lcdifDestroy(LCDIFNUM lcdIfNumber)
{
    LCDIFOBJ * localLcdIfObj;           /* Stores local copy of pointer to    */
                                        /* linked list so we can insert next  */
                                        /* object in at the top               */
    LCDIFOBJ * tempLcdIfObj;            /* Temporary object pointer so we can */
                                        /* remove an object from the list     */
    unsigned int  interfaceNumber;      /* Used to calculate the interface    */
                                        /* number to return to caller         */
                                        
                                        /* Check that there are created       */
                                        /* objects                            */
    if(startOfLcdIfObjs != (LCDIFOBJ *) 0)
    {
                                        /* Search through the list to find    */
                                        /* this interface                     */
        localLcdIfObj = startOfLcdIfObjs;

                                        /* If this is the first object in the */
                                        /* list and the object is not open,   */
                                        /* simply remove it                   */
        if (startOfLcdIfObjs->lcdIfNum == lcdIfNumber && 
            !(localLcdIfObj->lcdIfFlags & LCDIF_OPEN))
        {
            startOfLcdIfObjs = startOfLcdIfObjs->nextLcdIfObj;
                                        /* Also note that we have one less    */
                                        /* active LCD interface               */
            activeLcdIfObjects &= ~lcdIfNumber;
            return 1;
        }
                                        /* Otherwise if this object is still  */
                                        /* open, return that is can't be      */
                                        /* destroyed                          */
        else if (startOfLcdIfObjs->lcdIfNum == lcdIfNumber &&
                 (localLcdIfObj->lcdIfFlags & LCDIF_OPEN))
        {
            return 0;
        }    
                                        /* Otherwise parse through list of    */
                                        /* objects until we find the object   */
                                        /* or find no object with that number */
        do
        {
                                        /* Save pointer to currect object in  */
                                        /* case next object will be removed   */
            tempLcdIfObj = localLcdIfObj;
                                        /* Ensure that this object actually   */
                                        /* points to another object and not   */
                                        /* to NULL                            */
            if(localLcdIfObj->nextLcdIfObj != (LCDIFOBJ *) 0)
            {
                                        /* Point to that next object in       */
                                        /* list...                            */                          
                localLcdIfObj = localLcdIfObj->nextLcdIfObj;
                                        /* ...and see if it is the            */
                                        /* interface we are searching for and */
                                        /* that the interface is not open     */
                if (localLcdIfObj->lcdIfNum == lcdIfNumber &&
                   !(localLcdIfObj->lcdIfFlags && LCDIF_OPEN))
                {
                                        /* If so, remove this item from list  */
                                        /* by pointing previous object to     */
                                        /* where this object points           */
                    tempLcdIfObj->nextLcdIfObj = localLcdIfObj->nextLcdIfObj;
                                        /* Also note that we have one less    */
                                        /* active LCD interface               */
                    activeLcdIfObjects &= ~lcdIfNumber;
                    return 1;
                }
            }    
            
        } while(localLcdIfObj->nextLcdIfObj != (LCDIFOBJ *) 0);

    }
                                        /* Couldn't destroy interface         */
    return 0;
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
HLCDIF lcdifOpen(LCDIFNUM lcdIfNumber)
{
	LCDIFOBJ * localLcdIfObj;           /* Stores local copy of pointer to    */
                                        /* linked list so we can insert next  */
                                        /* object in at the top               */
	
                                        /* Check that there are created       */
                                        /* objects                            */
    if(startOfLcdIfObjs != (LCDIFOBJ *) 0)
    {
                                        /* Check that a LCDIFNUM object       */
                                        /* exists with lcdIfNumber            */

        localLcdIfObj = startOfLcdIfObjs;
        
        do
        {
    	    if (localLcdIfObj->lcdIfNum == lcdIfNumber)
    	    {
        	                            /* Check buffer is not already open   */
                if (!(localLcdIfObj->lcdIfFlags & LCDIF_OPEN))
                {
                                        /* Note that it is now in use         */
                    localLcdIfObj->lcdIfFlags |= LCDIF_OPEN;
        	                            /* Return handle to it                */
        	        return localLcdIfObj;
        	    }
        	                            /* If buffer is already open, return  */
        	                            /* NULL handle pointer                */
        	    else
        	    {
            	    return (LCDIFOBJ *) 0;
            	}    
        	}    
        	else
        	{
            	localLcdIfObj = localLcdIfObj->nextLcdIfObj;
            }   	
    	} while (localLcdIfObj != (LCDIFOBJ *) 0);
    }
    	                                /* Return handle to NULL otherwise    */
    return (LCDIFOBJ *) 0;
}

/*******************************************************************************
* lcdifClose()
*
* Summary: 
*   Closes an LCD interface and releases the handle to it
*
* See also:
*   lcdifOpen()
*
* Arguments: 
*   hLcdIf          - handle to the open buffer
*
* Returns: 
*   - >0  	        - number of LCD interface object if it was was open
*   - 0             - if the LCD interface was not open
*
* Callers: 
*   User application
*
* Notes : 
* 1. Caller must have 'created' at least one LCD interface object before
*    calling this function
*
*******************************************************************************/
LCDIFNUM lcdifClose(HLCDIF const hLcdIf)
{
                                        /* Check that there are created       */
                                        /* objects                            */
    if(startOfLcdIfObjs != (LCDIFOBJ *) 0)
    {
    	                                /* Check LCD interface is actually    */
    	                                /* open                               */
        if (hLcdIf->lcdIfFlags & LCDIF_OPEN)
        {
        	                            /* Note that this LCD interface       */
        	                            /* object is closed                   */
            hLcdIf->lcdIfFlags &= ~LCDIF_OPEN;
                                        /* Return LCD interface object's      */
                                        /* interface number                   */
            return hLcdIf->lcdIfNum;
        }
    }
                                        /* Otherwise return 0 to say that     */
                                        /* buffer object wasn't open          */
    return (LCDIFNUM) 0;
}    

/*******************************************************************************
* lcdifGetPb()
*
* Summary: 
*   Attempts to aquire the peripheral bus for use by the LCD interface module
*
* See also:
*   lcdifReturnPb()
*
* Arguments: 
*   hLcdIf          - handle to the open LCD interface
*
* Returns: 
*   - 1  	        - this LCD interface (hLcdIf) owns the peripheral bus
*   - 0             - the peripheral bus is currently in use by another task
*
* Callers: 
*   User application
*
* Notes : 
* 1. Caller must have 'created' at least one LCD interface object before
*    calling this function
*
*******************************************************************************/
unsigned char lcdifGetPb(HLCDIF const hLcdIf)
{
                                        /* Attempt to get the peripheral bus  */
                                        /* for this hLcdIf object             */
    if (pbifGetBusMutex(&hLcdIf->pbIfObject->mutex))
    {
                                        /* If we were successful, note the    */
                                        /* fact in the LCD interface's flags  */
                                        /* variable                           */
        hLcdIf->lcdIfFlags |= LCDIF_OWNPB;
        return 1;
    }
    else
    {
                                        /* If we failed in our attempt,       */
                                        /* return 0                           */
        return 0;   
    }        
}    

/*******************************************************************************
* lcdifReturnPb()
*
* Summary: 
*   Returns the peripheral bus for use by other modules
*
* See also:
*   lcdifGetPb()
*
* Arguments: 
*   hLcdIf          - handle to the open LCD interface
*
* Returns: 
*   void
*
* Callers: 
*   User application
*
* Notes : 
* 1. Caller must have 'created' at least one LCD interface object before
*    calling this function
*
*******************************************************************************/
void lcdifReturnPb(HLCDIF const hLcdIf)
{
                                        /* Return the peripheral bus          */
    pbifReturnBusMutex(&hLcdIf->pbIfObject->mutex);
                                        /* Note this in the LCD interface's   */
                                        /* flags variable                     */
    hLcdIf->lcdIfFlags &= ~LCDIF_OWNPB;
}    

/*******************************************************************************
* lcdifWriteData()
*
* Summary: 
*   Writes data to the LCD interface
*
* See also:
*   lcdifReadData()
*
* Arguments: 
*   hLcdIf          - handle to the open LCD interface
*   data            - data to write to the LCD interface
*
* Returns: 
*   - LCDIF_BUSY    - if the LCD parallel bus is in use
*   - LCDIF_SUCCESS - if the LCD write completed
*
* Callers: 
*   User application
*
* Notes : 
* 1. Caller must have 'created' at least one LCD interface object before
*    calling this function
* 2. In the lcdifReadxxxx and lcdifWritexxxx functions, no assumptions are made
*    about the state of the pins. The functions use the pins as desired and then
*    leave them in that state. For this write function that also means that the 
*    PORT pins being used for data output are left as outputs. The only rule is 
*    to leave the ENABLE pin low when finished
* 3. You must have called lcdifGetPb() successfully before calling this function
*******************************************************************************/
unsigned char lcdifWriteData(HLCDIF const hLcdIf, unsigned char data)
{
    unsigned char tempData;
                                        /* Check if we own the peripheral bus */
    if (hLcdIf->lcdIfFlags && LCDIF_OWNPB)
    {   
                                        /* If it is a 4-bit bus, use a 4-bit  */
                                        /* access                             */
        if (hLcdIf->lcdIfFlags & LCDIF_PBWIDTH4BITS)
        {
                                        /* The following is a 4-bit write     */
                                        /* data sequence as per HD44780       */
                                        /* datasheets                         */
                                        
                                        /* Clear RW pin                       */
            *hLcdIf->pbIfObject->RW_LAT &= ~hLcdIf->pbIfObject->RW_BIT;
                                        /* Set RS pin                         */
            *hLcdIf->pbIfObject->RS_LAT |= hLcdIf->pbIfObject->RS_BIT;
                                        /* Get high nibble of data            */
            tempData = data >> 4;
                                        /* Get data in correct position for   */
                                        /* 4-bit bus                          */
            tempData <<= startOfLcdIfObjs->lcdIfFlags & LCDIF_SHIFTDATAMASK;
                                        /* Clear data pins                    */
            *hLcdIf->pbIfObject->DATA_LAT &= ~hLcdIf->pbIfObject->DATA_MASK;
                                        /* Set desired data pins              */
            *hLcdIf->pbIfObject->DATA_LAT |= tempData;
                                        /* Set data pins to outputs           */
            *hLcdIf->pbIfObject->DATA_TRIS &= ~hLcdIf->pbIfObject->DATA_MASK;
                                        /* Set E pin                          */
            *hLcdIf->E_LAT |= hLcdIf->E_BIT;
                                        /* Clear E pin                        */
            *hLcdIf->E_LAT &= ~hLcdIf->E_BIT;
                                        /* Clear data pins                    */
            *hLcdIf->pbIfObject->DATA_LAT &= ~hLcdIf->pbIfObject->DATA_MASK;
                                        /* Get low nibble of data             */
            tempData = data & 0x0F;
                                        /* Get data in correct position for   */
                                        /* 4-bit bus                          */
            tempData <<= startOfLcdIfObjs->lcdIfFlags & LCDIF_SHIFTDATAMASK;
                                        /* Set desired data pins              */
            *hLcdIf->pbIfObject->DATA_LAT |= tempData;
                                        /* Set E pin                          */
            *hLcdIf->E_LAT |= hLcdIf->E_BIT;
                                        /* Clear E pin                        */
            *hLcdIf->E_LAT &= ~hLcdIf->E_BIT;
                                        /* Write data process finished        */
        }
        else
        {
                                        /* The following is an 8-bit write    */
                                        /* data sequence as per HD44780       */
                                        /* datasheets                         */
                                        
                                        /* Clear RW pin                       */
            *hLcdIf->pbIfObject->RW_LAT &= ~hLcdIf->pbIfObject->RW_BIT;
                                        /* Set RS pin                         */
            *hLcdIf->pbIfObject->RS_LAT |= hLcdIf->pbIfObject->RS_BIT;
                                        /* Write data to pins                 */
            *hLcdIf->pbIfObject->DATA_LAT = data;
                                        /* Set data pins to outputs           */
            *hLcdIf->pbIfObject->DATA_TRIS = 0x00;
                                        /* Set E pin                          */
            *hLcdIf->E_LAT |= hLcdIf->E_BIT;
                                        /* Clear E pin                        */
            *hLcdIf->E_LAT &= ~hLcdIf->E_BIT;
                                        /* Write data process finished        */
        }    
                                        /* Inform caller that write succeeded */
        return LCDIF_SUCCESS;       
    }
                                        /* Inform caller that bus is busy     */
    return LCDIF_BUSY;    
}

/*******************************************************************************
* lcdifReadData()
*
* Summary: 
*   Reads data from the LCD interface. The data read will be the contents of a
*   CGRAM address if the previous instruction set a CGRAM address. If the
*   previous instruction set a DDRAM address, the DDRAM address will be read
*
* See also:
*   lcdifWriteData()
*
* Arguments: 
*   hLcdIf          - handle to the open LCD interface
*   * data          - variable in which to store read data
*
* Returns: 
*   - LCDIF_BUSY    - if the LCD parallel bus is in use
*   - LCDIF_SUCCESS - if the LCD write completed
*
* Callers: 
*   User application
*
* Notes : 
* 1. Caller must have 'created' at least one LCD interface object before
*    calling this function
* 2. In the lcdifReadxxxx and lcdifWritexxxx functions, no assumptions are made
*    about the state of the pins. The functions use the pins as desired and then
*    leave them in that state. For this write function that also means that the 
*    PORT pins being used for data output are left as outputs. The only rule is 
*    to leave the ENABLE pin low when finished
* 3. You must have called lcdifGetPb() successfully before calling this function
*******************************************************************************/
unsigned char lcdifReadData(HLCDIF const hLcdIf, unsigned char * const data)
{
    unsigned char tempData;
    unsigned char tempData2;
                                        /* Check if we own the peripheral bus */
    if (hLcdIf->lcdIfFlags && LCDIF_OWNPB)
    {   
                                        /* If it is a 4-bit bus, use a 4-bit  */
                                        /* access                             */
        if (hLcdIf->lcdIfFlags & LCDIF_PBWIDTH4BITS)
        {
                                        /* The following is a 4-bit read      */
                                        /* data sequence as per HD44780       */
                                        /* datasheets                         */
                                        
                                        /* Set RW pin                         */
            *hLcdIf->pbIfObject->RW_LAT |= hLcdIf->pbIfObject->RW_BIT;
                                        /* Set RS pin                         */
            *hLcdIf->pbIfObject->RS_LAT |= hLcdIf->pbIfObject->RS_BIT;
                                        /* Set data pins to inputs            */
            *hLcdIf->pbIfObject->DATA_TRIS |= hLcdIf->pbIfObject->DATA_MASK;
                                        /* Set E pin                          */
            *hLcdIf->E_LAT |= hLcdIf->E_BIT;
                                        /* Read high nibble of data and shift */
                                        /* into low four bytes of tempData    */
            tempData = (*hLcdIf->pbIfObject->DATA_PORT & 
                                                hLcdIf->pbIfObject->DATA_MASK);
            tempData >>= (startOfLcdIfObjs->lcdIfFlags & LCDIF_SHIFTDATAMASK);
                                        /* Clear E pin                        */
            *hLcdIf->E_LAT &= ~hLcdIf->E_BIT;
                                        /* Shift data into high nibble of     */
                                        /* tempData                           */
            tempData <<= 4;
                                        /* Set E pin                          */
            *hLcdIf->E_LAT |= hLcdIf->E_BIT;
                                        /* Read low nibble of data and add    */
                                        /* into low four bytes of tempData    */
            tempData2 = (*hLcdIf->pbIfObject->DATA_PORT& 
                                                hLcdIf->pbIfObject->DATA_MASK);
            tempData2 >>= (startOfLcdIfObjs->lcdIfFlags & LCDIF_SHIFTDATAMASK);
                                        /* Formulate whole data byte          */
            tempData += tempData2;
                                        /* Give value read back to caller     */
            * data = tempData;          
                                        /* Clear E pin                        */
            *hLcdIf->E_LAT &= ~hLcdIf->E_BIT;
                                        /* Read data process finished         */
        }
        else
        {
                                        /* The following is an 8-bit read     */
                                        /* data sequence as per HD44780       */
                                        /* datasheets                         */
                                        
                                        /* Set RW pin                         */
            *hLcdIf->pbIfObject->RW_LAT |= hLcdIf->pbIfObject->RW_BIT;
                                        /* Set RS pin                         */
            *hLcdIf->pbIfObject->RS_LAT |= hLcdIf->pbIfObject->RS_BIT;
                                        /* Set data pins to inputs            */
            *hLcdIf->pbIfObject->DATA_TRIS = 0xFF;
                                        /* Set E pin                          */
            *hLcdIf->E_LAT |= hLcdIf->E_BIT;
                                        /* Read high nibble of data and shift */
                                        /* into low four bytes of tempData    */
            tempData = *hLcdIf->pbIfObject->DATA_PORT;
                                        /* Clear E pin                        */
            *hLcdIf->E_LAT &= ~hLcdIf->E_BIT;
                                        /* Give value read back to caller     */
            * data = tempData;          
                                        /* Read data process finished         */   
        }    
                                        /* Inform caller that write succeeded */
        return LCDIF_SUCCESS;       
    }
                                        /* Inform caller that bus is busy     */
    return LCDIF_BUSY;    
}

/*******************************************************************************
* lcdifWriteInstruction()
*
* Summary: 
*   Writes an instruction to the LCD interface
*
* See also:
*   lcdifReadAddress()
*
* Arguments: 
*   hLcdIf          - handle to the open LCD interface
*   instruction     - instruction to write to the LCD interface
*
* Returns: 
*   - LCDIF_BUSY    - if the LCD parallel bus is in use
*   - LCDIF_SUCCESS - if the LCD write completed
*
* Callers: 
*   User application
*
* Notes : 
* 1. Caller must have 'created' at least one LCD interface object before
*    calling this function
* 2. In the lcdifReadxxxx and lcdifWritexxxx functions, no assumptions are made
*    about the state of the pins. The functions use the pins as desired and then
*    leave them in that state. For this write function that also means that the 
*    PORT pins being used for data output are left as outputs. The only rule is 
*    to leave the ENABLE pin low when finished
* 3. You must have called lcdifGetPb() successfully before calling this function
*******************************************************************************/
unsigned char lcdifWriteInstruction(HLCDIF const hLcdIf, 
                                   unsigned char instruction)
{
    unsigned char tempInstruction;
                                        /* Check if we own the peripheral bus */
    if (hLcdIf->lcdIfFlags && LCDIF_OWNPB)
    {   
                                        /* If it is a 4-bit bus, use a 4-bit  */
                                        /* access                             */
        if (hLcdIf->lcdIfFlags & LCDIF_PBWIDTH4BITS)
        {
                                        /* The following is a 4-bit write     */
                                        /* data sequence as per HD44780       */
                                        /* datasheets                         */
                                        
                                        /* Clear RW pin                       */
            *hLcdIf->pbIfObject->RW_LAT &= ~hLcdIf->pbIfObject->RW_BIT;
                                        /* Clear RS pin                       */
            *hLcdIf->pbIfObject->RS_LAT &= ~hLcdIf->pbIfObject->RS_BIT;
                                        /* Get high nibble of instruction     */
            tempInstruction = instruction >> 4;
                                        /* Get data in correct position for   */
                                        /* 4-bit bus                          */
            tempInstruction <<= startOfLcdIfObjs->lcdIfFlags & 
                                                            LCDIF_SHIFTDATAMASK;
                                        /* Clear data pins                    */
            *hLcdIf->pbIfObject->DATA_LAT &= ~hLcdIf->pbIfObject->DATA_MASK;
                                        /* Set desired data pins              */
            *hLcdIf->pbIfObject->DATA_LAT |= tempInstruction;
                                        /* Set data pins to outputs           */
            *hLcdIf->pbIfObject->DATA_TRIS &= ~hLcdIf->pbIfObject->DATA_MASK;
                                        /* Set E pin                          */
            *hLcdIf->E_LAT |= hLcdIf->E_BIT;
                                        /* Clear E pin                        */
            *hLcdIf->E_LAT &= ~hLcdIf->E_BIT;
                                        /* Clear data pins                    */
            *hLcdIf->pbIfObject->DATA_LAT &= ~hLcdIf->pbIfObject->DATA_MASK;
                                        /* Get low nibble of instruction      */
            tempInstruction = instruction & 0x0F;
                                        /* Get instruction in correct         */
                                        /* position for 4-bit bus             */
            tempInstruction <<= startOfLcdIfObjs->lcdIfFlags & 
                                                            LCDIF_SHIFTDATAMASK;
                                        /* Set desired data pins              */
            *hLcdIf->pbIfObject->DATA_LAT |= tempInstruction;
                                        /* Set E pin                          */
            *hLcdIf->E_LAT |= hLcdIf->E_BIT;
                                        /* Clear E pin                        */
            *hLcdIf->E_LAT &= ~hLcdIf->E_BIT;
                                        /* Write instruction process finished */
        }
        else
        {
                                        /* The following is a 8-bit write     */
                                        /* data sequence as per HD44780       */
                                        /* datasheets                         */
                                        
                                        /* Clear RW pin                       */
            *hLcdIf->pbIfObject->RW_LAT &= ~hLcdIf->pbIfObject->RW_BIT;
                                        /* Clear RS pin                       */
            *hLcdIf->pbIfObject->RS_LAT &= ~hLcdIf->pbIfObject->RS_BIT;
                                        /* Write data to pins                 */
            *hLcdIf->pbIfObject->DATA_LAT = instruction;
                                        /* Set data pins to outputs           */
            *hLcdIf->pbIfObject->DATA_TRIS = 0;
                                        /* Set E pin                          */
            *hLcdIf->E_LAT |= hLcdIf->E_BIT;
                                        /* Clear E pin                        */
            *hLcdIf->E_LAT &= ~hLcdIf->E_BIT;
                                        /* Write instruction process finished */
        }    
                                        /* Inform caller that write succeeded */
        return LCDIF_SUCCESS;       
    }
                                        /* Inform caller that bus is busy     */
    return LCDIF_BUSY;    
}

/*******************************************************************************
* lcdifReadAddress()
*
* Summary: 
*   Reads address counter value and busy flag
*
* See also:
*   lcdifWriteData()
*
* Arguments: 
*   hLcdIf          - handle to the open LCD interface
*   * address       - variable in which to store read address value
*
* Returns: 
*   - LCDIF_BUSY    - if the LCD parallel bus is in use
*   - LCDIF_SUCCESS - if the LCD write completed
*
* Callers: 
*   User application
*
* Notes : 
* 1. Caller must have 'created' at least one LCD interface object before
*    calling this function
* 2. In the lcdifReadxxxx and lcdifWritexxxx functions, no assumptions are made
*    about the state of the pins. The functions use the pins as desired and then
*    leave them in that state. For this write function that also means that the 
*    PORT pins being used for data output are left as outputs. The only rule is 
*    to leave the ENABLE pin low when finished
* 3. During tested it was noticed that some chipsets return the address read in
*    low-nibble/high-nibble format, instead of high/low! This should be checked
*    during the initialisation phase of the LCD
* 4. You must have called lcdifGetPb() successfully before calling this function
*******************************************************************************/
unsigned char lcdifReadAddress(HLCDIF const hLcdIf, 
                      unsigned char * const address)
{
    unsigned char tempAddress;
    unsigned char tempAddress2;
                                        /* Check if we own the peripheral bus */
    if (hLcdIf->lcdIfFlags && LCDIF_OWNPB)
    {   
                                        /* If it is a 4-bit bus, use a 4-bit  */
                                        /* access                             */
        if (hLcdIf->lcdIfFlags & LCDIF_PBWIDTH4BITS)
        {
                                        /* The following is a 4-bit read      */
                                        /* data sequence as per HD44780       */
                                        /* datasheets                         */
                                        
                                        /* Set RW pin                         */
            *hLcdIf->pbIfObject->RW_LAT |= hLcdIf->pbIfObject->RW_BIT;
                                        /* Clear RS pin                       */
            *hLcdIf->pbIfObject->RS_LAT &= ~hLcdIf->pbIfObject->RS_BIT;
                                        /* Set data pins to inputs            */
            *hLcdIf->pbIfObject->DATA_TRIS |= hLcdIf->pbIfObject->DATA_MASK;
                                        /* Set E pin                          */
            *hLcdIf->E_LAT |= hLcdIf->E_BIT;
                                        /* Read high nibble of address and    */
                                        /* shift into low four bytes of       */
                                        /* tempAddress                        */
            tempAddress = (*hLcdIf->pbIfObject->DATA_PORT & 
                                                hLcdIf->pbIfObject->DATA_MASK);
            tempAddress >>= startOfLcdIfObjs->lcdIfFlags & LCDIF_SHIFTDATAMASK;
                                        /* Clear E pin                        */
            *hLcdIf->E_LAT &= ~hLcdIf->E_BIT;
                                        /* Shift address into high nibble of  */
                                        /* tempAddress                        */
            tempAddress <<= 4;
                                        /* Set E pin                          */
            *hLcdIf->E_LAT |= hLcdIf->E_BIT;
                                        /* Read low nibble of data and add    */
                                        /* into low four bytes of tempAddress */
            tempAddress2 = (*hLcdIf->pbIfObject->DATA_PORT & 
                                                hLcdIf->pbIfObject->DATA_MASK);
            tempAddress2 >>= startOfLcdIfObjs->lcdIfFlags & LCDIF_SHIFTDATAMASK;
                                        /* Clear E pin                        */
            *hLcdIf->E_LAT &= ~hLcdIf->E_BIT;
                                        /* Formulate whole address            */
            tempAddress += tempAddress2;
                                        /* Return address to caller           */
            *address = tempAddress;
                                        /* Read data process finished         */
        }
        else
        {
                                        /* The following is an 8-bit read     */
                                        /* data sequence as per HD44780       */
                                        /* datasheets                         */
                                        
                                        /* Set RW pin                         */
            *hLcdIf->pbIfObject->RW_LAT |= hLcdIf->pbIfObject->RW_BIT;
                                        /* Clear RS pin                       */
            *hLcdIf->pbIfObject->RS_LAT &= ~hLcdIf->pbIfObject->RS_BIT;
                                        /* Set data pins to inputs            */
            *hLcdIf->pbIfObject->DATA_TRIS = 0xFF;
                                        /* Set E pin                          */
            *hLcdIf->E_LAT |= hLcdIf->E_BIT;
                                        /* Read data                          */
            tempAddress = *hLcdIf->pbIfObject->DATA_PORT;
                                        /* Clear E pin                        */
            *hLcdIf->E_LAT &= ~hLcdIf->E_BIT;
                                        /* Return address to caller           */
            *address = tempAddress;
                                        /* Read data process finished         */
        }    
                                        /* Inform caller that write succeeded */
        return LCDIF_SUCCESS;       
    }
                                        /* Inform caller that bus is busy     */
    return LCDIF_BUSY;    
}

/*******************************************************************************
* lcdif4BitFunctionSet()
*
* Summary: 
*   Writes a 4-bit instruction to the LCD interface. This is required as part
*   of the software initialisation sequence. In the event that the hardware 
*   power-on reset doesn't reset the chipset, a "software initialisation" is 
*   required. All researched chipsets (Hitachi, Sitronix and Samsung), when 
*   using a 4-bit data bus, require that the first bus access after power on be
*   a function set instruction but only passing the high nibble. Depending on
*   the data sheet you read, the DL (data length) bit may need to be 1 or 0
*   *even though* the bus is only 4-bits wide.
*   Subsequent accesses to the bus are then performed in two seperate nibble
*   accesses as normal in 4-bit bus mode.
*
* See also:
*   None
*
* Arguments: 
*   hLcdIf          - handle to the open LCD interface
*   instruction     - nibble instruction to write to the LCD interface
*                     !NOTE! The value here expected is either 0x02 or 0x03.
*                     Check the datasheet for the chipset used on your display
*                     to find out what value to use here. Look for the heading
*                     "Initialising by instruction - 4-bit interface mode".
*
* Returns: 
*   - LCDIF_BUSY    - if the LCD parallel bus is in use
*   - LCDIF_SUCCESS - if the LCD write completed
*
* Callers: 
*   User application
*
* Notes : 
* 1. Caller must have 'created' at least one LCD interface object before
*    calling this function
* 2. In this function, no assumptions are made about the state of the pins. The 
*    functions use the pins as desired and then leave them in that state. For 
*    this write function that also means that the PORT pins being used for data 
*    output are left as outputs. The only rule is to leave the ENABLE pin low 
*    when finished.
* 3. This function is only required when using 4-bit bus mode. *Do not* use this
*    function in 8-bit bus mode.
* 4. You must have called lcdifGetPb() successfully before calling this function
*******************************************************************************/
unsigned char lcdif4BitFunctionSet(HLCDIF const hLcdIf, 
                                  unsigned char instruction)
{
    unsigned char tempInstruction;
                                        /* Check if we own the peripheral bus */
    if (hLcdIf->lcdIfFlags && LCDIF_OWNPB)
    {   
                                        /* The following is a single nibble   */
                                        /* write as required for initialising */
                                        /* the display in 4-bit bus mode      */
                                        
                                        /* Clear RW pin                       */
        *hLcdIf->pbIfObject->RW_LAT &= ~hLcdIf->pbIfObject->RW_BIT;
                                        /* Clear RS pin                       */
        *hLcdIf->pbIfObject->RS_LAT &= ~hLcdIf->pbIfObject->RS_BIT;
                                        /* Make copy of low nibble of         */
                                        /* instruction                        */
        tempInstruction = instruction & 0x0F;
                                        /* Get data in correct position for   */
                                        /* 4-bit bus                          */
        tempInstruction <<= startOfLcdIfObjs->lcdIfFlags & LCDIF_SHIFTDATAMASK;
                                        /* Clear data pins                    */
        *hLcdIf->pbIfObject->DATA_LAT &= ~hLcdIf->pbIfObject->DATA_MASK;
                                        /* Set desired data pins              */
        *hLcdIf->pbIfObject->DATA_LAT |= tempInstruction;
                                        /* Set data pins to outputs           */
        *hLcdIf->pbIfObject->DATA_TRIS &= ~hLcdIf->pbIfObject->DATA_MASK;
                                        /* Set E pin                          */
        *hLcdIf->E_LAT |= hLcdIf->E_BIT;
                                        /* Clear E pin                        */
        *hLcdIf->E_LAT &= ~hLcdIf->E_BIT;
                                        /* Write instruction process finished */

                                        /* Inform caller that write succeeded */
        return LCDIF_SUCCESS;       
    }    
    else
    {
                                        /* Inform caller that bus is busy     */
        return LCDIF_BUSY;    
    }    
}

/*******************************************************************************
* lcdifGetPbBusWidth()
*
* Summary: 
*   Returns the width of the parallel data bus. This is necessary so that the
*   upper layer can correctly issue the "Initialising by Instruction" sequence
*   which is different depending on the data bus width in use
*
* See also:
*   None
*
* Arguments: 
*   hLcdIf          - handle to the open LCD interface
*
* Returns: 
*   - BUS4BITSWIDE      - if the parallel bus is 4 bits wide
*   - BUS8BITSWIDE      - if the parallel bus is 8 bits wide
*
* Callers: 
*   User application
*
* Notes : 
* 1. Caller must have 'created' at least one LCD interface object before
*    calling this function
*******************************************************************************/
unsigned char lcdifGetPbBusWidth(HLCDIF const hLcdIf)
{
    if (hLcdIf->lcdIfFlags && LCDIF_PBWIDTH4BITS)
    {
        return BUS4BITSWIDE;
    }
    else
    {
        return BUS8BITSWIDE;
    }
}


/*******************************************************************************
*
*                               LCDIF MODULE END
*
*******************************************************************************/
