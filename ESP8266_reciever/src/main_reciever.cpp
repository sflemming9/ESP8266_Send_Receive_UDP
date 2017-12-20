#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <stdio.h>

/*Connect a ESP8266 to a network and assign itself a static IP IPAddress
then listen send a UDP packet with a delay interval in between
Sabrina Flemming
November 2017 */

/*To upload onto specific port, type the following into terminal:
platformio run --target upload --upload-port COM8
- Example for port 8*/
//Don't forget to choose upload port under platformio.ini or in the terminal


/*This code utilizes the code from ESP8266_Connect_to_network and adds Code
based on the sketch:
http://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/udp-examples.html
*/

//assign static IP address
IPAddress ip(192, 168, 42, 3);  //raspberry pi IP address is 192.168.42.1
IPAddress gateway(192, 168, 42, 21);
IPAddress subnet(255, 255, 255, 0);

IPAddress broadcastIP(192, 168, 42, 255);

// WiFi parameters to be configured
const char* ssid = "PI_ME310";
const char* password = "ME310_criticalprototype";

//WiFiUDP object called Udp (used to handle programming UDP over wifi)
WiFiUDP Udp;
unsigned int localUdpPort = 4210;   //port 4210 is unregistered
//https://en.wikipedia.org/wiki/List_of_TCP_and_UDP_port_numbers
//sending and receiving port must be the same

unsigned int loop_count = 0;
unsigned int message_count = 0;

char* receive(const char replyPacket[]);

void setup() {

  Serial.begin(115200);
  delay(10);

  // Set static IP address
  WiFi.config(ip, gateway, subnet);

  // Connect to WiFi
  WiFi.begin(ssid, password);

  // while wifi not connected yet, print '.'
  // then after it connected, get out of the loop
  while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.println(WiFi.status());
     /*Serial.println("\n");
     Serial.println(WL_CONNECTED); //3
     Serial.println(WL_CONNECT_FAILED); //4
     Serial.println(WL_CONNECTION_LOST); //5
     Serial.println(WL_NO_SSID_AVAIL); //1
     Serial.println(WL_SCAN_COMPLETED); //2
     Serial.println(WL_DISCONNECTED); //6
     Serial.println(WL_NO_SHIELD); //255
     Serial.println(WL_IDLE_STATUS); //0 */


  }
  //print a new line, then print WiFi connected and the IP address
  Serial.println("");
  Serial.println("WiFi connected");
  // Print the IP address
  Serial.println(WiFi.localIP());

  //start listening to incoming UDP packets
  Udp.begin(localUdpPort);

}


void loop() {
  loop_count++;
  char* incoming = receive("Message Received.");
}

char* receive(const char replyPacket[]) {

  char incomingPacket[255];   //character array of size 255
  //this needs to be reinitialized every time the receive function is called

  //Udp.begin(localUdpPort);  //here or in setup...

  /*Check for the presence of a UDP packet, and report the
  size. parsePacket() must be called before reading the buffer with
  UDP.read() */
  int packetSize = Udp.parsePacket(); //

  //if the packet has a non-zero size (i.e. evaluates to TRUE)
  if (packetSize){
    Serial.printf("Reciever: Received %d message in loop %d\n", message_count,
                  loop_count);
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    // We want to send out a reply immediately to minimize time delays

    Udp.write(replyPacket);
    Udp.endPacket();
    message_count++;

  }

  //return (const char) *Udp.remoteIP().toString().c_str();
  return incomingPacket;
}
