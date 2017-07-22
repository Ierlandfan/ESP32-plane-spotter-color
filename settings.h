/**The MIT License (MIT)

Copyright (c) 2015 by Daniel Eichhorn

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

See more at https://blog.squix.org




#define PIN_D0 16
#define PIN_D1  5
#define PIN_D2  4 //TOUCH_CS
#define PIN_D3  0 //TFT_DC
#define PIN_D4  2
#define PIN_D5 14 // SCLK
#define PIN_D6 12 // MISO
#define PIN_D7 13 // MOSI
#define PIN_D8 15 //TFT_CS
#define PIN_D9  3
#define PIN_D10 1
#define USE_HW_CS

*/

#define PIN_D0 16
#define PIN_D1  5
#define PIN_D2  4 //TOUCH_CS
#define PIN_D3  0 //TFT_DC
#define PIN_D4  2
#define PIN_D5 14 // SCLK
#define PIN_D6 12 // MISO
#define PIN_D7 13 // MOSI
#define PIN_D8 15 //TFT_CS
#define PIN_D9  3
#define PIN_D10 1
#define USE_HW_CS

#define TOUCH_CS  PIN_D2 // XPT2046 chip select
#define TFT_CS  PIN_D8 // TFT chip select
#define TFT_DC PIN_D3 // TFT DC
#define LED_PIN PIN_D0 // LED not used


// Needed for loading the map. If you want to be save better get your own key here:
// https://developer.mapquest.com/plan_purchase/steps/business_edition/business_edition_free/register
#define MAP_QUEST_API_KEY "r19I8UVBfwIkmE4EZR9S6yMR43eMiRDZ"

// Need for loading map by google static map api. If you want to be save better create one here
// https://developers.google.com/maps/documentation/static-maps/get-api-key?hl=de
#define GOOGLE_API_KEY "AIzaSyBw0G8jCBry0IATNmysuyPd2fBblndS3jU"

#define MAP_ZOOM 10 // For lines otherwise not working
#define MAP_WIDTH 480
#define MAP_HEIGHT 252 // This defines the black text area under the map
// How many pixels outside the visible map should planes be requested
#define MAP_REQUEST_MARGIN 40


