/* Written By James Mare
This is a partial rewrite commited to git on 5/2/15
*/

//library includes
#include <LiquidCrystal.h>
#include <Encoder.h>
#include "Timer.h"
#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>
Timer t; //start timer



//menu setup
Encoder myEnc(18, 19); //start the encoder library with the interupt pins
const int ENC_PUSH_PIN = 27; //push button pin
int ENC_PUSH_STATE = 0; 
int menu_var = 0;
int menu_pos_old = 1;
int menu_pos = 1;
int menu_change = 0;

//bargraph variables
int activation_val; 
int shift_val; 
int segment_int; 

int barval;

#define NEOPIN 10

Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, NEOPIN, NEO_GRB + NEO_KHZ800); 


//Color variables for direct use with strips
uint32_t color1 = strip.Color(79,105,224);
uint32_t color2 = strip.Color(235,169,16);
uint32_t color3 = strip.Color(255,0,0);
uint32_t flclr1; 
uint32_t flclr2; 


// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(22, 24, 32, 30, 28, 26);



//Define the pins that connect to the HC-05
#define RxD 50
#define TxD 51
#define CmdPin 52

//Establish max retries for our functions
#define OBD_CMD_RETRIES 5
#define BT_CMD_RETRIES 5


//abort flags and counters
boolean obdabort;
boolean btabort;
boolean obd_retries;

//data calculation variables inc ring buffer

char rxData[50];
long int hexAint;
long int hexBint;
int rxIndex = 0;
int rpmstored = 0;
int spdstored = 0;
int tmpstored = 0;
int vltstored = 0;

//Establish serial connection to the HC-05
SoftwareSerial btSerial(RxD, TxD);

void setup()
{
	strip.begin();
	strip.show();

	activation_val = 3000;
	shift_val = 5500;

	strip.setBrightness(10);
	  // set up the LCD's number of columns and rows:
  	lcd.begin(16, 2);

  //Set the encoder push button input
  	pinMode(ENC_PUSH_PIN, INPUT);
	pinMode(RxD, INPUT);
   	pinMode(TxD, OUTPUT);
   	pinMode(CmdPin, OUTPUT);

   	//Establish connection to computer
   	Serial.begin(38400);
   	Serial.println("Serial connected");

   	lcd.clear();
   	lcd.setCursor(0,0);
   	lcd.print("BT Connecting");

   	//establish connection with HC-05 to the ELM327
  	setupBTcon();
   	if (btabort == true)
  	{
    	while (btabort == true)
    	{
     		abortloop("BT ABORT - RESET");
    	}
  	}

  	Serial.println("setupBTcon() complete");

   	lcd.clear();
   	lcd.setCursor(0,0);
   	lcd.print("OBD initialize");
  	OBD_init();
	if (obdabort == true)
  	{
    	while (obdabort == true)
    	{
      		abortloop("OBD ABORT - RESET");
    	}
  	}

  	Serial.println("setup() complete"); 
  	lcd.clear();

  	//int rpmevent = t.every(50,getRPM);
  	//int spdevent = t.every(200,getSPD);
  	//int tmpevent = t.every(2000,getTMP);
  	//int vltevent = t.every(2000,getVLT);
  	int dataevent = t.every(100,getdata);
  	int lcdevent = t.every(50,printlcd);
}

long oldPosition  = 0;

void loop()
{
	t.update();

	barval = rpmstored;
	writebar();
	
	// Poll the push button on the encoder
	ENC_PUSH_STATE = digitalRead(ENC_PUSH_PIN);

	//poll the encoder and calculate the menu position
	read_enc();

  while (obdabort == true)
    {
      abortloop("OBD ABORT - RESET");
    }
}

void writebar()
{
	segment_int = (shift_val - activation_val) / 8;

	if (barval > activation_val){
		strip.setPixelColor(7, color1);
		strip.setPixelColor(8, color1);
	}
	else{
		strip.setPixelColor(7,strip.Color(0, 0, 0));
		strip.setPixelColor(8,strip.Color(0, 0, 0));
	}

	if ((barval-activation_val) > (segment_int)) { 
		strip.setPixelColor(6, color1); 
		strip.setPixelColor(9, color1); 
	} 
	else { 
		strip.setPixelColor(6, strip.Color(0, 0, 0)); 
		strip.setPixelColor(9, strip.Color(0, 0, 0)); 
	} 


	if ((barval-activation_val) > (segment_int * 2)) { 
		strip.setPixelColor(5, color1); 
		strip.setPixelColor(10, color1); 
	} 
	else { 
		strip.setPixelColor(5, strip.Color(0, 0, 0)); 
		strip.setPixelColor(10, strip.Color(0, 0, 0)); 
	} 

	if ((barval-activation_val) > (segment_int * 3)) { 
		strip.setPixelColor(4, color1); 
		strip.setPixelColor(11, color1); 
	} 
	else { 
		strip.setPixelColor(4, strip.Color(0, 0, 0)); 
		strip.setPixelColor(11, strip.Color(0, 0, 0)); 
	} 


	if ((barval-activation_val) > (segment_int * 4)) { 
		strip.setPixelColor(3, color2); 
		strip.setPixelColor(12, color2); 
	} 
	else { 
		strip.setPixelColor(3, strip.Color(0, 0, 0)); 
		strip.setPixelColor(12, strip.Color(0, 0, 0)); 
	} 


	if ((barval-activation_val) > (segment_int * 5)) { 
		strip.setPixelColor(2, color2); 
		strip.setPixelColor(13, color2); 
	} 
	else { 
		strip.setPixelColor(2, strip.Color(0, 0, 0)); 
		strip.setPixelColor(13, strip.Color(0, 0, 0)); 
	} 


	if ((barval-activation_val) > (segment_int * 6)) { 
		strip.setPixelColor(1, color3); 
		strip.setPixelColor(14, color3); 
	} 
	else { 
		strip.setPixelColor(1, strip.Color(0, 0, 0)); 
		strip.setPixelColor(14, strip.Color(0, 0, 0)); 
	} 



	if ((barval-activation_val) > (segment_int * 7)) { 
		strip.setPixelColor(0, color3); 
		strip.setPixelColor(15, color3); 
	} 
	else { 
		strip.setPixelColor(0, strip.Color(0, 0, 0)); 
		strip.setPixelColor(15, strip.Color(0, 0, 0)); 
	} 

	strip.show();
}

void printlcd()
{

	if (menu_pos != menu_pos_old)
	{
		lcd.clear();
	}

	switch (menu_pos)
	{
		case 1:

			lcd.setCursor(0,0);
			lcd.print("Speed:");
			lcd.print("          ");
			lcd.setCursor(0,1);
			lcd.print(spdstored);
			lcd.print("     ");
			break;

		case 2:
			lcd.setCursor(0,0);
			lcd.print("RPM:");
			lcd.print("          ");
			lcd.setCursor(0,1);
			lcd.print(rpmstored);
			lcd.print("     ");
			break;
		case 3:
			lcd.setCursor(0,0);
			lcd.print("Coolant Temp: ");
			lcd.print("         ");
			lcd.setCursor(0,1);
			lcd.print(tmpstored);
			lcd.print("    ");
			break;
		case 4:
			lcd.setCursor(0,0);
			lcd.print("Battery Voltage: ");
			lcd.print("           ");
			lcd.setCursor(0,1);
			lcd.print(vltstored);
			lcd.print("    ");
			break;
	}
}

void read_enc()
{
	menu_pos_old = menu_pos;
	long newPosition = myEnc.read();
	if (newPosition != oldPosition)
	{
		menu_change = (newPosition - oldPosition);

		oldPosition = newPosition;

		//this if stops menu var getting outside 1-16
		if ((menu_var + menu_change <= 16) && (menu_var + menu_change >= 0))
		{
		menu_var = menu_var + menu_change;
		}
		else if (menu_var + menu_change >=16)
		{
			menu_var = 16;
		}
		else if (menu_var + menu_change <=0)
		{
			menu_var = 0;
		}

		if ((menu_var >=0) && (menu_var < 4))
		{
			menu_pos = 1;
		}
		else if ((menu_var >=4) && (menu_var < 8))
		{
			menu_pos = 2;
		}
		else if ((menu_var >=8) && (menu_var < 12))
		{
			menu_pos = 3;
		}
		else if ((menu_var >=12) && (menu_var <= 16))
		{
			menu_pos = 4;
		}

	}
	//return(menu_pos);
}

void getdata(void)
{
	//Serial.println("getdata");
	OBD_read("010D050C3");
	//Serial.println(rxData);

	char hexA[3] = {rxData[18], rxData[19], '\0'};
	spdstored = strtol(hexA, NULL, 16);

	//hexA[] = {rxData[22], rxData[23], '\0'};
    hexA[0] = rxData[22];
    hexA[1] = rxData[23];
    hexA[2] = '\0';
	tmpstored = strtol(hexA, NULL, 16) - 40;

	//hexA[] = {rxData[28], rxData[29], '\0'};
        hexA[0] = rxData[28];
        hexA[1] = rxData[29];
        hexA[2] = '\0';
	char hexB[3] = {rxData[30], rxData[31], '\0'};
	hexAint = strtol(hexA, NULL, 16);
	hexBint = strtol(hexB, NULL, 16);
	rpmstored = ((hexAint * 256) + hexBint) / 4;

	//Serial.println("getvlt");
	//OBD_read("AT RV");
	//Serial.println(rxData);
}

void OBD_read(char *command)
{
	boolean prompt,valid;
	char c;
	memset(&rxData[0], 0, sizeof(rxData));

	if (!(obdabort)){
		valid = false;
		prompt = false;
		btSerial.print(command);
		//Serial.println(command);
		btSerial.print("\r");
		while (btSerial.available() <= 0);

		rxIndex = 0;

		while ((btSerial.available()>0) && (!prompt))
		{
			c = btSerial.read();
			if ((c != '>') && (c != '\r') && (c != '\n') && (c != ' ')) //Keep these out of our buffer
      		{
       			rxData[rxIndex++] = c; //Add whatever we receive to the buffer
      		}
      		if (c == 62) 
      			{
      				prompt=true;
      			}
      	}

      	rxData[rxIndex++] = '\0';
      	//Serial.println(rxData);

      	if (0 == 0/*(rxData[7]==command[2]) && (rxData[8]==command[3])*/){ //if first four chars match our command chars
		valid=true;                                                                  //corr response
		//Serial.println("valid=true");
		} 
		else {
		valid=false;  
		//Serial.println("valid=false");                                                               //else we dont
		}

		if (valid){
			obd_retries = 0;
		}

	}

	if (!valid) {
		obd_retries+=1;

		if (obd_retries>=OBD_CMD_RETRIES) {
			obdabort = true;
		}
	}
}

void setupBTcon()
{
	btabort=false;                    //set bluetooth error flag to false

	Serial.println("Entering AT Mode");
	lcd.setCursor(0,1);
	lcd.print("Entering AT Mode");

	enterATMode();                          //enter HC-05 AT mode
	delay(500);
	Serial.println("Sending AT Commands");
	sendATCommand("RESET");                  //send to HC-05 RESET
	delay(500);
	sendATCommand("ORGL");                   //send ORGL, reset to original properties
	sendATCommand("ROLE=1");                 //send ROLE=1, set role to master
	sendATCommand("CMODE=0");                //send CMODE=0, set connection mode to specific address
	sendATCommand("PSWD=1234");
	Serial.println("HC-05 Configured, attempting to pair");
	sendATCommand("BIND=AABB,CC,112233");    //send BIND=??, bind HC-05 to OBD bluetooth address
	sendATCommand("INIT");                   //send INIT, cant connect without this cmd 
	delay(500); 
	sendATCommand("PAIR=AABB,CC,112233,20"); //send PAIR, pair with OBD address
	delay(500);  
	sendATCommand("LINK=AABB,CC,112233");    //send LINK, link with OBD address
	delay(500); 
	Serial.println("AT Commants sent, entering comms mode");
	lcd.setCursor(0,1);
	lcd.print("Entering comms Mode");
	lcd.print("       ");
	enterComMode();                          //enter HC-05 comunication mode
	delay(500);
}

void enterATMode()
{
	btSerial.flush();
	delay(100);
	Serial.println("Pulling CmdPin to high");
	digitalWrite(CmdPin, HIGH);
	//resetBT();
	delay(100);
	btSerial.begin(38400);//HC-05 AT mode baud rate is 38400

}

void enterComMode()
{
	btSerial.flush();
	delay(100);
	digitalWrite(CmdPin, LOW);
	//resetBT();
	delay(100);
	btSerial.begin(38400); //default communication baud rate of HC-05 is 38400
}

void sendATCommand(char *command)
{
	char recvChar;
	char str[2];
	int i,retries;
	boolean OK_flag;

	if (!(btabort))
	{                                  //if no bluetooth connection error
		retries=0;
		OK_flag=false;

		while ((retries<BT_CMD_RETRIES) && (!(OK_flag))){     //while not OK and bluetooth cmd retries not reached
		  
			Serial.print("Sending AT Command: AT+");
			Serial.print(command);
			Serial.println();

			lcd.setCursor(0,1);
			lcd.print("AT+");
			lcd.print(command);
			lcd.print("        ");

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
			delay(200);
			}
			else
			{
			delay(500);
			}
		}

		if (retries>=BT_CMD_RETRIES) {                        //if bluetooth retries reached
		  btabort=true;                                 //set bluetooth error flag to true
		  Serial.println("BT_CMD_RETRIES reached");
		}
	}
}

void abortloop(char m[]){
  Serial.println(m);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(m);

  delay(3000);

}

void OBD_init()
{
 
	Serial.println("Starting OBD Initilization");
	obdabort = false;
	send_OBD_cmd("ATZ");
	//send_OBD_cmd("ATSP0");

	//send_OBD_cmd("0100");
	//delay(1000);
	//send_OBD_cmd("0120");
	//delay(1000);
	//send_OBD_cmd("0140");
	//delay(1000);
	send_OBD_cmd("010C1");
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
        if (recvChar == 62) 
        {
        prompt = true;
        Serial.println("Matching response identified");
    	}
      }
      retries = retries + 1;
      delay(100);
    }
    if (retries >= OBD_CMD_RETRIES)
    {
      obdabort = true;
      Serial.println("OBD command failed");
    }
  }
}