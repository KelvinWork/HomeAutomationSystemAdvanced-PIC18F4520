#pragma config OSC=XT       //Oscillator Selection bits(XT Oscill)) 

#pragma config LVP=OFF      //Single-Supply ICSP Enable bit 

#pragma config WDT=OFF      //Wacthdog TImer Enable bit 

 

#define _XTAL_FREQ 4000000  

 

#define LCD_DATA PORTD 

#define LCD_E PORTCbits.RC0   //E signal for LCD 

#define LCD_RS PORTAbits.RA5  //RS signal for LCD 

 

 

#include <xc.h> 

#include <stdlib.h> 

 

#define RED_LED PORTCbits.RC3  

#define GREEN_LED PORTCbits.RC4  

 

void interrupt high_priority ISRPowerManagement(void); 

void interrupt low_priority ISRadc(void); 

 

void Speakerbeep(); 

void Speaker500(); 

void timer(); 

 

void Init_LCD(void);    // Initialize the LCD 

void W_ctr_4bit(char);  // 4-bit control word for LCD 

void W_data_4bit(char); // 4-bit text data for LCD 

 

 

unsigned char LCD_TEMP, i;   

unsigned int count; 

 

const char MESS[]= "IDLE MODE";  

const char MESS2[]="WAKE UP";    

const char MESS3[]="SLEEP MODE"; 

const char MESS4[]="ENTERING SLEEP"; 

const char MESS5[]="LOW WAKE UP"; 

 

int result, result2, result3, y; 

int pattern[10]={0b11000000,0b11111001,0b10100100,0b10110000,0b10011001,0b10010010,0b10000010,0b11111000,0b10000000,0b10010000}; 

 

unsigned char const ssC[10] = {0b00000000, 0b00100000, 0b10000000, 0b00000000, 0b00100000, 0b01000000, 0b01000000, 0b00000000, 0b00000000, 0b00000000}; 

unsigned char const ssD[10] = {0b00001000, 0b00001111, 0b00000100, 0b00000110, 0b00000011, 0b00000010, 0b00000000, 0b00001111, 0b00000000, 0b00000010}; 

unsigned char counter = 10; 

int subCounter = 0; //flash 7 seg 

void main(void){ 

ADCON0 = 0b00000001;        // Select channel 0 and turn on ADC 

ADCON1 = 0b00001110;        // AN0 channel and internal voltage referencing 

ADCON2 = 0b00000010;        // Result left justified, manual acquisition time, Fosc/4 

T2CON =  0b00000101; 

          

OSCCON = 0;// PRI_RUN mode (Important) 

  

TRISA = 0b00001111; 

TRISB = 0x0F; 

TRISC = 0; 

TRISD = 0;  

TRISE = 0b11101000; 

 

PORTC = 0b11100000; 

PORTD = 0b11111111; 

 

 

INTCONbits.GIE      =0;//Disable Global Interrupt 

RCONbits.IPEN       =1; //Enable Interrupt Priority  

 

//CONFIGURE Interrupt Priority 

 

//Interrupt0 / RB0 / Push Button S8 

 

INTCONbits.INT0IF   =0;     //Clear INTO Interrupt Flag 

INTCON2bits.INTEDG0 =1;     //Configure INT0 for rising edge trigger 

INTCONbits.INT0IE   =1;     //Enable INT0 Interrupt 

 

//Interrupt1 /RB1 /Push Button S7 

//Used By ADC  

INTCON3bits.INT1IP  =0;     //Enable Low Priority 

INTCON3bits.INT1IF  =0;     //Clear IF 

INTCON2bits.INTEDG1 =1;     //Rising Edge  

INTCON3bits.INT1IE  =1;     //Enable Interrupt 

 

//Interrupt2 /RB2 / Push Button S3 

//Used By Power Management 

INTCON3bits.INT2IP  =1; //High priority 

INTCON3bits.INT2IF  =0; //cleared interrupt flag 

INTCON2bits.INTEDG2 =1; //Rising Edge 

INTCON3bits.INT2IE  =1; //Enable INT2 Interrupt 

 

INTCONbits.GIE      =1; 

INTCONbits.GIEL     =1; //Low Global Interrupt 

 

CCP1CON = 0b00000000; //DISABLED PWM Speaker 

CCP2CON = 0b00000000; // DISABLED PWM Lightbulb 

 

Init_LCD(); //Init LCD 4-Bit Interface,multiple Line 

 

RED_LED     =0; //OFF  

GREEN_LED   =0; //OFF 

while (1){ 

  

   __delay_ms(1000); //1 Second 

   count++; 

   if(count<5){ 

        

    GREEN_LED=1; 

    //LightBulb Max Brightness 

    PR2 = 62; 

    CCPR2L = 0b00111110; 

    CCP2CON = 0b00101100; 

   } 

    

   if(count>=5&&count<10){   

        

    OSCCONbits.IDLEN=1;         //IDLEN MODE 

    Init_LCD(); 

    W_ctr_4bit(0b10000000);     //Tells the class to put it in the first line 

    for(i=0;MESS[i];i++)        //OutPut message to LCD 

    W_data_4bit(MESS[i]);       //Sends the string of chars into the LCD  

    //LightBubl Dim Brightness 20% Duty Cycle 

    PR2 =62;  

    CCPR2L = 0b00001100; 

    CCP2CON = 0b00101100; 

   } 

    

   if(count>=10&&count<15){ 

        

    GREEN_LED=0;    

    RED_LED=~RED_LED; 

    //Speakerbeep(); 

    Init_LCD(); 

     

    W_ctr_4bit(0b10000000);     //Tells the class to put it in the first line 

    for(i=0;MESS4[i];i++){      //OutPut message to LCD 

        W_data_4bit(MESS4[i]);  //Sends the string of chars into the LCD 

    } //for loop 

   }// if  

    

    

   if(count>=15)    

 

   { 

    

    

    

        OSCCONbits.IDLEN=0; 

        Init_LCD(); 

        W_ctr_4bit(0b10000000);     //Tells the class to put it in the first line 

        for(i=0;MESS3[i];i++){      //OutPut message to LCD 

            W_data_4bit(MESS3[i]);  //Sends the string of chars into the LCD  

        }// for loop 

        __delay_ms(500); 

        Init_LCD(); 

        __delay_ms(1000); 

        Speakerbeep(); 

       

        SLEEP(); 

    

   } 

     

  }//while 

} //main 

 

void interrupt high_priority ISRPowerManagement(void){ 

   if (INTCON3bits.INT2IF ==1){  

        

        Init_LCD(); 

        GREEN_LED=1; 

        W_ctr_4bit(0b10000000);     //Tells the class to put it in the first line 

        for(i=0;MESS2[i];i++)       //OutPut message to LCD 

        W_data_4bit(MESS2[i]);      //Sends the string of chars into the LCD 

        count=0; 

        Speaker500(); 

         

        CCP1CON = 0b00000000; 

        INTCON3bits.INT2IF = 0;     // Clear interrupt flag  

         

        }//if loop 

   __delay_ms(1000); 

   Init_LCD(); 

}//ISRPowerManagement 

 

void Speaker500(){ 

    //Speaker, 500Hz, 50% Duty Cycle 

     

    T2CON = 0b00000111; 

    PR2 =124; 

    CCPR1L = 0b00111110; 

    CCP1CON = 0b00101100; 

    _delay(1000000); 

    CCP1CON = 0b00000000; 

} 

void Speakerbeep(){ 

    //4KHz, 0% Duty Cycle 

     

    PR2 = 62;  

    CCPR1L = 0b00001100; 

    CCP1CON = 0b00101100; 

     _delay(1000000); 

   //   PR2 = 499; 

   // CCPR1L = 0b11111010; 

   // CCP1CON = 0b00001100; 

     

}//LightBulbOff 

 

void interrupt low_priority ISRadc(void){ 

    if(INTCON3bits.INT1IF ==1){ 

         

    for(y=0; y<3000; y++){ 

         

    _delay(8);                                      //Acquisition time is 8us, which is more than 5us 

    ADCON0bits.GO = 1;                              //Start conversion 

    while(ADCON0bits.DONE);                         //Wait for conversion to be finished      

    result = 15 + ((ADRESH & 0b11110000) >> 4);     //Store upper 4 nibbles and shift 

    result2 = result % 10;                          //Right integer 

    result3 = result / 10;                          //Left integer 

 

    PORTEbits.RE1=1; 

    PORTEbits.RE2=0; 

    PORTC = (pattern[result2] & 0b00000111) << 5; 

    PORTD = (pattern[result2] & 0b01111000) >> 3; 

    _delay(1000); 

    PORTEbits.RE1=0; 

    PORTEbits.RE2=1; 

    PORTC = (pattern[result3] & 0b00000111) << 5; 

    PORTD = (pattern[result3] & 0b01111000) >> 3; 

     

    _delay(1000);                                   //1ms delay 

    } 

    PORTC = 0b11100000; 

    PORTD = 0b00001111; 

    Speaker500(); 

    INTCON3bits.INT1IF = 0; 

     

    }//Interrupt Loop 

}//ISRadc  

 

 

 

void timer(){ 

    TMR0H = 0x00; 

            TMR0L = 0x06; 

            if (counter == 0) 

            { 

                PORTCbits.RC4 = 0; 

            } 

            else 

            { 

                subCounter++; 

                if (subCounter == 1000) 

                { 

                    counter--; 

                    subCounter = 0; 

 

                } 

                if (counter == 0) 

                { 

                    //additional features 

                      RED_LED=1; 

                } 

            }             

            // Update 7 segments 

            unsigned char value = counter / 10; 

     

            // TODO: Select the correct seven-segment 

            PORTEbits.RE1 = 0; 

            PORTEbits.RE2 = 1; 

 

            // Update TRISC 

            unsigned char cTemp = PORTC;    // Get TRISC 

            cTemp = cTemp & 0b00011111;     // Clear seven segment bits 

            cTemp = cTemp | ssC[value];     // Set the seven segment bits 

            PORTC = cTemp; 

 

            // Update TRISD 

            unsigned char dTemp = PORTD;    // Get TRISD 

            dTemp = dTemp & 0b11110000;     // Clear seven segment bits 

            dTemp = dTemp | ssD[value];     // Set the seven segment bits 

            PORTD = dTemp; 

             

            // get value to be displayed 

            unsigned char value = counter % 10; 

 

            // TODO: Select the correct seven-segment 

            PORTEbits.RE1 = 1; 

            PORTEbits.RE2 = 0; 

 

            // Update TRISC 

            unsigned char cTemp = PORTC;    // Get TRISC 

            cTemp = cTemp & 0b00011111;     // Clear seven segment bits 

            cTemp = cTemp | ssC[value];     // Set the seven segment bits 

            PORTC = cTemp; 

 

            // Update TRISD 

            unsigned char dTemp = PORTD;    // Get TRISD 

            dTemp = dTemp & 0b11110000;     // Clear seven segment bits 

            dTemp = dTemp | ssD[value];     // Set the seven segment bits 

            PORTD = dTemp; 

} 

 

 

 

 

 

 

 

//BELOW IS LCD CONFIGURATION 

//BELOW IS LCD CONFIGURATION 

void Init_LCD()    //Function to initialize LCD 

{ 

     

    _delay(15);             // a)15ms LCD power up Delay 

    W_ctr_4bit(0x03);       // b)Function Set(DB4-DB7:8-bit Interface) 

    _delay(5);              // c)5ms delay 

    W_ctr_4bit(0x02);       // d)Function Set(DB4-DB7:4bit Interface) 

    W_ctr_4bit(0b00101000); // Function Set  4-bit, 2 lines, 5x7 

    W_ctr_4bit(0b00001100); // Display On,Cursor Off 

    W_ctr_4bit(0b00000110); // Entry mode _ inc addr, no shift 

    W_ctr_4bit(0b00000001); // Clear Display & home position 

     

} 

 

void W_ctr_4bit(char x) 

{ 

     

    LCD_RS=0;           // Logic 0 

    LCD_TEMP=x;         // Store Control word 

    LCD_E=1;            // Logic 1 

    LCD_DATA=LCD_TEMP|0b00001111;  // Send upper nibble of Control word 

    _delay(1000);       // 1ms Delay 

    LCD_E=0;            // Logic 0 

    _delay(1000);       // 1ms Delay 

    LCD_TEMP=x;         // Store Control Word 

    LCD_TEMP <<=4;      // Shift lower nibble to upper nibble 

    LCD_E=1;            // Logic 1 

    LCD_DATA=LCD_TEMP|0b00001111;  // Send lower nibble to control word 

    _delay(1000);       // 1ms delay 

    LCD_E=0;            // Logic 0 

    _delay(1000);       // 1ms delay 

} 

 

void W_data_4bit(char x) 

{ 

     

    LCD_RS=1;           // Logic 0 

    LCD_TEMP=x;         // Store Text Data 

    LCD_E=1;            // Logic 1 

    LCD_DATA=LCD_TEMP|0b00001111;  // Send upper Nibble of Text data 

    _delay(1000);       // 1ms delay 

    LCD_E=0;            // Logic 0 

    _delay(1000);       // 1ms Delay 

    LCD_TEMP=x;         // Store Text data 

    LCD_TEMP <<=4;      // Shift lower nibble to upper Nibble 

    LCD_E=1;            // Logic 1 

    LCD_DATA=LCD_TEMP|0b00001111;  // Send upper nibble of text data 

    _delay(1000);       // 1ms Delay  

    LCD_E=0;            // Logic 0 

    _delay(1000);       // 1ms Delay 

} 