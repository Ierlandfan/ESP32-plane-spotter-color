# ESP32-plane-spotter-color

ESP32 version of planeSpotter

# And this is how it all started 

So, this was my challenge. I bought myself an 3.5 inch Arduino compatible TFT Touch screen to be connected to an ESP8266 to be used for PlaneSpotter with a little experience with the Arduino IDE. Unfortunately this was an 8Bit parallel version and the ESP8266 doesn't have that many analogue connection. So I ordered another one, this time an SPI version TFT and connected that one to the ESP8266. Bodmer has a great driver library that included the ILI948 Rpi compatible TFT and after fiddling around everything worked, even touch. After more and more time I figured out the problem with the old PlaneSpotter version (Using Freefont crashed my ESP8266) fixed that, added some functions to make planeSpotter happy using this particulair TFT library and I got the hang of how the planespotter code worked.  

Succesfully changed a lot of things (I used to have some coding experience, now I have a lot more) 
to suit my needs and then I wanted more.
I wanted to create a more Flightradar24 based style  Touch a plane and it show more info. 
I wanted touch, I wanted a real plane (Not the red triangle) I wanted Pan and to zoom in and out, show real image of the plane when touching the Silhouette, jump to specific airfields...etc.etc. That became too much for the poor ESP866. Eventhough it worked, it wasn't very stable. I could try to find the errors (Ram issue with the Josn data) and optimize the Ram but I decided to postpone that.  
So, Job done (for now)

In the meantime I also stepped in the wonderfull world of ADSB antenna's, creating and optimizing them myself so I could feed adsb-exchange and Flightradar24 (There was poor coverage where I live) in return. Finding out that my old RTL_SDR FC0013 tuner was aging and became deaf above the 1000MHZ, the ordered one wasn't what I bought (FC0012is also deaf above 1000MHZ) I already wasted enough time (succesfully) convincing some suppliers they sold me fake products or that the produtcs never arrived. I skipped China and bought an real (way more expensive) RT820 in in a Dutch store. Just to make sure. It workes flawlessly. 
As a bonus I received a free business description on Flighradar24! 
So, Job done and Job done 

Back to the original plan.  
  
The 8bit parallel TFT I originally bought for this project was still untouched laying around, bothering me endlessly.
So I ordered a ESP32 and after finding and using konkrog's fork of TFT_eSPI (Supporting his Parallel TFT display for the ESP32) which was then just a few day old, I soldered the TFT (Lot of wiring) and nothing. Fast forwarding, After figuring how to install and how to modify the ESP32 compiler for the Arduino IDE (I had to use the same options as the ESP8266 compiler to allow warnings) and more fast forwarding in time, long before, I attached it to an Arduino so I had an idea what kind of display it was. (But it wasn't an ILI9488) So after some trial and error I also wrote the driver for the ILI9481 TFT which my TFT used and had to add them to the konkrogs TFT library. 

So I was finally ready for ESP32 version of planeSpotter.

More fast forwarding, I needed a modified version of SPIFFS for the ESP32, written just a few months before. I needed a unofficial  WiFiManager, DNSSserver and Webserver, it doesn't exist officially,  I needed to add my changes in the fork of konkrog (Some Adafruit specific functions to make planeSpotter happy.) Had to change a lot of references to the ESP32 WIFI library in planeSpotter and finally had to modify planespotter and the JPEG decoder to add a reference to the new SPIFFS library. Oh, and I had to add "/" tot the bitmap drawing part otherwise the map wasn't drawn. 

Finally it compiled. Pheww. Very stable but there's something wrong with the updating part. I have to investigate that but for now I wanted to put up the repo so someone else can use it as a base for other projects.

So for this to work you need:

ESP32 compiler: 
https://github.com/espressif/arduino-esp32
ESP32 SPIFFS
https://github.com/copercini/arduino-esp32-SPIFFS     
JPEG_DECODER (ESP8266 version will do)
https://github.com/fredericplante/JPEG_CODEC

Attention: You'll also have to open User_config.h in Arduino/libraries/JPEGDecoder-master and change

#define USE_SD_CARD
//#define USE_SPIFFS
into

//#define USE_SD_CARD
#define USE_SPIFFS


ESP32 WIFIManager
ESP32 DNSserver
ESP32 Webserver 
ESP32 mDNS 
Links are here
https://github.com/tzapu/WiFiManager/issues/241#issuecomment-315088825

All the other libraries can be installed by using the library manager. 

## Libraries

Install the following libraries:

Json Streaming Parser by Daniel Eichhorn

![Json Streaming Parser] (images/JsonStreamingParserLib.png)



Credits

This project wouldn't be possible if not for many open source contributors. Here are some I'd like to mention:

Daniel Eichhorn fot the hard work and the original concept (https://twitter.com/squix78) 
Bodmer for the original eTFT library and a lot of other ideas (bitmap drawing, touch, creating driver)
Forum: https://forum.arduino.cc/index.php?topic=443787.0)
Github https://github.com/Bodmer/TFT_eSPI
Konkrog for the Parallel TFT fork of TFT_eSPI 
Github: https://github.com/konkrog/TFT_eSPI
Frédéric Plante for his adaptations of the JPEGDecoder library
Rene Nyfenegger for the base64 encoder I got from here: http://www.adp-gmbh.ch/cpp/common/base64.html
Adafruit for the ILI9341 driver and potentially also for the original designs of the TFT display
 
 
