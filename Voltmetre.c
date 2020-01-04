/**
	*A digital voltmeter using TM4C123 series development board
	*
	*It can only read between 0 - 3.3V and it definitely can't
	*read negative voltages so don't even try it.
	*It also sends the data via serial COM (using UART0) to a
	*computer program for graphical monitoring
  *
	*To read voltage between two nodes:
	*    Connect PE1 pin to the node with more voltage
	*		 Connect GND pin to the node with less voltage
	*This way you can read the voltage difference between two nodes
	*The read voltage amount will also be displayed using 3 digit
	*%4.2f formatted display 
	*(1 digit before and 2 digits after the dot)
	*
  *05160000657 Muhammed Nurullah EMSEN
  *05160000784 Elifnaz ÖKLÜ
  *05160000283 Oguzhan KATI
  *05150000628 Aysenur BAG
  *
  *2.01.2020
  *
**/



#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "TM4C123GH6PM.h"
#include <string.h>

// 7 Segment display tanimlari
#define out7_0 0x7E
#define out7_1 0x48
#define out7_2 0x3D
#define out7_3 0x6D
#define out7_4 0x4B
#define out7_5 0x67
#define out7_6 0x77
#define out7_7 0x4C
#define out7_8 0x7F
#define out7_9 0x6F

#define F_CPU 16000000
#define					load_val				(F_CPU/1000)
#define					calib_m  					1	   //


volatile int i = 0; 
volatile uint16_t value = 0;
//volatile bool flag = false;
volatile uint8_t digitFlag = 0x02;
volatile uint16_t hundreds =out7_0;
volatile uint16_t tens =out7_0;
volatile uint16_t ones =out7_0;
volatile bool flagUP = false;
volatile bool flagDOWN = false;


void PORTB_init(); //Main port driving the displays
void PORTF_init(); //Supports onboard buttons and display enabling/disabling switch pins
void TIMER0A_init(); //Used for switching between displays
void ADC0_init(); //Main voltage reading unit
void UART0_init(uint32_t baud_rate); //Used for communication with the monitoring program on the computer
//other prototypes
void U0_snd_chr(uint8_t one_byte);
char U0_get_chr(void);
void empty_buf(void);
void sndCommand(char c);
void delay_ms(uint32_t delay);
 
int main(void){
	uint32_t sample;
	//Main component initializations
	SystemInit();
  PORTB_init();
	PORTF_init();
	ADC0_init();
	UART0_init(9600);
	TIMER0A_init();
	GPIOB->DATA = out7_1 |0x80;

	GPIOF->DATA = 0x06; //At first display for ones will be enabled
	
	/**
	Main loop:
	
	It divides the digits and then assign real 7-segment pin values 
	according to the digits
	**/
	while(1){
		ADC0->PSSI |= 1;            // start a conversion at sequencer 0
    while((ADC0->RIS & 1) == 0);   // wait for analog read
    sample = ADC0->SSFIFO0;     // read conversion result from FIFO for seq 0
		ADC0->ISC = 1;              // clear flag
		
		//if one of the buttons pressed send scaling command
		if(flagUP){//scale UP
			sndCommand('+');
			flagUP = false;
		}else if(flagDOWN){ //Scale DOWN
			sndCommand('-');
			flagDOWN = false;
		}
		//Continuously sends data to monitoring program
		//RE: '$'(byte)(byte)'%'
	  U0_snd_chr('$');
	  U0_snd_chr(sample/256);
	  U0_snd_chr(sample%256);
	  U0_snd_chr('%');
	  delay_ms(25);
		
		//Compute value for onboard 3-digit display
		value = (((sample * 3300)/10)/4096);
		uint8_t birler = value % 10;
		uint8_t onlar = (value / 10) % 10;
		uint8_t yuzler = value / 100;
		
		//Assign values to each one of the three display digits according to value
		switch (yuzler){
			case 0:
				hundreds = out7_0;
				break;
			case 1:
				hundreds = out7_1;
				break;
			case 2:
				hundreds = out7_2;
				break;
			case 3:
				hundreds = out7_3;
				break;
			case 4:
				hundreds = out7_4;
				break;
			case 5:
				hundreds = out7_5;
				break;
			case 6:
				hundreds = out7_6;
				break;
			case 7:
				hundreds = out7_7;
				break;
			case 8:
				hundreds = out7_8;
				break;
			case 9:
				hundreds = out7_9;
				break;
		}
		switch (onlar){
			case 0:
				tens = out7_0;
				break;
			case 1:
				tens = out7_1;
				break;
			case 2:
				tens = out7_2;
				break;
			case 3:
				tens = out7_3;
				break;
			case 4:
				tens = out7_4;
				break;
			case 5:
				tens = out7_5;
				break;
			case 6:
				tens = out7_6;
				break;
			case 7:
				tens = out7_7;
				break;
			case 8:
				tens = out7_8;
				break;
			case 9:
				tens = out7_9;
				break;
		}
		
		switch (birler){
			case 0:
				ones = out7_0;
				break;
			case 1:
				ones = out7_1;
				break;
			case 2:
				ones = out7_2;
				break;
			case 3:
				ones = out7_3;
				break;
			case 4:
				ones = out7_4;
				break;
			case 5:
				ones = out7_5;
				break;
			case 6:
				ones = out7_6;
				break;
			case 7:
				ones = out7_7;
				break;
			case 8:
				ones = out7_8;
				break;
			case 9:
				ones = out7_9;
				break;
		}
			
	}
}
/* interrupt handler for TIMER0A
	 it switches between PF1, PF_2 and PF_3 enabling the first, second and third displays
	 and sets PORTF output for corresponding display
*/
void TIMER0A_Handler(void){
	TIMER0->ICR|= (1<<0); //Clear interrupt flag for t0
	if(digitFlag == 0x02){
		GPIOF->DATA = 0x0C; // 0b0000 1100 PF_1 LOW hundreds digit active
		GPIOB->DATA = hundreds | 0x80;
		digitFlag = 0x04;
	}else if(digitFlag == 0x04){
		GPIOF->DATA = 0x0A; // 0b0000 1010 PF_2 LOW tens digit active
		digitFlag = 0x08;
		GPIOB->DATA = tens;
	}else if(digitFlag == 0x08){
		GPIOF->DATA = 0x06; // 0b0000 0110 PF_3 LOW ones digit active
		digitFlag = 0x02;
		GPIOB->DATA = ones;
	}

}

/* 
 * interrupt handler for PORTF
 * Sets one oft the flags (flagUP or flagDOWN) according to the button pressed
 */

void GPIOF_Handler(void){
	for(i = 0; i <100000; i++);
	i=0;
	
	if(GPIOF->RIS & (1<<0)){ // left button pressed
		GPIOF->ICR|= (1<<0); //clear int flag
		flagDOWN =true;
		
	}
	else if(GPIOF->RIS & (1<<4)){ //right button pressed
		GPIOF->ICR|= (1<<4); //clear int flag
		flagUP =true;
		
	}
}
/*
 *PORTB initialization function
 *PORTB is the main GPIO port for driving the 7 segment displays
 *
*/

void PORTB_init(){
	SYSCTL->RCGCGPIO|= (1<<1); //Enable clock to PORTB
	GPIOB->LOCK=0x4C4F434B;//unlock PORTB to write on GPIOCR register
	GPIOB->CR=0xFF; //unlock changes for all pins on DIR, DEN and other pullup and pulldown regisers
	GPIOB->AFSEL = 0x00; //disable alternate functions
	GPIOB->AMSEL = 0x00; //disable analog inputs
  GPIOB->DIR=0xFF;	 //Setting the direction IO pins as OUTPUT
  GPIOB->DEN = 0xFF; // digital enable portf all pins
	
}

/*
 *PORTF initialization function
 *PORTF is used for:
 *   1) External interrupts on button 1 and 2
 *	 2) Switching between displays
 *
*/

void PORTF_init(){
	SYSCTL->RCGCGPIO|= (1<<5); //Enable clock to PORTF
	GPIOF->LOCK=0x4C4F434B;//unlock PORTF to write on GPIOCR register
	GPIOF->CR=0xFF; //unlock changes for all pins on DIR, DEN and other pullup and pulldown regisers
	GPIOF->AFSEL = 0x00; //disable alternate functions
	GPIOF->AMSEL = 0x00; //disable analog inputs
  GPIOF->DIR=0xEE;	 //Setting the direction IO pins PF_0 and PF_4 input; others output
	GPIOF->PUR=0x11;		//pullup for PF_0 and PF_4; these pins are onboard button pins and they will be configured for falling edge interrupts
  GPIOF->DEN = 0xFF; // digital enable portf all pins
	
	GPIOF->IS&= ~(1<<4)& ~(1<<0); // Make PF4 and PF0 edge sensitive 
	GPIOF->IBE&= ~(1<<4)& ~(1<<0);// clear the bits for corresponding PF4 and PF0 as we are using only the fallin edge triggers
	GPIOF->IEV&=~(1<<4)& ~(1<<0); // Make PF4 and PF0 falling edge triggered
	GPIOF->ICR|= (1<<4)|(1<<0); // Clear any previous interrupt at PF4 and PF0
	GPIOF->IM|= (1<<4)|(1<<0);  // unmask the interrupt for PF4 and PF0
	
	// Assign priority as 3 for when one of the buttons pressed it wont affect the displays
	// If the priority is not configured this way, the displays will flicker
	NVIC_SetPriority(GPIOF_IRQn, 3); 
	
	NVIC_EnableIRQ(GPIOF_IRQn); // enable interrupt requests for PORTF
	
}

void TIMER0A_init(){
	SYSCTL->RCGCTIMER |= 0x01; // enable use of timer0
	TIMER0 -> CFG = 0x00; //32 bit timer configuration
	TIMER0 ->CTL &= 0x01; // disable timer0 for further configuration
	TIMER0 -> TAMR |= 0x02; //count down and periodic
	TIMER0 -> TAILR = 100000;
	TIMER0 -> IMR = 0x01; //set interrupt mask for TIMER0
	TIMER0 -> CTL |= 0x01; // enable timer0
	
	// Assign priority as 2 in order to make the display continuous in case of a button interrupt
	NVIC_SetPriority(TIMER0A_IRQn, 2); 

	NVIC_EnableIRQ(TIMER0A_IRQn); // enable interrupt requests for TIMER0A
}

void ADC0_init(){

	SYSCTL->RCGCGPIO |= 0x10;       // enable clock to GPIOE
  SYSCTL->RCGCADC |= 1;           // enable clock to ADC0
    // initialize port pin PE1 for ADC0 input
  GPIOE->AFSEL |= 0x02;           // enable alternate function
  GPIOE->DEN &= ~0x02;            // disable digital function
  GPIOE->AMSEL |= 0x02;           // enable analog function
    // initialize ADC0 Sample Sequencer 0
  ADC0->ACTSS &= ~1;              // disable ADC0 sequencer 0 during configuration
  ADC0->EMUX &= ~0x000F;          // software trigger for sequencer 0
  ADC0->SSMUX0 &= ~0xFFFFFFFF;    // clear channel selects first
  ADC0->SSMUX0 |= 0x00000002;     // set channel 1 for first and only sample 0x02 -> pin PE1; 
  ADC0->SSCTL0 |= 0x00000006;     // finish sequence at 1st sample and post RIS bit 
	ADC0->SAC = 0x6; 								// 64x hardware oversampling
  ADC0->ACTSS |= 1;               // enable ADC0 sequencer 0
}

void UART0_init (uint32_t baud_rate){
	float baud_value=(F_CPU/(16.0*baud_rate));
	int baud_Ivalue=baud_value;
	int baud_Fvalue=(((baud_value-baud_Ivalue)*64)+0.5);
	SYSCTL->RCGCUART|=1;	//Enabling UART 
	SYSCTL->RCGCGPIO|=1;	//Enabling UART0
	UART0->CTL&=~0x1;	//uart disable during setting
	UART0->IBRD=baud_Ivalue; //int part
	UART0->FBRD=baud_Fvalue;	//float part
	UART0->LCRH|=((1<<6)|(1<<5)|(1<<4)); //8 bit FIFO enable 
	UART0->CC=0x5;								// 
	UART0->CTL|=0x1; //uart enable
	GPIOA->AFSEL|=0x3; //Enabling alternate function
	GPIOA->DEN|=0x3;		//digital enable
	GPIOA->PCTL=((GPIOA->PCTL&0xFFFFFF00)+0x11);
	GPIOA->AMSEL&=~0x3; //no analog
}

/*
 *A function sends a character using UART0
 */

void U0_snd_chr(uint8_t one_byte){
	empty_buf();
	while((UART0->FR&0x20)!=0);
	UART0->DR=one_byte;
}

/*
 *A function gets a character using UART0 (not used)
 */

char U0_get_chr(void){
  while((UART0->FR&0x10) != 0);
	return(UART0->DR&0xFF);
}

/*
 *A function flushes UART0 buffer (not used)
 */

void empty_buf(void){
	volatile char trash=0;
	 while((UART0->FR&0x10) == 0) trash=(UART0->DR&0xFF);
}

/*
 *A function sends change scale commands using serial COM
 *RE: '#'('+' | '-')'%'
 */

void sndCommand(char c){
	U0_snd_chr('#');
	  U0_snd_chr(c);
	  U0_snd_chr('%');
	  delay_ms(25);
}

/*
 *A 1 ms delay function
 */

void delay_ms(uint32_t delay){
		for(;delay>0;--delay){
		SysTick->LOAD=load_val-calib_m;
		SysTick->CTRL|=0x5;
		while((SysTick->CTRL&(1<<16))==0);
		}
		SysTick->CTRL=0;
}


