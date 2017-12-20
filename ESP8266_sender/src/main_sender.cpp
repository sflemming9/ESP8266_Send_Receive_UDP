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
IPAddress ip(192, 168, 42, 2);  //raspberry pi IP address is 192.168.42.1
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

bool signalsent = false;
bool signalrecieved = false;

unsigned long rssi = 0;
unsigned long total_rssi = 0;
unsigned long average_rssi = 0;

unsigned int t_sentSignal = 0;
unsigned int t_returnSignal = 0;
unsigned long timelapsed_signal = 0;
unsigned long total_timelapsed_signal = 0;
unsigned long average_timelapsed_signal = 0;
unsigned int exp_dist = 0;
unsigned int exp_dist1 = 0;
unsigned int loop_count = 0;
unsigned int message_count = 0;


char* receive(const char replyPacket[]);
void send(const char outgoingMessage[]);

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
  char* incoming = receive("Signal returned");

  // if a signal has been recieved update signal sent to allow another to be sent
  if (signalrecieved == true){
    //delay(1000);
    signalsent = false;
    signalrecieved == false;
  }

  // sends off a signal and waits until response is received before trying again
  if (signalsent == false) {
    message_count++;
    //char outgoingPacket[100];
    //sprintf(outgoingPacket,"Hello World; loop counter = %d; message = %d\0",
    //  loop_count, message_count);
    //outgoingPacket = "Hello World".c_str();
    send("Hello World");
    //t_sentSignal = micros();
    t_sentSignal = ESP.getCycleCount();
    signalsent == true;
  }

  if (message_count < 10000)
    total_timelapsed_signal += timelapsed_signal;
    total_rssi += rssi;

  if (message_count >= 10000) {
    average_timelapsed_signal = total_timelapsed_signal/message_count;
    average_rssi = total_rssi/message_count;

    exp_dist = (average_rssi - 62.476)/0.3115;
    exp_dist1 = (average_rssi - 65.487)/0.2358;

    printf("Average Time of signal %d microseconds with an RSSI strength of %d at an expected distance of %d, %d\n", average_timelapsed_signal, average_rssi, exp_dist, exp_dist1);
    message_count = 0;
    total_timelapsed_signal = 0;
    average_timelapsed_signal = 0;
    total_rssi = 0;
    average_rssi = 0;
    timelapsed_signal = 0;
    exp_dist = 0;
    exp_dist1 = 0;


    delay(1000);
  }


  // incase nothing happens
  //if ((micros() - t_sentSignal) > 10000000)
  //  signalsent = FALSE;
  loop_count++;
}


void send(const char outgoingMessage[]) {

  uint8_t ipBuffer[4] = {broadcastIP[0], broadcastIP[1], broadcastIP[2], broadcastIP[3]};
  Udp.beginPacket(ipBuffer, localUdpPort);
  //Starts a connection to write UDP data to the remote connection
  //Udp.remoteIP gets the IP address of the remote connection.
  //Udp.MremotePort get the port of the remote connection.

  //create the outgoingPacket which will include the counter number
  char outgoingPacket[100];
  sprintf(outgoingPacket, "%s\0", outgoingMessage);

  //Serial.println(outgoingPacket);

  //Send out the Udp packet
  Udp.write(outgoingPacket);
  Udp.endPacket();

}

char* receive(const char replyPacket[]) {

  char incomingPacket[255];   //character array of size 255
  //this needs to be reinitialized every time the receive function is called

  //Udp.begin(localUdpPort);  //here or in setup...

  /*Check for the presence of a UDP packet, and report the
  size. parsePacket() must be called before reading the buffer with
  UDP.read() */
  int packetSize = Udp.parsePacket(); //
  //long udprssi = Udp.RSSI();
  //if the packet has a non-zero size (i.e. evaluates to TRUE)
  if (packetSize){

    signalrecieved = true;
    //t_returnSignal = micros();
    t_returnSignal = ESP.getCycleCount();
    rssi = abs(WiFi.RSSI());
    timelapsed_signal = 6.25*(t_returnSignal - t_sentSignal);
    //unsigned int tof = 6.25*(t_returnSignal - t_sentSignal);
    //Serial.printf("Sender: Received return signal (%d) after %d microseconds \n", rssi,   timelapsed_signal);
    int len = Udp.read(incomingPacket, 255);
    //read up to the 255th place (the maximum length)

    /*Packet Length specify the size of the whole packet include header,
    trailer and the data that send on that packet. But Packet Size specify
    size of the header on the packet. */

    //add a zero to the end of the incomingPacket to allow for proper printing
    if (len > 0){
      incomingPacket[len] = 0;
    }

    //Serial.printf("UDP packet contents: %s\n", incomingPacket);
    /*Once a packet is received, the code will printing out the IP address
    and port of the sender as well as the length of received packet. If the
    packet is not empty, its contents will be printed out as well.*/

    /*To prevent an infinte loop or "Message Received" going back and forth,
    we say that if the incoming message is "Message Received", do not reply*/
    String packetString((char *)incomingPacket);
    if (!(packetString == "Message Received..")) {

      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      //Starts a connection to write UDP data to the remote connection
      //Udp.remoteIP gets the IP address of the remote connection.
      //Udp.remotePort get the port of the remote connection.

      Udp.write(replyPacket);
      Udp.endPacket();
    }
  }

  //return (const char) *Udp.remoteIP().toString().c_str();
  return incomingPacket;
}
