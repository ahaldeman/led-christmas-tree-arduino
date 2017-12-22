#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>

#define PIN D2

//define modes for LEDs
#define COLOR_WHEEL_BLUE blue
#define COLOR_WHEEL_GREEN green
#define COLOR_WHEEL_RED red
#define RAINBOW rainbow

WiFiServer server(80); //Initialize the server on port 80

//ssid and password for WiFi network
const char* ssid = "Fios-OKAZD";
const char* password = "jet5036fade34rick";

//constants for HTTP responses
const String responseHeader = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>";
const String responseFooter = "</html>\n";

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
  setupWiFiServer();

  //setup for LED strip
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void setupWiFiServer() {
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop() {
  handleHTTPRequest();
  switch
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
  String errorResponse = responseHeader + "\r\nInvalid request. Valid requests are: \n"
      + "<serverIP>/mode/colorWipe/blue\n"
      + "<serverIP>/mode/colorWipe/green\n"
      + "<serverIP>/mode/colorWipe/red\n" 
      + "<serverIP/mode/rainbow\n"
      + responseFooter;
  //evaluate the request
  if (request.indexOf("/mode/colorWipe") != -1) {
    //change the mode to color wipe
    //check for color
    if (request.indexOf("/blue") != -1) {
      validResponse = "\r\nMode changed to colorWipe/blue\n";
    } else if (request.indexOf("/green")) { 
      validResponse = "\r\nMode changed to colorWipe/green\n";
    } else if (request.indexOf("/red")) { 
      validResponse = "\r\nMode changed to colorWipe/red\n";
    } else {
      client.print(errorResponse);
      delay(1);
      client.stop();
      return;
    }
  } 
  else if (request.indexOf("/mode/rainbow") != -1) {
    //change the mode to static rainbow
    validResponse = "\r\nMode changed to rainbow\n";
  }
  else {
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
  
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
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

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
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
