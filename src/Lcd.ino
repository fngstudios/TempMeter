byte customChar[8] = {0b01100,0b10010,0b10010,0b01100,0b00000,0b00000,0b00000,0b00000};



void LCD_Init(){
lcd.begin(16, 4);  
lcd.createChar(1, customChar); 
setContraste();
  
}

void setContraste(){
  analogWrite(9,(100-Current_Config.CONTRASTE));  
}



void LCD_SetCursor(char forced){
  static char last_Pos = 20;
  if(forced){ last_Pos = 20;}
  static char page = 0;
  switch (Estado){
    
    case 0:
            if (Cur_Pos != last_Pos){
            lcd.setCursor(0,last_Pos);
            lcd.print(' ');
            last_Pos = Cur_Pos;
            lcd.setCursor(0,Cur_Pos);
            lcd.write(126);
            }
            break;
   case 3:    
              if (Cur_Pos != last_Pos){
                last_Pos = Cur_Pos;
                lcd.clear();
               if (Cur_Pos == 2){
                 page = 1;
               }
               if (Cur_Pos == 0){
                 page = 0;
               }
               if (page == 0){
                 lcd.setCursor(1,0);
                 lcd.print(Current_Config.PROFILES[0].name);

                 lcd.setCursor(1,1);
                 lcd.print(Current_Config.PROFILES[1].name);
               }
               if (page == 1){
                 lcd.setCursor(1,0);
                 lcd.print(Current_Config.PROFILES[1].name);

                 lcd.setCursor(1,1);
                 lcd.print(Current_Config.PROFILES[2].name);
                 
               }
                 lcd.setCursor(0,Cur_Pos-page);
                            lcd.write(126);
            }
           break;
    case 7:    //Oven Config
               if (Cur_Pos != last_Pos){
                           last_Pos = Cur_Pos;  
                 switch (Cur_Pos){
                   
                   case 0:        lcd.setCursor(0,1);
                                  lcd.print("                ");
                                  lcd.setCursor(0,1);
                                  lcd.print("Contraste: ");
                                  break;

                   case 1:        lcd.setCursor(0,1);
                                  lcd.print("                ");
                                  lcd.setCursor(0,1);
                                  lcd.print("TC TYPE: ");
                                  break;

                   case 2:        lcd.setCursor(0,1);
                                  lcd.print("                ");
                                  lcd.setCursor(0,1);
                                  lcd.print("KP: ");
                                  break;

                   case 3:        lcd.setCursor(0,1);
                                  lcd.print("                ");
                                  lcd.setCursor(0,1);
                                  lcd.print("KI: ");
                                  break;

                   case 4:        lcd.setCursor(0,1);
                                  lcd.print("                ");
                                  lcd.setCursor(0,1);
                                  lcd.print("KD: ");
                                  break;
                 }
               }
                break;          
         }
}

void refreshLCD(){
static unsigned char last_estado = 50;  
if (Estado != last_estado){
  last_estado = Estado;
  switch(Estado){
    case 0:      lcd.clear();      //MainMenu
                 lcd.setCursor(1,0);
                 lcd.print("BAKE");
                 LCD_SetCursor(1);
                 lcd.setCursor(1,1);
                 lcd.print("REFLOW");
                 break;

    case 1:      lcd.clear();        //Bake Menu
                 lcd.setCursor(1,0);
                 lcd.print("BAKE Menu");
                 break;
    
    case 2:      lcd.clear();
                 lcd.setCursor(0,0);
                 lcd.print("Baking!");
                 break;
    
    case 3:      
                  lcd.clear();      //ReflowMenu
                 lcd.setCursor(1,0);
                 lcd.print(Current_Config.PROFILES[0].name);
                 lcd.setCursor(1,1);
                 lcd.print(Current_Config.PROFILES[1].name);
                 LCD_SetCursor(1);
                 break;
    case 6:      lcd.clear();
                 lcd.setCursor(0,0);                  
                 break;
 

    case 7:      lcd.clear();
                 lcd.print("Oven Config");
                 LCD_SetCursor(1);
                 break;

}
}
}





void LCD_Update(){
  
  switch(Estado){
    
    case 0:        LCD_SetCursor(0);
                   break;
    case 1:        lcd.setCursor(1,1);
                   lcd.print("T:");
                   lcd.print(Bake_Setpoint);
                   lcd.write(1);
                   break;
                   
   case 2:         lcd.setCursor(10,0);
                   lcd.print("O:");
                   temp = String(showOutput());
                   if (temp.length() == 1){temp = "00"+temp;}else if(temp.length() == 2){temp = "0"+temp;}
                   lcd.print(temp);
                   lcd.print('%');
                   lcd.setCursor(0,1);
                   lcd.print("T:");
                   temp = String(int(Temperature));
                   if (temp.length() == 1){temp = "00"+temp;}else if(temp.length() == 2){temp = "0"+temp;}
                   lcd.print(temp);
                   lcd.write(1);
                   lcd.print(" >> S:");
                   temp = String(int(setPoint));
                   if (temp.length() == 1){temp = "00"+temp;}else if(temp.length() == 2){temp = "0"+temp;}
                   lcd.print(temp);
                   lcd.write(1);
                 break;

   case 3:        LCD_SetCursor(0);
                  break;
  
  case 6:         lcd.setCursor(0,0);
                  lcd.print(profileStatusName[profileStatus]);
                  lcd.setCursor(10,0);
                   lcd.print("O:");
                   temp = String(showOutput());
                   if (temp.length() == 1){temp = "00"+temp;}else if(temp.length() == 2){temp = "0"+temp;}
                   lcd.print(temp);
                   lcd.print('%');
                   lcd.setCursor(0,1);
                   lcd.print("T:");
                   temp = String(int(Temperature));
                   if (temp.length() == 1){temp = "00"+temp;}else if(temp.length() == 2){temp = "0"+temp;}
                   lcd.print(temp);
                   lcd.write(1);
                   lcd.print(" >> S:");
                   temp = String(int(setPoint));
                   if (temp.length() == 1){temp = "00"+temp;}else if(temp.length() == 2){temp = "0"+temp;}
                   lcd.print(temp);
                   lcd.write(1);
                 break;
                   
  case 7:         LCD_SetCursor(0);
                  break;
 
 
 
  
  case 8:        switch (Cur_Pos){
                      case 0:         //Contraste
                                      lcd.setCursor(10,1);
                                      lcd.print(temp_Value,0);
                                      break;    
                      case 1:         //TC_TYPE
                                      lcd.setCursor(10,1);
                                      lcd.print(TC_Desc[int(temp_Value)]);
                                      break;      
                      case 2:         //KP
                                      lcd.setCursor(4,1);
                                      lcd.print(temp_Value,0);
                                      lcd.print("  ");
                                      break;    
                      case 3:         //KD
                                      lcd.setCursor(4,1);
                                      lcd.print(temp_Value,3);
                                      lcd.print("  ");
                                      break;    
                      case 4:         //KI
                                      lcd.setCursor(4,1);
                                      lcd.print(temp_Value,0);
                                      lcd.print("  ");
                                      break;

                   }
  } 
  
}


int showOutput(){
  return (float(Output/255))*100;
}


