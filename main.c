/*
 * File:   main.c
 * Author: parth
 *
 * Created on 13 December, 2021, 7:22 PM
 */


#include <xc.h>
#include "clcd.h"
#include "matrix_keypad.h"
#include "main.h"
#include  "timers.h"


#pragma config WDTE = OFF   // Watchdog Timer Enable bit (WDT enabled)
char sec = 0,min =0, temp = 0;
unsigned char operation_mode , reset_flag , convec_mode_op;
unsigned char flag;

static void init_config(void) {
    
    //Write your initialization code here
    init_clcd();             // initialization of CLCD module
    init_matrix_keypad();    // initialization of matrix keypad module
    init_timer2();           // initialization of Timer 2 module
    
    PEIE = 1;
    GIE  = 1;
   
    
    FAN_DDR = 0;  // FAN pin set as output
    FAN = 0;      // Initially FAN is kept off
    
    BUZZER_DDR = OFF;
    BUZZER = OFF;
    
    DOOR_DDR = 1;  //input
}

void main(void) 
{
    unsigned char key;
    init_config();
    power_on_screen();
    clear_clcd();
    operation_mode = COOKING_MODE_DISPLAY;

    while (1) 
    {   
          //Write application code here
        key = read_matrix_keypad(STATE); // 0,1,2,....,9,*,#, ALL_RELEASED
        if (operation_mode == MICRO_MODE   ||  operation_mode == GRILL_MODE  ||  operation_mode == CONVECTION_MODE)
        {
            ;
        }
        
        else if(  key == 1  &&  (operation_mode  !=  TIME_DISPLAY) ) //  key 1 is pressed
        {
            operation_mode = MICRO_MODE;
            reset_flag = MODE_RESET;
            clear_clcd();
            clcd_print(" Power = 900W",LINE2(0));
            __delay_ms(3000);
            clear_clcd();
        }
        
        else if ( key == 2 &&  (operation_mode  !=  TIME_DISPLAY) )
        {
            operation_mode = GRILL_MODE;
            reset_flag     = MODE_RESET;
            clear_clcd();
        }
        
        else if (key == 3)  // Convection mode
        {
            operation_mode = CONVECTION_MODE ;
            convec_mode_op = SET_TEMP;
            reset_flag     = RESET_TEMP ;
            clear_clcd();
        }
        
        else if (key == 4)
        {
            
            if(operation_mode == COOKING_MODE_DISPLAY)
            {
                min = 0;
                sec = 30;
                TMR2ON = ON;
                FAN = ON;
                clear_clcd();
                operation_mode = TIME_DISPLAY;
                
            }
            
            else if ( operation_mode == TIME_DISPLAY)
            {
                //Increment time by 30 sec
                sec = sec + 30 ;
                if(sec >= 60)
                {
                    min ++;
                    sec = sec - 60 ;
                }
            }
            
            else if (operation_mode == PAUSE)  //if task is paused then resume it
            {
                TMR2ON = ON;
                FAN    = ON;
                operation_mode = START;
            }
                
        }
        
        else if (key == 5)      //pause the current task
        {
            operation_mode = PAUSE;
            
        }
        
        else if (key == 6)     //   stop the current task
        {
            operation_mode = STOP;
            clear_clcd();
        }
        
        switch(operation_mode)
        {  
            case COOKING_MODE_DISPLAY :  cooking_mode_display();
                                         break;
                 
            case  MICRO_MODE          :  set_time(key);
                                         break;
                                         
            case  GRILL_MODE          :  set_time(key);
                                         break;
                                         
            case  CONVECTION_MODE     :  if( convec_mode_op  == SET_TEMP) // To call set temp function
                                         {
                                            set_temp(key);
                                            if ( convec_mode_op == SET_TIME)
                                            { 
                                                continue;
                                            }
                                            
                                         }
            
                                         else if (convec_mode_op  == SET_TIME)
                                         {
                                             set_time(key);
                                         }
                                         break;
                                         
            case TIME_DISPLAY         :  time_display_screen();
                                         break;
                                         
            case  STOP                :  TMR2ON = OFF;
                                         FAN    = OFF;
                                         operation_mode = COOKING_MODE_DISPLAY;
                                         break;
                                         
            case  PAUSE               :  TMR2ON = OFF;
                                         FAN    = OFF;
                                         break;
                                         
            case  START               :  time_display_screen();
                                         break;
                
                
                 
                 
        }
        
        reset_flag = RESET_NOTHING;
    }
    return;
}
void clear_clcd(void)
{
    clcd_write(CLEAR_DISP_SCREEN ,INST_MODE);
    __delay_us(100);
}
void power_on_screen (void)
{
    for (int i =0; i <16;i++)
    {
       clcd_putch(BLOCK ,LINE1(i));
    }
    clcd_print("  Powering ON   ",LINE2(0));
    clcd_print(" Microwave Oven ",LINE3(0));
    for (int i =0; i<16; i++)
    {  
        clcd_putch(BLOCK ,LINE4(i));
    }
    __delay_ms(3000);
}

void cooking_mode_display (void)
{
     clcd_print("1.Micro",LINE1(0));
     clcd_print("2.Grill",LINE2(0));
     clcd_print("3.Convection",LINE3(0));
     clcd_print("4.Start",LINE4(0));
    
}


void set_time (unsigned char key)
{   
    static int wait , blink, key_count, blink_pos;
    if (reset_flag == MODE_RESET)
    {
          wait = 15;
          blink = 0;
          key_count = 0;
          sec = 0;
          min = 0;
          blink_pos = 0;
          key = ALL_RELEASED;
          clcd_print("SET TIME <MM:SS> ", LINE1(0));
          clcd_print("TIME- ",LINE2(0));
          clcd_putch(':', LINE2(8));  // ':'
          clcd_print("*:CLEAR  #:ENTER",LINE4(0));
    }
    
    //Based on matrix keypad key press we will read sec and min
    //key allowed : 0,1,2....9
    //keys not allowed : '*' , '#' , ALL_RELEASED
    
    if(key!= '*'  && key!= '#' &&  key!= ALL_RELEASED)
    {
        key_count++;  // 1 2
        if(key_count <= 2)
        {
            sec = sec*10 + key;   // 5
            blink_pos = 0;
        }
        else if(key_count > 2  && key_count <= 4)
        {
            min = min*10 + key;
            blink_pos = 1;
        }
    }
    
    if(key == '*')
    {
         //To clear sec field
         if(key_count <= 2)
         {
             sec = 0;
             key_count = 0;
         }
         else if (key_count >2  &&  key_count <= 4)
         {
             min =0;
             key_count = 2;
         }
    }
    
    if(key == '#')  //Enter key
    {
        clear_clcd();
        operation_mode = TIME_DISPLAY;
        FAN = 1;   // Turn on the fan
        
        TMR2ON = 1  ;   // Switching on Timer 2
    }
    
    
    if(wait ++ == 15)
    {
        wait = 0;
        blink = !blink;
        clcd_putch(sec/10 + '0',LINE2(9)); // '1'
        clcd_putch(sec%10 + '0',LINE2(10)); //'2'
        clcd_putch(min/10 + '0', LINE2(6)); //'1'
        clcd_putch(min%10 + '0', LINE2(7)); //'2'
    }
    
    if(blink)
    {  
        switch(blink_pos)
        {
            case 0 :   clcd_print("  ", LINE2(9)); //To blink sec field
                       break;
                       
            case 1 :   clcd_print("  ", LINE2(6)); //To blink min field
                       break;
        
       
        
        }
    
    }
    
     
    
  
}

void door_status_check(void)
{
     // if RB3 key is pressed --> door open
    if(DOOR == OPEN)  // RB3 == 0
    {
         //stop/pause the oven
        TMR2ON = OFF;
        FAN   = OFF;
        BUZZER = ON;
        clear_clcd();
        clcd_print("DOOR STATUS:OPEN",LINE2(0));
        clcd_print("PLEASE CLOSE!!", LINE3(0));
        
        //wait still the door is not  closed
        while(DOOR == OPEN) //RB3 == 1
        {
            ;
        }
        BUZZER = OFF;
        TMR2ON = ON;
        FAN   = ON;
        clear_clcd();
    }
}

void time_display_screen (void)
{
    
     door_status_check();
     
     
     clcd_print(" TIME =  ", LINE1(0));
     
     // min 9th 10
     // min = 50
     
     clcd_putch(min/10 + '0', LINE1(9)); //'1'
     clcd_putch(min%10 + '0', LINE1(10)); //'2'
     
     //11th -> :
     
     clcd_putch(':', LINE1(11));
     
     //sec -> 12th 13th
     
     clcd_putch(sec/10 + '0',LINE1(12)); // '1'
     clcd_putch(sec%10 + '0',LINE1(13));
     
     //print options
     
     clcd_print(" 4.Start/Resume", LINE2(0));
     clcd_print(" 5.Pause", LINE3(0));
     clcd_print(" 6.Stop", LINE4(0));
     
     if((sec == 0)  &&  (min == 0))
     {
         clear_clcd();
         clcd_print("TIME UP!!", LINE2(0));
         TMR2ON = OFF;  //Timer 2 Off
         FAN = OFF;
         BUZZER = ON;
         __delay_ms(3000);  //3sec delay
         BUZZER = OFF;
         clear_clcd();
         operation_mode = COOKING_MODE_DISPLAY ;
     }
     
     
}

void set_temp (unsigned char key)
{
    static int  wait, blink , key_count ;
    static char temp_arr[4];
    if  (reset_flag   ==  RESET_TEMP)
    {
        key = ALL_RELEASED;
        temp = 0;
        wait = 15;
        blink = 0;
        key_count = 0;
        clcd_print(" SET TEMP ( C)", LINE1(0));
        clcd_putch(DEGREE, LINE1(11));
        clcd_print(" TEMP =", LINE2(0));
        clcd_print("*:CLEAR  #:ENTER", LINE4(0));
        
    }
    
    //To read temp : press the key
    if(key!= '*'  && key!= '#' &&  key!= ALL_RELEASED)
    {
        key_count++;  // 1 2 3
        if(key_count <= 3)
        {
            temp  = temp*10 + key;   // key = 2,3,1
            
        }
    }    
        
    else if (key == '*')  //Clear temp
    {
            temp = 0;
            key_count = 0;
            
    }
        
    else if (key == '#')  //enter key is pressed
    {
            clear_clcd();
            clcd_print("  Pre-Heating   ", LINE1(0));
            clcd_print("Time Rem.= ", LINE3(0));
            
            //set pre-heating time for 3 min = 180 sec
            
            sec = 180;
            TMR2ON = ON;
            while(sec != 0) 
            {
                clcd_putch(sec/100 + '0',LINE3(10)); 
                clcd_putch((sec/10)%10 + '0',LINE3(11));
                clcd_putch(sec%10 + '0',LINE3(12));
            }
            
            //After pre-heating
            TMR2ON = OFF;
            clear_clcd();
            convec_mode_op = SET_TIME;
            reset_flag  = MODE_RESET;
    }
        
        
    
    
    
    //To display the temperature
        // say temp = 231
        // temp_arr[0] = '2' , temp_arr[1] = '3' , temp_arr[2] = '1'
        temp_arr[0] =  (temp / 100) + '0';  // '2'
        temp_arr[1] =  (temp / 10)  % 10 + '0';  //  23%10  -> '3'
        temp_arr[2] =  (temp % 10) + '0';  // '1'
        temp_arr[3] =  '\0';
        //temp_arr ->  "231"
        
         if(wait ++ == 15)
        {
              wait = 0;
              blink = !blink;
              clcd_print(temp_arr ,LINE2(8));
              
        }
    
        if(blink)
        {  
        
            clcd_print("   " ,LINE2(8));
        
        }
       
    
    
    
       
    
}