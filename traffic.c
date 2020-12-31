#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <math.h>
#include <p18f4620.h>

#pragma config OSC      =   INTIO67
#pragma config BOREN    =   OFF
#pragma config WDT      =   OFF
#pragma config LVP      =   OFF
#pragma config CCP2MX   =   PORTBE


#define TFT_RST     PORTBbits.RB4                                               // TFT Reset
#define TFT_CS      PORTBbits.RB2                                               //TFT Chip Select
#define TFT_DC      PORTBbits.RB5                                               // TFT D/C
#define NS_PED_SW   PORTAbits.RA4                                               //  North/South PED
#define NS_LT_SW    PORTAbits.RA5                                               // North/South Left Turn
#define EW_PED_SW   PORTBbits.RB0                                               //  for East/West PED
#define EW_LT_SW    PORTBbits.RB1                                               //  East/West Left Turn
#define SPKR        PORTBbits.RB3                                               //  Speaker
#define SEC_LED     PORTEbits.RE0                                               //  Second LED

void Initialize_Screen(void);

void update_Count(char , char );                                                 // initilization of varibales 
void update_color(char, char);
void update_RGB(char, char);
void update_misc(void);
void wait_one_second(void);
void wait_one_second_beep(char);

void wait_N_seconds(char, char);
void wait_N_seconds_PED(char, char);
void gen_1khz_sound(void);
void gen_2khz_sound(void);
void turn_off_sound(void);
void delay_ms(int);
unsigned int get_full_ADC(void);
void init_ADC(void);

#define _XTAL_FREQ      8000000                                                 // it will be set 8 Mhz
#define TMR_CLOCK       _XTAL_FREQ/4                                            // the Timer Clock 2 Mhz
#define COUNT_PER_MS    TMR_CLOCK/1000                                          // count will be ms = 2000
#define COUNT_SCALED    COUNT_PER_MS/32                                        
   
#define Circle_Size     7                                                       // Size of the circle
#define Circle_Offset   15                                                      // Circle will be here 
#define TS_1            1                                                       // Normal Text
#define TS_2            2                                                       // PED Text
#define Count_Offset    10                                                      //  Count

#define XTXT            30                                                      // X location of Title Text
#define XRED            40                                                      // X location of Red Circle
#define XYEL            60                                                      // X location of Yellow Circle
#define XGRN            80                                                      // X location of Green Circle
#define XCNT            100                                                     // X location of Sec Count

#define NS              0                                                       // Number definition of North/South
#define NSLT            1                                                       // Number definition of North/South Left Turn
#define EW              2                                                       // Number definition of East/West
#define EWLT            3                                                       // Number definition of East/West Left Turn
 
#define Color_Off       0                                                       // Number definition of Off Color
#define Color_Red       1                                                       // Number definition of Red Color
#define Color_Green     2                                                       // Number definition of Green Color
#define Color_Yellow    3                                                       // Number definition of Yellow Color

#define NS_Txt_Y        20
#define NS_Cir_Y        NS_Txt_Y + Circle_Offset
#define NS_Count_Y      NS_Txt_Y + Count_Offset
#define NS_Color        ST7735_BLUE

#define NSLT_Txt_Y      50
#define NSLT_Cir_Y      NSLT_Txt_Y + Circle_Offset
#define NSLT_Count_Y    NSLT_Txt_Y + Count_Offset
#define NSLT_Color      ST7735_MAGENTA

#define EW_Txt_Y        80
#define EW_Cir_Y        EW_Txt_Y + Circle_Offset
#define EW_Count_Y      EW_Txt_Y + Count_Offset
#define EW_Color        ST7735_CYAN

#define EWLT_Txt_Y      110
#define EWLT_Cir_Y      EWLT_Txt_Y + Circle_Offset
#define EWLT_Count_Y    EWLT_Txt_Y + Count_Offset
#define EWLT_Color      ST7735_WHITE

#define PED_NS_Count_Y  30
#define PED_EW_Count_Y  90

#define Switch_Txt_Y    140

#define PED_Count_NS            2
#define PED_Count_EW            2

#define RED PORTDbits.RD0                                                       // Set RED as name for value of RD0
#define GREEN PORTDbits.RD1                                                     // Set GREEN as name for value of RD1      
#define RED1 PORTDbits.RD2                                                      // Set RED1 as name for value of RD2      
#define GREEN1 PORTDbits.RD3                                                    // Set GREEN1 as name for value of RD3      
#define RED2 PORTDbits.RD4                                                      // Set RED2 as name for value of RD4      
#define GREEN2 PORTDbits.RD5                                                    // Set GREEN2 as name for value of RD5      
#define RED3 PORTDbits.RD6                                                      // Set RED3 as name for value of RD6      
#define GREEN3 PORTDbits.RD7                                                    // Set GREEN3 as name for value of RD7      

#include "ST7735_TFT.inc"

char buffer[31];                                                                // general buffer for display purpose
char *nbr;                                                                      // general pointer used for buffer
char *txt;

char NS_Count[]     = "00";                                                     // text storage for NS Count
char NSLT_Count[]   = "00";                                                     // text storage for NS Left Turn Count
char EW_Count[]     = "00";                                                     // text storage for EW Count
char EWLT_Count[]   = "00";                                                     // text storage for EW Left Turn Count

char PED_NS_Count[] = "00";                                                     // text storage for NS Pedestrian Count
char PED_EW_Count[] = "00";                                                     // text storage for EW Pedestrian Count

char SW_NSPED_Txt[] = "0";                                                      // text storage for NS Pedestrian Switch
char SW_NSLT_Txt[]  = "0";                                                      // text storage for NS Left Turn Switch
char SW_EWPED_Txt[] = "0";                                                      // text storage for EW Pedestrian Switch
char SW_EWLT_Txt[]  = "0";                                                      // text storage for EW Left Turn Switch
char Mode_Txt[]     = "D";                                                      // text storage for Mode Light Sensor

char dir;
char Count;                                                                     // RAM variable for Second Count
char PED_Count;                                                                 // RAM variable for Second Pedestrian Count

char SW_NSPED;                                                                  // RAM variable for NS Pedestrian Switch
char SW_NSLT;                                                                   // RAM variable for NS Left Turn Switch
char SW_EWPED;                                                                  // RAM variable for EW Pedestrian Switch
char SW_EWLT;                                                                   // RAM variable for EW Left Turn Switch
char SW_MODE;                                                                   // RAM variable for Mode Light Sensor

char i;

void putch (char c)
{  
    while (!TRMT);      
    TXREG = c;
}

void init_UART()
{
    OpenUSART (USART_TX_INT_OFF & USART_RX_INT_OFF & USART_ASYNCH_MODE & USART_EIGHT_BIT & USART_CONT_RX & USART_BRGH_HIGH, 25);
    OSCCON = 0x60;
}

void init_ADC()
{
    ADCON0 = 0x01;                                                              // Select channel 1
    ADCON1 = 0x1B;                                                              // Set AN0-3 as A/D converter
    ADCON2 = 0xA2;                                                              // Right justified, 8TAD, Fosc/32
}

void main()
{
  init_UART();                                                                  // initialize UART (for debugging purpose)
  init_ADC();                                                                   // initialize ADC port
  OSCCON =0x70;                                                                 // set the processor to run at 8 Mhz
  nRBPU = 0;                                                                    // Enable PORTB internal pull up resistor
  TRISA = 0xFF;                                                                 // Set port a as input
  TRISB = 0x03;                                                                 // Set port b RB0 and RB1 as input rest as output
  TRISC = 0x10;                                                                 // Set port c RC4 as input rest as output
  TRISD = 0x00;                                                                 // Set port d as all outputs
  TRISE = 0x00;                                                                 // Set port e as all outputs
  PORTD = 0x00;                                                                
 
  Initialize_Screen();                                                          // Initialize the TFT screen

  update_color(NS,   Color_Off);                                                // Turn off all the Lights
  update_color(NSLT, Color_Off);  
  update_color(EW,   Color_Off);    
  update_color(EWLT, Color_Off);  
  update_misc();                                                                // Take in switch inputs from the dipswitch and display the corresponding switch value on the display
  update_color(NS, Color_Red);                                                  // Change all lights to red
  update_color(NSLT, Color_Red);
  update_color(EW, Color_Red);
  update_color(EWLT, Color_Red);
  while(TRUE)
  {  
      if(SW_MODE == 0)
          day_mode();                                             // If SW_MODE is equal to 1 then run night mode
      else
          night_mode();                                                          // If SW_MODE is equal to 0 then run day mode
  }
}

void Initialize_Screen()                                                        // Create display on LCD screen
{
  LCD_Reset();
  TFT_GreenTab_Initialize();
  fillScreen(ST7735_BLACK);
 
  /* TOP HEADER FIELD */
  txt = buffer;
  strcpy(txt, "ECE3301L Fall 2019");  
  drawtext(2, 2, txt, ST7735_WHITE, ST7735_BLACK, TS_1);
 
  /* MODE FIELD */
  strcpy(txt, "Mode:");
  drawtext(2, 10, txt, ST7735_WHITE, ST7735_BLACK, TS_1);
  drawtext(35,10, Mode_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);

  /* SECOND UPDATE FIELD */
  strcpy(txt, "*");
  drawtext(120, 10, txt, ST7735_WHITE, ST7735_BLACK, TS_1);
     
  /* NORTH/SOUTH UPDATE FIELD */
  strcpy(txt, "NORTH/SOUTH");
  drawtext  (XTXT, NS_Txt_Y  , txt, NS_Color, ST7735_BLACK, TS_1);
  drawRect  (XTXT, NS_Cir_Y-8, 60, 18, NS_Color);
  drawCircle(XRED, NS_Cir_Y  , Circle_Size, ST7735_RED);
  drawCircle(XYEL, NS_Cir_Y  , Circle_Size, ST7735_YELLOW);
  fillCircle(XGRN, NS_Cir_Y  , Circle_Size, ST7735_GREEN);
  drawtext  (XCNT, NS_Count_Y, NS_Count, NS_Color, ST7735_BLACK, TS_2);
   
  /* NORTH/SOUTH LEFT TURN UPDATE FIELD */
  strcpy(txt, "N/S LT");
  drawtext  (XTXT, NSLT_Txt_Y, txt, NSLT_Color, ST7735_BLACK, TS_1);
  drawRect  (XTXT, NSLT_Cir_Y-8, 60, 18, NSLT_Color);
  fillCircle(XRED, NSLT_Cir_Y, Circle_Size, ST7735_RED);
  drawCircle(XYEL, NSLT_Cir_Y, Circle_Size, ST7735_YELLOW);
  drawCircle(XGRN, NSLT_Cir_Y, Circle_Size, ST7735_GREEN);  
  drawtext  (XCNT, NSLT_Count_Y, NSLT_Count, NSLT_Color, ST7735_BLACK, TS_2);
 
  /* EAST/WEST UPDATE FIELD */
  strcpy(txt, "EAST/WEST");
  drawtext  (XTXT, EW_Txt_Y, txt, EW_Color, ST7735_BLACK, TS_1);
  drawRect  (XTXT, EW_Cir_Y-8, 60, 18, EW_Color);
  fillCircle(XRED, EW_Cir_Y, Circle_Size, ST7735_RED);
  drawCircle(XYEL, EW_Cir_Y, Circle_Size, ST7735_YELLOW);
  drawCircle(XGRN, EW_Cir_Y, Circle_Size, ST7735_GREEN);  
  drawtext  (XCNT, EW_Count_Y, EW_Count, EW_Color, ST7735_BLACK, TS_2);

  /* EAST/WEST LEFT TURN UPDATE FIELD */
  strcpy(txt, "E/W LT");
  drawtext  (XTXT, EWLT_Txt_Y, txt, EWLT_Color, ST7735_BLACK, TS_1);
  drawRect  (XTXT, EWLT_Cir_Y-8, 60, 18, EWLT_Color);  
  fillCircle(XRED, EWLT_Cir_Y, Circle_Size, ST7735_RED);
  drawCircle(XYEL, EWLT_Cir_Y, Circle_Size, ST7735_YELLOW);
  drawCircle(XGRN, EWLT_Cir_Y, Circle_Size, ST7735_GREEN);  
  drawtext  (XCNT, EWLT_Count_Y, EWLT_Count, EWLT_Color, ST7735_BLACK, TS_2);

  /* NORTH/SOUTH PEDESTRIAM UPDATE FIELD */
  strcpy(txt, "PNS");  
  drawtext(3, NS_Txt_Y, txt, NS_Color, ST7735_BLACK, TS_1);
  drawtext(2, PED_NS_Count_Y, PED_NS_Count, NS_Color, ST7735_BLACK, TS_2);
 
  /* EAST/WEST PEDESTRIAM UPDATE FIELD */  
  drawtext(2, PED_EW_Count_Y, PED_EW_Count, EW_Color, ST7735_BLACK, TS_2);
  strcpy(txt, "PEW");  
  drawtext(3, EW_Txt_Y, txt, EW_Color, ST7735_BLACK, TS_1);
     
  /* MISCELLANEOUS UPDATE FIELD */  
  strcpy(txt, "NSP NSLT EWP EWLT MD");
  drawtext(1,  Switch_Txt_Y, txt, ST7735_WHITE, ST7735_BLACK, TS_1);
  drawtext(6,  Switch_Txt_Y+9, SW_NSPED_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);
  drawtext(32, Switch_Txt_Y+9, SW_NSLT_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);
  drawtext(58, Switch_Txt_Y+9, SW_EWPED_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);
  drawtext(87, Switch_Txt_Y+9, SW_EWLT_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);  
  drawtext(112,Switch_Txt_Y+9, Mode_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);
}

void update_color(char direction, char Color)                                   // Update colors of the traffic lights when they change
{
char Circle_Y;
   
    update_RGB(direction, Color);
   
    Circle_Y = NS_Cir_Y + direction * 30;    
   
    if (Color == Color_Off)                                                     // the traffic lights are off  
    {                                                                           // filling of the circle
            fillCircle(XRED, Circle_Y, Circle_Size, ST7735_BLACK);              // red to black
            fillCircle(XYEL, Circle_Y, Circle_Size, ST7735_BLACK);              // yellow to black 
            fillCircle(XGRN, Circle_Y, Circle_Size, ST7735_BLACK);              // green to black
            drawCircle(XRED, Circle_Y, Circle_Size, ST7735_RED);                // drawing red to red 
            drawCircle(XYEL, Circle_Y, Circle_Size, ST7735_YELLOW);             // yellow to yellow
            drawCircle(XGRN, Circle_Y, Circle_Size, ST7735_GREEN);              // green to green       
    }    
   
    if (Color == Color_Red)                                                     // if the traffic light s red 
    {                                                                           // filling the circle 
            fillCircle(XRED, Circle_Y, Circle_Size, ST7735_RED);                // red with red
            fillCircle(XYEL, Circle_Y, Circle_Size, ST7735_BLACK);              // yellow/ black
            fillCircle(XGRN, Circle_Y, Circle_Size, ST7735_BLACK);              // green/black
            drawCircle(XRED, Circle_Y, Circle_Size, ST7735_RED);                // drawing  red /  red
            drawCircle(XYEL, Circle_Y, Circle_Size, ST7735_YELLOW);             // yellow/ yellow
            drawCircle(XGRN, Circle_Y, Circle_Size, ST7735_GREEN);              // green/ green        
    }
         
    if (Color == Color_Yellow)                                                  //  if traffic lights is yellow
    {
            fillCircle(XRED, Circle_Y, Circle_Size, ST7735_BLACK);              // red circle to black
            fillCircle(XYEL, Circle_Y, Circle_Size, ST7735_YELLOW);             // yellow/ yellow
            fillCircle(XGRN, Circle_Y, Circle_Size, ST7735_BLACK);              // green/black
            drawCircle(XRED, Circle_Y, Circle_Size, ST7735_RED);                // red/red
            drawCircle(XYEL, Circle_Y, Circle_Size, ST7735_YELLOW);             // yellow/ yellow
            drawCircle(XGRN, Circle_Y, Circle_Size, ST7735_GREEN);              // green/green        
    }
         
    if (Color == Color_Green)                                                   // traffic light = green
    {
            fillCircle(XRED, Circle_Y, Circle_Size, ST7735_BLACK);              // red with black
            fillCircle(XYEL, Circle_Y, Circle_Size, ST7735_BLACK);              // yellow with black
            fillCircle(XGRN, Circle_Y, Circle_Size, ST7735_GREEN);              // green/ green
            drawCircle(XRED, Circle_Y, Circle_Size, ST7735_RED);                // drawing red to red
            drawCircle(XYEL, Circle_Y, Circle_Size, ST7735_YELLOW);             // yellow= yellow
            drawCircle(XGRN, Circle_Y, Circle_Size, ST7735_GREEN);              // and finally green= green        
    }
         
}

void update_RGB(char direction, char Color)                                     // Rgb light update based on conditions 
{
    if((direction==NS) && (Color==Color_Green))                                 // do this if direction is NS and Color of light is green
                                                                                // 0 off 1 on 
    {
        RED = 0;                                                                
        GREEN = 1;                                                              
    }
    else if ((direction==NS) && (Color==Color_Yellow))                          // direction is NS and Color of light is yellow
    {
        RED = 1;                                                                // produce yellow light turning on  green and red 
        GREEN = 1;                                                              
    }
    else if ((direction==NS) && (Color==Color_Red))                             //  if direction is NS and Color of light is red
    {
        RED = 1;                                                                // red is on other are off
        GREEN = 0;                                                              
    }
    else if ((direction==NS) && (Color==Color_Off))                             // if direction is NS and Color of light is off
    {
        RED = 0;                                                                // all the light are turn off
        GREEN = 0;                                                             
    }
    else if((direction==NSLT) && (Color==Color_Green))                          // if direction is NSLT and Color of light is green
    {
        RED1 = 0;                                                               // only green is on 
        GREEN1 = 1;                                                             
    }
    else if ((direction==NSLT) && (Color==Color_Yellow))                        //  if direction is NSLT and Color of light is yellow
    {
        RED1 = 1;                                                               // green and red on to produce yellow
        GREEN1 = 1;                                                             
    }
    else if ((direction==NSLT) && (Color==Color_Red))                           // if direction is NSLT and Color of light is red
    {
        RED1 = 1;                                                               // red only on 
        GREEN1 = 0;                                                            
    }
    else if ((direction==NSLT) && (Color==Color_Off))                           // if direction is NSLT and Color of light is off
    {
        RED1 = 0;                                                               // all colors are off
        GREEN1 = 0;                                                            
    }
    else if((direction==EW) && (Color==Color_Green))                            //  if direction is EW and Color of light is green
    {
        RED2 = 0;                                                               // green only one
        GREEN2 = 1;                                                            
    }
    else if ((direction==EW) && (Color==Color_Yellow))                          //  if direction is EW and Color of light is yellow
    {
        RED2 = 1;                                                               // yellow light
        GREEN2 = 1;                                                           
    }
    else if ((direction==EW) && (Color==Color_Red))                             // if direction is EW and Color of light is red
    {
        RED2 = 1;                                                               // red only on
        GREEN2 = 0;                                                             
    }
    else if((direction==EW) && (Color==Color_Off))                              // if direction is EW and Color of light is off
    {
        RED2 = 0;                                                               // everything off
        GREEN2 = 0;                                                            
    }
    else if((direction==EWLT) && (Color==Color_Green))                          // if direction is EWLT and Color of light is green
    {
        RED3 = 0;                                                               // Turn green on red off
        GREEN3 = 1;                                                            
    }
    else if ((direction==EWLT) && (Color==Color_Yellow))                        // if direction is EWLT and Color of light is yellow
    {
        RED3 = 1;                                                               // yellow light
        GREEN3 = 1;                                                             
    }
    else if ((direction==EWLT) && (Color==Color_Red))                           // if direction is EWLT and Color of light is red
    {
        RED3 = 1;                                                               // red light produce 
        GREEN3 = 0;                                                             
    }
    else if ((direction==EWLT) && (Color==Color_Off))                           // if direction is EWLT and Color of light is off
    {
        RED3 = 0;                                                               //  all off
        GREEN3 = 0;                                                            
    }
}

void update_Count(char direction, char count)                                   // changing the different directions
{
   switch (direction)
   {
      case NS:                                                                  // update the count for NS if NS its on the right light                
        NS_Count[0] = count/10  + '0';
        NS_Count[1] = count%10  + '0';        
        drawtext(XCNT, NS_Count_Y, NS_Count, NS_Color, ST7735_BLACK, TS_2);
        break;                                                                      // same for the NSLT if NSLT at the  light  
     
      case NSLT:                                                                   
        NSLT_Count[0] = count/10  + '0';
        NSLT_Count[1] = count%10  + '0';
        drawtext(XCNT, NSLT_Count_Y, NSLT_Count,
                NSLT_Color, ST7735_BLACK, TS_2);        
        break;
     
      case EW:                                                                  // update the  count for  the EW if EW if its on the light        
        EW_Count[0] = count/10  + '0';
        EW_Count[1] = count%10  + '0';
        drawtext(XCNT, EW_Count_Y, EW_Count, EW_Color, ST7735_BLACK, TS_2);      
        break;
           
      case EWLT:                                                                // update the count for EWLT             
        EWLT_Count[0] = count/10  + '0';
        EWLT_Count[1] = count%10  + '0';
        drawtext(XCNT, EWLT_Count_Y, EWLT_Count,
                EWLT_Color, ST7735_BLACK, TS_2);    
        break;
       
    }  
}

void update_PED_Count(char direction, char count)                               // doing the samething like above but this time with the pedestrian 
{
   switch (direction)
   {
      case NS:                                                                   // we will update the count everytime the pedestrian is present
        PED_NS_Count[0] = count/10  + '0';
        PED_NS_Count[1] = count%10  + '0';
        drawtext(PED_Count_NS, PED_NS_Count_Y, PED_NS_Count,
                NS_Color, ST7735_BLACK, TS_2);          
        break;
       
      case EW:      
        PED_EW_Count[0] = count/10  + '0';
        PED_EW_Count[1] = count%10  + '0';
        drawtext(PED_Count_EW, PED_EW_Count_Y, PED_EW_Count,
                EW_Color, ST7735_BLACK, TS_2);        
        break;
   }
   
}

void update_misc()                                                              // this will be controlling the the switches and photo resistor voltage 
{
int STEP;                                                                         // also it will display the values on the  LCD
float Volt;
    SW_NSPED = NS_PED_SW;                                                       // switch declared values will be  =  SW var
    SW_NSLT = NS_LT_SW;
    SW_EWPED = EW_PED_SW;
    SW_EWLT = EW_LT_SW;
   
    STEP = get_full_ADC();                                                      // the voltage = whats the return of the get_full_ ADC value                                                      // Set voltage equal to returned get_full_

    Volt = (STEP*4) / 1000;    
    // Set Volt equal Step times 4 plus 1000
    if (Volt < 2.0) SW_MODE = 0;  else SW_MODE = 1;                            // If the volt <  2000, the SW_MODE will be zero  if not then one 
    (int)Volt;
     printf("R=%f\r\n",Volt);
   
    if (SW_NSPED == 0) SW_NSPED_Txt[0] = '0'; else SW_NSPED_Txt[0] = '1';       // this will set the switch value in the  LCD display
    if (SW_NSLT == 0) SW_NSLT_Txt[0] = '0'; else SW_NSLT_Txt[0] = '1';
    if (SW_EWPED == 0) SW_EWPED_Txt[0] = '0'; else SW_EWPED_Txt[0] = '1';
    if (SW_EWLT == 0) SW_EWLT_Txt[0] = '0'; else SW_EWLT_Txt[0] = '1';  
    if (SW_MODE == 0) Mode_Txt[0] = 'D'; else Mode_Txt[0] = 'N';                // the Mode text =  D if (prv) photo resistor voltage is low and N if high
   
    drawtext(6,   Switch_Txt_Y+9, SW_NSPED_Txt,                                 // writing the right switch on the LCD display either 1 or zero
            ST7735_WHITE, ST7735_BLACK, TS_1);
    drawtext(32,  Switch_Txt_Y+9, SW_NSLT_Txt,
            ST7735_WHITE, ST7735_BLACK, TS_1);
    drawtext(58,  Switch_Txt_Y+9, SW_EWPED_Txt,
            ST7735_WHITE, ST7735_BLACK, TS_1);
    drawtext(87,  Switch_Txt_Y+9, SW_EWLT_Txt,
            ST7735_WHITE, ST7735_BLACK, TS_1);
    drawtext(112, Switch_Txt_Y+9, Mode_Txt,                                     
            ST7735_WHITE, ST7735_BLACK, TS_1);
}

void wait_one_second()                                                          //Starting the time and lED
{
    SEC_LED = 1;                                                                //ON the sec_led
    strcpy(txt, "*");                                                           // Put ?*? on the lcd
    drawtext(120, 10, txt, ST7735_WHITE, ST7735_BLACK, TS_1);                  
    delay_ms(500);                                                              //a 500 msec delay by calling the routine delay_ms(int ms)
    SEC_LED = 0;                                                                //OFF the sec_led
    strcpy(txt, " ");                                                           //clear the above character 
    drawtext(120, 10, txt, ST7735_WHITE, ST7735_BLACK, TS_1);                  
    delay_ms(500);                                                              //a 500 msec dealy 
    update_misc();                                                              //routine on and calling the function
}

void wait_one_second_beep(char direction)                                       //timer that will produce the sound
{
    SEC_LED = 1;                                                                //ON the sec_led
    strcpy(txt, "*");                                                           //Putting ?*? 
    drawtext(120, 10, txt, ST7735_WHITE, ST7735_BLACK, TS_1);                   //where the color of the test is at 
    if(direction==NS)                                                           //creating a  2Khz sound if theres NS.
    {
        gen_2khz_sound();
    }
    else if(direction==EW)                                                      //If  EW then 1Khz 
    {
        gen_1khz_sound();
    }
    delay_ms(500);                                                              // 500 msec dealy 
    SEC_LED = 0;                                                                //sec_led OFF
    strcpy(txt, " ");                                                           // clear txt
    drawtext(120, 10, txt, ST7735_WHITE, ST7735_BLACK, TS_1);                   //where the color of the text is at 
    turn_off_sound();                                                           //sound off
    delay_ms(500);                                                              //another delay of 500 msec
    update_misc();                                                              //calling the function by the routine 
}

void wait_N_seconds(char direction, char sec)                                   //a counter for the seconds 
{
int s;
    for (s=sec-1;s>=0;s--)                                                      //a loop counter 
    {  
       wait_one_second();                                                       //calling the function
       update_Count(direction, s);                                              //changing the value
    }
}

void wait_N_seconds_PED(char direction, char sec)                               //direction depends on the timer 
{
int s;
    for (s=sec-1;s>=0;s--)                                                      
    {
        wait_one_second_beep(direction);                                        //find the sound by the direction 
        update_PED_Count(direction, s);                                         //the LED count 
    }
}

void delay_ms(int ms)
{
int count;
    count = (0xffff - COUNT_SCALED) - 1;
    count = count * ms;
 
T0CON = 0x04;                                                               // this a pre scaler of 1:32 in  Timer 0, 16-bit  
TMR0L = count & 0x00ff;                                                     // low byte of TMR
TMR0H = count >> 8;                                                         // high byte of TMR
INTCONbits.TMR0IF = 0;                                                      // clearing the flag 
T0CONbits.TMR0ON = 1;                                                       // turn the timer on

while (INTCONbits.TMR0IF == 0);                                          // until the timer flag goes to one
T0CONbits.TMR0ON = 0;                                                       // turn off the timer 
}

void gen_1khz_sound()                                                           //a 1kHz sound to buzzer
{
    PR2 = 0b11111001 ;
    T2CON = 0b00000101 ;
    CCPR2L = 0b01001010 ;
    CCP2CON = 0b00111100 ;
}

void gen_2khz_sound()                                                           // 2kHz buzzer sound
{
    PR2 = 0b01111100 ;
    T2CON = 0b00000101 ;
    CCPR2L = 0b00111110 ;
    CCP2CON = 0b00011100 ;
}

void turn_off_sound()                                                           // buzzer off
{
    CCP2CON = 0b0;
    SPKR = 0;

}

unsigned int get_full_ADC(void)
{
int result;
ADCON0bits.GO=1;                                                        
while(ADCON0bits.DONE==1);                                        
result =((ADRESH*0x100)+ADRESL);                          
return result;                                                              
}

day_mode()                                                                      //Day Mode
{
    Mode_Txt[0] = 'D';                                                          //displaying the letter  a D on top
    drawtext(35,10, Mode_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);                //where the color text is located at 
    update_color(NS, Color_Green);                                              //until 9 seconds and the NS RGB will light GREEN
    if (SW_NSPED == 1)                                                          //Pedestrian countdown will start if 1 and  for the NS direction 
    {                                                                           // after 12 seconds 
        wait_N_seconds_PED(NS, 12);
    }
    wait_N_seconds(NS, 8);
    update_color(NS, Color_Yellow);                                             //NS will be YELLOW just for 4 sec
    wait_N_seconds(NS, 4);
    update_color(NS, Color_Red);                                                // then NS to red 
    update_color(EWLT, Color_Green);                                            //the  EWLT turn GREEN for 8 sec
    wait_N_seconds(EWLT, 9);
    update_color(EWLT, Color_Yellow);                                           //then the EWLT will light up YELLOW. again for 4 sec 
    wait_N_seconds(EWLT, 4);
    update_color(EWLT, Color_Red);                                              // the EWLT is off and light RGB RED
    
    update_color(EW, Color_Green);                                              // the EW ON and GREEN
    if  (SW_EWPED == 1)                                                        
    {
        wait_N_seconds_PED(EW, 7);
    }
    wait_N_seconds(EW, 10);
    update_color(EW, Color_Yellow);                                            
    wait_N_seconds(EW, 4);
    update_color(EW, Color_Red);  
    
    update_color(NSLT, Color_Green);                                          
    wait_N_seconds(NSLT, 8);
    update_color(NSLT, Color_Yellow);                                          
    wait_N_seconds(NSLT, 4);
    update_color(NSLT, Color_Red);                                            
}


night_mode()                                                                          // night mode                                                                            
{
    Mode_Txt[0] = 'N';                                                          // display N on lcd
    drawtext(35,10, Mode_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);                // location of the text color 
    update_color(NS, Color_Green);                                              // unitl 9 sec the NS will come to be gren 
    wait_N_seconds(NS, 9);                                                  
    update_color(NS, Color_Yellow);                                             // yellow to 3 sec ns
    wait_N_seconds(NS, 3);                                                      
    update_color(NS, Color_Red);                                                // pedestrian present then ewlt green and NS will stay red 
    if (SW_EWLT == 1)                                                          
    {
        update_color(EWLT, Color_Green);                                        
        wait_N_seconds(EWLT, 6);
        update_color(EWLT, Color_Yellow);                                       // the EWLT YELLOW for 3 sec 
        wait_N_seconds(EWLT, 3);
        update_color(EWLT, Color_Red);                                          // EWLT oof and  RED
    }
    update_color(EW, Color_Green);                                              //) EW GREEN for 14 sec
    wait_N_seconds(EW, 9);
    update_color(EW, Color_Yellow);                                             // EW to YELLOW only for 3 sec
    wait_N_seconds(EW, 3);
    update_color(EW, Color_Red);                                                // the EW to red 
    if (SW_NSLT == 1)                                                           //If 1 the continue 
    {
        update_color(NSLT, Color_Green);                                        // the NSLT RGB  GREEN until 10 sec
        wait_N_seconds(NSLT, 9);
        update_color(NSLT, Color_Yellow);                                       // update the NSLT to YELLOW. 3 sec 
        wait_N_seconds(NSLT, 3);
        update_color(NSLT, Color_Red);                                          //finally the NSLT to red 
    }
}

