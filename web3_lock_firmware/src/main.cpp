#include "esp_wifi.h"
#include <Arduino.h>
#include <Web3.h>
#include <WiFi.h>
// #include <stdint.h>
#include <functional>
#include <string>
// #include <vector>
#include <map>
#include <ScriptClient.h>

const char *ssid = "Himonday_Wi-Fi5";//"ChinaNet-2.4G-B758";
const char *password = "18688881010";//"kaug7562";

#define DOOR_CONTRACT "0x77e4bA74DF5C4a15d4620D80DbF82A072EBf7657"
//"0x37EE5C1fCf940be4e79C735F21D4E2f650f63a85" //Your NFT token address
const char *seedWords[] = {"Apples", "Oranges", "Grapes", "DragonFruit", "BreadFruit", "Pomegranate", "Aubergine", "Fungi", "Falafel", "Cryptokitty", "Kookaburra", "Elvis", "Koala", 0};
Web3 *web3;
#define CONTROL_PIN 8  
string currentChallenge;
WiFiServer server(80);

const char *apiRoute = "api/";

//define API routes
enum APIRoutes
{
  api_unknown,
  api_getChallenge,
  api_checkSignature,
  api_End
};

std::map<std::string, APIRoutes> s_apiRoutes;


//decleare functions
void Initialize();
void setupWifi();
void handleAPI(APIReturn *apiReturn, ScriptClient *client);
void OpenDoor();
bool QueryBalance(const char *contractAddr, std::string *userAddress);
void updateChallenge();

void setup() {
  Serial.begin(115200);
  web3 = new Web3(FUJI_TEST_ID);
  pinMode(CONTROL_PIN, OUTPUT);

  setupWifi();

  Initialize(); //init after wifi setup to change startup delay
}

void loop() {
  WiFiClient c = server.available(); // Listen for incoming clients
  ScriptClient *client = (ScriptClient*) &c;

  if (*client)
  {
      Serial.println("New Client.");
      client->checkClientAPI(apiRoute, &handleAPI); //method handles connection close etc.
  }
}

void setupWifi()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    return;
  }

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.persistent(false);
    WiFi.mode(WIFI_OFF);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
  }

  int wificounter = 0;
  while (WiFi.status() != WL_CONNECTED && wificounter < 10)
  {
    delay(500);
    Serial.print(".");
    wificounter++;
  }

  if (wificounter >= 10)
  {
    Serial.println("Restarting ...");
    ESP.restart(); //targetting 8266 & Esp32 - you may need to replace this
  }

  esp_wifi_set_ps(WIFI_PS_NONE);
  delay(10);

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

//Format for API call from wallet/utilitiy using the bridge is <bridge server address>/<IoT device ethereum address>/<API route>?<arg1>=<your data>&<arg2>=<your data> etc.
//When you call the API this is the extension you use EG www.bridgeserver.com/0x123456789ABCDEF0000/getChallenge
void Initialize()
{
  s_apiRoutes["getChallenge"] = api_getChallenge; 
  s_apiRoutes["checkSignature"] = api_checkSignature;
  s_apiRoutes["end"] = api_End;

  updateChallenge(); 
  random32v(micros()); //initial init of random seed AFTER wifi connection - since startup after wifi will be a different value of micros each start cycle
}

//Handle API return:
void handleAPI(APIReturn *apiReturn, ScriptClient *client)
{
    Serial.println(apiReturn->apiName.c_str());
    switch(s_apiRoutes[apiReturn->apiName.c_str()])
    {
        case api_getChallenge:
            Serial.print("api_getChallenge:");
            Serial.println(currentChallenge.c_str());
            client->print(currentChallenge.c_str());
            break;
        case api_checkSignature:
            {
				//EC-Recover address from signature and challenge
                //Serial.print("api_checkSignature->params[sig]:");
                //Serial.println(&apiReturn->params["sig"].c_str());
                Serial.print("currentChallenge:");
                Serial.println(currentChallenge.c_str());
                string address = Crypto::ECRecoverFromPersonalMessage(&apiReturn->params["sig"], &currentChallenge);  
				//Check if this address has our entry token
                Serial.print("address:");
                Serial.println(address.c_str());
                boolean hasToken = QueryBalance(DOOR_CONTRACT, &address);
                
                updateChallenge(); //generate a new challenge after each check
                if (hasToken)
                {
                    Serial.println("hasToken");
                    client->print("pass");
                    OpenDoor(); //Call your code that opens a door or performs the required 'pass' action
                }
                else
                {
                    Serial.println("no Token");
                    client->print("fail: doesn't have token");
                    //OpenDoor(); 
                }
            }
            break;
    }

}

void updateChallenge()
{
  // generate a new challenge
  int size = 0;
  while (seedWords[size] != 0)
    size++;
  Serial.println(size);
  char buffer[32];

  int seedIndex = random(0, size);
  currentChallenge = seedWords[seedIndex];
  currentChallenge += "-";
  long challengeVal = random32();
  currentChallenge += itoa(challengeVal, buffer, 16);

  Serial.print("Challenge: ");
  Serial.println(currentChallenge.c_str());
}

void OpenDoor()
{
  digitalWrite(CONTROL_PIN,HIGH);
  delay(9000);
  digitalWrite(CONTROL_PIN,LOW);
}

bool QueryBalance(const char *contractAddr, std::string *userAddress)
{
  // transaction
  bool hasToken = false;
  Contract contract(web3, contractAddr);
  string func = "balanceOf(address)";
  string param = contract.SetupContractData(func.c_str(), userAddress);
  string result = contract.ViewCall(&param);

  Serial.println(result.c_str());

  // break down the result
  uint256_t baseBalance = web3->getUint256(&result);

  if (baseBalance > 0)
  {
    hasToken = true;
    Serial.println("Has token");
  }

  return hasToken;
}