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


*/

#include "PlaneSpotter.h"

PlaneSpotter::PlaneSpotter(TFT_eSPI* tft, GeoMap* geoMap) {
  tft_ = tft;
  geoMap_ = geoMap;
}

void PlaneSpotter::copyProgmemToSpiffs(const uint8_t *data, unsigned int length, String filename) {
  File f = SPIFFS.open(filename, "w+");
  uint8_t c;
  for(int i = 0; i < length; i++) {
    c = pgm_read_byte(data + i);
    f.write(c);
  }
  f.close();
}


void PlaneSpotter::drawSPIFFSJpeg(String filename, int xpos, int ypos) {
  
  Serial.println(filename);
  char buffer[filename.length() + 1];
  filename.toCharArray(buffer, filename.length() + 1);
  JpegDec.decodeFile(buffer);
  renderJPEG(xpos, ypos);
  
}

void PlaneSpotter::renderJPEG(int xpos, int ypos) {

  uint16_t  *pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t mcu_pixels = mcu_w * mcu_h;
  uint32_t drawTime = millis();

  while( JpegDec.read()){
    
    pImg = JpegDec.pImage;
    int mcu_x = JpegDec.MCUx * mcu_w + xpos;
    int mcu_y = JpegDec.MCUy * mcu_h + ypos;
    if ( ( mcu_x + mcu_w) <= tft_->getWidth() && ( mcu_y + mcu_h) <= tft_->getHeight()){
      
      tft_->setAddrWindow(mcu_x, mcu_y, mcu_x + mcu_w - 1, mcu_y + mcu_h - 1);
      uint32_t count = mcu_pixels;
      while (count--) {tft_->pushColor(*pImg++);}
      // Push all MCU pixels to the TFT window, ~18% faster to pass an array pointer and length to the library
      //tft_->pushColor16(pImg, mcu_pixels); //  To be supported in HX8357 library at a future date

    }

    else if( ( mcu_y + mcu_h) >= tft_->getHeight()) JpegDec.abort();
  
  }


}

void PlaneSpotter::drawAircraftHistory(Aircraft aircraft, AircraftHistory history) {
    Coordinates lastCoordinates;
    lastCoordinates.lat = aircraft.lat;
    lastCoordinates.lon = aircraft.lon;
    for (int j = 0; j < min(history.counter, MAX_HISTORY); j++) {

      AircraftPosition position = history.positions[j];
      Coordinates coordinates = position.coordinates;
      CoordinatesPixel p1 = geoMap_->convertToPixel(coordinates);

      CoordinatesPixel p2 = geoMap_->convertToPixel(lastCoordinates);
      uint16_t color = heightPalette_[min(position.altitude / 4000, 9)];
      tft_->drawLine(p1.x, p1.y, p2.x, p2.y, color);
      tft_->drawLine(p1.x+1, p1.y+1, p2.x+1, p2.y+1, color);

      lastCoordinates = coordinates;
      // Serial.println(String(j) + ". " + String(historyIndex) + ". " + String(coordinates.lat) + ", " + String(coordinates.lon));
    }
}

// This draws callsign box
void PlaneSpotter::drawPlane(Aircraft aircraft, boolean isSpecial) {
  Coordinates coordinates;
  coordinates.lon = aircraft.lon;
  coordinates.lat = aircraft.lat;  
  CoordinatesPixel p = geoMap_->convertToPixel(coordinates);
  
  setTextColor(TFT_WHITE, TFT_BLACK);
  setTextAlignment(LEFT);
  drawString(p.x + 8,p.y - 5, aircraft.call);
  
  // This draws the red Triangle
  int planeDotsX[planeDots_];
  int planeDotsY[planeDots_];
  //isSpecial = false;
  for (int i = 0; i < planeDots_; i++) {
    planeDotsX[i] = cos((-450 + (planeDeg_[i] + aircraft.heading)) * PI / 180) * planeRadius_[i] + p.x; 
    planeDotsY[i] = sin((-450 + (planeDeg_[i] + aircraft.heading)) * PI / 180) * planeRadius_[i] + p.y; 
  }
  
  if (isSpecial) {
    tft_->fillTriangle(planeDotsX[0], planeDotsY[0], planeDotsX[1], planeDotsY[1], planeDotsX[2], planeDotsY[2], TFT_RED);
    tft_->fillTriangle(planeDotsX[2], planeDotsY[2], planeDotsX[3], planeDotsY[3], planeDotsX[4], planeDotsY[4], TFT_RED);
  } else {
      for (int i = 1; i < planeDots_; i++) {
        tft_->drawLine(planeDotsX[i], planeDotsY[i], planeDotsX[i-1], planeDotsY[i-1], TFT_RED);
     }
  }
}

 


// Todo 
//   int species;
//   species = geomap.getSpecies;   
//  if (species =


void PlaneSpotter::drawInfoBox(Aircraft closestAircraft) {
  int line1 = geoMap_->getMapHeight() + 7;
  int line2 = geoMap_->getMapHeight() + 20;
  int line3 = geoMap_->getMapHeight() + 31;
  int line4 = geoMap_->getMapHeight() + 42;
  int line5 = geoMap_->getMapHeight() + 53;
  int linetype = geoMap_->getMapHeight() + 26;
    int rightTab0 = tft_->getWidth() - 3; //
  int leftTab1 = 0;
  int leftTab2 = 60; // was 40
  int leftTab3 = tft_->getWidth() / 2 + 40 ; // was / 2
  int leftTab4 = leftTab3 + 60; // was leftfTab3 + 40
  int leftTab5 = leftTab4 + 55;
  int leftTabtype = tft_->getWidth() /2 -70;  
  int towerMenuX = geoMap_->getMapHeight()+312;
  int towerMenuY = geoMap_->getMapWidth () +310 ;     
  //tft_->setFont(&Dialog_plain_9);
/* Text modified  by Ierlandfan for bigger screen */ 


//Heading as a circle drawn in infobox

//int degrees[] = {0, 170, 190, 0};
//tft_->drawCircle(leftTab4, line2, 10, TFT_WHITE); // x,y is the spot where it will be drawn
//tft_->fillTriangle(planeDotsX[0], planeDotsY[0], planeDotsX[1], planeDotsY[1], planeDotsX[2], planeDotsY[2], TFT_RED);
//int radius = 8;  
//for (int i = 0; i < 3; i++) {
//int x1 = cos((-450 + (closestAircraft.heading + degrees[i])) * PI / 180) * planeRadius_[i] + leftTab5; 
//int y1 = sin((-450 + (closestAircraft.heading + degrees[i])) * PI / 180) * planeRadius_[i] + line2; 
//int x2 = cos((-450 + (closestAircraft.heading + degrees[i+1])) * PI / 180) * planeRadius_[i] + leftTab5; 
//int y2 = sin((-450 + (closestAircraft.heading + degrees[i+1])) * PI / 180) * planeRadius_[i] + line2; 

 //    tft_->drawLine(x1, y1, x2, y2, TFT_RED);
//}
  //End Heading draw  

 
  tft_->fillRect(0, geoMap_->getMapHeight(), tft_->getWidth(), tft_->getHeight() - geoMap_->getMapHeight(), TFT_BLACK);
//drawSPIFFSJpeg("/tower.jpg",towerMenuX,towerMenuY);  
  if (closestAircraft.call != "") {          
    
    setTextAlignment(LEFT);
    setTextColor(TFT_GREEN);
    drawString(leftTab1, line1, "Reg: ");
    setTextColor(TFT_WHITE);
    drawString(leftTab2, line1, closestAircraft.registration);
   
    setTextColor(TFT_GREEN);
    drawString(leftTab3, line1, "Brand: "); //Changed to Brand because of type declared later for type of airplane
    setTextColor(TFT_WHITE);


    
    drawString(leftTab4, line1, closestAircraft.aircraftType);
    
    setTextColor(TFT_GREEN);
    drawString(leftTab1, line2, "Altitude: ");
    setTextColor(TFT_WHITE);
    drawString(leftTab2, line2, String(closestAircraft.altitude) + " ft");

    setTextColor(TFT_GREEN);
    drawString(leftTab3, line2, "Heading: ");
    setTextColor(TFT_WHITE);
    drawString(leftTab4, line2, String(closestAircraft.heading, 0));

 
    setTextColor(TFT_GREEN);
    drawString(leftTab1, line3, "Distance: ");
    setTextColor(TFT_WHITE);
    drawString(leftTab2, line3, String(closestAircraft.distance, 2) + " km");
    setTextColor(TFT_GREEN);
    drawString(leftTab3, line3, "HSpeed: ");
    setTextColor(TFT_WHITE);
    drawString(leftTab4, line3, String(closestAircraft.speed, 0) + " kn");
    setTextColor(TFT_GREEN);
    drawString(leftTab3, line4, "VSpeed: ");
    setTextColor(TFT_WHITE);
    drawString(leftTab4, line4, String(closestAircraft.vspeed, 0) + " ft/s");
// Species/TypeOfAircraft 
    setTextColor(TFT_GREEN);
     drawString(leftTabtype, line1, "Type: ");


   int Species = closestAircraft.species;
const char * tag = closestAircraft.registration.c_str ();
   String chassis;      
uint16_t chassiscolor = TFT_WHITE;
String enginetype = closestAircraft.enginetype;   
   switch(Species) {
      case 1: {
Serial.println("TAG is:");
Serial.println(tag);
int i =0;
for (i=0;i<strlen(tag); i++){

if ( (tag[i] == 'P') && (tag[i] == 'H')&& (tag[i] == '-') && ((tag[i] >= '0' && tag[i] <= '9')) && ((tag[i] >= '0' && tag[i] <= '9'))&& (( tag[i] >= '0' && tag[i] <= '9')) && ((tag[i] >= '0' && tag[i] <= '9')))  
// Glider
{
chassis = "Glider";
}

else if ( (tag[i] == 'P') && (tag[i] == 'H') &&  (tag[i] == '-') &&  ((tag[i] >= '0' && tag[i] <= '9')) &&  ((tag[i] >= 'A' && tag[i] <= 'Z')) && ((tag[i] >= 'A' && tag[i] <= 'Z')) )  
// Drone
{
chassis = "Drone";
}
else if 
( (tag[i] == 'P')&& (tag[i] == 'H') && (tag[i] == '-') &&  ((tag[i] >= '0' && tag[i] <= '9')) && ((tag[i] >= 'A' && tag[i] <= 'Z')) && ((tag[i] >= '0' && tag[i] <= '9')) )
//Ultralight
{
chassis = "Ultralight";
//draw jpeg ultralight
chassiscolor = TFT_RED;
drawSPIFFSJpeg("/ultralight.jpg", leftTabtype, linetype);
}
}     
if(enginetype == "1")
{
//Draw GA like plane 
chassiscolor = TFT_RED;
chassis = "GA";
drawSPIFFSJpeg("/ga.jpg", leftTabtype, linetype); //This draws the type of plane 
}

else if (enginetype == "2")

{
chassis = "Turboprop";
//draw turboprop like
}
else if (enginetype == "3")
{
chassis = "Jet";
//draw jet
drawSPIFFSJpeg("/jet.jpg", leftTabtype, linetype); //This draws the type of plane 
//drawSPIFFSJpeg("/heli.jpg", 240 , 160); //This draws the type of plane
}
 
else if (enginetype == "4")
{
chassis = "Electric";
}
else
{
chassis = "Land Plane";
}
   } //case 1
      
       break;
      case 2: {
       chassis = "Sea plane"; 
      chassiscolor = TFT_BLUE;
      }
         break;
      case 3: {
         chassiscolor = TFT_GREEN;
         chassis = "Amphibian";
      }
         break;
      case 4: {
        chassiscolor = TFT_NAVY;
        chassis = "Helicopter";
drawSPIFFSJpeg("/heli.jpg", leftTabtype ,linetype);
      }
         break;
      case 5: {
        chassis = "Gyrocopter";
      }
         break;
      case 6: {
        chassis = "Tiltwing";
      }
         break;
      case 7: {
        chassis = "Ground Vehicle";
      }
         break;
      case 8: {
         chassis = "Tower";
      }
      break;       
     default: {
        chassis = "N/A";
     }
   }
    setTextColor(chassiscolor);
    drawString(leftTabtype + 40, line1, chassis);
  }
   if (closestAircraft.fromShort != "" && closestAircraft.toShort != "") {
      setTextColor(TFT_GREEN);
      drawString(leftTab1, line5, "From: ");
      setTextColor(TFT_WHITE);
      drawString(leftTab1 + 45, line5, (String ( (closestAircraft.fromCode) + (closestAircraft.fromShort) ) ) ); // Changed, now under eacht other and made shorter to fit screen better
      setTextColor(TFT_GREEN);
      drawString(leftTab3, line5, "To: ");    // Changed, now under each other  
      setTextColor(TFT_WHITE);
      drawString(leftTab3 + 30, line5, (String( (closestAircraft.toCode) + (closestAircraft.toShort) ) ) ); // Changed, now under eacht other and made shorter to fit screen better
      Serial.println("(String( (closestAircraft.toCode) + (closestAircraft.toShort) ) )");
  

    }
  }
void PlaneSpotter::drawMainMenu() {
  tft_->setTextFont(2);
  String commands[] = {"Presets", "Track", "Weather Station", "Planespotter", "Back"};
  String menutitle = {"Main Menu"};
  tft_->fillScreen(TFT_BLACK);
  int numberOfCommands = 5;
  int fontHeight = 24;
  int buttonHeight = 40;
  drawString(10, buttonHeight - fontHeight / 2, menutitle);  //Is this even working?
     for (int i = 0; i < numberOfCommands; i++) {
      tft_->drawFastHLine(0, i * buttonHeight, tft_->getWidth(), TFT_WHITE);
    drawString(20, i * buttonHeight + (buttonHeight - fontHeight) / 2, commands[i]); 

  }
}




// Todo -- Define defaults somewehere in Settings
//Jump to predefined menu's //

String Preset1 = "EHAM, Amsterdam Schiphol";
String Preset2 = "EHLE, Lelystad Airport"; 
String Preset3 = "EHEH, Eindhoven Airport";
String Preset4 = "EHRD, Rotterdam Airport";
String Preset5 = "EHKD, Den Helder";
String Preset6 = "EHTX, Texel Airport";
String Preset7 = "Current location";
String Preset8 = "Back";
//to do -- create menu to jump to predefined menu's 
void PlaneSpotter::drawPresetMenu() {
  tft_->setTextFont(2);
  //String commands[] = {"Presets", "Track", "Weather Station", "Planespotter"};
  String commands[] = {Preset1, Preset2, Preset3, Preset4, Preset5, Preset6, Preset7, Preset8};
  String menutitle = {"Preset Menu"};
  tft_->fillScreen(TFT_BLACK);
  int numberOfCommands = 7;
  int fontHeight = 24;
 int buttonHeight = 40;
  drawString(10, buttonHeight - fontHeight / 2, menutitle);  //Is this even working?
  for (int i = 0; i < numberOfCommands; i++) {
    tft_->drawFastHLine(0, i * buttonHeight, tft_->getWidth(), TFT_WHITE);
    drawString(20, i * buttonHeight + (buttonHeight - fontHeight) / 2, commands[i]); 
     

  }
}

void PlaneSpotter::drawZoomAndPanMenu() {
  tft_->setTextFont(2);
        int fontHeight = 24;
        String Zoom_in = "Zoom in";
        String Zoom_out = "Zoom out";
        String Reset    = "Reset";
        String UP = "Up";
        String Down = "Down"; 
        String Left = "Left";
        String Right = "Right";

//int PHline1 = geoMap_->getMapHeight() /2;
//int PVline1 = geoMap_->getMapWidth()  /2;
//int ZHline1 = geoMap_->getMapWidth()  /4; 
//int ZVline1 = geoMap_->getMapHeight() /4;  
     drawString(420, 40 - fontHeight, Zoom_in );
     drawString(420, 240 - fontHeight, Zoom_out );
     drawString(220, 125 - fontHeight, Reset );
     drawString(220, 20 - fontHeight, UP );
     drawString(220, 250 - fontHeight, Down );
     drawString(4, 125 - fontHeight, Left );
     drawString(460, 125 - fontHeight, Right );
}
 




void PlaneSpotter::drawString(int x, int y, char *text) {
  int16_t x1, y1;
  uint16_t w, h;
  tft_->setTextWrap(false);
  tft_->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  switch (alignment_) {
    case LEFT:
      x1 = x;
      break;
    case CENTER:
      x1 = x - w / 2;
      break;
    case RIGHT:
      x1 = x - w;
      break;
  }
  if (textColor_ != backgroundColor_) {
    tft_->fillRect(x1, y - h -1, w + 2, h + 3, backgroundColor_);
  }
  tft_->setCursor(x1, y);
  tft_->print(text);
}

void PlaneSpotter::drawString(int x, int y, String text) {
  char buf[text.length()+2];
  text.toCharArray(buf, text.length() + 1);
  drawString(x, y, buf);
}

void PlaneSpotter::setTextColor(uint16_t c) {
  setTextColor(c, c);
}
void PlaneSpotter::setTextColor(uint16_t c, uint16_t bg) {
  textColor_ = c;
  backgroundColor_ = bg;
  tft_->setTextColor(textColor_, backgroundColor_);
}

void PlaneSpotter::setTextAlignment(TextAlignment alignment) {
  alignment_ = alignment;
}

void PlaneSpotter::setTouchScreen(XPT2046_Touchscreen* touchScreen) {
  touchScreen_ = touchScreen;
}

void PlaneSpotter::setTouchScreenCalibration(uint16_t minX, uint16_t minY, uint16_t maxX, uint16_t maxY) {
  minX_ = minX;
  minY_ = minY;
  maxX_ = maxX;
  maxY_ = maxY;
}

CoordinatesPixel PlaneSpotter::getTouchPoint() {
    TS_Point pt = touchScreen_->getPoint();
    CoordinatesPixel p;
    p.y = tft_->getHeight() * (pt.y - minY_) / (maxY_ - minY_);
    p.x = tft_->getWidth() * (pt.x - minX_) / (maxX_ - minX_);
    return p;
}

