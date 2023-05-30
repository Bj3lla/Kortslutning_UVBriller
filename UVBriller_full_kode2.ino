//-----importerte biblioteker-----//
//Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSerifBold9pt7b.h>
/*
 * Tutorial page: https://arduinogetstarted.com/tutorials/arduino-neopixel-led-strip
 */

//Neopixel Led Lys
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

//konfigurerer OLED-Displayet
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET -1

//definerer porter og antal pixler for neopixlene
#define PIN_NEO_PIXEL  2   // Arduino pin that connects to NeoPixel
#define NUM_PIXELS     3  // The number of LEDs (pixels) on NeoPixel

//
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_NeoPixel NeoPixel(NUM_PIXELS, PIN_NEO_PIXEL, NEO_GRB + NEO_KHZ800);

//-----instansvariabler og porter-----//
int knapp = 8;
int ledlys = 2;
unsigned long UVTidSum; 
bool gronn = false;
bool gul = false;
bool oransje = false;

// diverse konstanter // 
//initieres til konstanter for hud av typen Medium
float OVER3 = 1; 
float OVER6 = 1.3;
float OVER8 = 1.86;
float OVER11 = 3.25;
//initieres til kortere tid for tesning
double MINEKS = 10000;
double MIDEKS = 30000;
double MAKSEKS = 50000;

unsigned long tid = 0; 
unsigned long sumTidLavUV = 0; //lagrer tiden bruker totalt har vært eksponert for lav UV

//UV sensor som leser inn volt og gjør om til UV 
float sensorVoltage;
float sensorValue;
float UV;

//-----setup-----//
void setup() {
  Serial.begin(115200); //initialiserer kommunikasjon på 115 200 bits per sekund (fordi displayet krever dette)

  //setup for innganger for ledlys og knapp
  pinMode(knapp, INPUT); //knapp
  pinMode(ledlys, INPUT); //ledlys

  //fjerner all tekst fra displayet
  display.clearDisplay(); 
  display.display(); 
  
  //Led-lysene 
  NeoPixel.setBrightness(10);
  NeoPixel.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  NeoPixel.clear();
  NeoPixel.show(); // send the updated pixel colors to the NeoPixel hardware.

  //Metoder for å bestemme én av tre hudtyper
  //settHudLys();
  //settHudMedium();
  //settHudMork();
  
 
  
}

//-----loop-----//
void loop() {
  //clearer displayet
  display.clearDisplay(); 
  display.display();
  //oppdaterer instansvariabelen UV
  oppdaterUV(); 
  //-----knappleser-----//
  while(digitalRead(knapp) == HIGH) { //så lenge knappen holdes inne skal visLysUV-metoden kalles. Dvs. at det holder å trykke på knappen én gang, men man kan også holde den inne for å kunne se lysene lenger. (5sek/10sek/15sek osv. siden metoden varer i 5 sek)
    knappTrykk(); //skrur på lys og viser UV på displayet
  }
  //-----hovedprogram-----//
  //når UV-en måles over 1 skal "klokken" starte
  if (UV > 0) { 
    tid = millis(); //dette "tidspunktet" lagres i instansvariabelen "tid". Om bruker går inn der UV == 0.0, og så går ut igjen til UV > 0.0, vil denne variabelen "tid" oppdatere seg.

    //-----Lav UV-----//
    //så lenge UV-en er mellom 0 og 2 (lavUV) skal brillene lyse grønt hele tiden, ingen sikkerhetstiltak er nødvendig, nyt solen! :)
    while (UV > 0 && UV <3) {
      if (digitalRead(knapp) == HIGH) {
        knappTrykk();

      }
      oppdaterUV();
      unsigned long naaTid = millis(); //tar tiden 
      sumTidLavUV += naaTid - tid; //legger tidsintervallet naaTid - tid inn i summen sumTidLavUV 
      tid = naaTid; //oppdaterer tid
      if (sumTidLavUV > 10000) { //sjekker om summmen er større enn 900 000 millisekunder = 15 min. NB! For testing: bruk kortere tidsintervall enn 15 min
          if (gronn == false) {
          gronnLys(); //skrur på grønt lys etter 15 min
          gronn = true; //endrer instansvariabelen til true så det grønne lyset slås på når knappen trykkes
          oppdaterUV();
          }
      } 
    }


    //-----Middels til Høy UV-----// 
    //så lenge UV-en er høyere enn 3 skal denne løkken kjøre
    while(UV >= 3) {
      oppdaterUV(); //måler og oppdaterer UV-en
      //sjekker forst om knappen blir trykket
      if (digitalRead(knapp) == HIGH) {
        knappTrykk();
      }
      visLysUV(); //sjekker så om noen led-lys skal slås på
      unsigned long naaTid = millis(); //tar tiden fra UV ble over 3

      if (UV > 11 ) { //sjekker om UV-en er større enn 11
        UVTidSum += (naaTid-tid)*UV*OVER11; //legger til tidsintervallet * UV-indeksen * konstanten OVER11 i variabelen UVTidSum. I denne variabelen lagres total UV-eksponering. Enhet: UVindeks millisekunder
      }

      else if (UV >= 8 ) { //sjekker om UV-en er mellom 8 og 10
        UVTidSum += (naaTid-tid)*UV*OVER8;
      }

      else if (UV >= 6 ) { //sjekker om UV-en er mellom 6 og 8
        UVTidSum += (naaTid-tid)*UV*OVER6; 
      }

      else if (UV >= 3 ) { //sjekker om UV-en er mellom 3 og 6
        UVTidSum += (naaTid-tid)*UV*OVER3;
      }

      visLysUV(); //sjekker total UV-eksponering og skrur på lys til tilsvarende UV-ekspionering
      tid = naaTid; //oppdaterer tid
    }
  }  
}

//-----metoder-----//
  void oppdaterUV() { //metode som leser inn analog input fra UV-sensor og oppdaterer instansvariabelen UV
    sensorValue = analogRead(A0);
    sensorVoltage = sensorValue/1023*5.0;
    UV = sensorVoltage/0.1;
  }

  void knappTrykk() { //viser nåværende UV på skjermen og skrur på led-lysene som representerer brukerens samlede eksponering så langt
    //feilmelding om skjermen ikke klarer å skrive ut tekst
     if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
   Serial.println("SSD1306 allocation failed");
   for(;;);
  }
  
    //Bestemmer font og størrelse på teksten som skal skrives ut
    display.clearDisplay(); 
    display.display();
    display.setFont(&FreeSerifBold9pt7b);
    display.setTextColor(WHITE);                    
    display.setTextSize(1);
    display.setCursor(10,20);
    display.println(UV); //viser UV på display
    display.display(); 
    
    //sjekker hvilke lys som allerede har blitt slått på, og slår på disse
    if(oransje == true) {
      oransjeLys();
    } else if (gul == true) {
      gulLys();
    } else if (gronn == true) {
      gronnLys();
    }
      else {
        delay(5000);
      }
    //slår av alle Led-lys og fjerner tekst fra skjerm
    NeoPixel.clear();
    NeoPixel.show();
    display.clearDisplay();
    display.display(); 

  }

  void visLysUV() { //metode som sjekker den totale UV-eksponering til brukeren og setter på led-lys som tilsvarer denne eksponeringen. 
    if (UVTidSum > MAKSEKS) { //sjekker om eksponeringen (UVTidSum) er over den maksimale grensen til brukeren. Dette tallet representerer øverste grense av UV-eksponering for brukeren i løpet av én dag. 
      if (oransje == false) { 
        oransjeLys(); //skrur på oransje, gult og grønt lys
        oransje = true; //instansvariabel settes til true sånn at lysene slås på når knappen trykkes
        oppdaterUV(); //måler og oppdaterer UV
      }
    }
    else if (UVTidSum > MIDEKS) { //ellers sjekkes det om UV er over nest høyeste grense
        if (gul == false) {
          gulLys(); //skrur på gult og grønt lys
          gul = true; //instansvariabel settes til true sånn at lysene slås på når knappen trykkes
          oppdaterUV(); //måler og oppdaterer UV
        }
    }
    else if (UVTidSum > MINEKS) { //ellers sjekkes det om UV er over laveste grense (fylt opp D-vitamin lagrene)
        if (gronn == false) {
          gronnLys(); //skrur på grønt lys 
          gronn = true; //instansvariabel settes til true sånn at lysene slås på når knappen trykkes
          oppdaterUV(); //måler og oppdaterer UV
          }
        }
  }

  void gronnLys() {
    //skrur på grønn
    NeoPixel.setPixelColor(2, NeoPixel.Color(0, 0, 255)); // it only takes effect if pixels.show() is called
    NeoPixel.show(); // send the updated pixel colors to the NeoPixel hardware.
    delay(5000); //venter i 5 sek 
    //skrur av grønn
    }
    
  void gulLys() {
    //skrur på grønn og gul
    NeoPixel.setPixelColor(2, NeoPixel.Color(0, 0, 225)); // it only takes effect if pixels.show() is called
    //skrur på gul
    NeoPixel.setPixelColor(1, NeoPixel.Color(110,127,127));
    NeoPixel.show(); // send the updated pixel colors to the NeoPixel hardware.
    delay(5000); //venter 5 sek
    //skrur av grønn og gul 
    }

  void oransjeLys() {

    //skrur på grønn, gul og rød
    NeoPixel.setPixelColor(2, NeoPixel.Color(0, 0, 225)); // it only takes effect if pixels.show() is called
    //skrur på gul
    NeoPixel.setPixelColor(1, NeoPixel.Color(110,127,127));
    //skrur på oransje
    NeoPixel.setPixelColor(0, NeoPixel.Color(255, 35, 0));
    NeoPixel.show(); // send the updated pixel colors to the NeoPixel hardware.
    delay(5000); // venter 5 sek
    //skrur av alle pixlene; oransje, gul og grønn
    }
    
  //metoder for å endre instansvariablene sånn at koden blir tilpasset ønsket hudtype
  void settHudLys() { 
    OVER3 = 1; 
    OVER6 = 1.3;
    OVER8 = 1.86;
    OVER11 = 3.25;
    MINEKS = 4200000; //minste eksponerinsgrense - bruker har fylt opp D-vitamin lagrene for dagen.
    MIDEKS = 10200000; //midterste eksponeringsgrense - bruker burde smøre seg med solkrem (minst faktor 30)
    MAKSEKS = 16200000; //maksimal eksponeringsgrense - bruker er over grense for sikker soletid, større fare for å bli solbrent 
  }

  void settHudMedium() {
    OVER3 = 1; 
    OVER6 = 1.3;
    OVER8 = 1.86;
    OVER11 = 3.25; 
    MINEKS = 8100000; //minste eksponerinsgrense - bruker har fylt opp D-vitamin lagrene for dagen.
    MIDEKS = 16200000; //midterste eksponeringsgrense - bruker burde smøre seg med solkrem (minst faktor 30)
    MAKSEKS = 24300000; //maksimal eksponeringsgrense - bruker er over grense for sikker soletid, større fare for å bli solbrent 
  }

  void settHudMork() {
    OVER3 = 1; 
    OVER6 = 1.3;
    OVER8 = 1.86;
    OVER11 = 3.25;
    MINEKS = 12150000; //minste eksponerinsgrense - bruker har fylt opp D-vitamin lagrene for dagen.
    MIDEKS = 22275000; //midterste eksponeringsgrense - bruker burde smøre seg med solkrem (minst faktor 30)
    MAKSEKS = 32400000; //maksimal eksponeringsgrense - bruker er over grense for sikker soletid, større fare for å bli solbrent 
  }


