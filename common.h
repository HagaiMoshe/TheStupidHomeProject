/* common implementations for the verious stupid home devices
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include "privateData.h"

//    consts
static const int MILLISEC_TO_SEC = 1000;
static const unsigned long READ_TIMEOUT = 3 * MILLISEC_TO_SEC;
static const int SECONDS_PER_LOOP = 10;
//wifi
static const char* SSID_NAME = PRIVATE_DATA__SSID;
static const char* BSS_PASS = PRIVATE_DATA__BSS_PASS;
static const int CONNECT_TO_WIFI_TIMEOUT = 20;
static const int SERVER_PORT = 80;
//http
static const char* SERVER_USER = PRIVATE_DATA__USER;
static const char* SERVER_PASS = PRIVATE_DATA__PASS;
static const String DATA_BASE_SERVER = "http://" + PRIVATE_DATA__SERVER_NAME + ".000webhostapp.com/protected/";
// https
static const char* AUTH_SERVER_FINGER_PRINT = PRIVATE_DATA__AUTH_SERVER_FINGER_PRINT;
static const char* DB_SERVER_FINGER_PRINT = PRIVATE_DATA__DB_SERVER_FINGER_PRINT;
static const int SERVER_PORT_S = 443;
static const String FIREBASE_USER = PRIVATE_DATA__USER + "@clients.com";
static const char* FIREBASE_USER_PASS = PRIVATE_DATA__FIREBASE_USER_PASS;
static const char* FIREBASE_KEY = PRIVATE_DATA__FIREBASE_KEY;
static const char* FIREBASE_DATABASE_SERVER_NAME = PRIVATE_DATA__SERVER_NAME;


static inline void debugPrint(String msg)
{
#ifdef DEBUG_PRINT
	Serial.println(msg);
#endif
}

// ------------------------------ http -----------------------------
// this is to fix the issue of the original post request not adding Content-Type to the header
class HTTPClient_ : public HTTPClient
{
public:
	int sendRequest(const char * type, uint8_t * payload, size_t size)
	{
		if(!strcmp(type, "POST")){
			addHeader(F("Content-Type"), F("application/x-www-form-urlencoded"));
		}
		return HTTPClient::sendRequest(type, payload, size);
	}

	int POST(String payload)
	{
		return sendRequest("POST", (uint8_t *) payload.c_str(), payload.length());
	}
};

static inline int postToDataBase(const String dataBaseName, const String status)
{
  const String dataBaseHttp = DATA_BASE_SERVER + dataBaseName;
  //send the requests
  HTTPClient_ http;
  http.begin(dataBaseHttp);
  http.setAuthorization(SERVER_USER, SERVER_PASS);

  int httpCode = http.POST(status);

  http.end();
  return httpCode;
}


static inline String readDataBase(const String dataBaseName)
{
  const String dataBaseHttp = DATA_BASE_SERVER + dataBaseName;
  //send the requests
  HTTPClient http;
  http.begin(dataBaseHttp);
  http.setAuthorization(SERVER_USER, SERVER_PASS);
  int httpCode = http.GET();
  //http.sendRequest("GET", READ_DATA_BASE);
  String readString = http.getString();
  http.end();
  return readString;
}

// ------------------------------- https ------------------------------
static String httpsAuthenticate()
{
	HTTPClient client;
	debugPrint("sending credentials");

	String authenticationUrl = String("https://www.googleapis.com/identitytoolkit/v3/relyingparty/verifyPassword?key=") + FIREBASE_KEY;
	String credentials = String("{\"email\":\"") + FIREBASE_USER + "\",\"password\":\"" + FIREBASE_USER_PASS + "\",\"returnSecureToken\":true}";
	client.begin(authenticationUrl, AUTH_SERVER_FINGER_PRINT);
	client.addHeader("User-Agent", "BuildFailureDetectorESP8266");
	client.addHeader("Accept", "*/*");
	client.addHeader("Content-Type", "application/json");
	client.addHeader("Connection", "close");
	int httpCode = client.POST(credentials);

	if(httpCode == -1){
		Serial.println(String("send credentials failed ") + String(httpCode));
		return "";
	}

	String payload = client.getString();
	client.end();
	//Serial.println(payload);

	int startToken = payload.indexOf(String("\"idToken\": \"")) + sizeof("\"idToken\": \"") -1;
	int endOfToken = payload.indexOf(String("\","), startToken);
	return payload.substring(startToken, endOfToken);
}

static inline String httpsReadDataBase(const char *databaseVariablePath, String& idToken)
{
	// send get request
	HTTPClient client;
	String payload = "";

	String dataBaseUrl = String("https://") + DATABASE_SERVER_NAME + ".firebaseio.com/" + databaseVariablePath + "?auth=" + idToken;
	client.begin(dataBaseUrl, String(DB_SERVER_FINGER_PRINT));

	int getStatus = client.GET();
	if(getStatus == 200)
		payload = client.getString();
	else
		payload = String(getStatus);
	client.end();

	debugPrint(String("read payload ") + payload);
	return payload;
}

static int httpsUpdateDataBase(const char *databaseVariablePath, const char *value, String& idToken)
{
	HTTPClient client;
	String dataBaseUrl = String("https://") + DATABASE_SERVER_NAME + ".firebaseio.com/" + databaseVariablePath + "?auth=" + idToken;
	String postRequestData = String("{") + value + "}";
	debugPrint(String("update database ") + databaseVariablePath + ". data: " + postRequestData);

	client.begin(dataBaseUrl, String(DB_SERVER_FINGER_PRINT));
	client.addHeader("User-Agent", "BuildFailureDetectorESP8266");
	client.addHeader("Accept", "*/*");
	client.addHeader("Content-Type", "application/json");
	client.addHeader("Connection", "close");
	int httpCode = client.PUT(postRequestData);
	client.end();
	return httpCode;
}

static String getValueFromDb(const char *dbName, String& idToken)
{
  String readString = httpsReadDataBase(dbName, idToken);
  if(readString == "401"){
    Serial.println("requesting id token");
    idToken = httpsAuthenticate();
    readString = httpsReadDataBase(dbName, idToken);
  }
  return readString;
}

static int updateValueToDb(const char *dbName, const char *value, String& idToken)
{
  int postStatus = httpsUpdateDataBase(dbName, value, idToken);
  if(postStatus == 401){
    Serial.println("requesting id token");
    idToken = httpsAuthenticate();
    postStatus = httpsUpdateDataBase(dbName, value, idToken);
  }
  return postStatus;
}
// ------------------------------- wifi ------------------------------
static inline void connectToWifi()
{
  int counter = 0;
  Serial.println("\nconnecting to wifi");
  WiFi.begin(SSID_NAME, BSS_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");

    if(counter++ > CONNECT_TO_WIFI_TIMEOUT){
      WiFi.disconnect();
      return;
    }
  }
  Serial.print("\nWiFi connected: ");
  Serial.println(WiFi.localIP());
}

static inline void setupWifi()
{
  delay(500);
  WiFi.mode(WIFI_OFF);
  delay(500);
  WiFi.mode(WIFI_STA);
}

// ------------------------------- SNTP ------------------------------

static void requestSNTP(const time_t& timeOut)
{
  time_t startTime = time(nullptr);
  time_t nowTime = startTime;
  debugPrint("Setting time using SNTP");

  configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  while (nowTime < startTime + timeOut) {
    delay(500);
    Serial.print(".");
    nowTime = time(nullptr);
  }

  struct tm timeinfo;
  gmtime_r(&nowTime, &timeinfo);
  debugPrint(String("\nCurrent time: ") + asctime(&timeinfo));
}
