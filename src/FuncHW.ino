/*
 * LCD RS pin to digital pin 2
 * LCD E  pin to digital pin 4
 * LCD D4 pin to digital pin 8
 * LCD D5 pin to digital pin 7
 * LCD D6 pin to digital pin 6
 * LCD D7 pin to digital pin 5
 * LCD Contraste digital pin 9
 * Boton UP   A0
 * Boton DOWN A1
 * Boton Prog A2
 * Boton OK   A3
 * Out1       A4
 * Out2       A5
 * Out3       A6
 * Out4       A7
 * Servo digital pin 3
 * CS digital pin 10 (Max31856)
 * Mosi digital pin 11
 * Miso digital pin 12
 * Sck digital pin 13
 */


unsigned char CR0_INIT;
unsigned char CR1_INIT;
unsigned char MASK_INIT;

void Init_HW(){
pinMode(9,OUTPUT);
pinMode(3,OUTPUT);
pinMode(A4,OUTPUT);
pinMode(A5,OUTPUT);
pinMode(A6,OUTPUT);
pinMode(A7,OUTPUT);
pinMode(A0,INPUT);
pinMode(A1,INPUT);
pinMode(A2,INPUT);
pinMode(A3,INPUT);



  Themocouple = new MAX31856(11, 12, 10, 13);
  setTC_Type();

  myPID.SetOutputLimits(0, 255);
  myPID.SetSampleTime(500);
  myPID.SetMode(AUTOMATIC);
  myPID.SetTunings(Current_Config.KP, Current_Config.KI, Current_Config.KD);  
}

void setTC_Type(){
  CR0_INIT = (CR0_AUTOMATIC_CONVERSION + CR0_OPEN_CIRCUIT_FAULT_TYPE_K /* + CR0_NOISE_FILTER_50HZ */); 
  CR1_INIT = (CR1_AVERAGE_2_SAMPLES + Current_Config.TC_TYPE);
  MASK_INIT = (~(MASK_VOLTAGE_UNDER_OVER_FAULT + MASK_THERMOCOUPLE_OPEN_FAULT));

  Themocouple->writeRegister(REGISTER_CR0, CR0_INIT);
  Themocouple->writeRegister(REGISTER_CR1, CR1_INIT);
  Themocouple->writeRegister(REGISTER_MASK, MASK_INIT);
}

void readTC(){
     Temperature = Themocouple->readThermocouple(CELSIUS);
     Input = Temperature;
}

void turnON(){
    myPID.SetMode(AUTOMATIC);
}

void turnOFF(){
    profileStatus = 0;
    myPID.SetMode(MANUAL);
    Output = 0;
}

void setProcessTime(){
  profileStartTime = millis();
}

unsigned long getProcessTime(){
  return ((millis()-profileStartTime)/1000);
}

