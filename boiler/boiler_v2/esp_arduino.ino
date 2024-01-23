/* boiler V2
 * using ESP8266
 * the esp set a webserver with two buttons
 */

#include <ESP8266WiFi.h>

//consts
const char* SSID_NAME = PRIVATE_DATA__SSID;
const char* SSID_PASS = PRIVATE_DATA__BSS_PASS
const int LISTEN_PORT = 80;
const int RELAY_PIN = 2;
const char* SERVER_PASS = PRIVATE_DATA__SERVER_PASS;
const char* SETIP_SERVER = PRIVATE_DATA__SETIP_SERVER;
const char* SETIP_PASS = PRIVATE_DATA__SETIP_PASS;
const int LOOP_DELAY = 500; //in ms
const int TIME_TO_SET_IP = 2 * 60; //one minute



// assamble the server page string - this is the http code to send to the client
const String HEADER_1     = "HTTP/1.1 200 OK\n";
const String HEADER_2     = "Content-Type: text/html\n";
const String HEADER_3     = "Connnection: close\n";
const String HEADER_4     = "<!DOCTYPE HTML>\n";
const String HEADER_5     = "\n<HTML>\n<HEAD>\n";
const String SERVER_STYLE = "<link rel='stylesheet' type='text/css' href='http://randomnerdtutorials.com/ethernetcss.css' />\n";
const String BACKGROUNG_1 = "<body background=";
const String BACKGROUNG_2 = "http://bigbackground.com/wp-content/uploads/2013/08/fire-background.jpg"; 
const String BACKGROUNG_3 = " dir=\"rtl\">\n";
const String BACKGROUNG_4 = "</HEAD>\n<BODY>\n";
const String TITLE_H1     = "<font face=\"Courier\" size=\"10\" color=\"red\"><H1>The Boiler of DOOM</H1></font>\n";
const String TITLE_H2     = "<font size=\"5\" color=\"white\"><H2>! A decision shell be made</H2></font>\n";
const String BUTTON_ON    = "<a {font-size: 150px} href=\"/?button1on\"  style=\"color:red\" > INFLAME   </a>\n";
const String BUTTON_OFF   = "<a {font-size: 150px} href=\"/?button1off\" style=\"color:blue\"> EXTINGUISH</a>\n";   
const String FOOTER       = "</BODY>\n</HTML>";

const String STATUS_ON    = "<p><font size=\"6\" color=\"red\" >The inferno's flame has been lit</font></p>";
const String STATUS_OFF   = "<p><font size=\"6\" color=\"blue\">The inferno's flame has died out</font></p>";


const String response_on = HEADER_1 + HEADER_2 + HEADER_3 + HEADER_4 + HEADER_5 +
                                   SERVER_STYLE + BACKGROUNG_1 + BACKGROUNG_2 + BACKGROUNG_3 + 
                                   BACKGROUNG_4 + TITLE_H1 + TITLE_H2 + BUTTON_ON + BUTTON_OFF +
                                   STATUS_ON + FOOTER;

const String response_off = HEADER_1 + HEADER_2 + HEADER_3 + HEADER_4 + HEADER_5 +
                                   SERVER_STYLE + BACKGROUNG_1 + BACKGROUNG_2 + BACKGROUNG_3 + 
                                   BACKGROUNG_4 + TITLE_H1 + TITLE_H2 + BUTTON_ON + BUTTON_OFF +
                                   STATUS_OFF + FOOTER;


// assemble the Authentification page string
const String M1_401 = "HTTP/1.1 401 Authorization Required\n";
const String M2_401 = "WWW-Authenticate: Basic realm=\"Secure Area\"\n";
const String M3_401 = "Content-Type: text/html\n";
const String M4_401 = "Connnection: close\n";
const String M5_401 = "\n<!DOCTYPE HTML>\n";
const String M6_401 = "<HTML>  <HEAD>   <TITLE>Error</TITLE>\n";
const String M7_401 = " </HEAD> <BODY><H1>401 Unauthorized.</H1></BODY> </HTML>\n";

const String MSG_401 = M1_401 + M2_401 + M3_401 + M4_401 + M5_401 + M6_401 + M7_401;


// assemble the setIpRequest
const String M1_SETIP = "GET /setIp/setIp.html HTTP/1.1\n";
const String M2_SETIP = "host: ";
const String M3_SETIP = SETIP_SERVER;
const String M4_SETIP = "\nAuthorization: Basic ";
const String M5_SETIP = SETIP_PASS;
const String M6_SETIP = "\n";

const String MSG_SETIP = M1_SETIP + M2_SETIP + M3_SETIP + M4_SETIP + M5_SETIP + M6_SETIP;

// -----------------------------------------------------------------

void HandleRequest(WiFiClient& client, String &request){
  static int stat = LOW; // this is the status of the relay

  // verrify autentification
  if(request.indexOf(SERVER_PASS) <0){
    client.println(MSG_401); // no autentification found - send Authentification page
    return;
  }
  
  // find the type of the request
  if(request.indexOf("GET /?button1on HTTP" ) >=0) // button set to on
    stat = HIGH;
  else if(request.indexOf("GET /?button1off HTTP") >=0) // button set to off
    stat = LOW;
  else if (request.indexOf("GET / HTTP") >=0) // empty GET - no button set
    {}
  else { // any other request will not be handled - stoping client
    client.stop();
    return;
  }
 
  digitalWrite(RELAY_PIN, stat); // toggeling the relay's pin

  if(stat)// sending response to client
    client.println(response_on);
  else
    client.println(response_off);

  client.stop(); // ending communication
}//HandleRequest


void sendSetIpRequest(WiFiClient& client)
{
  int ret = client.connect(SETIP_SERVER, 80);
  if(ret < 0){
    Serial.print("failed to connect to remote server. error: ");
    Serial.println(ret);
    return;
  }

  client.println(MSG_SETIP);
  client.stop();
}


WiFiServer server(LISTEN_PORT);
void setup(void)
{  
  // Start Serial
  Serial.begin(115200);
  
  // Connect to WiFi
  WiFi.begin(SSID_NAME, SSID_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
 
  // Start the server
  server.begin();
  Serial.println("Server started");
  
  // Print the IP address
  Serial.println(WiFi.localIP());

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
}// setup




void loop() {
  WiFiClient client = server.available();
  static String readString = "";
  static String request = "";
  static int loopsCounter = TIME_TO_SET_IP;

  if(TIME_TO_SET_IP <= loopsCounter){
    sendSetIpRequest(client);
    loopsCounter = 0;
  }
  delay(LOOP_DELAY);

  if (client){
    while (client.connected()) {
      if (client.available()) {
        
        //read line HTTP request
        char c = client.read();
        readString += c;
        if(c == '\n'){
          request += readString;
          
          if(readString == "\r\n"){
            Serial.println(request);
            HandleRequest(client, request);
            request = "";
          }
            
          readString="";
        }// if(c == '\n')
      }// if(client.available())
    }// while
  }// if(client)
}// loop



