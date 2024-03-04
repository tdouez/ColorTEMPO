//--------------------------------------------------------------------
//                 +/
//                 `hh-
//        ::        /mm:
//         hy`      -mmd
//         omo      +mmm.  -+`
//         hmy     .dmmm`   od-
//        smmo    .hmmmy    /mh
//      `smmd`   .dmmmd.    ymm
//     `ymmd-   -dmmmm/    omms
//     ymmd.   :mmmmm/    ommd.
//    +mmd.   -mmmmm/    ymmd-
//    hmm:   `dmmmm/    smmd-
//    dmh    +mmmm+    :mmd-
//    omh    hmmms     smm+
//     sm.   dmmm.     smm`
//      /+   ymmd      :mm
//           -mmm       +m:
//            +mm:       -o
//             :dy
//              `+:
//--------------------------------------------------------------------
//   __|              _/           _ )  |
//   _| |  |   ` \    -_)   -_)    _ \  |   -_)  |  |   -_)
//  _| \_,_| _|_|_| \___| \___|   ___/ _| \___| \_,_| \___|
//--------------------------------------------------------------------
// Code mise à disposition selon les termes de la Licence Creative Commons Attribution
// Pas d’Utilisation Commerciale.
// Partage dans les Mêmes Conditions 4.0 International.
//--------------------------------------------------------------------
// 2023/12/20 - FB V1.0.0
//--------------------------------------------------------------------
#include <Arduino.h>
#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <WiFiManager.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebSrv.h>
#include <NTPClient.h>
#include <time.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include <ArduinoOTA.h>

#define VERSION   "v1.0.0"
 
#define LED_TOUR1	0
#define LED_TOUR2	1
#define LED_TOUR3	2
#define LED_TOUR4	3
#define LED_CENTRE	0
 
#define PIN_LED_TOUR 	  A1
#define PIN_LED_CENTRE 	A2
#define PIN_BP      	  A3
#define NUMPIXELS_TOUR   4
#define NUMPIXELS_CENTRE 1
 
#define DEF_URL_EDF_PART "https://particulier.edf.fr/services/rest/referentiel/searchTempoStore?dateRelevant="
#define DEF_MODULE_NAME  "ColorTEMPO"

#define MAX_BUFFER      64
#define MAX_BUFFER_URL  200

#define HC          0
#define HP          1
#define HORAIRE_MATIN 6
#define HORAIRE_SOIR  22
 
const unsigned long TEMPS_MAJ  =  5000; // 5s

char buffer[MAX_BUFFER];
char module_name[MAX_BUFFER];
char couleur_jour[MAX_BUFFER];
char couleur_demain[MAX_BUFFER];
bool flag_call = true;
bool flag_first = true;
bool state_led = false;
uint8_t cpt_led = LED_TOUR1;
uint8_t minute_courante;
uint32_t lastTime_maj;


WiFiManager wm;
Adafruit_NeoPixel pixels_tour(NUMPIXELS_TOUR, PIN_LED_TOUR, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels_centre(NUMPIXELS_CENTRE, PIN_LED_CENTRE, NEO_GRB + NEO_KHZ800);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);
Ticker timer_blink;
Ticker timer_round;

const uint32_t black = pixels.Color(0, 0, 0);
const uint32_t red = pixels_tour.Color(255, 0, 0);
const uint32_t green = pixels_tour.Color(0, 255, 0);
const uint32_t blue = pixels_tour.Color(0, 0, 255);
const uint32_t white = pixels_tour.Color(255, 255, 255);
const uint32_t purple = pixels_tour.Color(255, 0, 255);
const uint32_t yellow = pixels_tour.Color(255, 255, 0);
const uint32_t orange = pixels_tour.Color(255, 165, 0);
uint32_t color_led = red;


//-----------------------------------------------------------------------
void round()
{
  if (cpt_led > 0) pixels_tour.setPixelColor(cpt_led-1, color_led);
  pixels_tour.show();
  cpt_led++;
  if (cpt_led > LED_TOUR4+1) {
    cpt_led = LED_TOUR1;
    pixels_tour.clear();
  }
}

//-----------------------------------------------------------------------
void blink()
{
  state_led = !state_led;
  if (state_led) pixels_tour.fill(color_led, LED_TOUR1, LED_TOUR4+1);
	  else pixels_tour.clear();
  pixels_tour.show();
}
  
//-----------------------------------------------------------------------
uint8_t checkHoraire()
{
  uint h = HC;

  uint heure_courante = timeClient.getHours();

  if (heure_courante >= 0 && heure_courante < HORAIRE_MATIN) h = HP;
  if (heure_courante >= HORAIRE_MATIN && heure_courante < HORAIRE_SOIR) h = HC;
  if (heure_courante >= HORAIRE_SOIR) h = HP;

  return h;
}

//-----------------------------------------------------------------------
String return_current_date()
{
  time_t rawtime = timeClient.getEpochTime();
  struct tm * ti;
  ti = localtime (&rawtime);
  uint16_t year = ti->tm_year + 1900;
  String yearStr = String(year);
  uint8_t month = ti->tm_mon + 1;
  String monthStr = month < 10 ? "0" + String(month) : String(month);

  return yearStr + "-" + monthStr + "-" + String(ti->tm_mday);
}

//-----------------------------------------------------------------------
String return_current_time()
{
  return String(timeClient.getHours()) + String(":") + String(timeClient.getMinutes()) + String(":") + String(timeClient.getSeconds());
}

//-----------------------------------------------------------------------
void configModeCallback (WiFiManager *myWiFiManager) {
  color_led = green;
  timer_round.attach_ms(400, round);
}

//-----------------------------------------------------------------------
void checkButton(){
  // check for button press
  if ( digitalRead(PIN_BP) == LOW ) {
    // poor mans debounce/press-hold, code not ideal for production
    delay(50);
    color_led = purple;
    timer_round.attach_ms(250, round);
    pixels_centre.clear();
    pixels_centre.show();

    if( digitalRead(PIN_BP) == LOW ){
      Serial.println("Button Pressed");
      // still holding button for 3000 ms, reset settings, code not ideaa for production
      delay(3000); // reset delay hold
      if( digitalRead(PIN_BP) == LOW ){
        Serial.println("Button Held");
        Serial.println("Erasing Config, restarting");
        color_led = yellow;
        timer_round.attach_ms(50, round);
        lastTime_maj = millis();
        while (millis() - lastTime_maj < 1000) {}
        wm.resetSettings();
        ESP.restart();
      }
   
      // start portal w delay
      Serial.println("Starting config portal");
      wm.setConfigPortalTimeout(120);
           
      if (!wm.startConfigPortal(module_name)) {
        Serial.println("failed to connect or hit timeout");
        // ESP.restart();
      } else {
        //if you get here you have connected to the WiFi
        Serial.println("connected...:)");
        color_led = green;
        timer_blink.attach_ms(200, blink);
        lastTime_maj = millis();
        while (millis() - lastTime_maj < 2000) {}
        flag_first = true;
      }
    }
  }
}

//-----------------------------------------------------------------------
void interrogation_tempo()
{
  String Url;
  HTTPClient http;
  int httpCode = 0;

  Serial.println(F("interrogation_tempo"));

  Url = String(DEF_URL_EDF_PART) + return_current_date();

  Serial.println(Url);
  http.begin(Url);

  pixels_centre.setPixelColor(LED_CENTRE, purple);
  pixels_centre.show();

  do 
  {
    httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
      pixels_centre.setPixelColor(LED_CENTRE, yellow);
      pixels_centre.show();
      delay(3000);
    }
  }
  while (httpCode != HTTP_CODE_OK);

  pixels_centre.setPixelColor(LED_CENTRE, purple);
  pixels_centre.show();

  String Reponse_tempo = http.getString();
  Serial.println(Reponse_tempo);

  DynamicJsonDocument jsonDoc(512);
  DeserializationError error = deserializeJson(jsonDoc, Reponse_tempo);
  if (error) Serial.println(F("Impossible de parser Reponse_tempo"));
  JsonObject json = jsonDoc.as<JsonObject>();
  //Serial.println(F("Contenu:"));
  //serializeJson(json, Serial);

  strcpy(couleur_jour, json["couleurJourJ"]);
  strcpy(couleur_demain, json["couleurJourJ1"]);

  Serial.print(F("Jour: "));
  Serial.println(couleur_jour);
  Serial.print(F("Demain: "));
  Serial.println(couleur_demain);

  http.end();

}

//-----------------------------------------------------------------------
void affiche_couleur()
{
	timer_round.detach();
	
	pixels_tour.clear();
	if (strstr(couleur_jour, "BLEU")) pixels_tour.fill(blue, LED_TOUR1, LED_TOUR4+1);
	if (strstr(couleur_jour, "BLANC")) pixels_tour.fill(white, LED_TOUR1, LED_TOUR4+1);
	if (strstr(couleur_jour, "ROUGE")) pixels_tour.fill(red, LED_TOUR1, LED_TOUR4+1);
	pixels_tour.show();

	pixels_centre.clear();
	if (strstr(couleur_demain, "BLEU")) pixels_centre.setPixelColor(LED_CENTRE, blue);
	if (strstr(couleur_demain, "BLANC")) pixels_centre.setPixelColor(LED_CENTRE, white);
	if (strstr(couleur_demain, "ROUGE")) pixels_centre.setPixelColor(LED_CENTRE, red);
	pixels_centre.show();
}

//-----------------------------------------------------------------------
void handlePreOtaUpdateCallback(){
  Update.onProgress([](unsigned int progress, unsigned int total) {
    color_led = orange;
    timer_round.attach_ms(100, round);
    Serial.printf("CUSTOM Progress: %u%%\r", (progress / (total / 100)));
  });
}

//-----------------------------------------------------------------------
void setup() {
    
  uint32_t currentTime; 
  
  pinMode(PIN_BP, INPUT_PULLUP);
  pinMode(PIN_LED_TOUR, OUTPUT);
  pinMode(PIN_LED_CENTRE, OUTPUT);
 
  Serial.begin(115200);
 
  Serial.println(F("   __|              _/           _ )  |"));
  Serial.println(F("   _| |  |   ` \\    -_)   -_)    _ \\  |   -_)  |  |   -_)"));
  Serial.println(F("  _| \\_,_| _|_|_| \\___| \\___|   ___/ _| \\___| \\_,_| \\___|"));
  Serial.print(F("   ColorTEMPO                                     "));
  Serial.println(VERSION);
 
  pixels_tour.begin(); // INITIALIZE NeoPixel
  pixels_tour.clear(); // Set all pixel colors to 'off'
  pixels_tour.setBrightness(60);
  pixels_tour.show();
  pixels_centre.begin(); // INITIALIZE NeoPixel
  pixels_centre.clear(); // Set all pixel colors to 'off'
  pixels_centre.setBrightness(90);
  pixels_centre.show();
  
  color_led = purple;
  timer_round.attach_ms(250, round);
  
  sprintf(module_name, "%s_%06X", DEF_MODULE_NAME, ESP.getEfuseMac());
  
  //----------------------------------------------------WIFI
  wm.setDebugOutput(true);
  wm.debugPlatformInfo();
  wm.setConfigPortalTimeout(120);
  wm.setAPCallback(configModeCallback);
  wm.setPreOtaUpdateCallback(handlePreOtaUpdateCallback);

  std::vector<const char *> menu = {"wifi","sep","update","restart","exit"};
  wm.setMenu(menu);
 
  bool res = wm.autoConnect(module_name);
   
  timer_round.detach();
  
  if(!res) {
      Serial.println("Failed to connect");
      // ESP.restart();
	    color_led = red;
      timer_blink.attach_ms(200, blink);
      lastTime_maj = millis();;
      while (millis() - lastTime_maj < 5000) {}
	    timer_blink.detach();
      ESP.restart();
  }
  else {
      //if you get here you have connected to the WiFi   
      Serial.println("connected... :)");
      color_led = green;
      timer_blink.attach_ms(200, blink);
      lastTime_maj = millis();
      while (millis() - lastTime_maj < 2000) {}
      timer_blink.detach();
      timeClient.begin();
      flag_first = true;
      flag_call = true;
      
  }

  
  timeClient.update();
  pixels_tour.clear();
  pixels_tour.show();
}
 
//-----------------------------------------------------------------------
void loop() {
uint32_t currentTime = millis();

  timeClient.update();

  if (flag_first) {
    flag_first = false;
    interrogation_tempo();
  }

  minute_courante = timeClient.getMinutes();
  if (minute_courante == 0 || minute_courante == 15 || minute_courante == 30) {
    if (flag_call == true) interrogation_tempo();
    flag_call = false;
  }
  else flag_call = true;
  
  if (currentTime - lastTime_maj > TEMPS_MAJ) {
	  affiche_couleur();
	  lastTime_maj = currentTime;
  }

  checkButton();
 
}
