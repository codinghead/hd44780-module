                                    /* Setup config bits for PICDEM 2 Plus    */
                                    /* using PIC18F4520                       */
    #pragma config OSC = INTIO67    /* Internal 8MHz Oscillator               */
    #pragma config FCMEN = OFF      /* Fail-safe clock monitor off            */
    #pragma config IESO = OFF       /* Oscillator Switchover mode disabled    */
    #pragma config PWRT = OFF       /* Power-up Timer off                     */
    #pragma config BOREN = OFF      /* Brown-out Reset disabled in hardware   */
                                    /* and software                           */
    #pragma config WDT = OFF        /* WDT disabled (control is placed on the */
                                    /* SWDTEN bit)                            */
    #pragma config MCLRE = ON       /* MCLR pin enabled; RE3 input pin        */
                                    /* disabled                               */
    #pragma config LPT1OSC = ON     /* Timer1 configured for low-power        */
                                    /* operation                              */
    #pragma config PBADEN = OFF     /* PORTB<4:0> pins are configured as      */
                                    /* digital I/O on Reset                   */
    #pragma config CCP2MX = PORTC   /* CCP2 input/output is multiplexed with  */
                                    /* RC1                                    */
    #pragma config STVREN = ON      /* Stack full/underflow will cause Reset  */
    #pragma config LVP = OFF        /* Single-Supply ICSP disabled            */
    #pragma config XINST = OFF      /* Instruction set extension and Indexed  */
                                    /* Addressing mode disabled (Legacy mode) */
    #pragma config CP0 = OFF        /* Code protect off                       */
    #pragma config CP1 = OFF
    #pragma config CP2 = OFF
    #pragma config CP3 = OFF
    #pragma config CPB = OFF
    #pragma config CPD = OFF
    #pragma config WRT0 = OFF       /* Write protect off                      */
    #pragma config WRT1 = OFF
    #pragma config WRT2 = OFF
    #pragma config WRT3 = OFF
    #pragma config WRTB = OFF
    #pragma config WRTC = OFF
    #pragma config WRTD = OFF
    #pragma config EBTR0 = OFF      /* Table read protect off                 */
    #pragma config EBTR1 = OFF
    #pragma config EBTR2 = OFF
    #pragma config EBTR3 = OFF
    #pragma config EBTRB = OFF
