#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>

#define PIN D2

//define modes for LEDs
const uint8_t OFF = 0;
const uint8_t COLOR_WIPE_BLUE = 1;
const uint8_t COLOR_WIPE_GREEN = 2;
const uint8_t COLOR_WIPE_RED = 3;
const uint8_t RAINBOW = 4;
const uint8_t RAINBOW_CYCLE = 5;
const uint8_t THEATER_CHASE_WHITE = 6;
const uint8_t THEATER_CHASE_RED = 7;
const uint8_t THEATER_CHASE_GREEN = 8;

//store current mode
uint8_t MODE = OFF;
boolean isOff = true;

//WiFi password
const char WiFiAPPSK[] = "christmas";

WiFiServer server(80); //Initialize the server on port 80

//constants for JSON responses
const String responseHeader = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"data\": {";
const String responseFooter = "\r\n}\r\n}\n";
const String successStatus = "\r\n\"status\":\"success\",\n";
const String failureStatus = "\r\n\"status\":\"failure\",\n";
const String invalidRequestMessage = "\"message\":\"Invalid request.\"";

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(32, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

void setup() {
  //setup for WiFiServer
  Serial.begin(115200);
  delay(10);
  setupWiFi();

  //setup for LED strip
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void setupWiFi() {
  WiFi.mode(WIFI_AP);

  // Do a little work to get a unique-ish name. Append the
  // last two bytes of the MAC (HEX'd) to "Thing-":
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  String AP_NameString = "LED TREE " + macID;

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i=0; i<AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);

  WiFi.softAP(AP_NameChar, WiFiAPPSK);
  server.begin();
}

void loop() {
  handleHTTPRequest();
  switch (MODE) {
    case OFF:
      off();
      isOff = true;
      off();
      break;
    case COLOR_WIPE_BLUE:
      isOff = false;
      colorWipe(strip.Color(0, 0, 255), 50);
      break;
    case COLOR_WIPE_GREEN:
      isOff = false;
      colorWipe(strip.Color(0, 255, 0), 50);
      break;
    case COLOR_WIPE_RED:
      isOff = false;
      colorWipe(strip.Color(255, 0, 0), 50);
      break;
    case RAINBOW:
      isOff = false;
      rainbow(20);
      break;
    case RAINBOW_CYCLE:
      isOff = false;
      rainbowCycle(5);
      break;
    case THEATER_CHASE_WHITE:
      isOff = false;
      theaterChase(strip.Color(127, 127, 127), 50);
      break;
    case THEATER_CHASE_RED:
      isOff = false;
      theaterChase(strip.Color(127, 0, 0), 50);
      break;
    case THEATER_CHASE_GREEN:
      isOff = false;
      theaterChase(strip.Color(0, 127, 0), 50);
      break;
    default:
      break;
  }
}

void handleHTTPRequest() {
  //check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  //Wait for the client to send a request
  while(!client.available()) {
    delay(1);
  }

  //read the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  String validResponse;
  String errorResponse = responseHeader + failureStatus + invalidRequestMessage
      + responseFooter;
  //evaluate the request
  if (request.indexOf("/mode/off") != -1) {
    validResponse = successStatus + "\"message\":\"Mode changed to off.\"";
    MODE = OFF;
  } else if (request.indexOf("/mode/colorWipe") != -1) {
    //change the mode to color wipe
    //check for color
    if (request.indexOf("/blue") != -1) {
      validResponse = successStatus + "\"message\":\"Mode changed to colorWipe/blue\"";
      MODE = COLOR_WIPE_BLUE;
    } else if (request.indexOf("/green") != -1) { 
      validResponse = successStatus + "\"message\":\"Mode changed to colorWipe/green\"";
      MODE = COLOR_WIPE_GREEN;
    } else if (request.indexOf("/red") != -1) { 
      validResponse = successStatus + "\"message\":\"Mode changed to colorWipe/red\"";
      MODE = COLOR_WIPE_RED;
    } else {
      client.print(errorResponse);
      delay(1);
      client.stop();
      return;
    }
  } else if (request.indexOf("/mode/rainbow") != -1 
        && request.indexOf("/mode/rainbowCycle") == -1) {
    //change the mode to static rainbow
    validResponse = successStatus + "\"message\":\"Mode changed to rainbow\"";
    MODE = RAINBOW;
  } else if (request.indexOf("/mode/rainbowCycle") != -1) {
    validResponse = successStatus + "\"message\":\"Mode changed to rainbowCycle\"";
    MODE = RAINBOW_CYCLE;
  } else if (request.indexOf("/mode/theaterChase") != -1) {
     //change mode to theater chase
     //check for color
     if (request.indexOf("/white") != -1) {
      validResponse = successStatus + "\"message\":\"Mode changed to theaterChase/white\"";
      MODE = THEATER_CHASE_WHITE;
    } else if (request.indexOf("/green") != -1) { 
      validResponse = successStatus + "\"message\":\"Mode changed to theaterChase/green\"";
      MODE = THEATER_CHASE_GREEN;
    } else if (request.indexOf("/red") != -1) { 
      validResponse = successStatus + "\"message\":\"Mode changed to theaterChase/red\"";
      MODE = THEATER_CHASE_RED;
    } else {
      client.print(errorResponse);
      delay(1);
      client.stop();
      return;
    }
  } else {
    //invalid request - send back response indicating valid requests
    client.print(errorResponse);
    delay(1);
    client.stop();
    return;
  }

  client.flush();

  //prepare a response for a valid request
  String response = responseHeader + validResponse + responseFooter;

  // Send the response to the client
  client.print(response);
  delay(1);
  Serial.println("Client disonnected");
}

void off() {
  if (!isOff) {
    colorWipe(strip.Color(255, 0, 0), 50);
    strip.show();
    strip.show();
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(0 ,0 , 0));
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
