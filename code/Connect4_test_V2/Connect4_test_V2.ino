/*
_____________________________________________________
 ATMEL ATTINY 25/45/85 / ARDUINO

                  +-\/-+
 Ain0 (D 5) PB5  1|    |8  Vcc
 Ain3 (D 3) PB3  2|    |7  PB2 (D 2) Ain1
 Ain2 (D 4) PB4  3|    |6  PB1 (D 1) pwm1
            GND  4|    |5  PB0 (D 0) pwm0
                  +----+
_____________________________________________________
The Button Circuit:
 
       Pin A3
          |
GND--10K--|--------|--------|-------|-------|
          |        |        |       |       |
         btn1     btn2     btn3    btn4    btn5
          |        |        |       |       |
       330 Ohm  680 Ohm  2.2 Ohm   4.7K    10K
          |--------|--------|-------|-------|-- +5V
            
Button code and wiring ispired by: Michael Pilcher
_____________________________________________________
Code by Eliot Wachtel
July 24, 2021
*/

// RGB LED control variables ////////////////////////////////////////////////////
  // RGB LED library
#include <FAB_LED.h>

  // Requires headers for AVR defines and ISR function
#include <avr/io.h>
#include <avr/interrupt.h>

  // Declare the LED protocol and the port
ws2812b<D,0>  strip;

  // How many pixels to control
const uint8_t numPixels = 49;

  // How bright the LEDs will be (max 255)
const uint8_t maxBrightness = 16;

  // The pixel array to display
grb  pixels[numPixels] = {};

char activeColor = 0;
////////////////////////////////////////////////////////////////////////////////

// Sets the array to specified color ///////////////////////////////////////////
void updateColors(char r, char g, char b)
{
  for(int i = 0; i < numPixels; i++)
  {
    pixels[i].r = r;
    pixels[i].g = g;
    pixels[i].b = b;
  }
}
////////////////////////////////////////////////////////////////////////////////

// button input variables //////////////////////////////////////////////////////
  // Analog pin to read the buttons.
int analogpin = A3;
  // How many times we have seen new value.
int counter = 0;
  // The last time the output pin was sampled.
long timer = 0;
  // Number of millis/samples to consider before declaring a debounced input.
int debounce_count = 50;
  // The debounced input value.
int current_state = 0;
  // Stores the current analog value on the button input pin.
int ButtonVal;
////////////////////////////////////////////////////////////////////////////////

// connect four variables //////////////////////////////////////////////////////
// I use uint8_t because it is only one byte and all values will be small unsigned integers.
  // Need to set up with all zeros. Then a red piece can be 1 and a blue piece can be 2. (array[row][column])
uint8_t gameBoard[6][7];
  // The position from left to right of the piece currently waiting to drop (range: 0 to 6).
uint8_t dropPosition = 0;
  // The status of the game. Also controls the color of the status LED which is LED #43.
uint8_t gameStatus = 0;
////////////////////////////////////////////////////////////////////////////////


void setup()
{
  strip.clear(2 * numPixels);
  pinMode(analogpin, INPUT);
}

void loop()
{
   // If we have gone on to the next millisecond
  if (millis() != timer)
  {
//    // check analog pin for the button value and save it to ButtonVal
//    ButtonVal = analogRead(analogpin);
//    if(ButtonVal == current_state && counter > 0)
//    { 
//      counter--;
//    }
//    if(ButtonVal != current_state)
//    {
//      counter++;
//    }
//    // If ButtonVal has shown the same value for long enough let's switch it
//    if (counter >= debounce_count)
//    {
//      counter = 0;
//      current_state = ButtonVal;
//      //Checks which button or button combo has been pressed
//      if (ButtonVal > 0)
//      {
//        ButtonCheck();
//      }
//    }
    
    ButtonCheck();
    timer = millis();
  }

  if(activeColor == 0)
  {
    // Write the pixel array red
    updateColors(maxBrightness, 0 , 0);
    // Display the pixels on the LED strip
    strip.sendPixels(numPixels, pixels);
    // Wait 0.1 seconds
    delay(50);
  }
  if(activeColor == 1)
  {
    // Write the pixel array green
    updateColors(0, maxBrightness , 0);
    // Display the pixels on the LED strip
    strip.sendPixels(numPixels, pixels);
    // Wait 0.1 seconds
    delay(50);
  }
  if(activeColor == 2)
  {
    // Write the pixel array blue
    updateColors(0, 0 , maxBrightness);
    // Display the pixels on the LED strip
    strip.sendPixels(numPixels, pixels);
    // Wait 0.1 seconds
    delay(50);
  }
  if(activeColor == 3)
  {
    // Write the pixel array white
    updateColors(maxBrightness, maxBrightness , 0);
    // Display the pixels on the LED strip
    strip.sendPixels(numPixels, pixels);
    // Wait 0.1 seconds
    delay(50);
  }
}

void ButtonCheck()
{
  // read the analog value of the button input pin.
  ButtonVal = analogRead(analogpin);
  
  // analog signal to button pressed decoding:
  //(The voltage divider between the button resistor and the 10K ground resistor gives the different analog values)
  if(ButtonVal >= 990 && ButtonVal <= 1004) // 330 Ohm (function button)
  {
    activeColor = 1; // green
  }
  if(ButtonVal >= 940 && ButtonVal <= 980) // 680 Ohm (left button)
  {
    activeColor = 2; // blue
  }
  if(ButtonVal >= 750 && ButtonVal <= 900) // 2.2 Ohm (down button)
  {
    activeColor = 0; // red
  }
  if(ButtonVal >= 600 && ButtonVal <= 700) // 4.7 Ohm (up button)
  {
    activeColor = 1; // green
  }
  if(ButtonVal >= 500 && ButtonVal <= 600) // 10K Ohm (right button)
  {
    activeColor = 3; // white
  }
}
