/*
Cassiopeia:
lighting system for growing bonsai indoors during the 
cold dark winter days
*/

#include <SPI.h>
#include <WiFiNINA.h>

#include "arduino_secrets.h" 

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN_LED_1 3
#define PIN_LED_2 4

#define NUM_PIXELS_1 21
#define NUM_PIXELS_2 36

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels_1 = Adafruit_NeoPixel(NUM_PIXELS_1, PIN_LED_1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels_2 = Adafruit_NeoPixel(NUM_PIXELS_2, PIN_LED_2, NEO_GRB + NEO_KHZ800);

// CUSTOM COLORS
const uint32_t colores[] = {
  pixels_1.Color(0, 0, 0),  //black
  pixels_1.Color(255, 0, 255),   //pink
  pixels_1.Color(255, 255, 255)  //red
};
uint32_t black = pixels_1.Color(0, 0, 0);

// WiFi stuffs
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;        // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                 // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

WiFiServer server(80);

// current state
int state = 0;                    // initial state is purple

void setup() {
// This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
#if defined(__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  // End of trinket special code

  pinMode(LED_BUILTIN, OUTPUT);  // PIN 13 LED
  pinMode(PIN_LED_1, OUTPUT);  
  pinMode(PIN_LED_2, OUTPUT);  

  pixels_1.begin();                 // This initializes the NeoPixel library
  pixels_2.begin();

  int state = 0;                    // initial state is purple
  pattern1(1);                      // turn lights on and make them purple

  Serial.begin(9600);               // DEBUG

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }  

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();                           // start the web server on port 80
  printWifiStatus();                        // you're connected now, so print out the status     
}

void loop() {
  // For a set of NeoPixels the first NeoPixel is 0, second is 1, all the way up to the count of pixels minus one.
  //Serial.println("Head of loop");  // DEBUG

    WiFiClient client = server.available();   // listen for incoming clients

    if (client) {                             // if you get a client,
        Serial.println("new client");           // print a message out the serial port
        String currentLine = "";                // make a String to hold incoming data from the client
        while (client.connected()) {            // loop while the client's connected
            if (client.available()) {             // if there's bytes to read from the client,
                char c = client.read();             // read a byte, then
                Serial.write(c);                    // print it out the serial monitor
                if (c == '\n') {                    // if the byte is a newline character

                    // if the current line is blank, you got two newline characters in a row.
                    // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0) {
                        // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                        // and a content-type so the client knows what's coming, then a blank line:
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println();

                        // the content of the HTTP response follows the header:
                        client.print("Click <a href=\"/white\">here</a> turn the LEDs white<br>");
                        client.print("Click <a href=\"/purple\">here</a> turn the LEDs purple<br>");
                        client.print("Click <a href=\"/off\">here</a> turn the LEDs off<br>");

                        // The HTTP response ends with another blank line:
                        client.println();
                        // break out of the while loop:
                        break;
                    } else {    // if you got a newline, then clear currentLine:
                        currentLine = "";
                    }
                } else if (c != '\r') {  // if you got anything else but a carriage return character,
                    currentLine += c;      // add it to the end of the currentLine
                }

                // Check to see if the client request was "GET /H" or "GET /L":
                if (currentLine.endsWith("GET /purple")) {
                    state = 1;
                }
                if (currentLine.endsWith("GET /white")) {
                    state = 2;
                }                                    
                if (currentLine.endsWith("GET /off")) {
                    state = 0;
                }
            }
        }

        // close the connection:
        client.stop();
        Serial.println("client disonnected");
    }

  if (state == 0) {  
    pattern1(0); // off
  }
  if (state == 1) {  
    pattern1(1); // purple
  }
  if (state == 2) {  
    pattern1(2); // white
  }  
}

// Grow light color fill
void pattern1(int i) {
  pixels_1.clear();
  pixels_2.clear();
  pixels_1.setBrightness(255);
  pixels_2.setBrightness(255);
  uint32_t color = colores[i];
  for (int j = 1; j <= NUM_PIXELS_1; j++) {
    //pixels_1.clear();
    pixels_1.fill(color, 0, j);
    pixels_1.show();
    delay(50);
  }
  for (int j = 1; j <= NUM_PIXELS_2; j++) {
    //pixels_2.clear();
    pixels_2.fill(color, 0, j);
    pixels_2.show();
    delay(50);
  }
  state = 4;
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}