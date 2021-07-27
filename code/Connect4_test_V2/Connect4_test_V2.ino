/*
_____________________________________________________
 ATMEL ATTINY 25/45/85 / ARDUINO

                  +-\/-+
 Ain0 (D 5) PB5  1|    |8  Vcc
 Ain3 (D 3) PB3  2|    |7  PB2 (D 2) Ain1
 Ain2 (D 4) PB4  3|    |6  PB1 (D 1) pwm1
            GND  4|    |5  PB0 (D 0) pwm0
                  +----+
 In this project: 
 - Ain3 is used to take all button inputs using alalog read.
 - PB0 is used to control the WS2813 LEDs using the FAB_LED library.
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
const uint8_t numPixels = 43;

  // How bright the LEDs will be (max 255)
const uint8_t maxBrightness = 16;

  // The pixel array to display
grb  pixels[numPixels] = {};
  
//char activeColor = 0;

  // whether the LED array needs to be updated
bool LEDchange = true;

////////////////////////////////////////////////////////////////////////////////

// Sets the array (aka all LEDs) to specified color ////////////////////////////
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
  // whether the code is ready for a new button input
bool buttonDebounce = true;
////////////////////////////////////////////////////////////////////////////////

// connect four variables //////////////////////////////////////////////////////
// I use uint8_t because it is only one byte and all values will be small unsigned integers.
  // Need to set up with all zeros. Then a red piece can be 1 and a blue piece can be 2. (array[row][column])
uint8_t gameBoard[6][7] = {
  {2, 1, 0, 0, 0, 0, 0},
  {0, 0, 1, 0, 0, 0, 0},
  {0, 0, 2, 0, 0, 0, 0},
  {0, 0, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0}
};
  // The position from left to right of the piece currently waiting to drop (range: 0 to 6).
uint8_t dropPosition = 0;
  // The status of the game. Also controls the color of the status LED which is LED #43.
uint8_t gameStatus = 0;
  // Stores the game defined state of the button input.
uint8_t buttonState = 0; // 0 = no input, 1 = up arrow, 2 = down arrow, 3 = left arrow, 4 = right arrow
////////////////////////////////////////////////////////////////////////////////
bool pressedTest = false;

void setup()
{
    // reset the LEDs
  strip.clear(2 * numPixels);
    // setup the pin for button inputs
  pinMode(analogpin, INPUT);
}

void loop()
{
  // button press handling //////////////////////////////
  if (millis() != timer && buttonDebounce) // If we have gone on to the next millisecond and have no debounce needs.
  {
    ButtonCheck();
    timer = millis();
  } else if (millis() > timer + 500) // If the debounce of 50 milliseconds has been reached
  {
    // debounce has been met so it is reset.   
    buttonDebounce = true;
  } 
  ///////////////////////////////////////////////////////
  // button action handling /////////////////////////////
  switch (buttonState)
  {
    case 1:
        // Write the pixel array red
      updateColors(maxBrightness, 0, 0);
      LEDchange = true; // LEDs have changed
        // task complete
      buttonTaskComplete();
      break;
    case 2:
//        // Write the pixel array green
//      updateColors(0, maxBrightness, 0);
        // drop the current hanging piece
      dropPiece();
        // task complete
      buttonTaskComplete();
      break;
    case 3:
//      // Write the pixel array blue
//      updateColors(0, 0, maxBrightness);
        // If the hanging piece is not at the leftmost side
        // move hanging piece to the left.
      if (dropPosition > 0)
      {
        dropPosition -= 1;
      }
        // task complete
      buttonTaskComplete();
      break;
    case 4:
//        // Write the pixel array yellow
//      updateColors(maxBrightness, maxBrightness, maxBrightness);
        // If the hanging piece is not at the rightmost side
        // move hanging piece to the right.
      if (dropPosition < 6)
      {
        dropPosition += 1;
      }
        // task complete
      buttonTaskComplete();
      break;
    case 5:      
//        // Write the pixel array cyan
//      updateColors(0, maxBrightness , maxBrightness);
      convertGameToLightList();
      LEDchange = true; // LEDs have changed
        // task complete
      buttonTaskComplete();
  }
  ///////////////////////////////////////////////////////
  // game draw handling /////////////////////////////////
    // if a change has been made update the display
  if (LEDchange)
  {
      // Display the pixels on the LED strip
    strip.sendPixels(numPixels, pixels);
      // reset change
    LEDchange = false;
  }
  ///////////////////////////////////////////////////////
}

// code run after a button activated task is completed /////////////////////////
void buttonTaskComplete()
{
    // Reset the buttonState.
  buttonState = 0;
}
////////////////////////////////////////////////////////////////////////////////

// Sets the array (aka all LEDs) to specified color ////////////////////////////
void convertGameToLightList()
{
  for(uint8_t i = 0; i < 6; i++) // looping through rows
  {
    for(uint8_t ii = 0; ii < 7; ii++) // looping through columns
    {
        // the position in the pixels array of the LED corresponding to position 
        // [i][ii] in the game.
      uint8_t arraySpot = (i * 7) + ii; // will not exceed 42 so uint8_t is fine.
        // colors the pixel based on game board value. 0 --> green | 1 --> red | 2 --> blue
      switch (gameBoard[i][ii])
      {
        case 0:
          pixels[arraySpot].r = 0;
          pixels[arraySpot].g = maxBrightness/2;
          pixels[arraySpot].b = 0;
          break;
        case 1:
          pixels[arraySpot].r = maxBrightness;
          pixels[arraySpot].g = 0;
          pixels[arraySpot].b = 0;
          break;
        case 2:
          pixels[arraySpot].r = 0;
          pixels[arraySpot].g = 0;
          pixels[arraySpot].b = maxBrightness;
          break;
      }
    }
  }
}
////////////////////////////////////////////////////////////////////////////////

// drops hanging piece /////////////////////////////////////////////////////////
void dropPiece()
{
  if (dropPosition == 6){
    updateColors(0, 0, maxBrightness);
  } else {
    updateColors(0, maxBrightness, 0);
  }
  
  LEDchange = true; // LEDs have changed
}
////////////////////////////////////////////////////////////////////////////////

// decodes analog input into individual button presses /////////////////////////
void ButtonCheck()
{
  // read the analog value of the button input pin.
  ButtonVal = analogRead(analogpin);
  
  // analog signal to button pressed decoding:
  //(The voltage divider between the button resistor and the 10K ground resistor gives the different analog values)
  if(ButtonVal >= 990 && ButtonVal <= 1004) // 330 Ohm (function button)
  {
    buttonState = 5; // function button
    buttonDebounce = false;
  }
  if(ButtonVal >= 940 && ButtonVal <= 980) // 680 Ohm (left button)
  {
    buttonState = 3; // left button
    buttonDebounce = false;
  }
  if(ButtonVal >= 750 && ButtonVal <= 900) // 2.2 Ohm (down button)
  {
    buttonState = 2; // down button
    buttonDebounce = false;
  }
  if(ButtonVal >= 600 && ButtonVal <= 700) // 4.7 Ohm (up button)
  {
    buttonState = 1; // up button
    buttonDebounce = false;
  }
  if(ButtonVal >= 500 && ButtonVal <= 600) // 10K Ohm (right button)
  {
    buttonState = 4; // right button
    buttonDebounce = false;
  }
}
////////////////////////////////////////////////////////////////////////////////
// resets the game /////////////////////////////////////////////////////////////
void resetGame()
{
    // Reset the buttonState.
  buttonState = 0;
    // Reset board.
}
////////////////////////////////////////////////////////////////////////////////
