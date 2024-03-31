 #include <LabMovEsp32.h>
#include <FS.h>
#include <SD_MMC.h>
#include "BluetoothSerial.h"

String device_name = "LabMovil";

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT;

BME680C Sensor;
LiquidCrystal_I2C lcd(0x27,20,4);
RTC_DS3231 rtc;

char daysOfTheWeek[7][12] = {"Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado"};
char  DatoBlue;
String HoraActual = "";
String FechaAnt = "";
String NombreDoc = "";
bool Memory = 1;
bool MenuMemory = 1;
bool MenuInicia = 1;
uint8_t LimpiarLCD = 0;
bool LimpiaMemoria = 0;
String DataModeSelect_;
uint32_t DataModeSelect;
String DataSensor;
float DataSensorF;
float TemperaturaN[15];
int NumDataTemp = 0;
String NumData_;
int NumData;
String Lectura = "";
char Carac;
String ValorLuz = "";
String ValorVol = "";
String ValorCor = "";
String ValorRes = "";
String ValorCap = "";
String ValorCapN = "";
String ValorDist = "";

void Inicializacion();
void TiempoActual();
void MemoriaSD();
void DatosAmbiente();
void LCDvalores();
void LecturaSerial();
void SelecDatos();
void ImprimirMenu();
void LimpiarMemoria();

void setup() {
  Serial.begin(38400);
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  lcd.clear();
  SerialBT.begin(device_name); //Bluetooth device name
  if (!SD_MMC.begin()) {
    Memory = 0;
  }
  while(!Sensor.begin()){
  }
  Sensor.setParam();
  
  if (!rtc.begin()) {
    
  }
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))+240);
  }
}

void loop() {
  TiempoActual();
  if(SerialBT.available()){
    DatoBlue = SerialBT.read();
  }
  if(Serial.available()>0){
    if (MenuMemory == 1){
      Inicializacion();
    }
    else{
      LecturaSerial();
      if(MenuInicia == 1){
        lcd.clear();
        Inicializacion();
      }
    }
  }
}


//------------------- Funciones ----------------//

void Inicializacion(){
  
  if(MenuInicia == 1){
    char flecha = (char)127;
    lcd.setCursor(0,0);
    lcd.print("Laboratorio Movil V1");
    delay(3);
    lcd.setCursor(0,1);
    lcd.print("                    ");
    delay(3);
    lcd.setCursor(0,2);
    lcd.print("        Menu        ");
    lcd.setCursor(0,3);
    lcd.print(flecha);
    lcd.print(" B1            B2 ");
    lcd.print('~');
    LimpiarMemoria();
  }
  
  Lectura = "";
  while(Serial.available() == 0){}
  while(Serial.available()>0){
    Carac = Serial.read();
    Lectura += String(Carac);
    delay(3);
  }
  DataModeSelect_ = "";
  for( int i = 0; i <= Lectura.length(); i++){
    if(Lectura.charAt(i) == '~'){  // Caracter de separacion de datos
      DataModeSelect = DataModeSelect_.toInt();
      delay(1);
      DataModeSelect_ = "";
      MenuInicia = 0;
      ImprimirMenu();
    }
    if(Lectura.charAt(i) == '`'){   // Caracter de menu
      MenuInicia = 1;
      MenuMemory = 1;
      return;
    }
    if(Lectura.charAt(i) == '@'){    // Caracter de menu/inicio de medicion
      MenuMemory = 0;
      MenuInicia = 0;
      lcd.clear();
      return;
    }
    DataModeSelect_ += String(Lectura.charAt(i));
  }
  return;
}
void LimpiarMemoria(){
  ValorLuz = "";
  ValorVol = "";
  ValorCor = "";
  ValorRes = "";
  ValorCap = "";
  ValorCapN = "";
  ValorDist = "";
  NumDataTemp = 1;
  return;
}

void ImprimirMenu(){
  bool limpiaP = 0;
  lcd.setCursor(0,1);
  delay(2);
  switch(DataModeSelect){
    case 0:
      lcd.print("    Temperatura     ");
      limpiaP = 1;
      break;
    case 1:
      lcd.print("     Luxometro      ");
      limpiaP = 1;
      break;
    case 2:
      lcd.print("      Voltaje       ");
      break;
    case 3:
      lcd.print("    Resistencia     ");
      limpiaP = 1;
      break;
    case 4:
      lcd.print("    Capacitancia    ");
      limpiaP = 1;
      break;
    case 5:
      lcd.print("     Distancia      ");
      limpiaP = 1;
      break;
    case 6:
      lcd.print("     Corriente      ");
      limpiaP = 1;
      break;
    case 7:
      lcd.print("   de componentes   ");
      delay(2);
      lcd.setCursor(0,0);
      delay(2);
      lcd.print("   Medidor (ESR)    ");
      delay(2);
      lcd.setCursor(0,2);
      delay(2);
      lcd.print(" Reiniciar con B_1  ");
      break;
    case 8:
      lcd.print(" Datos ambientales  ");
      limpiaP = 1;
      break;
    default:
      break;
  }
  if(limpiaP == 1){
    delay(2);
    lcd.setCursor(0,0);
    lcd.print("                    ");
    delay(2);
    lcd.setCursor(0,2);
    lcd.print("                    ");
    limpiaP = 0;
  }
  delay(2);
  lcd.setCursor(0,3);
  delay(2);
  lcd.print("   Presione Modo    ");
  return;
}
void TiempoActual(){
  DateTime now = rtc.now();
  HoraActual = String(now.hour());
  HoraActual += String(':');
  if(now.minute() >= 10){
    HoraActual += String(now.minute());
  }
  else{
    HoraActual += String(0);
    HoraActual += String(now.minute());
  }
  HoraActual += String(':');
  if(now.second() >= 10){
    HoraActual += String(now.second());
  }
  else{
    HoraActual += String(0);
    HoraActual += String(now.second());
  }
  delay(5);
  String Fecha = String(now.day());
  Fecha += String('-');
  Fecha += String(now.month());
  Fecha += String('-');
  Fecha += String(now.year());
  if(!(Fecha.equals(FechaAnt))){
    FechaAnt = String(Fecha);
    NombreDoc = String('/');
    NombreDoc += String(Fecha);
    NombreDoc += String(".txt");
  }
}

void MemoriaSD(){
  if(Memory){
    uint8_t Nuevo = 0;
    if(!(SD_MMC.exists(NombreDoc))){
      Nuevo = 1;
    }
    File file = SD_MMC.open(NombreDoc, FILE_APPEND);
    if (!file) {
      Serial.println("Fallo en la apertura del archivo");
      return;
    }
    if(Nuevo){
      file.println("Hora,TA(°C),P(hPa),H(%),L(lx),V(v),I(A),R(Ω),C,(pF/nF/uF),D(cm),T#(°C);...");
    }
    file.print(HoraActual);
    file.print(",");
    file.print(String(Sensor.SensorValor(0), 2));
    file.print(",");
    file.print(String(Sensor.SensorValor(1)/100, 2));
    file.print(",");
    file.print(String(Sensor.SensorValor(2), 1));
    file.print(",");
    file.print(ValorLuz);
    file.print(",");
    file.print(ValorVol);
    file.print(",");
    file.print(ValorCor);
    file.print(",");
    file.print(ValorRes);
    file.print(",");
    file.print(ValorCap);
    file.print(",");
    file.print(ValorCapN);
    file.print(",");
    file.print(ValorDist);
    for(int num = 1; num <= NumDataTemp ; num++ ){
      file.print(",");
      file.print(String(TemperaturaN[num],2));
    }
    file.println("");
    file.close();
    delay(3); 
    lcd.setCursor(13,0);
    lcd.print("Memoria");
  }
  else{
    lcd.setCursor(10,0);
    lcd.print("No Memoria");
    if (SD_MMC.begin()) {
      delay(3);
      lcd.setCursor(3,2);
      lcd.println("Memoria encontrada");
      Memory = 1;
      delay(2000);
      lcd.clear();
      return;
    }
    else{
      lcd.setCursor(10,0);
      lcd.print("No Memoria");
      Memory = 0;
      return;
    }
  }
}

void LecturaSerial(){
  Lectura = "";
  Carac;
  uint8_t contador = 0;
  uint8_t bandera = 0;
  uint8_t EstadoESR = 0;
  DataModeSelect_ = "";
  DataSensor = "";
  NumData_ = "";
  while(Serial.available()>0){
    Carac = Serial.read();
    Lectura += String(Carac);
    delay(3);
  }
  for( int i = 0; i <= Lectura.length(); i++){
    if(Lectura.charAt(i) == '~'){
      if(Lectura.charAt(i+1) == '@'){
        lcd.setCursor(0,0);
        lcd.print(HoraActual);
        delay(2);
        Sensor.readSensors();
        DatosAmbiente();
        MemoriaSD();
        return;
      }
      bandera = 13;
    }
    if(Lectura.charAt(i) == '`'){
      MenuMemory = 1;
      MenuInicia = 1;
      return;
    }
    switch(bandera){
      case 0:
        DataModeSelect_ += String(Lectura.charAt(i));
        EstadoESR = 0;
        break;
      case 1:
        DataSensor += String(Lectura.charAt(i));
        EstadoESR = 0;
        break;
      case 2:
        NumData_ += String(Lectura.charAt(i));
        EstadoESR = 0;
        break;
      case 3:
        DataModeSelect = DataModeSelect_.toInt();
        NumData = NumData_.toInt();
        DataSensorF = DataSensor.toFloat();  
        SelecDatos();
        LCDvalores();
        contador = 0;
        DataModeSelect_ = "";
        DataSensor = "";
        NumData_ = "";
        EstadoESR = 0;
        break;
      default:
        EstadoESR++;
        if(EstadoESR == 2){
          uint8_t n = 0;
          lcd.clear();
          lcd.setCursor(0,0);
          delay(2);
          i++;
          for(i; i <= (Lectura.length() -1) ; i++){
            if(Lectura.charAt(i) == '\n'){
              n++;
              lcd.setCursor(0,n);
              delay(2);
            }
            else if(Lectura.charAt(i) == '\r');
            else{
              lcd.print(Lectura.charAt(i));
              delay(1);
            }
          }
          Lectura = "";
          delay(10);
          return;
        }
        contador++;
        bandera = contador;
        break;
    }
  }
  Sensor.readSensors();
  MemoriaSD();
  return;
}

void SelecDatos(){
  switch(DataModeSelect){
    case 0:
      TemperaturaN[NumData] = DataSensorF; 
      break;
    case 1:
      ValorLuz = String(DataSensor);
      break;
    case 2:
      ValorVol = String(DataSensor);
      break;
    case 3:
      ValorRes = String(DataSensor);
      break;
    case 4:
      ValorCap = String(DataSensor);
      ValorCapN = String(NumData_);
      break;
    case 5:
      ValorDist = String(DataSensor);
      break;
    case 6:
      ValorCor = String(DataSensor);
      break;
    default:
      break;
  }
}

void LCDvalores(){
  if(LimpiarLCD == 50){
    LimpiarLCD = 0;
    lcd.clear();
  }
  LimpiarLCD++;
  int NumDataI;
  lcd.setCursor(0,2);
  switch(DataModeSelect){
    case 0:
      NumDataTemp = max(NumData,NumDataTemp);
      for(int num = 1; num <= NumDataTemp ; num++ ){
        SerialBT.print("Temp ");
        SerialBT.print(num);
        SerialBT.print(" :");
        SerialBT.print(TemperaturaN[num]);
        SerialBT.println(" C");
      }
      delay(3);
      NumDataI = min(NumData,3);
      for(int num = 1; num <= NumDataI ; num++ ){
        lcd.setCursor(0,num);
        delay(2);
        lcd.print(" Temp ");
        lcd.print(num);
        lcd.print(" :");
        lcd.print(TemperaturaN[num]);
        lcd.print(" C  ");
      }
      break;
    case 1:
      SerialBT.print("Luz: ");
      SerialBT.print(ValorLuz);
      SerialBT.println(" lx");
      delay(3);
      lcd.print("  Luz: ");
      lcd.print(ValorLuz);
      lcd.print(" lx  ");
      break;
    case 2:
      SerialBT.print("Voltaje: ");
      SerialBT.print(ValorVol);
      SerialBT.println(" V");
      delay(3);
      lcd.print("Voltaje: ");
      lcd.print(ValorVol);      
      lcd.print(" V  ");
      break;
    case 3:
      SerialBT.print("Resistencia: ");
      delay(3);
      lcd.print("Resist: ");
      if (DataSensorF <= 1000) {
        lcd.print(DataSensorF, 2);
        lcd.print(" R  ");
        delay(3);
        SerialBT.print(DataSensorF, 2);
        SerialBT.println(" R");
      }
      else if (DataSensorF > 1001 && DataSensorF <= 1000000) {
        lcd.print(DataSensorF / 1000.0, 2);
        lcd.print(" KR  ");
        delay(3);
        SerialBT.print(DataSensorF / 1000.0, 2);
        SerialBT.println(" KR");
      }
      else if (DataSensorF > 1000001) {
        lcd.print(DataSensorF / 1000000.0, 2);
        lcd.print(" MR  ");
        delay(3);
        SerialBT.print(DataSensorF / 1000000.0, 2);
        SerialBT.println(" MR");
      }
      break;
    case 4:
      SerialBT.print("Capaciatencia: ");
      delay(3);
      lcd.print("  Cap: ");
      delay(1);
      if (NumData == 1) {
        lcd.print(DataSensor);
        lcd.print(" pF  ");
        delay(3);
        SerialBT.print(DataSensor);
        SerialBT.println(" pF");
      }
      else if (NumData == 2) {
        lcd.print(DataSensor);
        lcd.print(" nF  ");
        delay(3);
        SerialBT.print(DataSensor);
        SerialBT.println(" nF");
      }
      else if (NumData == 3) {
        lcd.print(DataSensor);
        lcd.print(" uF  ");
        delay(3);
        SerialBT.print(DataSensor);
        SerialBT.println(" uF");
      }
      break;
    case 5:
      SerialBT.print("  Distancia: ");
      SerialBT.print(ValorDist);
      SerialBT.println(" cm");
      delay(3);
      lcd.print("Dist: ");
      lcd.print(ValorDist);
      lcd.print(" cm  ");
      break;
    
    case 6:
      SerialBT.print("Corriente: ");
      SerialBT.print(ValorCor);
      SerialBT.println(" A");
      delay(3);
      lcd.print("Corriente: ");
      lcd.print(ValorCor);
      lcd.print(" A  ");
      break;
    default:
      break;
  }
}

void DatosAmbiente(){
  lcd.setCursor(0,1);
  lcd.print("Temp= "); 
  lcd.print( Sensor.SensorValor(0),2); 
  lcd.print(" C");     // degrees Celsius
  lcd.setCursor(0,2);
  lcd.print("Pres= "); 
  lcd.print(Sensor.SensorValor(1)/100,2);
  lcd.print(" hPa");   // hPa
  lcd.setCursor(0,3);
  lcd.print("Hum= "); 
  lcd.print(Sensor.SensorValor(2), 1);  
  lcd.print(" %");     // Relative Humidity (%)
}
