
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

  See more at http://blog.squix.ch
  (Tower picture was based on http://www.buildingsingapore.teoyusiang.com/)
  (Airplane picture were based on the blue encosed ones that adsbexchange are using for identifying in the infobox (rightdown corner box)
  ( Airport ICAO comes from https://openflights.org/data.html, please donate although not required)
*/


#include <Arduino.h>
//#include <Adafruit_GFX.h>
//SPIFFS stuff
#include <FS.h>
#include <SPI.h>

//TFT & Touch stuff

#include <TFT_eSPI.h> // Hardware-specific library
#include <XPT2046_Touchscreen.h>

// Wifi Libraries
#include <WiFi.h>
#include <mDNS.h>
#include <DNSServer.h>
#include <WebServer.h>

// Easy Wifi Setup
#include <WiFiManager.h>

// Go to settings to change important parameters
#include "settings.h"

// Project libraries
#include "WifiLocator.h"
#include "PlaneSpotter.h"
#include "artwork.h"
#include "AdsbExchangeClient.h"
#include "GeoMap.h"

// Initialize the TFT
TFT_eSPI tft = TFT_eSPI(); ;
WifiLocator locator;
AdsbExchangeClient adsbClient;
GeoMap geoMap(MapProvider::Google, GOOGLE_API_KEY, MAP_WIDTH, MAP_HEIGHT);
//GeoMap geoMap(MapProvider::MapQuest, MAP_QUEST_API_KEY, MAP_WIDTH, MAP_HEIGHT);
PlaneSpotter planeSpotter(&tft, &geoMap);


XPT2046_Touchscreen ts(TOUCH_CS);  // Param 2 - NULL - No interrupts
//XPT2046_Touchscreen ts(TOUCH_CS, 255);  // Param 2 - 255 - No interrupts
//XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);  // Param 2 - Touch IRQ Pin - interrupt enabled polling

const int UPDATE_INTERVAL_SECS_LONG = 5 * 1000; // Update every 5 seconds if no airplanes around //Default in original
const int UPDATE_INTERVAL_SECS_SHORT = 3 * 1000; // Update every 3 seconds if there are airplanes //Default in Planespotter --no-color (Oled) version

int currentUpdateInterval = UPDATE_INTERVAL_SECS_LONG;
long lastUpdate = 0;

Coordinates mapCenter;

// Check http://www.virtualradarserver.co.uk/Documentation/Formats/AircraftList.aspx
// to craft this query to your needs
// lat=47.424341887&lng=8.56877803&fDstL=0&fDstU=10&fAltL=0&fAltL=1500&fAltU=10000
//const String QUERY_STRING = "fDstL=0&fDstU=20&fAltL=0&fAltL=1000&fAltU=10000";
// airport zürich is on 1410ft => hide landed airplanes
//"fAltL=1500&trFmt=sa"; // Original, fmt draws the heading lines
//const String QUERY_STRING = "fDstL=0&fAltH=2000fDstU=30"; // Show only planes below 2000Ft and 30KM radius
const String QUERY_STRING = "fAltL=0&fDstU=20&trFmt=sa"; //Show all planes in 30KM radius, and from any (0) level, fmt draws the heading lines (f= full trails, s = short trails,  a= for altitudes, s for speeed)

void downloadCallback(String filename, uint32_t bytesDownloaded, uint32_t bytesTotal, boolean isFirstCall);
ProgressCallback _downloadCallback = downloadCallback;
void updatePlanesAndDrawMap();
void calibrateTouchScreen(); //calibration data


Coordinates northWestBound;
Coordinates southEastBound;

long millisAtLastUpdate = 0;
long millisAtLastTouch = 0;
uint8_t currentZoom = MAP_ZOOM;
char currentPage;
void setup() {

  // Start serial communication
  Serial.begin(115200);
  Serial.println("Free Heap: " + String(ESP.getFreeHeap()));
  // The LED pin needs to set HIGH
  // Use this pin to save energy
  //pinMode(LED_PIN, D8);
  //digitalWrite(LED_PIN, HIGH);

  // Init TFT
  tft.begin();
  tft.setRotation(1);  // landscape Have to fix Etft ILI9481 for 3
  tft.cp437(false);
  tft.setTextFont(2); // To do Change the default TextFont and make it bigger
  tft.fillScreen(TFT_BLACK);
  planeSpotter.setTextColor(TFT_WHITE, TFT_BLACK);
  planeSpotter.setTextAlignment(CENTER);
  planeSpotter.drawString(240, 200, "     Loading Splash...     "); //Changed for 320*480

  // Init file system
  if (!SPIFFS.begin()) {
    Serial.println("initialisation failed!");
    return;
  }
//SPIFFS.format();

  // copy files from code to SPIFFS
  planeSpotter.copyProgmemToSpiffs(splash, splash_len, "/splash.jpg");
  planeSpotter.copyProgmemToSpiffs(plane, plane_len, "/plane.jpg");
  planeSpotter.copyProgmemToSpiffs(ga, ga_len, "/ga.jpg");
  planeSpotter.copyProgmemToSpiffs(glider, glider_len, "/glider.jpg");
  planeSpotter.copyProgmemToSpiffs(heli, heli_len, "/heli.jpg");
  planeSpotter.copyProgmemToSpiffs(jet, jet_len, "/jet.jpg");
  planeSpotter.copyProgmemToSpiffs(tower, tower_len, "/tower.jpg");
planeSpotter.copyProgmemToSpiffs(ultralight, ultralight_len, "/ultralight.jpg");
  planeSpotter.drawSPIFFSJpeg("/splash.jpg", 120, 75); //30,75 Changed alignment for 320*480
    planeSpotter.setTextColor(TFT_WHITE, TFT_BLACK);
  planeSpotter.setTextAlignment(CENTER);
  planeSpotter.drawString(240, 200, "     Connecting to WiFi..     "); //Changed for 320*480


  WiFiManager wifiManager;
  // Uncomment for testing wifi manager
  //wifiManager.resetSettings();

  //wifiManager.setAPCallback(configModeCallback);

  //or use this for auto generated name ESP + ChipID
  wifiManager.autoConnect();

  // Set center of the map by using a WiFi fingerprint
  // Hardcode the values if it doesn't work or you want another location
  locator.updateLocation();
  //  mapCenter.lat = locator.getLat().toFloat(); // Auto find your location , Uncomment these and hash out the others below if you want to use this
  //  mapCenter.lon = locator.getLon().toFloat(); // Auto find your location, , Uncomment hese and hash out the others below if you want to use this
mapCenter.lat = 52.8153588; //EHMM
mapCenter.lon =  5.0353076; //EHMM
//  mapCenter.lat = 52.3745913; //EHAM, testing, lot of planes here!
//  mapCenter.lon =  4.8285746; //EHAM, testing, lot of planes here!
//    mapCenter.lat = 52.6141477; // Alkmaar, Netherlands
//    mapCenter.lon = 4.7125713; 
  

  planeSpotter.setTextColor(TFT_WHITE, TFT_BLACK);
  planeSpotter.setTextAlignment(CENTER);
  planeSpotter.drawString(240, 200, "          Loading map...          "); //Changed for 320*480
  geoMap.downloadMap(mapCenter, currentZoom, _downloadCallback);

  northWestBound = geoMap.convertToCoordinates({ -MAP_REQUEST_MARGIN, -MAP_REQUEST_MARGIN});
  southEastBound = geoMap.convertToCoordinates({MAP_WIDTH + MAP_REQUEST_MARGIN, MAP_HEIGHT + MAP_REQUEST_MARGIN});

  ts.begin();
  planeSpotter.setTouchScreen(&ts);
  //  currentPage ='0';
}


/* added */
 
void loop () {
  // If there are airplanes query often
  
  if (adsbClient.getNumberOfAircrafts() == 0) {
    currentUpdateInterval = UPDATE_INTERVAL_SECS_LONG;
  } else {
    currentUpdateInterval = UPDATE_INTERVAL_SECS_SHORT;
  }

  if (millis() - millisAtLastUpdate > currentUpdateInterval) {
    millisAtLastUpdate = millis();
    Serial.println("currentUpdateInterval is: ");
    Serial.println (millisAtLastUpdate); 
    Serial.println(currentUpdateInterval);  
    updatePlanesAndDrawMap() ;
 //Or just update when closestplane -- 
  }
}

/*
void loopB(){
  boolean isTouched = ts.touched();
  // Start loopPart1

  if (isTouched && millis() - millisAtLastTouch > 1000) {//If we want to touch a plane it will be less so this is quite high
      millisAtLastUpdate = millis();
      updatePlanesAndDrawMap();
  currentPage ='0';
  }
  }
}
*/

/*
  //Start loop Part2
  CoordinatesPixel pt = planeSpotter.getTouchPoint();
  if (currentPage == '0'){
  boolean isTouched = ts.touched();
  CoordinatesPixel pt = planeSpotter.getTouchPoint();
  if (isTouched && millis() - millisAtLastTouch > 3000 && ( (pt.x>=0) && (pt.x<=480) && (pt.y<=70) && (pt.y>=0) ) ) {//If we want to touch a plane it will be less so this is quite high
  if ((pt.x>=0) && (pt.x<=480) && (pt.y<=70) && (pt.y>=0)) { //Only when touched in the map we want this to show up
    Serial.println("PanAndZoom pressed, Page is 1");
    tft.fillCircle(pt.x, pt.y, 10, TFT_BLUE);
     millisAtLastTouch = millis();
     delay(60);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    planeSpotter.drawZoomAndPanMenu();
  currentPage ='1';
  }//touchpoints
  }//milis
  }

  //If we are in ZoomAndPanMenu and touched
  if (currentPage == '1'){
  //If we press the Zoom IN button
  int currentZoomlevel = geoMap.getCurrentZoomlevel(); //Current ZoomLevel
  if ((pt.x>=400) && (pt.x>=420) && (pt.x<=480) && (pt.y>=0) &&(pt.y<=60)) {
      Serial.println("ZOOM IN pressed, Page is 1");
       tft.fillCircle(pt.x, pt.y, 2, TFT_GREEN);
  int currentZoomlevel;
  int i=0;
  for (i = 0; i<2; i++){
  currentZoomlevel++;
  } // end For loop
  Serial.println("ZOOM level is test");
  geoMap.downloadMap(mapCenter, currentZoomlevel, _downloadCallback);
   tft.fillRect(0, geoMap.getMapHeight(), tft.getWidth(), tft.getHeight() - geoMap.getMapHeight(), TFT_BLACK);
  updatePlanesAndDrawMap();
  } //touch

  //still at page 1
  //If we press the Zoom OUT button
  if ((pt.x>=400) && (pt.x<=480) && (pt.y>=200) && (pt.y<=252)) {
  Serial.println("ZOOM OUT pressed, Page is 2");
       tft.fillCircle(pt.x, pt.y, 10, TFT_ORANGE);
  int currentZoomlevel;
  int i=0;
  for (i = 0; i>1; i--){
  currentZoomlevel--;
  Serial.println("ZOOM OUT pressed, Page is 2");
             } // end For loop
  Serial.println("ZOOM level Out test");
  geoMap.downloadMap(mapCenter, currentZoomlevel, _downloadCallback);
   tft.fillRect(0, geoMap.getMapHeight(), tft.getWidth(), tft.getHeight() - geoMap.getMapHeight(), TFT_BLACK);
  updatePlanesAndDrawMap();
  }//touch

  //still at page 1
  //If we press the reset button
   if ((pt.x>=200) && (pt.x<=280) && (pt.y>=120) && (pt.y<=200)) {
  Serial.println("Reset pressed");
       tft.fillCircle(pt.x, pt.y, 10, TFT_RED);
  Serial.println("Reset to default zoom");
  geoMap.downloadMap(mapCenter, currentZoom, _downloadCallback);  //currentzoom is default zoom
   tft.fillRect(0, geoMap.getMapHeight(), tft.getWidth(), tft.getHeight() - geoMap.getMapHeight(), TFT_BLACK);
  updatePlanesAndDrawMap();
   }

  //still at page 1
  //If we press the pan_left button
  //  if ((pt.x<=40) && (pt.x>=0) && (pt.y>=145) && (pt.y<=200)) {
  //        Serial.println("PAN LEFT pressed, Page is 2");
  //      tft.fillCircle(pt.x, pt.y, 10, TFT_ORANGE);
  //int MapWidth =  geoMap.getCurrentZoomlevel();
  //int currentMapWidth = MapWidth;
  //int i=0;
  //for (i = 0; i<2; i++){
  //  currentMapWidth++;
  //}
  //  geoMap.downloadMap(mapCenter, currentMapWidth, _downloadCallback);
  //    Serial.println("currentMapWidth +1");
  //   tft.fillRect(0, geoMap.getMapHeight(), tft.getWidth(), tft.getHeight() - geoMap.getMapHeight(), TFT_BLACK); //Shows the plane waiting sequence
  // updatePlanesAndDrawMap();

  //} //touch

  //still at page 1
  //If we press the pan_right button
   if ((pt.x>=400) && (pt.x<=480) && (pt.y>=130) && (pt.y<=190)) {
  Serial.println("PAN RIGHT pressed, Page is 2");
       tft.fillCircle(pt.x, pt.y, 10, TFT_ORANGE);

  //        }//code
  } //touch

  //still at page 1
  //If we press the pan_up button
  if ((pt.x>=210) && (pt.x<=270) && (pt.y>=145) && (pt.y<=200)) {
   Serial.println("PAN UP pressed, Page is 2");
       tft.fillCircle(pt.x, pt.y, 10, TFT_ORANGE);


   //   } //Code
  } //touch
  //still at page 1
  //If we press the pan_down button
  if ((pt.x>=210) && (pt.x<=270) && (pt.y>=0) && (pt.y<=60)) {
  Serial.println("PAN DOWN pressed, Page is 2");
       tft.fillCircle(pt.x, pt.y, 10, TFT_ORANGE);
           //   } //Code
        } //End touch

  }
  }
  }
*/

void downloadCallback(String filename, uint32_t bytesDownloaded, uint32_t bytesTotal, boolean isFirstCall) {
  if (isFirstCall) {
    tft.fillRect(0, geoMap.getMapHeight(), tft.getWidth(), tft.getHeight() - geoMap.getMapHeight(), TFT_BLACK);
  }
  Serial.println(String(bytesDownloaded) + " / " + String(bytesTotal));
  int width = 420; //changed to fit width of screen better
  int progress = width * bytesDownloaded / bytesTotal;
  tft.fillRect(10, 200, progress, 5, TFT_WHITE); //Changed hight a little
  planeSpotter.drawSPIFFSJpeg("/plane.jpg", 15 + progress, 200 - 15); //This draws the plane
}

void updatePlanesAndDrawMap() {
  Serial.println("Heap: " + String(ESP.getFreeHeap()));
  adsbClient.updateVisibleAircraft(QUERY_STRING + "&lat=" + String(mapCenter.lat, 6) + "&lng=" + String(mapCenter.lon, 6) + "&fNBnd=" + String(northWestBound.lat, 9) + "&fWBnd=" + String(northWestBound.lon, 9) + "&fSBnd=" + String(southEastBound.lat, 9) + "&fEBnd=" + String(southEastBound.lon, 9));
  long startMillis = millis();
  planeSpotter.drawSPIFFSJpeg("/" +geoMap.getMapName(), 0, 0);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  Aircraft closestAircraft = adsbClient.getClosestAircraft(mapCenter);
  for (int i = 0; i < adsbClient.getNumberOfAircrafts(); i++) {
    Aircraft aircraft = adsbClient.getAircraft(i);
    AircraftHistory history = adsbClient.getAircraftHistory(i);
    planeSpotter.drawAircraftHistory(aircraft, history);
    planeSpotter.drawPlane(aircraft, aircraft.call == closestAircraft.call);


  }

  planeSpotter.drawInfoBox(closestAircraft);
  // Draw center of map
  CoordinatesPixel p = geoMap.convertToPixel(mapCenter);
  tft.fillCircle(p.x, p.y, 2, TFT_BLUE);

  Serial.println(String(millis() - startMillis) + "ms for drawing");
}



