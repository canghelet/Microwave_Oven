/*
 * File:   main.c
 * Author: Crys
 *
 * Created on September 26, 2023, 8:57 PM
 */


#include "main.h"
#pragma config WDTE = OFF// Watchdog timer disabled


unsigned char sec = 0, min = 0, flag = 0; //0
int operation_flag = POWER_ON_SCREEN; // power on screen
static void init_config(void) {
    // Initialization of the CLDC module
    init_clcd();
    // Initialization of MKP module
    init_matrix_keypad();
    //RC2 pin as a output
    FAN_DDR = 0;
    FAN = OFF; // turn off the FAN
    
    BUZZER_DDR = 0;//RC1 pin as a output
    BUZZER = OFF;
    
    // Initialization of the Timer2 module
    init_timer2();
    
    PEIE = 1;
    GIE = 1;
}

void main(void) {
    init_config(); // Calling initializing function
    // Variable declaration
    unsigned char key;
    int reset_flag;
    while (1) {
        key = read_matrix_keypad(STATE);//1
        if(operation_flag == MENU_DISPLAY_SCREEN)
        {
            
            // SW1 is pressed
            if(key == 1)
            {
                operation_flag = MICRO_MODE;
                reset_flag = MODE_RESET;
                clear_screen();
                clcd_print(" power = 900W   ", LINE2(0));
                __delay_ms(3000);//3 sec
                clear_screen();
            }
        
            else if(key == 2) // Grill mode
            {
                operation_flag = GRILL_MODE;
                reset_flag = MODE_RESET;
                clear_screen();
            }
        
            else if(key == 3) // Convection mode
            {
                operation_flag = CONVECTION_MODE;
                reset_flag = MODE_RESET;
                clear_screen();
            }
        
            else if(key == 4) // Start mode
            {
                sec = 30;
                min = 0;
                FAN = ON; // Turning ON the FAN
                TMR2ON = ON; // Turning ON the Timer2
                clear_screen();
                operation_flag = TIME_DISPLAY;
            }
        
        }
        
        else if(operation_flag == TIME_DISPLAY)
        {
            if(key == 4)// start
            {
                // sec = 42 + 30, min = 11, 11:72
                sec = sec + 30; // 72
                if(sec > 59)
                {
                    min++; // 12
                    sec = sec - 60; // 76 - 60 // 12
                }
                // sec > 59, sec == 60 -> min 1
            }
            
            else if(key == 5)//Pause
            {
                operation_flag = PAUSE;
                
            }
            
            else if(key == 6)// Stop
            {
                
                operation_flag = STOP;
                clear_screen();
            }
            
            
        }
        
        
        else if(operation_flag == PAUSE)
        {
         
            if(key == 4)//resume
            {
                FAN = ON;
                TMR2ON = ON;
                operation_flag = TIME_DISPLAY;
            }
           
        }
        
        
        switch (operation_flag)
        {
            case POWER_ON_SCREEN:
                power_on_screen();
                operation_flag = MENU_DISPLAY_SCREEN; // menu display screen
                clear_screen();
                break;
            case MENU_DISPLAY_SCREEN:
                menu_display_screen();
                break;
            case GRILL_MODE:
                set_time(key, reset_flag);
                break;
            case MICRO_MODE:
                set_time(key, reset_flag);
                break;
            case CONVECTION_MODE:
                if(flag == 0)
                {
                    set_temp(key, reset_flag);
                    if(flag == 1)// # key
                    {
                        clear_screen();
                        reset_flag = MODE_RESET;
                        continue;
                    }
                }
                else if(flag == 1)
                {
                    set_time(key, reset_flag);
                }
                break;     
            case TIME_DISPLAY:
                time_display_screen();
                break;  
            case PAUSE:
                FAN = OFF;
                TMR2ON = OFF;
                break;
            case STOP:
                FAN = OFF;
                TMR2ON = OFF;
                operation_flag = MENU_DISPLAY_SCREEN;
                break;        
        }
        
        
            reset_flag = RESET_NOTHING;
        
        
        
    }
}

void time_display_screen(void)
{
    // LIne 1 display
    clcd_print(" TIME =  ", LINE1(0));
    //print min and sec
    clcd_putch(min/10 + '0', LINE1(9));
    clcd_putch(min%10 + '0', LINE1(10));
    clcd_putch(':', LINE1(11));
    
    //SEC
    clcd_putch(sec/10 + '0', LINE1(12));
    clcd_putch(sec%10 + '0', LINE1(13));
    
    //print options
    clcd_print(" 4.Start/resume" , LINE2(0));
    clcd_print(" 5.Pause" , LINE3(0));
    clcd_print(" 6.Stop" , LINE4(0));
    
    if(sec == 0 && min == 0)
    {
        
        clear_screen();
        clcd_print(" Time Up !!", LINE2(0));
        BUZZER = ON;
        __delay_ms(3000);//3 sec
        clear_screen();
        BUZZER = OFF;
        FAN = OFF;// Turn OFF the FAN
        /* Switching OFF the Timer2*/
        TMR2ON = OFF;   
        operation_flag = MENU_DISPLAY_SCREEN;
    }
    
}


void clear_screen(void)
{
    clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
    __delay_us(500);
}

void power_on_screen(void)
{
unsigned char i;
// LINE 1 -> Printing BAR on LINE1
for(i=0; i<16; i++)
{
    //clcd_write (0xFF, DATA_MODE);// 0b1111 1111
    clcd_putch (BAR, LINE1(i)); //i = 0 1 2 3 ... 15
    __delay_ms(100);
}
   
// Printing Power on message on LINE2 and LINE3
clcd_print("  Powering ON   ", LINE2(0));
clcd_print(" Microwave Oven ", LINE3(0));

// Printing BAR on LINE4
for(i=0; i<16; i++)
{
    //clcd_write (0xFF, DATA_MODE);// 0b1111 1111
    clcd_putch (BAR, LINE4(i)); //i = 0 1 2 3 ... 15
    __delay_ms(100);   
}
__delay_ms(3000); // 3 sec
}


void menu_display_screen(void)
{
    clcd_print ("1.Micro", LINE1(0)); // LINE1
    clcd_print ("2.Grill", LINE2(0)); // LINE2
    clcd_print ("3.Convection", LINE3(0)); // LINE3
    clcd_print ("4.Start", LINE4(0)); // LINE4
}


void set_temp(unsigned char key, int reset_flag)
{
    static unsigned char key_count, blink, temp;
    static int wait;
    if(reset_flag == MODE_RESET)
    {
        
        key_count = 0;
        wait = 0;
        blink = 0;
        temp = 0;
        flag = 0;
        key = ALL_RELEASED;
        clcd_print(" SET TEMP(oC) ", LINE1(0));
        clcd_print(" TEMP = ", LINE2(0));
        //sec -> 0 to 59
        clcd_print("*:CLEAR  #:ENTER", LINE4(0));
        
    }
    
    // Reading the temp
    if((key!='*') && (key != '#') && (key!= ALL_RELEASED))
    {
        // key = 1 5 4 7
        key_count++;//1 2 3
        // 120
        if(key_count <=3)// reading number of temp
        {
            temp = temp*10 + key;// temp = 120    
        }
        
    }
    
    else if(key == '*')// sec or min
    {
        temp = 0;
        key_count = 0;
    }
    
    else if(key == '#')// enter key
    {
        clear_screen();
       // start Pre-heating
        clcd_print("  Pre-Heating ", LINE1(0));
        clcd_print("Time Rem.= ", LINE3(0));
        TMR2ON = 1;// Switching ON the timer
        sec = 18;//0;
        while(sec != 0)// sec == 0, after 18 sec
        {
            //clcd_putch((sec/100) + '0', LINE3(11));// 8 - 123/100 -> 1
            clcd_putch((sec/10)%10 + '0', LINE3(12));// 9 - 123/10 -> 12%10 -> 2
            clcd_putch((sec%10) + '0', LINE3(13)); // 10 - 3 
        }
        
        // sec = 0
        if(sec == 0)
        {
            flag = 1;
            TMR2ON = 0;
            // Set time screen exactly like Grill mode
            //operation_flag = GRILL_MODE;   
        }
    }
    
    if(wait++ == 15)//0 1....1
    {
        wait = 0;
        blink =! blink;//1 0
        // Printing temp on set temp screen
        // temp = 123
        clcd_putch((temp/100) + '0', LINE2(8));// 8 - 123/100 -> 1
        clcd_putch((temp/10)%10 + '0', LINE2(9));// 9 - 123/10 -> 12%10 -> 2
        clcd_putch((temp%10) + '0', LINE2(10)); // 10 - 3       
    }
    if(blink)
    { 
        
        clcd_print("   ", LINE2(8));
                          
    }
    
}


void set_time(unsigned char key, int reset_flag)
{
    static unsigned char key_count, blink_pos, blink;
    static int wait;
    if(reset_flag == MODE_RESET)
    {
        key_count = 0;
        sec = 0;
        min = 0;
        blink_pos = 0;// sec
        wait = 0;
        blink = 0;
        key = ALL_RELEASED;
        clcd_print("SET TIME (MM:SS)", LINE1(0));
        clcd_print("TIME- ", LINE2(0));
        //sec -> 0 to 59
        clcd_print("*:CLEAR  #:ENTER", LINE4(0));
        
    }
    
    if((key!='*') && (key != '#') && (key!= ALL_RELEASED))
    {
        // key = 1 5 4 7
        key_count++;//1 2 3
        if(key_count <=2)// reading number of sec
        {
            sec = sec*10 + key;//sec = 15
            blink_pos = 0;
        }
        else if(key_count >2 && key_count  <=4)
        {
            min = min*10 + key;
            blink_pos = 1;
        }
    }
    else if(key == '*')// sec or min
    {
        if(blink_pos == 0)// To clear sec
        {
            sec = 0;
            key_count = 0;
        }
        else if(blink_pos == 1)
        {
            min = 0;
            key_count = 2;
        }
    }
    
    else if(key == '#')// enter key
    {
        clear_screen();
        operation_flag = TIME_DISPLAY;
        FAN = ON; //Turn ON the FAN
        /* Switching on the TIMER2*/
        TMR2ON = ON;
    }
    
    
    if(wait++ == 15)//0 1....1
    {
        wait = 0;
        blink =! blink;//1 0
        // Printing sec and min on the set time screen
        //MIN
        clcd_putch(min/10 + '0', LINE2(6));
        clcd_putch(min%10 + '0', LINE2(7));
        clcd_putch(':', LINE2(8));
    
        //SEC
        clcd_putch(sec/10 + '0', LINE2(9));
        clcd_putch(sec%10 + '0', LINE2(10));
    }
    if(blink)
    { 
        switch(blink_pos)
        {
            case 0://sec
                 clcd_print("  ", LINE2(9));
                break;
            case 1:// min
                 clcd_print("  ", LINE2(6));
                break;               
        }
       
    }
       
}