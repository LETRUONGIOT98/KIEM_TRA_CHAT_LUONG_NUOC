
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = "za5GHeb1dUn9NzGhigHIGhI7uJKyQ5u6";  //Thay đổi mã Blynk này khi thay đổi người dùng

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Thu Thao";   //Thay đổi tên wifi 
char pass[] = "12052002";      ///Thay đổi mật khẩu wifi
float gioihan = 100; //set giới hạn để kích thiết bị
#define van D7
int m =0;
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);

#define TdsSensorPin A0     ////////Chân nhận tín hiệu cảm biến
#define VREF 3.0      
#define SCOUNT  50           //  SỐ LẦN LẤY MẪU
int analogBuffer[SCOUNT];    // lưu trữ giá trị tương tự trong mảng, đọc từ ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0,copyIndex = 0;
float averageVoltage = 0,tdsValue = 0,temperature = 25;

void setup()
{
  // Debug console
  Serial.begin(9600);
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  pinMode(van, OUTPUT);
  digitalWrite(van, LOW);
  pinMode(TdsSensorPin,INPUT);
  delay(1000);  
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  lcd.print("  KET NOI WIFI ");
  lcd.setCursor(0, 1);
  Blynk.begin(auth, ssid, pass,"blynk-server.com",8080);
  Blynk.run();
  delay(1000);
  Blynk.virtualWrite(V1,0);
}
BLYNK_WRITE(V1)
{
  int Mode = param.asInt(); // assigning incoming value from pin V1 to a variable
  m = Mode;
}
BLYNK_WRITE(V2)
{ if(m == 1){
  int val = param.asInt(); // assigning incoming value from pin V1 to a variable
  digitalWrite(van, val);}
}
void loop()
{
  Blynk.run();

  static unsigned long analogSampleTimepoint = millis();
   if(millis()-analogSampleTimepoint > 40U)     //Đọc gái trị trong vòng 40s
   {
     analogSampleTimepoint = millis();
     analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
     analogBufferIndex++;
     if(analogBufferIndex == SCOUNT) 
         analogBufferIndex = 0;
   }   
   static unsigned long printTimepoint = millis();
   if(millis()-printTimepoint > 800U)
   {
      printTimepoint = millis();
      for(copyIndex=0;copyIndex<SCOUNT;copyIndex++)
        analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0; // đọc giá trị tương tự và lưu trữ vào bộ đệm
      float compensationCoefficient=1.0+0.02*(temperature-25.0);    //công thức bù nhiệt độ: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
      float compensationVolatge=averageVoltage/compensationCoefficient;  //sự cân bằng nhiệt độ
      tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; //chuyển đổi giá trị điện áp thành giá trị tds
      Serial.print("TDS Value:");
      Serial.print(tdsValue,0);
      Serial.println("ppm");
      tdsValue = tdsValue - 21.0;
      if(tdsValue <=0)tdsValue=0;
  lcd.setCursor(0, 1);
  lcd.print("TDS:         ppm");
  lcd.setCursor(6, 1);
  lcd.print(tdsValue,1);
  Blynk.virtualWrite(V0,tdsValue);
   }
   if(m==0){
    if(tdsValue >= gioihan){
      digitalWrite(van, LOW);
      lcd.setCursor(0, 0);
  lcd.print("BOM: TAT        ");
      Blynk.notify("CANH BAO: CHAT LUONG NUOC THAP");
       Blynk.virtualWrite(V2, digitalRead(van));
    }
    if(tdsValue < gioihan){
      digitalWrite(van, HIGH);
      lcd.setCursor(0, 0);
  lcd.print("BOM: BAT        ");
      Blynk.virtualWrite(V2, digitalRead(van));
    }
   }
}
int getMedianNum(int bArray[], int iFilterLen) 
{
      int bTab[iFilterLen];
      for (byte i = 0; i<iFilterLen; i++)
    bTab[i] = bArray[i];
      int i, j, bTemp;
      for (j = 0; j < iFilterLen - 1; j++) 
      {
    for (i = 0; i < iFilterLen - j - 1; i++) 
          {
      if (bTab[i] > bTab[i + 1]) 
            {
    bTemp = bTab[i];
          bTab[i] = bTab[i + 1];
    bTab[i + 1] = bTemp;
       }
    }
      }
      if ((iFilterLen & 1) > 0)
  bTemp = bTab[(iFilterLen - 1) / 2];
      else
  bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
      return bTemp;
}
