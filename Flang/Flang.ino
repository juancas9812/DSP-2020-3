int in_ADC0;  //variables for 2 ADCs values (ADC0, ADC1)
int POT0, out_DAC0; //Entrada pot 0, y salida DAC0
//int LED = 3;
//int FOOTSWITCH = 7; 
//int TOGGLE = 2; 

#define N_OSC 44100
//#define N_OSC 22050
#define MAX_DELAY 441 //MAX_DELAY=10ms (441)
//#define MAX_DELAY 221
uint16_t Buffer[MAX_DELAY+1];
unsigned int BufferCounter = 0;
//unsigned int Delay_Depth = MAX_DELAY;
unsigned int Flanger_Depth = 0;
uint16_t osc[N_OSC];
int freq=1;
int flanger_index=0;
int osc_ind=0;

void oscillator_setup(uint16_t *osc){
  for(int i=0;i<=N_OSC;i++){
    osc[i]=uint16_t(((1+sin(((2.0*PI)/N_OSC)*i))*MAX_DELAY)/2);
  }
}

void timer_setup(){
  //turn on the timer clock in the power management controller
  pmc_set_writeprotect(false);
  pmc_enable_periph_clk(ID_TC4);
 
  //we want wavesel 01 with RC 
  TC_Configure(TC1,1,TC_CMR_WAVE|TC_CMR_WAVSEL_UP_RC|TC_CMR_TCCLKS_TIMER_CLOCK2);
  TC_SetRC(TC1, 1, 238); // sets <> 44.1 Khz interrupt rate
  //TC_SetRC(TC1, 1, 476); // sets <> 22.05 Khz interrupt rate
  TC_Start(TC1, 1);
 
  // enable timer interrupts on the timer
  TC1->TC_CHANNEL[1].TC_IER=TC_IER_CPCS;
  TC1->TC_CHANNEL[1].TC_IDR=~TC_IER_CPCS;
 
  //Enable the interrupt in the nested vector interrupt controller 
  //TC4_IRQn where 4 is the timer number * timer channels (3) + the channel number 
  //(=(1*3)+1) for timer1 channel1 
  NVIC_EnableIRQ(TC4_IRQn);
}

void ADC_setup(){
  //ADC Configuration
  ADC->ADC_MR |= 0x80;   // DAC in free running mode.
  ADC->ADC_CR=2;         // Starts ADC conversion.
  ADC->ADC_CHER=0x1CC0;  // Enable ADC analog pins 0,1,8,9 and 10  
}

void setup()
{
  oscillator_setup(&osc[0]);
  timer_setup();
  ADC_setup();
 
  //DAC Configuration
  analogWrite(DAC0,0);  // Enables DAC0

}
 
void loop()
{
  //Read the ADCs
  while((ADC->ADC_ISR & 0x1CC0)!=0x1CC0);// wait for ADC 0, 1, 8, 9, 10 conversion complete.
  in_ADC0=ADC->ADC_CDR[7];               // read data from ADC0
  POT0=ADC->ADC_CDR[10];                 // read data from ADC8        
}
 
//Interrupt at 44.1KHz rate (every 22.6us) (22.05kHz every 45.2us)
void TC4_Handler()
{
  //Store current readings  
  Buffer[BufferCounter] = in_ADC0;
 
  //Adjust oscillator frequency from POT0 reading.(1 to 10Hz)
  freq=map(POT0>>2,0,1023,1,10);
  
  //Increse/reset buffer counter.   
  BufferCounter++;
  if(BufferCounter >= MAX_DELAY+1) BufferCounter = 0; 

  flanger_index = (BufferCounter + MAX_DELAY - osc[osc_ind])%(MAX_DELAY+1);

  out_DAC0 = in_ADC0+Buffer[flanger_index];

  osc_ind+=freq;
  if(osc_ind>=N_OSC) osc_ind = osc_ind%N_OSC;
 
  //Write the DACs
  dacc_set_channel_selection(DACC_INTERFACE, 0);       //select DAC channel 0
  dacc_write_conversion_data(DACC_INTERFACE, out_DAC0);//write on DAC
 
  //Clear status allowing the interrupt to be fired again.
  TC_GetStatus(TC1, 1);
}
