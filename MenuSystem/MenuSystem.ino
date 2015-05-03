
// include the library code:
#include <LiquidCrystal.h>

#include <Encoder.h>
Encoder myEnc(18, 19);
const int ENC_PUSH_PIN = 27;
int ENC_PUSH_STATE = 0;
int menu_var = 0;
int menu_change = 0;

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(22, 24, 32, 30, 28, 26);

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.print("hello, world!");
  Serial.begin(38400);
  Serial.println("Basic Encoder Test:");

  pinMode(ENC_PUSH_PIN, INPUT);
}

long oldPosition  = 0;

void loop() {

	ENC_PUSH_STATE = digitalRead(ENC_PUSH_PIN);

	if (ENC_PUSH_STATE == HIGH)
	{
		lcd.setCursor(8,1);
		lcd.print("PUSH");
	}
	else
	{
		lcd.setCursor(8,1);
		lcd.print("    ");
	}


	long newPosition = myEnc.read();
	if (newPosition != oldPosition)
	{
		menu_change = (newPosition - oldPosition);

		oldPosition = newPosition;
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

		lcd.setCursor(0,1);
		lcd.print(menu_var);
		lcd.print("   ");
		delay(500);
	}
}
