/*
LCD Shift light code
This will eventually be integrated into MegaOBD.ino

*/

#define NEOPIN 6

#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, NEOPIN, NEO_GRB + NEO_KHZ800); 

int activation_val; 
int shift_val; 
int segment_int; 

int barval;


//Color variables for direct use with strips
uint32_t color1; 
uint32_t color2; 
uint32_t color3; 
uint32_t flclr1; 
uint32_t flclr2; 

unsigned int Color(byte r, byte g, byte b) 
{ 
//Take the lowest 5 bits of each value and append them end to end 
return( ((unsigned int)g & 0x1F )<<10 | ((unsigned int)b & 0x1F)<<5 | (unsigned int)r & 0x1F); 
} 


void setup()
{
	Serial.begin(38400);
	strip.begin();
	strip.show();

	activation_val = 3000;
	shift_val = 5000;

}

void loop()
{

	barval =
	writebar();
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
		strip.setPixelColor(7,strip.Color(0, 0, 0));
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