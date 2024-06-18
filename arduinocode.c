#include <dht.h>

#include <LiquidCrystal.h>
#include <stdio.h>

#include <Wire.h>
#include "dht.h"

#include <Servo.h>
Servo myservo;  

LiquidCrystal lcd(6, 7, 5, 4, 3, 2);

#include <SoftwareSerial.h>
SoftwareSerial mySerial(8,9);

#define dht_apin A0
dht DHT;

int mic     = 13;
int mos     = 11;
int relay   = 10;
int buzzer  = A1;


char pastnumber[11];

int pos=0;
int cnt=0;
int tempc=0,humc=0;

int memsx=0,memsy=0;

int stsa=0,stsb=0;

int sti=0;
String inputString = "******";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete


void okcheck()
{
  unsigned char rcr;
  do{
      rcr = Serial.read();
    }while(rcr != 'K');
}

void gsm_send(String strs)
{
  delay(4000);delay(4000);
    Serial.write("AT+CMGS=\"");
    Serial.write(pastnumber);
    Serial.write("\"\r\n"); delay(3000);
    Serial.print(strs);
    Serial.write(0x1A);    
    delay(4000);delay(4000);delay(4000);  

}


char res[130];

void serialFlush()
{
  while(Serial.available() > 0) 
  {
    char t = Serial.read();
  }
} 

void myserialFlush()
{
  while(mySerial.available() > 0) 
  {
    char t = mySerial.read();
  }
} 


char check(char* ex,int timeout)
{
  int i=0;
  int j = 0,k=0;
  while (1)
  {
    sl:
    if(mySerial.available() > 0)
    {
      res[i] = mySerial.read();
      if(res[i] == 0x0a || res[i]=='>' || i == 100)
      {
        i++;
        res[i] = 0;break;
      }
      i++;
    }
    j++;
    if(j == 30000)
    {
      k++;
      j = 0;  
    }
    if(k > timeout)
    {
      return 1;
     }
  }
  if(!strncmp(ex,res,strlen(ex)))
  {
    return 0;
   }
  else
  {
    i=0;
    goto sl;
   }
} 

char rcv,ipaddress[20];

char buff[200],k=0;
void upload(unsigned int s1,unsigned int s2,unsigned int s3);
char readserver(void);
void clearserver(void);

const char* ssid = "iotserver";
const char* password = "iotserver123";

void swing()
{
  for(pos=10;pos<180;pos++)
     {
      myservo.write(pos);delay(2);
     }
     delay(900);
  for(pos=180;pos>10;pos--)
     {
      myservo.write(pos);delay(2);
     }
     delay(900);
  for(pos=10;pos<180;pos++)
     {
      myservo.write(pos);delay(2);
     }
     delay(900);
  for(pos=180;pos>10;pos--)
     {
      myservo.write(pos);delay(2);
     }
     delay(900);
  digitalWrite(11, LOW);   
}
void beep()
{
  digitalWrite(buzzer, LOW);delay(2000);digitalWrite(buzzer, HIGH);
}
void setup() 
{
 Serial.begin(1200);   //serialEvent();
 mySerial.begin(9600);
 
    myservo.attach(12);
    myservo.write(10);
    
    pinMode(buzzer, OUTPUT);
    pinMode(mic, INPUT);
    pinMode(mos, INPUT);
    pinMode(relay, OUTPUT);
    
    digitalWrite(buzzer, HIGH);
    digitalWrite(relay, LOW);

 // serialEvent();
    
  //63.  IOT based advanced smart cradle for baby monitoring system  
  lcd.begin(16, 2);lcd.cursor();
  lcd.print("IOT Based Advanced");
  lcd.setCursor(0,1);
  lcd.print("Smart Cradle For");
  delay(2000);
  lcd.clear();
  lcd.print(" Baby Monitoirng");
  lcd.setCursor(0,1);
  lcd.print("     System     ");
  delay(2000);

  
/*
  do{
     rcv = Serial.read();
    }while(rcv != '*');
     readSerial(ipaddress);
  delay(7000);
  
  lcd.clear();
  lcd.print(ipaddress);
  delay(5000);
*/
    
   wifiinit();

   gsminit();
  
   lcd.clear();
   lcd.setCursor(0,0);
   lcd.print("T:");//2,0
   lcd.setCursor(8,0);
   lcd.print("H:");//11,0
   digitalWrite(11, LOW);
}

int sts1=0,sts2=0;
char urines[20]="Baby-No_Urine";
char cryings[20]="Baby-No_Crying";
char tilts[20]="Baby-No_Tilt";

void loop() 
{
  
    DHT.read11(dht_apin);

    tempc = DHT.temperature;
    humc  = DHT.humidity;

    if(tempc >= 101 || humc >= 101)
      {
        ;
      }
    else
      {  
        lcd.setCursor(2,0);convertl(tempc);
        lcd.setCursor(10,0);convertl(humc);
      }
      
    if(tempc > 43)
      {
        stsa++;
        if(stsa >= 5)
          {
           lcd.setCursor(15,1);lcd.write('S');
           upload(tempc,humc,urines,cryings,tilts);
           String gsm_msg = "";
           gsm_msg = "High_Temp:" + String(tempc);
           gsm_send(gsm_msg);
           lcd.setCursor(15,1);lcd.write(' ');
          }  
      }
    if(tempc < 43)
      {
        stsa=0;   
      }
      
    if(humc > 70)
      {
        stsb++;
        if(stsb >= 5)
          {
           lcd.setCursor(15,1);lcd.write('S');
           upload(tempc,humc,urines,cryings,tilts);
           String gsm_msg = "";
           gsm_msg = "High_High:" + String(humc);
           gsm_send(gsm_msg);
           lcd.setCursor(15,1);lcd.write(' ');
          }
      }
    if(humc < 70)
      {
        stsb=0;  
      }
      
    memset(urines,'\0',strlen(urines));
    if(digitalRead(mos) == LOW)
      {
       lcd.setCursor(0,1);lcd.print("Urine "); 
       strcpy(urines,"Baby-Urine");
       digitalWrite(buzzer, LOW);
       sts1++;
       if(sts1 >= 2){sts1=2;}
       if(sts1 == 1)
         {
          lcd.setCursor(15,1);lcd.write('S');
          upload(tempc,humc,urines,cryings,tilts);
          String gsm_msg = "";
          gsm_msg = "Baby-Urine";
          gsm_send(gsm_msg);
          lcd.setCursor(15,1);lcd.write(' ');
         } 
      }
    if(digitalRead(mos) == HIGH)
      {
       lcd.setCursor(0,1);lcd.print("      "); 
        digitalWrite(buzzer, HIGH);
        sts1=0;
        strcpy(urines,"Baby-No_Urine");
      }

    memset(cryings,'\0',strlen(cryings));
    if(digitalRead(mic) == LOW)
      {
       lcd.setCursor(8,1);lcd.print("Crying   "); 
       strcpy(cryings,"Baby-Crying");
       swing(); 
       lcd.setCursor(15,1);lcd.write('S');
       upload(tempc,humc,urines,cryings,tilts);
          String gsm_msg = "";
          gsm_msg = "Baby-Crying";
          gsm_send(gsm_msg);
       lcd.setCursor(15,1);lcd.write(' ');   
       lcd.setCursor(8,1);lcd.print("         "); 
      }
    if(digitalRead(mic) == HIGH)
      {
       lcd.setCursor(8,1);lcd.print("         "); 
       strcpy(cryings,"Baby-No_Crying");
      }    
   
   
   memsx = analogRead(A4);delay(100);   
   memsy = analogRead(A5);delay(100);

  //lcd.setCursor(0,1);convertl(memsx);
  //lcd.setCursor(8,1);convertl(memsy);
  

   memset(tilts,'\0',strlen(tilts));
   if((memsx > 290 && memsx < 390) && (memsy > 290 && memsy < 390))
     {
      lcd.setCursor(8,1);lcd.print("         ");
       strcpy(tilts,"Baby-No_Tilt");
       digitalWrite(relay, LOW);
       sts2=0;
     }
   else 
     {       
       lcd.setCursor(8,1);lcd.print("Baby Tilt");
       strcpy(tilts,"Baby-Tilt");
       digitalWrite(relay, HIGH);
       sts2++;
       if(sts2 >= 2){sts2=2;}
       if(sts2 == 1)
         {
          lcd.setCursor(15,1);lcd.write('S');
          upload(tempc,humc,urines,cryings,tilts);
          String gsm_msg = "";
          gsm_msg = "Baby-Tilt";
          gsm_send(gsm_msg);
          lcd.setCursor(15,1);lcd.write(' ');
         }  
       //lcd.setCursor(8,1);lcd.print("         ");
     }
}


void serialEvent() 
{
   while(Serial.available()) 
        {         
         char inChar = (char)Serial.read();
          if(inChar == '*')
            {sti=1;
               inputString="";
            }
          if(sti == 1)
            {
                inputString += inChar;
            }
          if(inChar == '#')
            {sti=0;
              stringComplete = true;      
            }
        }
}

int readSerial(char result[]) 
{
  int i = 0;
  while (1) 
  {
    while (Serial.available() > 0) 
    {
      char inChar = Serial.read();
      if (inChar == '\n') 
         {
          result[i] = '\0';
          Serial.flush();
          return 0;
         }
      if (inChar != '\r') 
         {
          result[i] = inChar;
          i++;
         }
    }
  }
}

void gsminit()
{
  Serial.write("AT\r\n");                   okcheck();
  Serial.write("ATE0\r\n");                 okcheck();
  Serial.write("AT+CMGF=1\r\n");            okcheck();
  Serial.write("AT+CNMI=1,2,0,0\r\n");      okcheck();
  Serial.write("AT+CSMP=17,167,0,0\r\n");   okcheck();
    
  lcd.clear();
  lcd.print("SEND MSG STORE");
  lcd.setCursor(0,1);
  lcd.print("MOBILE NUMBER");  
  do{
     rcv = Serial.read();
    }while(rcv != '*');
     readSerial(pastnumber);
     pastnumber[10]='\0';
     
  lcd.clear();
  lcd.print(pastnumber);

    Serial.write("AT+CMGS=\"");
    Serial.write(pastnumber);
    Serial.write("\"\r\n"); delay(3000);
    Serial.write("Mobile no. registered\r\n");
    Serial.write(0x1A);    
    delay(4000);  delay(4000);  
}

char bf2[50];
void upload(int s1,int s2,const char *s3,const char *s4,const char *s5)
{
  delay(2000);
  lcd.setCursor(15, 1);lcd.print("U");
  myserialFlush();
  mySerial.println("AT+CIPSTART=4,\"TCP\",\"projectsfactoryserver.in\",80");
  
      delay(8000);
      memset(buff,0,strlen(buff));
      sprintf(buff,"GET http://projectsfactoryserver.in/storedata.php?name=iot864&s1=%u&s2=%u&s3=%s&s4=%s&s5=%s\r\n\r\n",s1,s2,s3,s4,s5);
         
      myserialFlush();
      sprintf(bf2,"AT+CIPSEND=4,%u",strlen(buff));
      mySerial.println(bf2);    
         delay(5000);             
      myserialFlush();
      mySerial.print(buff);   
         delay(2000);        
      mySerial.println("AT+CIPCLOSE");
      lcd.setCursor(15, 1);lcd.print(" ");  
}

char readserver(void)
{
  char t;
  delay(2000);
  lcd.setCursor(15, 1);lcd.print("R");
  myserialFlush();
  mySerial.println("AT+CIPSTART=4,\"TCP\",\"projectsfactoryserver.in\",80");

      delay(8000);
      memset(buff,0,strlen(buff));
      sprintf(buff,"GET http://projectsfactoryserver.in/last.php?name=iot4L\r\n\r\n");
      myserialFlush();
      sprintf(bf2,"AT+CIPSEND=4,%u",strlen(buff));
      mySerial.println(bf2);
      
         delay(5000);
                    
          myserialFlush();
          mySerial.print(buff);
          
        while(1)
        {
           while(!mySerial.available());
            t = mySerial.read();
            if(t == '*' || t == '#')
            {
              if(t == '#')return 0;
              while(!mySerial.available());
               t = mySerial.read();
               delay(1000);
                myserialFlush();
               return t;
            }
        }
              delay(2000);
              
       mySerial.println("AT+CIPCLOSE");
       lcd.setCursor(15, 1);lcd.print(" "); 
       delay(2000);  
return t;
}

void clearserver(void)
{
  delay(2000);
  lcd.setCursor(15, 1);lcd.print("C");
  myserialFlush();
  mySerial.println("AT+CIPSTART=4,\"TCP\",\"projectsfactoryserver.in\",80");

      delay(8000);
      memset(buff,0,strlen(buff));
      sprintf(buff,"GET http://projectsfactoryserver.in/storedata.php?name=iot4&s10=0\r\n\r\n");
      myserialFlush();
      sprintf(bf2,"AT+CIPSEND=4,%u",strlen(buff));
      mySerial.println(bf2);
      
         delay(5000);
                 
          myserialFlush();
          mySerial.print(buff);
          
          delay(2000);
          myserialFlush();
                    
       mySerial.println("AT+CIPCLOSE");
       lcd.setCursor(15, 1);lcd.print(" "); 
       delay(2000);  
}


void wifiinit()
{
 char ret;  
  st:
  mySerial.println("ATE0");
  ret  = check((char*)"OK",50);
  mySerial.println("AT");
  ret  = check((char*)"OK",50);
  if(ret != 0)
  {
    delay(1000);
   goto st; 
  }
  
     lcd.clear();lcd.setCursor(0, 0);lcd.print("CONNECTING");  
  mySerial.println("AT+CWMODE=1");
   ret  = check((char*)"OK",50);
 cagain:
    
  myserialFlush();
  mySerial.print("AT+CWJAP=\"");
  mySerial.print(ssid);
  mySerial.print("\",\"");
  mySerial.print(password);
  mySerial.println("\"");
  if(check((char*)"OK",300))goto cagain;    
  mySerial.println("AT+CIPMUX=1");
  delay(1000);

  lcd.clear();lcd.setCursor(0, 0);lcd.print("WIFI READY"); 
}

void converts(unsigned int value)
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
    
     
   Serial.write(a);
   Serial.write(c);
   Serial.write(e); 
   Serial.write(g);
   Serial.write(h);
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
    
     
   lcd.write(a);
   lcd.write(c);
   lcd.write(e); 
   lcd.write(g);
   lcd.write(h);
}
