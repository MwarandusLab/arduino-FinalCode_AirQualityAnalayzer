#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <MQUnifiedsensor.h>
#include <Wire.h>
#include <SPI.h>
#include <SdFat.h>

#define placa "Arduino NANO"
#define Voltage_Resolution 5
#define pin A1                 
#define type "MQ-135"          
#define ADC_Bit_Resolution 10  

SdFat sd;
int Sms = 0;
int Call = 0;
int Buzzer = 5;
SdFile dataFile;
bool SendSms = false;

int _timeout;
String _buffer;
String number = "+254748613509"; //-> change with your number

int OzoneSensor = A2;
const int chipSelect = 10;
bool fileInitialized = false;
const float customRo = 59.0;
MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);
LiquidCrystal_I2C lcd(0x3F, 20, 4);
SoftwareSerial sim(2, 3); // TX, RX GSM Module
void setup() {
  Serial.begin(9600);  // Init the serial port communication - for debugging the library
  sim.begin(9600);
  _buffer.reserve(50);
  // pinMode(Buzzer, OUTPUT);
  digitalWrite(Buzzer, LOW);
  // Set math model to calculate the PPM concentration and the value of constants
  MQ135.setRegressionMethod(1);  // _PPM = a * ratio^b
  MQ135.init();
  MQ135.setR0(customRo);  // Set the custom Ro value
  Serial.println("Using custom Ro value for calibration.");
  Serial.print("Ro = ");
  Serial.print(MQ135.getR0());
  Serial.println(" Ohms");
  lcd.init();       // initialize the LCD
  lcd.backlight();  // Turn on the blacklight

  if (!sd.begin(chipSelect, SPI_FULL_SPEED)) {
    Serial.println("SD card initialization failed.");
    while (true)
      ;
  }
  Serial.println("SD Card Initialised");
  delay(500);
  // print a message.
  lcd.clear();
  lcd.setCursor(6, 1);
  lcd.print("SYSTEM");
  lcd.setCursor(2, 2);
  lcd.print("INITIALIZATION");
  delay(3000);

}
void loop() {
  MQ135.update();  

  MQ135.setA(605.18);
  MQ135.setB(-3.937);             
  float CO = MQ135.readSensor(); 

  MQ135.setA(77.255);
  MQ135.setB(-3.18);                  
  float Alcohol = MQ135.readSensor();  

  MQ135.setA(110.47);
  MQ135.setB(-2.862);              
  float CO2 = MQ135.readSensor();  

  MQ135.setA(44.947);
  MQ135.setB(-3.445);                 
  float Toluen = MQ135.readSensor();  

  MQ135.setA(102.2);
  MQ135.setB(-2.473);              
  float NH4 = MQ135.readSensor();  

  MQ135.setA(34.668);
  MQ135.setB(-3.369);                 
  float Aceton = MQ135.readSensor();  

  int OzoneValue = analogRead(OzoneSensor);
  String category = "Loading...";
  if (OzoneValue > 0 && OzoneValue < 350) {
    category = " Normal";
    digitalWrite(Buzzer, LOW);
    Sms = 0;
    Call = 0;
  } else if (OzoneValue > 350 && OzoneValue < 700 ) {
    category = " Medium";
    digitalWrite(Buzzer, LOW);
    if(Sms == 0){
      SendMessage();
      Sms = 1;
    }
    Sms = 0;
  } else if (OzoneValue > 700) {
    category = "High";
    digitalWrite(Buzzer, LOW);
    if(Call == 0){
      callNumber();
      Call = 1;
    }
  }

  logDataToSD(CO, Alcohol, CO2, Toluen, NH4, Aceton);
  delay(1000);

  if(CO > 2500){
    digitalWrite(Buzzer, HIGH);
    if(Call == 0){
      callNumber();
      Call = 1;
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("AIR QUALITY ANALYZER");
    lcd.setCursor(0, 1);
    lcd.print("MQ131: O3 ");
    lcd.setCursor(10, 1);
    lcd.print(category);
    lcd.setCursor(0, 2);
    lcd.print("MQ135: CO ");
    lcd.setCursor(10, 2);
    lcd.print(CO, 1);
    lcd.setCursor(0, 3);
    lcd.print("STATUS: CRITICAL");
    delay(1500);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("AIR QUALITY ANALYZER");
    lcd.setCursor(0, 1);
    lcd.print("MQ131: O3 ");
    lcd.setCursor(10, 1);
    lcd.print(category);
    lcd.setCursor(0, 2);
    lcd.print("MQ135: Alcohol ");
    lcd.setCursor(15, 2);
    lcd.print(Alcohol, 1);
    lcd.setCursor(0, 3);
    lcd.print("STATUS: CRITICAL");
    delay(1500);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("AIR QUALITY ANALYZER");
    lcd.setCursor(0, 1);
    lcd.print("MQ131: O3");
    lcd.setCursor(10, 1);
    lcd.print(category);
    lcd.setCursor(0, 2);
    lcd.print("MQ135: CO2 ");
    lcd.setCursor(11, 2);
    lcd.print(CO2 + 400, 1);
    lcd.setCursor(0, 3);
    lcd.print("STATUS: CRITICAL");
    delay(1500);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("AIR QUALITY ANALYZER");
    lcd.setCursor(0, 1);
    lcd.print("MQ131: O3");
    lcd.setCursor(10, 1);
    lcd.print(category);
    lcd.setCursor(0, 2);
    lcd.print("MQ135: Toluen ");
    lcd.setCursor(14, 2);
    lcd.print(Toluen, 1);
    lcd.setCursor(0, 3);
    lcd.print("STATUS: CRITICAL");
    delay(1500);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("AIR QUALITY ANALYZER");
    lcd.setCursor(0, 1);
    lcd.print("MQ131: O3");
    lcd.setCursor(10, 1);
    lcd.print(category);
    lcd.setCursor(0, 2);
    lcd.print("MQ135: NH4 ");
    lcd.setCursor(11, 2);
    lcd.print(NH4, 1);
    lcd.setCursor(0, 3);
    lcd.print("STATUS: CRITICAL");
    delay(1500);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("AIR QUALITY ANALYZER");
    lcd.setCursor(0, 1);
    lcd.print("MQ131: O3");
    lcd.setCursor(10, 1);
    lcd.print(category);
    lcd.setCursor(0, 2);
    lcd.print("MQ135: Aceton ");
    lcd.setCursor(14, 2);
    lcd.print(Aceton, 1);
    lcd.setCursor(0, 3);
    lcd.print("STATUS: CRITICAL");
    delay(1500);

  }else if (CO > 500 && CO < 1000) {
    digitalWrite(Buzzer, LOW);
    if(Sms == 0){
      SendMessage();
      Sms = 1;
    }
    Sms = 0;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("AIR QUALITY ANALYZER");
    lcd.setCursor(0, 1);
    lcd.print("MQ131: O3 ");
    lcd.setCursor(10, 1);
    lcd.print(category);
    lcd.setCursor(0, 2);
    lcd.print("MQ135: CO ");
    lcd.setCursor(10, 2);
    lcd.print(CO, 1);
    lcd.setCursor(0, 3);
    lcd.print("STATUS: MEDIUM");
    delay(1500);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("AIR QUALITY ANALYZER");
    lcd.setCursor(0, 1);
    lcd.print("MQ131: O3 ");
    lcd.setCursor(10, 1);
    lcd.print(category);
    lcd.setCursor(0, 2);
    lcd.print("MQ135: Alcohol ");
    lcd.setCursor(15, 2);
    lcd.print(Alcohol, 1);
    lcd.setCursor(0, 3);
    lcd.print("STATUS: MEDIUM");
    delay(1500);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("AIR QUALITY ANALYZER");
    lcd.setCursor(0, 1);
    lcd.print("MQ131: O3");
    lcd.setCursor(10, 1);
    lcd.print(category);
    lcd.setCursor(0, 2);
    lcd.print("MQ135: CO2 ");
    lcd.setCursor(11, 2);
    lcd.print(CO2 + 400, 1);
    lcd.setCursor(0, 3);
    lcd.print("STATUS: MEDIUM");
    delay(1500);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("AIR QUALITY ANALYZER");
    lcd.setCursor(0, 1);
    lcd.print("MQ131: O3");
    lcd.setCursor(10, 1);
    lcd.print(category);
    lcd.setCursor(0, 2);
    lcd.print("MQ135: Toluen ");
    lcd.setCursor(14, 2);
    lcd.print(Toluen, 1);
    lcd.setCursor(0, 3);
    lcd.print("STATUS: MEDIUM");
    delay(1500);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("AIR QUALITY ANALYZER");
    lcd.setCursor(0, 1);
    lcd.print("MQ131: O3");
    lcd.setCursor(10, 1);
    lcd.print(category);
    lcd.setCursor(0, 2);
    lcd.print("MQ135: NH4 ");
    lcd.setCursor(11, 2);
    lcd.print(NH4, 1);
    lcd.setCursor(0, 3);
    lcd.print("STATUS: MEDIUM");
    delay(1500);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("AIR QUALITY ANALYZER");
    lcd.setCursor(0, 1);
    lcd.print("MQ131: O3");
    lcd.setCursor(10, 1);
    lcd.print(category);
    lcd.setCursor(0, 2);
    lcd.print("MQ135: Aceton ");
    lcd.setCursor(14, 2);
    lcd.print(Aceton, 1);
    lcd.setCursor(0, 3);
    lcd.print("STATUS: MEDIUM");
    delay(1500);
  }else if( CO < 500) {
    Sms = 0;
    Call = 0;
    digitalWrite(Buzzer, LOW);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("AIR QUALITY ANALYZER");
    lcd.setCursor(0, 1);
    lcd.print("MQ131: O3 ");
    lcd.setCursor(10, 1);
    lcd.print(category);
    lcd.setCursor(0, 2);
    lcd.print("MQ135: CO ");
    lcd.setCursor(10, 2);
    lcd.print(CO, 1);
    lcd.setCursor(0, 3);
    lcd.print("STATUS: NORMAL");
    delay(1500);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("AIR QUALITY ANALYZER");
    lcd.setCursor(0, 1);
    lcd.print("MQ131: O3 ");
    lcd.setCursor(10, 1);
    lcd.print(category);
    lcd.setCursor(0, 2);
    lcd.print("MQ135: Alcohol ");
    lcd.setCursor(15, 2);
    lcd.print(Alcohol, 1);
    lcd.setCursor(0, 3);
    lcd.print("STATUS: NORMAL");
    delay(1500);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("AIR QUALITY ANALYZER");
    lcd.setCursor(0, 1);
    lcd.print("MQ131: O3");
    lcd.setCursor(10, 1);
    lcd.print(category);
    lcd.setCursor(0, 2);
    lcd.print("MQ135: CO2 ");
    lcd.setCursor(11, 2);
    lcd.print(CO2 + 400, 1);
    lcd.setCursor(0, 3);
    lcd.print("STATUS: NORMAL");
    delay(1500);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("AIR QUALITY ANALYZER");
    lcd.setCursor(0, 1);
    lcd.print("MQ131: O3");
    lcd.setCursor(10, 1);
    lcd.print(category);
    lcd.setCursor(0, 2);
    lcd.print("MQ135: Toluen ");
    lcd.setCursor(14, 2);
    lcd.print(Toluen, 1);
    lcd.setCursor(0, 3);
    lcd.print("STATUS: NORMAL");
    delay(1500);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("AIR QUALITY ANALYZER");
    lcd.setCursor(0, 1);
    lcd.print("MQ131: O3");
    lcd.setCursor(10, 1);
    lcd.print(category);
    lcd.setCursor(0, 2);
    lcd.print("MQ135: NH4 ");
    lcd.setCursor(11, 2);
    lcd.print(NH4, 1);
    lcd.setCursor(0, 3);
    lcd.print("STATUS: NORMAL");
    delay(1500);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("AIR QUALITY ANALYZER");
    lcd.setCursor(0, 1);
    lcd.print("MQ131: O3");
    lcd.setCursor(10, 1);
    lcd.print(category);
    lcd.setCursor(0, 2);
    lcd.print("MQ135: Aceton ");
    lcd.setCursor(14, 2);
    lcd.print(Aceton, 1);
    lcd.setCursor(0, 3);
    lcd.print("STATUS: NORMAL");
    delay(1500);
  }

  
}
void logDataToSD(float co, float alcohol, float co2, float toluen, float nh4, float aceton){
  digitalWrite(Buzzer, LOW);
  if (!fileInitialized) {
    if (dataFile.open("data.csv", O_CREAT | O_WRITE | O_APPEND)) {
      dataFile.println("CO,Alcohol,CO2,Toluen,NH4,Aceton");  // Write headers in the first row
      dataFile.close();
      fileInitialized = true;
    } else {
      Serial.println("Error opening data.csv");
      delay(500);
      return;
    }
  }

  if (dataFile.open("data.csv", O_WRITE | O_APPEND)) {
    String data = String(co, 2) + "," + String(alcohol, 2) + "," + String(co2, 2) + "," + String(toluen, 2) + "," + String(nh4, 2) + "," + String(aceton, 2);
    dataFile.println(data);
    dataFile.close();
    Serial.println("Data written to SD card: " + data);
    delay(500);
  } else {
    Serial.println("Error opening data.csv");
    delay(500);
  }
}
void SendMessage(){
  //Serial.println ("Sending Message");
  sim.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(200);
  //Serial.println ("Set SMS Number");
  sim.println("AT+CMGS=\"" + number + "\"\r"); //Mobile phone number to send message
  delay(200);
  String SMS = "Welcome to DIY Development Board";
  sim.println(SMS);
  delay(100);
  sim.println((char)26);// ASCII code of CTRL+Z
  delay(200);
  _buffer = _readSerial();
}
void callNumber() {
  sim.print (F("ATD"));
  sim.print (number);
  sim.print (F(";\r\n"));
  _buffer = _readSerial();
  Serial.println(_buffer);
}
String _readSerial() {
  _timeout = 0;
  while  (!sim.available() && _timeout < 12000  )
  {
    delay(13);
    _timeout++;
  }
  if (sim.available()) {
    return sim.readString();
  }
}