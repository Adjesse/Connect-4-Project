#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */

#define LCD_DATA PORTK
#define LCD_CTRL PORTK
#define RS 0x01
#define EN 0x02

void COMWRT4(unsigned char);
void DATWRT4(unsigned char);
void lcdSetup(void);
void ready(void);
void player(void);
void musicoff(void);
void mSDelay(unsigned int millis_delay);
void serial_send(unsigned char character_out);
void keypad_input(void);
unsigned power(unsigned base, unsigned degree);
void full(void);
void Light_LEDS(unsigned int gameboard[4][5],unsigned int COL_LENGTHS[5],unsigned int POW[5], int PORTS[5],unsigned int i);
unsigned char Player_Number;
unsigned int SET_BIT;
const unsigned char keypad[4][4] =
{
'1','2','3','A',
'4','5','6','B',
'7','8','9','C',
'*','0','#','D'
};
unsigned int POW_OF_COL1_P1;
unsigned int POW_OF_COL1_P2;
unsigned int COL1_MASK_BITS;
unsigned char column,row;
unsigned char PLAYER_ENABLE;
unsigned int PORTB_VAR;

// Global music note
int note = 0;
void songSetup(void);
void music(void);

// Interrupt for Output Compare            
interrupt(((0x10000 - Vtimch5)/2)-1) void TC5_ISR(void); 



void main(void) {
    unsigned int COL_LENGTHS[5] = {3,3,3,3,3};
    unsigned int POW[5] = {0,0,0,0,0};
    int PORTS[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned int gameboard[4][5] = 
    {
      //P  //T  //M  //H  //B
      0x00,0x00,0x00,0x00,0x00,  //7 and 3
      0x00,0x00,0x00,0x00,0x00,  //6 and 2
      0x00,0x00,0x00,0x00,0x00,  //5 and 1
      0x00,0x00,0x00,0x00,0x00,  //4 and 0
    };
    PLAYER_ENABLE = 0;
    Player_Number = '1';
    
    SCI0CR1=0; //expect data passed to be 8 bits long
    SCI0CR2=0x0C;

    SCI0BDH=0;
    SCI0BDL=26; //set baud as 9600 bits 8Mhz/2*16*9600
    //DDRH |= 0b11110110; //D0 and D3 input
    DDRA = 0x0F; //Output
    
    PORTB_VAR = 0x00;
    
    DDRJ |=0x02; 
    PTJ &=~0x02;
                                //ACTIVATE LED ARRAY ON PORT B
    DDRP = 0xFF;                           
    DDRT = 0xFF;
    DDRM = 0xFF;
    DDRB = 0xFF;
    DDRH = 0xFF;
    DDRE = 0xFF;
    
    PTP = 0x00;
    PTT = 0x00;
    PTM = 0x00;
    PORTB = 0x00;
    PTH = 0x00;
    PORTE = 0x00;
    
    lcdSetup(); // initilize lcd
    music();//play music
      
while(1) {

    ready();         // print ready to lcd

    player();
    
    
    mSDelay(100);
    PLAYER_ENABLE = 1;
    
    while(PLAYER_ENABLE) {
      COL1_MASK_BITS = 0x00; 
      keypad_input();
       
      if(PORTB_VAR != 0) {
        if(PORTB_VAR == 0x31) {                    //column 1
          Light_LEDS(gameboard,COL_LENGTHS,POW,PORTS,0);
        } else if(PORTB_VAR == 0x32){   //column 2
          Light_LEDS(gameboard,COL_LENGTHS,POW,PORTS,1);
        } else if(PORTB_VAR == 0x33){   //column 3
          Light_LEDS(gameboard,COL_LENGTHS,POW,PORTS,2);
        } else if(PORTB_VAR == 0x34){   //column 4
          Light_LEDS(gameboard,COL_LENGTHS,POW,PORTS,3);
        } else if(PORTB_VAR == 0x35){   //column 5
          Light_LEDS(gameboard,COL_LENGTHS,POW,PORTS,4);
        }

         PTP = PTP|PORTS[0];
         PTT = PTT|PORTS[1];
         PTM = PTM|PORTS[2];
         PTH = PTH|PORTS[3];
         PORTB = PORTB|PORTS[4];
         
         PORTE = PORTE | PORTS[1];
         PORTE = PORTE >> 1;
         
         
         
         

      }
    }

}
}
  

unsigned power(unsigned base, unsigned degree)
{
    unsigned result = 1;
    unsigned term = base;
    while (degree)
    {
        if (degree & 1)
            result *= term;
        term *= term;
        degree = degree >> 1;
    }
    return result;
}
void full(void)  // function add below main
 {      
        COMWRT4(0x01);  //Clear display
        mSDelay(1);
        COMWRT4(0x80);  //set start posistion, home position
        mSDelay(1);
        DATWRT4('B');
        mSDelay(1);
        DATWRT4('O');
        mSDelay(1);
        DATWRT4('A');
        mSDelay(1);
        DATWRT4('R');    // displays board is full to lcd
        mSDelay(1);
        DATWRT4('D');
        mSDelay(1);
        DATWRT4(' ');
        mSDelay(1);
        DATWRT4('F');
        mSDelay(1);
        DATWRT4('U');
        mSDelay(1);
        DATWRT4('L');
        mSDelay(1);
        DATWRT4('L');
        mSDelay(1);
        mSDelay(3000);     
 }

void mSDelay(unsigned int itime){
unsigned int i; unsigned int j;
   for(i=0;i<itime;i++)
      for(j=0;j<4000;j++);
}

void Light_LEDS(unsigned int gameboard[4][5],\
unsigned int COL_LENGTHS[5],unsigned int POW[5],\
int PORTS[5],unsigned int i) {
     //make sure space is clear
    if(gameboard[COL_LENGTHS[i]][i] == 0){ 
       //Player 1 or Player 2
      if(Player_Number == '1') {         
         //mask unused bits
        COL1_MASK_BITS = gameboard[COL_LENGTHS[i]][i] | 0xF0; 
     
        SET_BIT = power(2,POW[i]);
        COL1_MASK_BITS = COL1_MASK_BITS | SET_BIT;
        
        //unmask bits
        COL1_MASK_BITS = COL1_MASK_BITS & 0x0F;           
        gameboard[COL_LENGTHS[i]][i] = \
        gameboard[COL_LENGTHS[i]][i] |  COL1_MASK_BITS;
        
        POW[i]++;
        
        PORTS[i] = \
        PORTS[i] | gameboard[COL_LENGTHS[i]][i];
        
        COL_LENGTHS[i]--;
        
        //next move
        PLAYER_ENABLE = 0;
        Player_Number = '2';
    } else{
        //mask unused bits
        COL1_MASK_BITS = gameboard[COL_LENGTHS[i]][i] | 0x0F;
         
        SET_BIT = power(2,(POW[i]+4));
        COL1_MASK_BITS = COL1_MASK_BITS | SET_BIT;
        
        //unmask bits
        COL1_MASK_BITS = COL1_MASK_BITS & 0xF0;           
        gameboard[COL_LENGTHS[i]][i] = \
        gameboard[COL_LENGTHS[i]][i] |  COL1_MASK_BITS;
        
        POW[i]++;
        
        PORTS[i] = \
        PORTS[i] | gameboard[COL_LENGTHS[i]][i];
        
        COL_LENGTHS[i]--;
        //next move
        PLAYER_ENABLE =0;
        Player_Number = '1';
    }
  }else{
    full();
  }

}
void keypad_input(){
 do{                                 //OPEN do1
     PORTA = PORTA | 0x0F;            //COLUMNS SET HIGH
     row = PORTA & 0xF0;              //READ ROWS
  }while(row == 0x00);                //WAIT UNTIL KEY PRESSED //CLOSE do1



  do{                                 //OPEN do2
     do{                              //OPEN do3
        mSDelay(1);                   //WAIT
        row = PORTA & 0xF0;           //READ ROWS
     }while(row == 0x00);             //CHECK FOR KEY PRESS //CLOSE do3
     
     mSDelay(20);                     //WAIT FOR DEBOUNCE
     row = PORTA & 0xF0;
  }while(row == 0x00);                //FALSE KEY PRESS //CLOSE do2

  while(1){                           //OPEN while(1)
     PORTA &= 0xF0;                   //CLEAR COLUMN
     PORTA |= 0x01;                   //COLUMN 0 SET HIGH
     row = PORTA & 0xF0;              //READ ROWS
     if(row != 0x00){                 //KEY IS IN COLUMN 0
        column = 0;
        break;                        //BREAK OUT OF while(1)
     }
     PORTA &= 0xF0;                   //CLEAR COLUMN
     PORTA |= 0x02;                   //COLUMN 1 SET HIGH
     row = PORTA & 0xF0;              //READ ROWS
     if(row != 0x00){                 //KEY IS IN COLUMN 1
        column = 1;
        break;                        //BREAK OUT OF while(1)
     }

     PORTA &= 0xF0;                   //CLEAR COLUMN
     PORTA |= 0x04;                   //COLUMN 2 SET HIGH
     row = PORTA & 0xF0;              //READ ROWS
     if(row != 0x00){                 //KEY IS IN COLUMN 2
        column = 2;
        break;                        //BREAK OUT OF while(1)
     }
     PORTA &= 0xF0;                   //CLEAR COLUMN
     PORTA |= 0x08;                   //COLUMN 3 SET HIGH
     row = PORTA & 0xF0;              //READ ROWS
     if(row != 0x00){                 //KEY IS IN COLUMN 3
        column = 3;
        break;                        //BREAK OUT OF while(1)
     }
     row = 0;                         //KEY NOT FOUND
  break;                              //step out of while(1) loop to not get stuck
  }                                   //end while(1)

  if(row == 0x10){
     PORTB_VAR=keypad[0][column];         //OUTPUT TO PORTB LED

  }
  else if(row == 0x20){
     PORTB_VAR=keypad[1][column];

  }
  else if(row == 0x40){
     PORTB_VAR=keypad[2][column];

  }
  else if(row == 0x80){
     PORTB_VAR=keypad[3][column];

  }

  do{
     mSDelay(15);
     PORTA = PORTA | 0x0F;            //COLUMNS SET HIGH
     row = PORTA & 0xF0;              //READ ROWS
  }while(row != 0x00);                //MAKE SURE BUTTON IS NOT STILL HELD
    
    
}

void lcdSetup(void)
{
  DDRK = 0xFF;   
        COMWRT4(0x33);   //reset sequence provided by data sheet
        mSDelay(1);
        COMWRT4(0x32);   //reset sequence provided by data sheet
        mSDelay(1);
        COMWRT4(0x28);   //Function set to four bit data length
                                         //2 line, 5 x 7 dot format
        mSDelay(1);
        COMWRT4(0x06);  //entry mode set, increment, no shift
        mSDelay(1);
        COMWRT4(0x0E);  //Display set, disp on, cursor on, blink off
        mSDelay(1);
        COMWRT4(0x01);  //Clear display
        mSDelay(1);
}

void COMWRT4(unsigned char command)
  {
        unsigned char x;
        
        x = (command & 0xF0) >> 2;         //shift high nibble to center of byte for Pk5-Pk2
      LCD_DATA =LCD_DATA & ~0x3C;          //clear bits Pk5-Pk2
        LCD_DATA = LCD_DATA | x;          //sends high nibble to PORTK
        mSDelay(1);
        LCD_CTRL = LCD_CTRL & ~RS;         //set RS to command (RS=0)
        mSDelay(1);
        LCD_CTRL = LCD_CTRL | EN;          //rais enable
        mSDelay(5);
        LCD_CTRL = LCD_CTRL & ~EN;         //Drop enable to capture command
        mSDelay(15);                       //wait
        x = (command & 0x0F)<< 2;          // shift low nibble to center of byte for Pk5-Pk2
        LCD_DATA =LCD_DATA & ~0x3C;         //clear bits Pk5-Pk2
        LCD_DATA =LCD_DATA | x;             //send low nibble to PORTK
        LCD_CTRL = LCD_CTRL | EN;          //rais enable
        mSDelay(5);
        LCD_CTRL = LCD_CTRL & ~EN;         //drop enable to capture command
        mSDelay(15);
  }
  
  void DATWRT4(unsigned char data)
  {
  unsigned char x;
        
        x = (data & 0xF0) >> 2;
        LCD_DATA =LCD_DATA & ~0x3C;                     
        LCD_DATA = LCD_DATA | x;
        mSDelay(1);
        LCD_CTRL = LCD_CTRL | RS;
        mSDelay(1);
        LCD_CTRL = LCD_CTRL | EN;
        mSDelay(1);
        LCD_CTRL = LCD_CTRL & ~EN;
        mSDelay(5);
       
        x = (data & 0x0F)<< 2;
        LCD_DATA =LCD_DATA & ~0x3C;                     
        LCD_DATA = LCD_DATA | x;
        LCD_CTRL = LCD_CTRL | EN;
        mSDelay(1);
        LCD_CTRL = LCD_CTRL & ~EN;
        mSDelay(15);
  }
  
  void ready(void)
 {      
        COMWRT4(0x01);  //Clear display
        mSDelay(1);
        COMWRT4(0x80);  //set start posistion, home position
        mSDelay(1);
        DATWRT4('R');
        mSDelay(1);
        DATWRT4('E');
        mSDelay(1);
        DATWRT4('A');
        mSDelay(1);
        DATWRT4('D');
        mSDelay(1);
        DATWRT4('Y');
        mSDelay(1);
        DATWRT4('?');
        mSDelay(3000);     
 }
 
  void player(void)
 {      
        COMWRT4(0x01);  //Clear display
        mSDelay(1);
        COMWRT4(0x80);  //set start posistion, home position
        mSDelay(1);
        DATWRT4('P');
        mSDelay(1);
        DATWRT4('L');
        mSDelay(1);
        DATWRT4('A');
        mSDelay(1);
        DATWRT4('Y');
        mSDelay(1);
        DATWRT4('E');
        mSDelay(1);
        DATWRT4('R');
        mSDelay(1);
        DATWRT4(' ');
        mSDelay(1);
        DATWRT4(Player_Number);
        mSDelay(1);
        DATWRT4(':');
        mSDelay(3000);     
 }
 
 void music(void)
{  	
 //Use this function to make
 //the song
 //NOTE: The smaller value given to 'note'
 // the higher the pitch will be
 int i = 0;
 int delay = 0;
  
  songSetup();
  for (i =0; i <2; i++) {
  delay =150;
    
    	  	note = 600000;
    	  	mSDelay(delay);
    	  	note = 700000;
    	  	mSDelay(delay);
    	  	note = 800000;
    	  	mSDelay(delay);
    	  	note = 900000;
    	  	mSDelay(delay);
    	  	note = 900000;
    	  	mSDelay(delay);
    	  	note = 800000;
    	  	mSDelay(delay);
    	  	note = 700000;
    	  	mSDelay(delay);
    	  	note = 600000;
    	  	mSDelay(delay);
    	  	note = 600000;
    	  	mSDelay(delay);
    	  	note = 700000;
    	  	mSDelay(delay);
    	  	note = 800000;
    	  	mSDelay(delay);
    	  	note = 900000;
    	  	mSDelay(delay);
    	  	note = 900000;
    	  	mSDelay(delay);
    	  	note = 800000;
    	  	mSDelay(delay);
    	  	note = 700000;
    	  	mSDelay(delay);
    	  	note = 600000;
    	  	mSDelay(delay);
  }
  
  musicoff();
}

void musicoff(void)
{  	  
  TSCR1 = 0x00;        
}

void songSetup(void)
{
    TSCR1 = 0x80;       // Timer Enable
    TIOS  = 0x20;       // Select Channel 5 for output compare
    TCTL1 = 0x04;       // Toggle PT5 pin upon match
    TIE   = 0x20;       // Enable Interrupt for Channel 5
    TFLG1 = 0x20;       // Clear C5F
    __asm(cli);          // Enable Interrupts Globally	
}

// Interrupt Service Routine to Output Compare 
interrupt(((0x10000 - Vtimch5)/2)-1) void TC5_ISR(void)
{
 
 TC5   = TC5 + note;             // Update to TC5 
 TFLG1 = 0x20;                        // Clear C5F

}






