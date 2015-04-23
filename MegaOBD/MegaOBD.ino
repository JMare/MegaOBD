#define RxD 11
#define TxD 12
#define CmdPin 3
//#define StatePin 22
#define OBD_CMD_RETRIES 3
#define BT_CMD_RETRIES 5
#define RPM_CMD_RETRIES 5    //Number of retries for RPM command
//#include <Arduino.h>
//#include "HC05.h"
#include <SoftwareSerial.h>
#define SPD_CMD_RETRIES 5
#include <Timer.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);    
unsigned int rpm,rpm_to_disp;//Variables for RPM
boolean obdabort;
boolean btabort;
boolean spdabort = false;
boolean spd_retries;
unsigned int spdk = 13;
char hexspeed[2];
char bufin[15];
boolean rpmabort;      //Variable for RPM error
boolean rpm_retries;         //Variable for RPM cmd retries

SoftwareSerial btSerial(RxD, TxD);
Timer t;

void setup() {
   lcd.begin(16, 2);               // start the library
   lcd.setCursor(0,0);             // set the LCD cursor   position 
   lcd.print("Power ON");  // print a simple message on the LCD
  
   pinMode(RxD, INPUT);
   pinMode(TxD, OUTPUT);
   pinMode(CmdPin, OUTPUT);
  
  Serial.begin(38400);
  
  Serial.println("Serial connected");
  
  //run bluetooth setup untill a connection is made or 5 times is exceeded
  
  Serial.println("Beginning bluetooth connection");
  lcd.clear();
  lcd.print("BT Connecting");
  
  setupBTcon(); //bluetooth connection function
  
  
  //If the connection loop was aborted without a connection go to the abort function
  if (btabort == true)
  {
    while (btabort == true)
    {
      abortloop("BT ABORT - RESET");
    }
  }
  Serial.println("Bluetooth Connection Achieved");
  lcd.clear();
  lcd.print("BT Connected");
 
  //run the OBD initilization function
  OBD_init();
  
  if (obdabort == true)
  {
     while (obdabort == true)
    {
      abortloop("OBD ABORT - RESET");
    }
  }
  Serial.println("Well done device pairing complete");
  lcd.clear();
  lcd.print("OBD Connected");
  Serial.println("SPD timer initializing - going to void loop()");
  
  t.every(250,rpm_calc);
 
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
    lcd.clear();
    lcd.print("RPM IS: ");
  while (!obdabort)
  {
  lcd.setCursor(8,0);
  lcd.print(rpm);
  lcd.print("   ");
  
    Serial.print("RPM IS:");
   Serial.print(rpm);
  Serial.println();
  delay(100);
  t.update();
  
  }
  while (obdabort == true)
    {
      abortloop("OBD ABORT - RESET");
    }
}

void abortloop(char m[]){
  lcd.clear();
  lcd.print(m);
  Serial.println(m);
  delay(3000);
}

void enterATMode()
{
 btSerial.flush();
 delay(500);
 Serial.println("Pulling CmdPin to high");
 digitalWrite(CmdPin, HIGH);
 //resetBT();
 Serial.println("Connect power now");
 delay(500);
 btSerial.begin(38400);//HC-05 AT mode baud rate is 38400

}

void enterComMode()
{
 btSerial.flush();
 delay(500);
 digitalWrite(CmdPin, LOW);
 //resetBT();
 delay(500);
 btSerial.begin(38400); //default communication baud rate of HC-05 is 38400
}

void setupBTcon()
{
  btabort=false;                    //set bluetooth error flag to false
  
  Serial.println("Entering AT Mode");
  enterATMode();                          //enter HC-05 AT mode
  delay(500);
  Serial.println("Sending AT Commands");
  sendATCommand("RESET");                  //send to HC-05 RESET
  delay(1000);
  sendATCommand("ORGL");                   //send ORGL, reset to original properties
  sendATCommand("ROLE=1");                 //send ROLE=1, set role to master
  sendATCommand("CMODE=0");                //send CMODE=0, set connection mode to specific address
  sendATCommand("PSWD=1234");
  Serial.println("HC-05 Configured, attempting to pair");
  sendATCommand("BIND=AABB,CC,112233");    //send BIND=??, bind HC-05 to OBD bluetooth address
  sendATCommand("INIT");                   //send INIT, cant connect without this cmd 
  delay(1000); 
  sendATCommand("PAIR=AABB,CC,112233,20"); //send PAIR, pair with OBD address
  delay(1000);  
  sendATCommand("LINK=AABB,CC,112233");    //send LINK, link with OBD address
  delay(1000); 
  Serial.println("AT Commants sent, entering comms mode");
  enterComMode();                          //enter HC-05 comunication mode
  delay(500);
}

void sendATCommand(char *command)
{
  
  char recvChar;
  char str[2];
  int i,retries;
  boolean OK_flag;
  
  if (!(btabort)){                                  //if no bluetooth connection error
    retries=0;
    OK_flag=false;
    
    while ((retries<BT_CMD_RETRIES) && (!(OK_flag))){     //while not OK and bluetooth cmd retries not reached
      
      Serial.print("Sending AT Command: AT+");
      Serial.print(command);
      Serial.println();
      
       btSerial.print("AT");                       //sent AT cmd to HC-05
       if(strlen(command) > 1){
         btSerial.print("+");
         btSerial.print(command);
       }
       btSerial.print("\r\n");
      
      while (btSerial.available()<=0);              //wait while no data
      
      i=0;
      while (btSerial.available()>0){               // while data is available
        recvChar = btSerial.read();                 //read data from HC-05
          if (i<2){
            str[i]=recvChar;                               //put received char to str
            i=i+1;
          }
      }
      retries=retries+1;                                  //increase retries 
      if ((str[0]=='O') && (str[1]=='K')) 
      {
        Serial.println("OK Response received");
      OK_flag=true;   //if response is OK then OK-flag set to true
      }
      delay(500);
    }
  
    if (retries>=BT_CMD_RETRIES) {                        //if bluetooth retries reached
      btabort=true;                                 //set bluetooth error flag to true
      Serial.println("failed AT Command");
    }
  }
  
}


void OBD_init()
{
 
  Serial.println("Starting OBD Initilization");
 obdabort = false;
 send_OBD_cmd("ATZ");
 delay(1000);
 //send_OBD_cmd("ATSP0");
 
 //send_OBD_cmd("0100");
 //delay(1000);
 //send_OBD_cmd("0120");
 //delay(1000);
 //send_OBD_cmd("0140");
 //delay(1000);
 send_OBD_cmd("010C1");
 delay(1000);
}

void send_OBD_cmd(char *obd_cmd)
{
  char recvChar;
  boolean prompt;
  int retries;
  
  if (!(obdabort))
  {
    prompt = false;
    retries = 0;
    while ((!prompt) && (retries<OBD_CMD_RETRIES))
    {
      Serial.print("Sending OBD command:");
      Serial.print(obd_cmd);
      Serial.println();
      btSerial.print(obd_cmd);
      btSerial.print("\r");
      
      while (btSerial.available() <= 0);
      
      while ((btSerial.available()>0) && (!prompt))
      {
        recvChar = btSerial.read();
        if (recvChar == 62) prompt = true;
        Serial.println("Matching response identified");
      }
      retries = retries + 1;
      delay(1000);
    }
    if (retries >= OBD_CMD_RETRIES)
    {
      obdabort = true;
      Serial.println("OBD command failed");
    }
  }
}

void speedpull()
{
  boolean prompt,valid;
  char recvChar;
  int i;
  
  if (!(obdabort))
  {
    valid=false;
    prompt=false;
    btSerial.print("010D1");
    btSerial.print("\r");
    while (btSerial.available() <= 0);
    i = 0;
    while ((btSerial.available()>0) && (i<12));
    {
      recvChar = btSerial.read();
      if ((i<15) && (!(recvChar==32)))
      {
        bufin[i]=recvChar;
        i=i+1;
      }
      if (recvChar==62) prompt=true;
    }
    
    if ((bufin[5]=='4') && (bufin[6]=='1') && (bufin[7]=='0') && (bufin[8]=='D'))
    {
      valid=true;
    }
    else
    {
      valid=false;
    }
    
    if (valid)
    {
      //here we work out the actual speed
      spd_retries = 0;
      spdabort = false;
      
      spdk=0;
      
      char hexspeed[2] = { bufin[10] , bufin[11] };
      int spddisp = (int) strtol(hexspeed, NULL, 16);
      
      spdk = spddisp;
    }
    if (!valid)
    {
      spdabort = true;
      spd_retries+=1;
      spdk=0;
      if (spd_retries>=SPD_CMD_RETRIES) obdabort=true;
 
    }     //need to work out how to pull the two speed characters and convert to dec.
  } 
     
}

void rpm_calc(){
   boolean prompt,valid;  
   char recvChar;
   char bufin[15];
   int i;

  if (!(obdabort)){                                   //if no OBD connection error

     valid=false;
     prompt=false;
     btSerial.print("010C1");                        //send to OBD PID command 010C is for RPM, the last 1 is for ELM to wait just for 1 respond (see ELM datasheet)
     btSerial.print("\r");                           //send to OBD cariage return char
     while (btSerial.available() <= 0);              //wait while no data from ELM
     i=0;
     while ((btSerial.available()>0) && (!prompt)){  //if there is data from ELM and prompt is false
       recvChar = btSerial.read();                   //read from ELM
       if ((i<15)&&(!(recvChar==32))) {                     //the normal respond to previus command is 010C1/r41 0C ?? ??>, so count 15 chars and ignore char 32 which is space
         bufin[i]=recvChar;                                 //put received char in bufin array
         i=i+1;                                             //increase i
       }  
       if (recvChar==62) prompt=true;                       //if received char is 62 which is '>' then prompt is true, which means that ELM response is finished 
     }

    if ((bufin[6]=='4') && (bufin[7]=='1') && (bufin[8]=='0') && (bufin[9]=='C')){ //if first four chars after our command is 410C
      valid=true;                                                                  //then we have a correct RPM response
    } else {
      valid=false;                                                                 //else we dont
    }
    if (valid){                                                                    //in case of correct RPM response
      rpm_retries=0;                                                               //reset to 0 retries
      rpmabort=false;                                                        //set rpm error flag to false
      
     //start calculation of real RPM value
     //RPM is coming from OBD in two 8bit(bytes) hex numbers for example A=0B and B=6C
     //the equation is ((A * 256) + B) / 4, so 0B=11 and 6C=108
     //so rpm=((11 * 256) + 108) / 4 = 731 a normal idle car engine rpm
      rpm=0;                                                                                            
      for (i=10;i<14;i++){                              //in that 4 chars of bufin array which is the RPM value
        if ((bufin[i]>='A') && (bufin[i]<='F')){        //if char is between 'A' and 'F'
          bufin[i]-=55;                                 //'A' is int 65 minus 55 gives 10 which is int value for hex A
        } 
         
        if ((bufin[i]>='0') && (bufin[i]<='9')){        //if char is between '0' and '9'
          bufin[i]-=48;                                 //'0' is int 48 minus 48 gives 0 same as hex
        }
        
        rpm=(rpm << 4) | (bufin[i] & 0xf);              //shift left rpm 4 bits and add the 4 bits of new char
       
      }
      rpm=rpm >> 2;                                     //finaly shift right rpm 2 bits, rpm=rpm/4
    }
      
    }
    if (!valid){                                              //in case of incorrect RPM response
      rpmabort=true;                                    //set rpm error flag to true
      rpm_retries+=1;                                         //add 1 retry
      rpm=0;                                                  //set rpm to 0
      //Serial.println("RPM_ERROR");
      if (rpm_retries>=RPM_CMD_RETRIES) obdabort=true;  //if retries reached RPM_CMD_RETRIES limit then set obd error flag to true
    }
}
