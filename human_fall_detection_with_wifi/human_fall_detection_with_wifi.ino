#include "Wire.h" 
#include <TridentTD_LineNotify.h>

#define SSID        "ToeiKanta"
#define PASSWORD    "88888888"
#define LINE_TOKEN  "vrOrMAwNggawTn6FIt67bWUq3I2SfF3BKH2QOn7YTIa"

const int MPU_addr=0x68;  // I2C address of the MPU-6050
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
float ax=0, ay=0, az=0, gx=0, gy=0, gz=0;
int LED_pin = 23;
int BTN_pin = 19;
int PulseSensorPurplePin = 15;  
int Signal;    

//int data[STORE_SIZE][5]; //array for saving past data
//byte currentIndex=0; //stores current data array index (0-255)
boolean fall = false; //stores if a fall has occurred
boolean trigger1=false; //stores if first trigger (lower threshold) has occurred
boolean trigger2=false; //stores if second trigger (upper threshold) has occurred
boolean trigger3=false; //stores if third trigger (orientation change) has occurred

byte trigger1count=0; //stores the counts past since trigger 1 was set true
byte trigger2count=0; //stores the counts past since trigger 2 was set true
byte trigger3count=0; //stores the counts past since trigger 3 was set true
int angleChange=0;

//=====line======
int alertTime = 6000; //miliSec before Alert to Line
int timeCountMax = alertTime/40;
int timeCountAlert = 0;

void setup(){
 Wire.begin();
 Wire.beginTransmission(MPU_addr);
 Wire.write(0x6B);  // PWR_MGMT_1 register
 Wire.write(0);     // set to zero (wakes up the MPU-6050)
 Wire.endTransmission(true);
 Serial.begin(9600);
  pinMode(BTN_pin,INPUT_PULLUP);
 pinMode(LED_pin, OUTPUT);
 digitalWrite(LED_pin, HIGH);
  //================Line connect ====================
  Serial.println(LINE.getVersion());
  
  WiFi.begin(SSID, PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
    digitalWrite(LED_pin, LOW);
    delay(100);
    digitalWrite(LED_pin, HIGH);
    delay(100);
  }
  
  //Serial.printf("\nWiFi connected\nIP : ");
  Serial.println(WiFi.localIP());  

  LINE.setToken(LINE_TOKEN);
}
void loop(){
  timeCountAlert = 0;
  while(digitalRead(BTN_pin) == LOW){
    timeCountAlert++;
    delay(1);
    if(timeCountAlert>=3000){
      digitalWrite(LED_pin, LOW); 
      delay(300);
      digitalWrite(LED_pin, HIGH);
      delay(300);
      digitalWrite(LED_pin, LOW); 
      delay(300);
      digitalWrite(LED_pin, HIGH);
      delay(300);
      LINE.notify("=================================");
      LINE.notify("ผู้ป่วยหมายเลข 1123 ต้องการความช่วยเหลือ รีบเข้าไปช่วยเหลือ หรือติดต่อ ด่วน !!!");
      LINE.notify("ผู้ป่วยพักอยู่ที่ห้อง 11 ชั้น 3 อาคารพักผู้ป่วย 1");
      LINE.notifyPicture("https://i.imgur.com/RZc8l13.jpg");
      timeCountAlert = -10000;
    }
  }
  timeCountAlert = 0;

  //PULSE SIGNAL
  //Signal = analogRead(PulseSensorPurplePin);  
  //Serial.println(Signal);  
  //delay(10);

  
  while(fall == true){
    timeCountAlert++;
    digitalWrite(LED_pin, LOW);
    if(timeCountAlert>=timeCountMax){
      LINE.notify("=================================");
      LINE.notify("ผู้ป่วยหมายเลข 1123 กำลังหกล้ม รีบเข้าไปช่วยเหลือ หรือติดต่อ ด่วน !!!");
      LINE.notify("ผู้ป่วยพักอยู่ที่ห้อง 11 ชั้น 3 อาคารพักผู้ป่วย 1");
      LINE.notifyPicture("https://i.imgur.com/RZc8l13.jpg");
      timeCountAlert = -100000;
    }
    if(digitalRead(BTN_pin) == LOW){
      fall = false;
      timeCountAlert = 0;
    }
    digitalWrite(LED_pin, LOW); 
    delay(20);
    digitalWrite(LED_pin, HIGH);
    delay(20);
  }
 mpu_read();
 //2050, 77, 1947 are values for calibration of accelerometer
 // values may be different for you
 ax = (AcX-2050)/16384.00;
 ay = (AcY-77)/16384.00;
 az = (AcZ-1947)/16384.00;

 //270, 351, 136 for gyroscope
 gx = (GyX+270)/131.07;
 gy = (GyY-351)/131.07;
 gz = (GyZ+136)/131.07;

 // calculating Amplitute vactor for 3 axis
 float Raw_AM = pow(pow(ax,2)+pow(ay,2)+pow(az,2),0.5);
 int AM = Raw_AM * 10;  // as values are within 0 to 1, I multiplied 
                        // it by for using if else conditions 
 Serial.print("AM");
 Serial.println(AM);
 //Serial.println(PM);
 //delay(500);

 if (trigger3==true){
    trigger3count++;
    Serial.println(trigger3count);
    if (trigger3count>=10){//3
       angleChange = pow(pow(gx,2)+pow(gy,2)+pow(gz,2),0.5);
       //delay(10);
       Serial.println(angleChange); 
       if ((angleChange>=0) && (angleChange<=10)){ //if orientation changes remains between 0-10 degrees
           fall=true; trigger3=false; trigger3count=0;
           Serial.println(angleChange);
             }
       else{ //user regained normal orientation
          trigger3=false; trigger3count=0;
          Serial.println("TRIGGER 3 DEACTIVATED");
       }
     }
  }
 if (fall==true){ //in event of a fall detection
   Serial.println("FALL DETECTED");
   

   //fall=false;
  // exit(1);
   }
 if (trigger2count>=6){ //allow 0.5s for orientation change
   trigger2=false; trigger2count=0;
   Serial.println("TRIGGER 2 DECACTIVATED");
   }
 if (trigger1count>=6){ //allow 0.5s for AM to break upper threshold
   trigger1=false; trigger1count=0;
   Serial.println("TRIGGER 1 DECACTIVATED");
   }
 if (trigger2==true){
   trigger2count++;
   //angleChange=acos(((double)x*(double)bx+(double)y*(double)by+(double)z*(double)bz)/(double)AM/(double)BM);
   angleChange = pow(pow(gx,2)+pow(gy,2)+pow(gz,2),0.5); Serial.println(angleChange);
   if (angleChange>=10 && angleChange<=400){ //if orientation changes by between 80-100 degrees
     trigger3=true; trigger2=false; trigger2count=0;//30
     Serial.println(angleChange);
     Serial.println("TRIGGER 3 ACTIVATED");
       }
   }
 if (trigger1==true){
   trigger1count++;
   if (AM>=12){ //if AM breaks upper threshold (3g)
     trigger2=true;
     Serial.println("TRIGGER 2 ACTIVATED");
     trigger1=false; trigger1count=0;
     }
   }
 if (AM<=5 && trigger2==false){ //if AM breaks lower threshold (0.4g)
   trigger1=true;//4 //6
   Serial.println("TRIGGER 1 ACTIVATED");
   }
//It appears that delay is needed in order not to clog the port
 delay(50);
}

void mpu_read(){
 Wire.beginTransmission(MPU_addr);
 Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
 Wire.endTransmission(false);
 Wire.requestFrom(MPU_addr,14,true);  // request a total of 14 registers
 AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
 AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
 AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
 Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
 GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
 GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
 GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
 }
