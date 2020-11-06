#include "Math.h"
const int N=256;   //Cambiar entre 256,512,1024,2048

typedef struct Complex {
float re;
float img;
} Complex;

volatile uint16_t adc_meas_buffer[N];
volatile bool adc_flag_conversion = false;
volatile bool full_buffer = false;
volatile int indx = 0;
int envio = 0;
uint32_t t1=0;
uint32_t t2=0;
uint32_t tmuestras=0;
uint32_t tenvmuestras=0;
uint32_t tfft=0;
uint32_t tenvfft=0;
uint32_t tmag=0;
uint32_t tenvmag=0;
uint32_t tphase=0;
uint32_t tenvphase=0;

Complex Wn[N/2];
Complex X[N];
float mag[N];
float phase[N];

Complex* apwn;
Complex* apx;


void setupWns(){
//  for(int k=0;k<4;k++){
//    Wn[k].re=float(cos((2*PI*k)/8));
//    Wn[k].img=float(-sin((2*PI*k)/8));
//    Serial.print(Wn[k].re);
//    Serial.print("+j");
//    Serial.println(Wn[k].img);
//  }
  for(int k=0;k<(N/2);k++){
    Wn[k].re=float(cos((2*PI*k)/N));
    Wn[k].img=float(-sin((2*PI*k)/N));
//    Serial.print("Wn[");Serial.print(k);Serial.print("]: ");Serial.print(Wn[k].re,5);Serial.print(" + ");Serial.print(Wn[k].img,5);Serial.println("j ");
  }
}
void adc_setup()
{
  PMC->PMC_PCER1 |= PMC_PCER1_PID37;        // ADC power on
  ADC->ADC_CR = ADC_CR_SWRST;               // Reset ADC
  ADC->ADC_MR |= ADC_MR_TRGEN_EN |          // Hardware trigger select
                 ADC_MR_PRESCAL(1) |        // the pre-scaler: as high as possible for better accuracy, while still fast enough to measure everything
                 ADC_MR_TRGSEL_ADC_TRIG3;   // Trigger by TIOA2 Rising edge

  ADC->ADC_IDR = ~(0ul); //== 0x00000001 Disable Interruptions
  ADC->ADC_CHDR = ~(0ul); // Disable channels
  ADC->ADC_CHER |= 0x00000001; //ADC_CHER_CH0; Enable CH0
  ADC->ADC_IER |= 0x00000001;  //ADC_IER_EOC0; Enable Interruption CH0
  ADC->ADC_PTCR |= ADC_PTCR_RXTDIS | ADC_PTCR_TXTDIS;   // Disable PDC DMA
  NVIC_EnableIRQ(ADC_IRQn);                             // Enable ADC interrupt
  ADC->ADC_CR = 2;                                      // Start 
}
void tc_setup()
{
  PMC->PMC_PCER0 |= PMC_PCER0_PID29;                        // TC2 power ON : Timer Counter 0 channel 2 IS TC2
  TC0->TC_CHANNEL[2].TC_CMR = TC_CMR_TCCLKS_TIMER_CLOCK1    // clock 2 has frequency MCK/2, clk on rising edge
                              | TC_CMR_WAVE                 // Waveform mode
                              | TC_CMR_WAVSEL_UP_RC         // UP mode with automatic trigger on RC Compare
                              | TC_CMR_ACPA_CLEAR           // Clear TIOA2 on RA compare match
                              | TC_CMR_ACPC_SET;            // Set TIOA2 on RC compare match

  constexpr int ticks_per_sample = 1905 ;   //84000000 / 2 / 22050;   // F_CPU / 2 is the timer clock frequency
  constexpr int ticks_duty_cycle = ticks_per_sample / 2;    // duty rate up vs down ticks over timer cycle; use 50%
  TC0->TC_CHANNEL[2].TC_RC = ticks_per_sample;
  TC0->TC_CHANNEL[2].TC_RA = ticks_duty_cycle;

  TC0->TC_CHANNEL[2].TC_CCR = TC_CCR_SWTRG | TC_CCR_CLKEN;  // Software trigger TC2 counter and enable
}
void ADC_Handler()
{
  if (full_buffer == false)
  {
    adc_meas_buffer [indx] = ADC->ADC_CDR[0];
    indx = indx + 1;
    if (indx == N)
    {
      indx = 0;
      full_buffer = true;
     }
    adc_flag_conversion = true;
  }
}

void setup() {
  Serial.begin(115200);
  setupWns();
//  for(int i=0;i<N;i++){
//    X[i].img=0.0;
//    X[i].re=1.0;
//  }
//  Serial.println("fft");
//  fftfreq(&X[0],&Wn[0],N);
//  for(int i=0;i<N;i++){
//    Serial.print(X[i].re,5);
//    Serial.print("+");
//    Serial.print(X[i].img,5);
//    Serial.println("j");
//  }
//  orderFFT(&X[0],N);
  PMC->PMC_PCER0 |= PMC_PCER0_PID12;                    // PIOB power ON
  PIOB->PIO_OER |= PIO_OER_P27;
  PIOB->PIO_OWER |= PIO_OWER_P27;                       // Built In LED output write enable
  
  adc_setup();
  tc_setup();
  t1=micros();
}

void loop() {
  if (adc_flag_conversion)
  {
    PIOB->PIO_ODSR ^= PIO_ODSR_P27; //Toggle LED
    adc_flag_conversion = false;
   }
  if (full_buffer)
  {
    PMC->PMC_PCDR1 |= 0x00000020;                       // ADC power off
    t2=micros();
    tmuestras=t2-t1;
    
    //envio de las muestras en tiempo
    //Serial.println("Muestras");
    t1=micros();
    for (int i= 0; i < N; i++){
      Serial.println(adc_meas_buffer[i]);
    }
    t2=micros();
    tenvmuestras=t2-t1;
    
    for(int i=0;i<N;i++){
      X[i].re=float(adc_meas_buffer[i]);
      X[i].img=0.0;
    }
    //inicio calculo FFT
    t1=micros();
    fftfreq(&X[0],&Wn[0],N);
    orderFFT(&X[0],N);
    t2=micros();
    tfft=t2-t1;
    
    //inicio envio del resultado de la fft
    Serial.println("fft");
    t1=micros();
    for(int i=0;i<N;i++){
      Serial.print(X[i].re,5);Serial.print(" + ");Serial.print(X[i].img,5);Serial.println("i ");
    }
    t2=micros();
    tenvfft=t2-t1;
    
    //Inicio calculo magnitud
    t1=micros();
    for(int i=0;i<N;i++){
       mag[i]=Mag(X[i]);
    }
    t2=micros();
    tmag=t2-t1;

    //Inicio envio de la magnitud
    Serial.println("Mag");
    t1=micros();
    for(int i=0;i<N;i++){
      Serial.println(mag[i],5);
    }
    t2=micros();
    tenvmag=t2-t1;

    //Inicio calculo de fase
    t1=micros();
    for(int i=0;i<N;i++){
      phase[i]=Phase(X[i]);
    }
    t2=micros();
    tphase=t2-t1;

    //Inicio envio fase
    Serial.println("Phase");
    t1=micros();
    for(int i=0;i<N;i++){
      Serial.println(phase[i],5);
    }
    t2=micros();
    tenvphase=t2-t1;
    
    Serial.print("Tiempo muestras: ");
    Serial.println(tmuestras);
    Serial.print("Tiempo de envio de las muestras: ");
    Serial.println(tenvmuestras);
    Serial.print("Tiempo de la FFT: ");
    Serial.println(tfft);
    Serial.print("Tiempo envio de la FFT: ");
    Serial.println(tenvfft);
    Serial.print("Tiempo calculo de magnitud: ");
    Serial.println(tmag);
    Serial.print("Tiempo envio de magnitud: ");
    Serial.println(tenvmag);
    Serial.print("Tiempo calculo de fase: ");
    Serial.println(tphase);
    Serial.print("Tiempo envio de fase: ");
    Serial.println(tenvphase);
    full_buffer = false; 
  }

}

void fftfreq(Complex *X, Complex *W, int N){
  Complex fac,op1,op2;
  Complex *apw;
  int indW=1;
  int l = N/2; 
  while(l > 0) {
    apw = W;
    for(int j = 0; j < l; j++){
      fac= *apw;
      for(int i = j ; i < N ; i = i + 2*l){
        op1 = cSum(X[i],X[i+l]);
        op2 = cDiff(X[i],X[i+l]);
        X[i] = op1;
        X[i+l] = cMult(op2,fac);
      }
      apw += indW;
    }
    indW = indW * 2;
    l=l/2;
  }
}

void orderFFT(Complex *X,int N){
  int j = 0;
  Complex aux;
  for(int i = 1; i < (N-1); i++){
    int k = N/2;
    while(k <= j){
      j -= k;
      k /= 2;
    }
    j+=k;
    if(i < j){
      aux = X[j];
      X[j]=X[i];
      X[i]=aux;
    }
  }
}

Complex cMult(Complex num1, Complex num2){
  Complex res;
  res.re = (num1.re)*(num2.re) - (num1.img)*(num2.img);
  res.img = (num1.re)*(num2.img) + (num1.img)*(num2.img);
  return res;
}

Complex cSum(Complex num1, Complex num2){
  Complex res;
  res.re = (num1.re) + (num2.re);
  res.img = (num1.img) + (num2.img);
  return res;
}

Complex cDiff(Complex num1, Complex num2){
  Complex res;
  res.re = (num1.re) - (num2.re);
  res.img = (num1.img) - (num2.img);
  return res;
}

float Phase(Complex num){
  double res=atan2(double(num.img),double(num.re));
  return float(res);
}
float Mag(Complex num){
  double res=sqrt(pow(double(num.re),double(2))+pow(double(num.img),double(2)));
  return float(res);
}
