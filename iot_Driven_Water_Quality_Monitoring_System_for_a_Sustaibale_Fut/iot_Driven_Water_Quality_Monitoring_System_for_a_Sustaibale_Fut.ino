#include <OneWire.h>
#include <DallasTemperature.h>

#include <LiquidCrystal.h>
#include <stdio.h>

#include <WiFi.h>
#include <HTTPClient.h>

LiquidCrystal lcd(13, 12, 14, 27, 26, 25);

char phonenumber[11]="##########"; //Phone number

HTTPClient http;

//set up your username and password

const char *ssid = "username";
const char *password = "password";

String field1 = "&s1=";
String field2 = "&s2=";
String field3 = "&s3=";

const char* serverName = "http://api.thingspeak.com/update";
String apiKey = "R2OLINILTI78DW30"; //API Key

const int oneWireBus = 23;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

#define RXD2 16
#define TXD2 17

int tempc=0,ti=0,phsv=0,temp=0;
unsigned char phs[10],var[10];
unsigned char r1;
char phmsg[50];
int tdsv=0;

void serialFlush()
{
  while(Serial.available() > 0)
  {
    Serial.read();
  }
}

void things_send()
{
  lcd.setCursor(15,1);
  lcd.print("T");
  HTTPClient http;
  http.begin(serverName);
  String httpRequestData = "api_key=" + apiKey + "&field1=" + String(tempc) + "&field2=" + String(phsv) + "&field3=" + String(tdsv);          
  int httpResponseCode = http.POST(httpRequestData);
  http.end();
  delay(2000);
 lcd.setCursor(15,1);
 lcd.print(" ");
}

void okcheck()
{
  unsigned char rc;
  do{
      rc = Serial2.read();
    }while(rc != 'K');
}

int sts1=0,sts2=0,sts3=0,sts4=0;
int cnt=0;

void gsm_send(String strs1)
{
  lcd.setCursor(15,0);
  lcd.print("G");
  delay(4000);
  delay(4000);
  delay(4000);  
  delay(4000);
             Serial2.write("AT+CMGS=\"");
             Serial2.write(phonenumber);
             Serial2.write("\"\r\n");
             delay(3000);
             Serial2.print(strs1);
             Serial2.write(0x1A);    
             delay(4000);
             delay(4000);  
             delay(4000);  
  lcd.setCursor(15,0);
  lcd.print(" ");
}
void gsm_call()
{
  lcd.setCursor(15,0);
  lcd.print("C");
    delay(10000);
    Serial2.print("ATD");
    Serial2.print(phonenumber);
    Serial2.println(";");
    delay(30000);
 lcd.setCursor(15,0);
 lcd.print(" ");
}
         
void setup()
{
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  lcd.begin(16, 2);  
  lcd.print(" Water Quality");
  lcd.setCursor(0,1);
  lcd.print("   Monitoring");
  delay(2500);
 
  sensors.begin();
 
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED)
       {
           delay(500);
       }
  lcd.clear();
  lcd.print("WIFI Connected");
  delay(3000);

  gsminit();
 
  lcd.clear();
  lcd.print("PH:");  
  lcd.setCursor(8,0);
  lcd.print("T:");  
  lcd.setCursor(0,1);
  lcd.print("TDS:");  
}

void loop()
{    
 readSerial1(phmsg);
     phmsg[49]='\0';
         ti=0;
         do{
            r1 = phmsg[ti];
             ti++;      
           }while(r1 != 'H');

              phs[0] =  phmsg[ti+1];
              phs[1] =  phmsg[ti+2];
              phs[2]='\0';

          if(phmsg[ti+2] == '.')
            {
             phsv  = (phs[0]-48);
            }
          else
            {
             phsv  = (((phs[0]-48)*10) + (phs[1]-48));  
            }

             if(phsv >= 15)
               {
                phsv=15;
               }
           
           ti=0;
     
        do{
           r1 = phmsg[ti];
             ti++;  
          }while(r1 != 'W');

          if(phmsg[ti+1] == ' ')
            {
              var[0]='0';
            }
          else
            {
              var[0]=phmsg[ti+1];  
            }
          var[1]=phmsg[ti+2];


          temp = (((var[0]-48)*10) + (var[1]-48));

       memset(phmsg,'\0',50);

       if(phsv >= 15)
       {
        phsv=0;
       }
       lcd.setCursor(3,0);
       convertl(phsv);    
       if(phsv <= 0)
         {
          lcd.setCursor(10,1);
          lcd.print("    ");
         }
       if(phsv >= 1 && phsv < 6)
         {
          lcd.setCursor(10,1);
          lcd.print("Acid");
          sts1++;
          sts2=0;
          if(sts1 >= 2)
          {
            sts1 = 2;
          }
          if(sts1 == 1)
            {
             things_send();
           
             String msg_string="";
             msg_string = "LOW_PH:" + String(phsv);
             gsm_send(msg_string);
           
             gsm_call();        
             serialFlush();
            }
         }
       if(phsv > 6 && phsv < 10)
         {
          lcd.setCursor(10,1);
          lcd.print("Nrml");  
         }
       if(phsv > 10)
         {
          lcd.setCursor(10,1);
          lcd.print("Base");
          sts2++;
          sts1=0;
          if(sts2 >= 2)
          {
            sts2 = 2;
          }
          if(sts2 == 1)
            {
             things_send();
           
             String msg_string="";
             msg_string = "HIGH_PH:" + String(phsv);
             gsm_send(msg_string);
           
             gsm_call();  
             serialFlush();    
            }
         }
         delay(800);

      sensors.requestTemperatures();
      tempc = sensors.getTempCByIndex(0);
      lcd.setCursor(10,0);
      convertl(tempc);

      if(tempc > 38)
        {
          sts4++;
          if(sts4 >= 2)
          {
            sts4=2;
          }
          if(sts4 == 1)
            {
              things_send();
           
             String msg_string="";
             msg_string = "High_Temp:" + String(tempc);
             gsm_send(msg_string);
           
             gsm_call();
             serialFlush();
            }
        }
      else
        {
          sts4=0;  
        }

      tdsv = analogRead(35);
      tdsv = (tdsv/4.2);
      lcd.setCursor(4,1);
      convertl(tdsv);
      if(tdsv > 350)
        {
          sts3++;
          if(sts3 >= 2)
          {
            sts3=2;
          }
          if(sts3 == 1)
            {
              things_send();
           
             String msg_string="";
             msg_string = "High_TDS:" + String(tdsv);
             gsm_send(msg_string);
           
             gsm_call();
             serialFlush();
            }
        }
      else
        {
          sts3=0;  
        }
      cnt++;
     
      if(cnt >= 40)
        {
          cnt=0;
            things_send();

          lcd.setCursor(15,0);
          lcd.write('G');
            delay(4000);  
            delay(4000);
            delay(4000);
            delay(4000);
          Serial2.write("AT+CMGS=\"");
          Serial2.write(phonenumber);
          Serial2.write("\"\r\n");
          delay(3000);
          Serial2.write("Temp:");
          Serial2.print(tempc);
          Serial2.write(" PH:");
          Serial2.print(phsv);
          Serial2.write(" TDS:");
          Serial2.print(tdsv);
          Serial2.write(0x1A);    
          delay(4000);  
          delay(4000);  
 
          serialFlush();
          lcd.setCursor(15,0);
          lcd.write(' ');
        }
}


int readSerial1(char result[])  
{
  int i = 0,sts1=0;
  while (1)
  {
    while (Serial.available() > 0)
    {
      char inChar = Serial.read();
      if (inChar == 'L')
         {
          result[i] = '\0';
          sts1=0;
          i=0;
          Serial.flush();
          return 0;
         }
      if (inChar == 'P')
         {
          sts1=1;
         }
       if(sts1 == 1)
         {  
          result[i] = inChar;
          i++;
         }
    }
  }
}


void convertl(unsigned int value)
{
  unsigned int a,b,c,d,e,f,g,h;

      a=value/10000;
      b=value%10000;
      c=b/1000;
      d=b%1000;
      e=d/100;
      f=d%100;
      g=f/10;
      h=f%10;

      a=a|0x30;              
      c=c|0x30;
      e=e|0x30;
      g=g|0x30;              
      h=h|0x30;
   
     
   lcd.write(c);
   lcd.write(e);
   lcd.write(g);
   lcd.write(h);
}


void gsminit()
{
  Serial2.write("AT\r\n");                  
  okcheck();
  Serial2.write("ATE0\r\n");                
  okcheck();
  Serial2.write("AT+CMGF=1\r\n");            
  okcheck();
  Serial2.write("AT+CNMI=1,2,0,0\r\n");      
  okcheck();
  Serial2.write("AT+CSMP=17,167,0,0\r\n");  
  okcheck();
   
  lcd.clear();
  lcd.print(phonenumber);
  phonenumber[10]='\0';

    delay(4000);  
    delay(4000);
    Serial2.write("AT+CMGS=\"");
    Serial2.write(phonenumber);
    Serial2.write("\"\r\n");
    delay(3000);
    Serial2.write("GSM Init\r\n");
    Serial2.write(0x1A);    
    delay(4000);  
    delay(4000);  
}
