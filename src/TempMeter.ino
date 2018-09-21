/*
    SolderStation v0.1
    fngStudios
    25/08/2016
    Ezequiel Pedace
    
    


 */
//-------------------------------------------------------------------------------------
//    Includes

#include <MAX31856.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <Button.h>
#include <PID_v1.h>
//------------------------------------------------------------------------------------- 
//    Defines

#define PROFILE_NUMBER     3
#define BAKE_MIN           50
#define BAKE_MAX           250
#define LONG_PRESS         1000
#define DEBOUNCE_MS         25
#define DEFAULT_TC_TYPE     3
#define MAX_CONTRAST        100
#define MIN_CONTRAST        30
#define DEFAULT_CONTRAST    60
#define MAX_KP              100
#define MIN_KP              2
#define DEFAULT_KP          10
#define MAX_KI              0.1
#define MIN_KI              0.001
#define DEFAULT_KI          0.001
#define MAX_KD              30
#define MIN_KD              0
#define DEFAULT_KD          5
#define VALID_CONFIG        'i'
#define CONFIG_EEPROM_ADDR  20
//------------------------------------------------------------------------------------- 
//    Variables de programa

const char profileStatusName[5][10] = {"NORMALIZE","PREHEAT  ","SOAK     ","PEAK     ","COOL     "};
char TC_Desc[9]="BEJKNRST";
unsigned int Bake_Setpoint = 90;
float Temperature;
double Output;
double Input;
double  setPoint;
unsigned char Estado = 0;  //0:Main_Menu 1:Bake_Menu 2:Baking 3:Reflow_Menu 4:Reflow_Config 5: Reflow_Config_Parameter 6:Reflowing 7:Oven_Config 8:Oven_Config_Parameter
unsigned char Menu_Page = 0;
String temp;
char Cur_Pos = 0;
double temp_Value = 0;
unsigned long reflowTime;
unsigned long profileStartTime = 0;
unsigned char profileStatus = 0;
unsigned char currentProfile = 0;
//------------------------------------------------------------------------------------- 
//   Estructura de configuracion
 
 
 struct Reflow_Phase_T {
   unsigned char TEMPERATURE;
   unsigned char TIME;
 };
 struct Reflow_Profile_T {
   char name[10];
   Reflow_Phase_T PHASE[5]; // Normalize,Preheat,Soak,Peak,Cool
   unsigned char TIME_TOL;
   unsigned char TEMPERATURE_TOL;
 };
  
 struct Configuration_T {
  Reflow_Profile_T PROFILES[PROFILE_NUMBER]; 
  unsigned char CONTRASTE;
  unsigned char TC_TYPE;
  double KP;
  double KI;
  double KD;
  byte VALID; // Ultimo, para comprobaciones de integridad.
} Current_Config;

 
//------------------------------------------------------------------------------------- 
//    Constructores

LiquidCrystal lcd(2, 4, 8, 7, 6, 5);
MAX31856 *Themocouple;
Button btnProg(A2, false, true, DEBOUNCE_MS);
Button btnOk(A3, false, true, DEBOUNCE_MS);
Button btnUp(A0, false, true, DEBOUNCE_MS);
Button btnDown(A1, false, true, DEBOUNCE_MS);
PID myPID(&Input, &Output, &setPoint, Current_Config.KP, Current_Config.KI, Current_Config.KD, DIRECT);
//-------------------------------------------------------------------------------------

void setup() {

    Serial.begin(9600);
    Serial.println("Reset!");
    loadConfig();
    Init_HW();
    LCD_Init();
    turnOFF();
}


void loop() {
  checkBtns();
  readTC();  
  doWork();  
  refreshLCD();
  LCD_Update();  
  myPID.Compute(); 
  doDebug();


}

void doDebug(){
  if (Serial.available()){
    char entrada = Serial.read();
    if (entrada == ' '){
        for (unsigned char i = 0;i < 3;i++){
    Serial.println(Current_Config.PROFILES[i].name);
    Serial.println(Current_Config.PROFILES[i].TEMPERATURE_TOL);
    Serial.println(Current_Config.PROFILES[i].TIME_TOL);
    for (char n = 0;n<5;n++){
      Serial.println(Current_Config.PROFILES[i].PHASE[n].TEMPERATURE);  //Normalize
     Serial.println(Current_Config.PROFILES[i].PHASE[n].TIME);
    }
  }
}
  }
}



void checkBtns(){
  
   btnProg.read();
   btnOk.read();
   btnDown.read();
   btnUp.read();
  
   
}



void doWork(){
  
  switch(Estado){
    case 0:        //Main_Menu
                   if (btnUp.wasPressed()){Cur_Pos--;if (Cur_Pos <0){Cur_Pos = 0;}}
                   if (btnDown.wasPressed()){Cur_Pos++;if (Cur_Pos >1){Cur_Pos = 1;}}
                   if (btnOk.wasPressed()){if (Cur_Pos ==0){Estado = 1;Cur_Pos = 0;}}
                   if (btnOk.wasPressed()){if (Cur_Pos ==1){Estado = 3;Cur_Pos = 0;}}
                   if (btnProg.pressedFor(LONG_PRESS)){Estado = 7;Cur_Pos = 0;}
                   
                   break;
                   
    case 1:        //Bake_Menu
                   if (btnUp.wasPressed()){Bake_Setpoint+=10;if (Bake_Setpoint > BAKE_MAX){Bake_Setpoint = BAKE_MAX;}}
                   if (btnDown.wasPressed()){Bake_Setpoint-=10;if (Bake_Setpoint < BAKE_MIN){Bake_Setpoint = BAKE_MIN;}}
                   if (btnProg.wasPressed()){Estado = 0;Cur_Pos = 0; }
                   if (btnOk.wasPressed()){Estado = 2;setPoint = Bake_Setpoint; }
                   break;

    case 2:        //Baking
                   if (btnProg.wasPressed()){Estado = 0; }
                   Input=Temperature;
                   break;
    
    case 3:        //Reflow_Menu
                   if (btnDown.wasPressed()){Cur_Pos++;if (Cur_Pos >2){Cur_Pos = 2;}}
                   if (btnUp.wasPressed()){Cur_Pos--;if (Cur_Pos <0){Cur_Pos = 0;}}
                   if (btnProg.wasPressed()){Estado = 0; Cur_Pos = 0;}
                   if (btnOk.wasPressed()){Estado = 6;currentProfile = Cur_Pos; turnON(); setProcessTime(); profileStatus = 0;}
                   break;                   

    case 4:        //Reflow_Config
    
    case 5:        //Reflow_Config_Parameter
    
    case 6:       //Reflowing!
                  setPoint = Current_Config.PROFILES[currentProfile].PHASE[profileStatus].TEMPERATURE;
                  if(getProcessTime() > Current_Config.PROFILES[currentProfile].PHASE[profileStatus].TIME)
                  { setProcessTime();
                    profileStatus++;
                    if (profileStatus > 4)
                    {Estado = 0;
                     turnOFF();
                     Cur_Pos = 0;
                    }
                  }
                  

    case 7:        //Oven_Config
                   if (btnDown.wasPressed()){Cur_Pos++;if (Cur_Pos >4){Cur_Pos = 4;}}
                   if (btnUp.wasPressed()){Cur_Pos--;if (Cur_Pos <0){Cur_Pos = 0;}}
                   if (btnOk.wasPressed()){Estado = 8;
                   switch (Cur_Pos){
                   
                     case 0:  temp_Value = Current_Config.CONTRASTE;
                     break;
                     case 1:  temp_Value = Current_Config.TC_TYPE;
                     break;
                     case 2:  temp_Value = Current_Config.KP;
                     break;
                     case 3:  temp_Value = Current_Config.KI;
                     break;
                     case 4:  temp_Value = Current_Config.KD;
                     break;
                   }                  
                   }
                   if (btnProg.wasPressed()){saveConfig();Estado = 0;Cur_Pos = 0;}
                   break;

    case 8:        //Oven_Config_Parameter
                   switch (Cur_Pos){
                      case 0:         //Contraste
                                       if (btnUp.wasPressed()){temp_Value+=5;if (temp_Value > MAX_CONTRAST){temp_Value = MAX_CONTRAST;}}
                                       if (btnDown.wasPressed()){temp_Value-=5;if (temp_Value < MIN_CONTRAST){temp_Value = MIN_CONTRAST;}}
                                       if (btnProg.wasPressed()){Estado = 7;}
                                       if (btnOk.wasPressed()){Estado = 7;Current_Config.CONTRASTE = temp_Value; setContraste(); }                                        
                                      break;    
                      case 1:         // TC TYPE
                                       if (btnUp.wasPressed()){temp_Value+=1;if (temp_Value > 7){temp_Value = 7;}}
                                       if (btnDown.wasPressed()){temp_Value-=1;if (temp_Value < 0){temp_Value =0;}}
                                       if (btnProg.wasPressed()){Estado = 7;}
                                       if (btnOk.wasPressed()){Estado = 7;Current_Config.TC_TYPE = temp_Value;setTC_Type(); }
                                      break;    
                      case 2:         //KP
                                       if (btnUp.wasPressed()){temp_Value+=1;if (temp_Value > MAX_KP){temp_Value = MAX_KP;}}
                                       if (btnDown.wasPressed()){temp_Value-=1;if (temp_Value < MIN_KP){temp_Value = MIN_KP;}}
                                       if (btnProg.wasPressed()){Estado = 7;}
                                       if (btnOk.wasPressed()){Estado = 7;Current_Config.KP = temp_Value;   myPID.SetTunings(Current_Config.KP, Current_Config.KI, Current_Config.KD); }
                                       break; 
                      case 3:         //KD
                                       if (btnUp.wasPressed()){temp_Value+=0.001;if (temp_Value > MAX_KI){temp_Value = MAX_KI;}}
                                       if (btnDown.wasPressed()){temp_Value-=0.001;if (temp_Value < MIN_KI){temp_Value = MIN_KI;}}
                                       if (btnProg.wasPressed()){Estado = 7;}
                                       if (btnOk.wasPressed()){Estado = 7;Current_Config.KI = temp_Value;   myPID.SetTunings(Current_Config.KP, Current_Config.KI, Current_Config.KD); }
                                       break; 
                      case 4:         //KI
                                       if (btnUp.wasPressed()){temp_Value+=1;if (temp_Value > MAX_KD){temp_Value = MAX_KD;}}
                                       if (btnDown.wasPressed()){temp_Value-=1;if (temp_Value < MIN_KD){temp_Value = MIN_KD;}}
                                       if (btnProg.wasPressed()){Estado = 7;}
                                       if (btnOk.wasPressed()){Estado = 7;Current_Config.KD = temp_Value;   myPID.SetTunings(Current_Config.KP, Current_Config.KI, Current_Config.KD); }
                                       break; 

                   }

                   break;

          
}
}




void loadConfig(){
  EEPROM.get(CONFIG_EEPROM_ADDR,Current_Config);
    if ( Current_Config.VALID != VALID_CONFIG){

      //Prof 0
      temp = "Leaded";
      temp.toCharArray(Current_Config.PROFILES[0].name,10);
      Current_Config.PROFILES[0].TEMPERATURE_TOL = 10;  
      Current_Config.PROFILES[0].TIME_TOL = 15;

      Current_Config.PROFILES[0].PHASE[0].TEMPERATURE = 80;  //Normalize
      Current_Config.PROFILES[0].PHASE[0].TIME = 100;

      Current_Config.PROFILES[0].PHASE[1].TEMPERATURE = 140;  //Preheat
      Current_Config.PROFILES[0].PHASE[1].TIME = 120;

      Current_Config.PROFILES[0].PHASE[2].TEMPERATURE = 180;  //Soak
      Current_Config.PROFILES[0].PHASE[2].TIME = 90;

      Current_Config.PROFILES[0].PHASE[3].TEMPERATURE = 225;  //Peak
      Current_Config.PROFILES[0].PHASE[3].TIME = 60;
      
      Current_Config.PROFILES[0].PHASE[4].TEMPERATURE = 20;  //Cool
      Current_Config.PROFILES[0].PHASE[4].TIME = 250;

//Prof 1
      strcpy(Current_Config.PROFILES[1].name,"Lead Free");
      Current_Config.PROFILES[1].TEMPERATURE_TOL = 10;  //Normalize
      Current_Config.PROFILES[1].TIME_TOL = 15;

      Current_Config.PROFILES[1].PHASE[0].TEMPERATURE = 80;  //Normalize
      Current_Config.PROFILES[1].PHASE[0].TIME = 90;

      Current_Config.PROFILES[1].PHASE[1].TEMPERATURE = 160;  //Preheat
      Current_Config.PROFILES[1].PHASE[1].TIME = 120;

      Current_Config.PROFILES[1].PHASE[2].TEMPERATURE = 210;  //Soak
      Current_Config.PROFILES[1].PHASE[2].TIME = 100;

      Current_Config.PROFILES[1].PHASE[3].TEMPERATURE = 250;  //Peak
      Current_Config.PROFILES[1].PHASE[3].TIME = 75;
      
      Current_Config.PROFILES[1].PHASE[4].TEMPERATURE = 20;  //Cool
      Current_Config.PROFILES[1].PHASE[4].TIME = 250;

//Prof 2
      strcpy(Current_Config.PROFILES[2].name,"Custom");
      Current_Config.PROFILES[2].TEMPERATURE_TOL = 10;  //Normalize
      Current_Config.PROFILES[2].TIME_TOL = 15;

      Current_Config.PROFILES[2].PHASE[0].TEMPERATURE = 80;  //Normalize
      Current_Config.PROFILES[2].PHASE[0].TIME = 10;

      Current_Config.PROFILES[2].PHASE[1].TEMPERATURE = 140;  //Preheat
      Current_Config.PROFILES[2].PHASE[1].TIME = 12;

      Current_Config.PROFILES[2].PHASE[2].TEMPERATURE = 180;  //Soak
      Current_Config.PROFILES[2].PHASE[2].TIME = 9;

      Current_Config.PROFILES[2].PHASE[3].TEMPERATURE = 225;  //Peak
      Current_Config.PROFILES[2].PHASE[3].TIME = 6;
      
      Current_Config.PROFILES[2].PHASE[4].TEMPERATURE = 20;  //Cool
      Current_Config.PROFILES[2].PHASE[4].TIME = 25;      
      
      Current_Config.CONTRASTE = DEFAULT_CONTRAST;
      Current_Config.TC_TYPE = DEFAULT_TC_TYPE;
      Current_Config.KP = DEFAULT_KP;
      Current_Config.KI = DEFAULT_KI;
      Current_Config.KD = DEFAULT_KD;
      Current_Config.VALID = VALID_CONFIG;

      EEPROM.put(CONFIG_EEPROM_ADDR,Current_Config);

    }
}

void saveConfig(){
      EEPROM.put(CONFIG_EEPROM_ADDR,Current_Config);
      myPID.SetTunings(Current_Config.KP, Current_Config.KI, Current_Config.KD);
      setTC_Type();

}  
