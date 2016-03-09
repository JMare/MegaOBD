	/* Written By James Mare

This is a fork of MegaOBD.ino which uses the same setup functions to establish a connection to the elm327
This program allows a serial terminal interface with the ELM327

Baud: 38400


*/

#include <SoftwareSerial.h>

//Define the pins that connect to the HC-05
#define RxD 50
#define TxD 51
#define CmdPin 52

//Establish max retries for our functions
#define OBD_CMD_RETRIES 3
#define BT_CMD_RETRIES 5


//abort flags and counters
boolean obdabort;
boolean btabort;
boolean obd_retries;

//Ringbuffer
char rxData[30];
char rxIndex = 0;

//Establish serial connection to the HC-05
SoftwareSerial btSerial(RxD, TxD);

void setup()
{
	pinMode(RxD, INPUT);
   	pinMode(TxD, OUTPUT);
   	pinMode(CmdPin, OUTPUT);

   	//Establish connection to computer
   	Serial.begin(38400);
   	Serial.println("Serial connected");

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

  	OBD_init();
	if (obdabort == true)
  	{
    	while (obdabort == true)
    	{
      		abortloop("OBD ABORT - RESET");
    	}
  	}

  	Serial.println("setup() complete"); 
}

void loop()
{

	if (Serial.available()) {
    	int inByte = Serial.read();
    	btSerial.write(inByte); 
  }
  

  	if (btSerial.available()) {
		boolean prompt;

		char c;
		memset(&rxData[0], 0, sizeof(rxData)); //reset ring buffer
		prompt = false; //prompt false
		rxIndex = 0; //reset index

		while ((btSerial.available()>0) && (!prompt))
		{
			c = btSerial.read();
			if ((c != '>') && (rxIndex<14) && (c != 10) && (c != 13) && (c != ' ')) //Keep these out of our buffer
	  		{
	   			rxData[rxIndex++] = c; //Add whatever we receive to the buffer
	  		}
	  		if (c == 62) prompt=true; //if c=> then set prompt to true and exit loop
	  	}

	  	//rxData[rxIndex++] = '\0'; //convert to string

	  	Serial.print(rxData); //once data is collected print to serial terminal and go back to waiting for serial input
                Serial.println();  	
}

  while (obdabort == true)
    {
      abortloop("OBD ABORT - RESET");
    }

}

void setupBTcon()
{
	btabort=false;                    //set bluetooth error flag to false

	Serial.println("Entering AT Mode");

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
		  Serial.println("BT_CMD_RETRIES reached");
		}
	}
}

void abortloop(char m[]){
  Serial.println(m);
  delay(3000);

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
