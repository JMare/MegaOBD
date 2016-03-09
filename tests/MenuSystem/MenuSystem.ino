
// include the library code:
#include <LiquidCrystal.h>

#include <Encoder.h>
Encoder myEnc(18, 19); //start the encoder library with the interupt pins
const int ENC_PUSH_PIN = 27; //push button pin
int ENC_PUSH_STATE = 0; 
int menu_var = 0;
int menu_pos_old = 1;
int menu_pos = 1;
int menu_change = 0;

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(22, 24, 32, 30, 28, 26);

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  //Set the encoder push button input
  pinMode(ENC_PUSH_PIN, INPUT);
}

long oldPosition  = 0;

void loop() {

	// Poll the push button on the encoder
	ENC_PUSH_STATE = digitalRead(ENC_PUSH_PIN);

	//poll the encoder and calculate the menu position
	read_enc();

	if (menu_pos != menu_pos_old)
	{
		lcd.clear();
	}

	switch (menu_pos)
	{
		case 1:
			lcd.setCursor(0,0);
			lcd.print("Speed:");
			lcd.setCursor(0,1);
			lcd.print("35 km/h");
			break;

		case 2:
			lcd.setCursor(0,0);
			lcd.print("RPM:");
			lcd.setCursor(0,1);
			lcd.print("1241");
			break;
		case 3:
			lcd.setCursor(0,0);
			lcd.print("Coolant Temp: ");
			lcd.setCursor(0,1);
			lcd.print("88 C");
			break;
		case 4:
			lcd.setCursor(0,0);
			lcd.print("Battery Voltage: ");
			lcd.setCursor(0,1);
			lcd.print("11.7 V");
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

void menu()
{
	//stub
}