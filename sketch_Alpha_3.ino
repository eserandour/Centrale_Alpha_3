//////////////////////////////////////////////////////////////////////////////////////////
/*
   Centrale Alpha 3
   Copyright 2013, 2014, 2015, 2016, 2019 - Eric Sérandour
   http://3615.entropie.org
*/
const String VERSION = "2019.06.11";
/*   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
//////////////////////////////////////////////////////////////////////////////////////////

/*
  L'électronique :
  
  * Carte Arduino Mega 2560 R3.
  * Shield mémoire 2.0 de Snootlab (carte SD + horloge RTC).
  * Afficheur LCD 4x20 caractères DEM 20485 SYH-LY.
  * Clavier 12 touches matricielles KB12M.
  * Boussole CMPS10
  * Baromètre BMP180
  * Accéléromètre 2D ADXL322

  Le circuit :
  
    Les entrées analogiques :
  =============================
  * Entrée blanche => A0
  * Entrée bleue => A1
  * Entrée jaune => A2
  * Accéléromètre 2D
    X : X direction output voltage => A8
    Y : Y direction output voltage => A9
 
    Les entrées / sorties numériques :
  ======================================
  * Ecran LCD sur D2, D3, D4, D5, D6, D7 de la carte Arduino
    Brochage de l'afficheur compatible HD44780 :
    1  : Vss : GND (fil noir)
    2  : Vdd : Power Supply +5V (fil rouge)
    3  : V0 : Contrast Adjust. Point milieu du potentiomètre (fil gris)
    4  : RS : Register Select Signal. => D2 de l'Arduino (fil bleu)
    5  : R/W : Data Read/Write. Relié à GND (fil noir)
    6  : E : Enable Signal. => D3 de l'Arduino (fil vert)
    7  : non connecté
    8  : non connecté
    9  : non connecté
    10 : non connecté
    11 : DB4 : Data Bus Line. => D4 de l'arduino (fil jaune)
    12 : DB5 : Data Bus Line. => D5 de l'arduino (fil orange)
    13 : DB6 : Data Bus Line. => D6 de l'arduino (fil rouge)
    14 : DB7 : Data Bus Line. => D7 de l'arduino (fil marron)
    15 : LED+ : Power supply for BKL(+). Relié à +5V (fil rouge)
    16 : LED- : Power supply for BKL(-). Relié à GND (fil noir)
  * LED verte, jaune, rouge sur D24, D26, D28 (chacune en série avec une résistance de 330 ohms)
  * Buzzer sur D8.        D8 -> R = 1 kohms -> Buzzer -> GND.
  * Entrées numérique sur D32 et D34. (Entrées vertes)
  * Clavier sur D42, D44, D46, D48 (pour les lignes du clavier).
                D36, D38, D40 (pour les colonnes du clavier).
    Brochage du clavier KB12M acheté chez Gotronic (vu de dessous) :
    R = Rangée. C = Colonne.
    1 (C3) => D36 de l'Arduino
    2 (C2) => D38 de l'Arduino
    3 (R1) => D48 de l'Arduino
    4 (C1) => D40 de l'Arduino
    5 (R2) => D46 de l'Arduino
    6 (C2) => Non connecté (car les broches 2 et 6 sont reliées en interne)
    7 (R3) => D44 de l'Arduino
    8 (C1) => D40 de l'Arduino (les broches 4 et 8 ne sont pas reliées en interne)
    9 (R4) => D42 de l'Arduino
  * La carte SD est reliée au bus SPI (bus série normalisé) de la manière suivante :
  ** SS - D10 du shield Snootlab -> D53. (voir la constante CHIP_SELECT)
  ** MOSI - D11 du shield Snootlab -> D51. (Master Out Slave In : Sortie de données séries)
  ** MISO - D12 du shield Snootlab -> D50. (Master In Slave Out : Entrée de données séries)
  ** SCK - D13 du shield Snootlab -> D52. (Serial ClocK : Pour synchroniser les échanges de données)
  Remarque : on peut accéder aux broches MOSI, MISO et SCK par l'intermédiaire du connecteur ICSP.
  L'intéret est que cela ne demande pas un recablage lorsque que l'on passe de la Mega à la Uno
  et inversement. Reste le cas de SS.

    Les ports de communication :
  ======================================
  L'horloge RTC du shield mémoire est reliée à :
  * Bus I2C : SDA (C20 sur l'Arduino Mega, A4 sur l'Arduino Uno)
  * Bus I2C : SCL (C21 sur l'Arduino Mega, A5 sur l'Arduino Uno)
  Remarque : on peut accéder à ces broches par l'intermédiaire du connecteur à 10 points,
  après AREF : SDA pour la 9ème broche et SCL pour la 10ème broche. L'intéret est que cela
  ne demande pas un recablage lorsque que l'on passe de la Mega à la Uno et inversement.
  Pour le shield Snootlab mémoire 2.0, ceci est exploité au niveau de la carte, pas pour
  la version 1.0.
  
    Bilan :
  ===========
    Sont utilisées : A0, A1, A2, A8, A9, A10,
                     D2, D3, D4, D5, D6, D7, D8,
                     D10, D11, D12, D13, 
                     D24, D26, D28,
                     D30, D32, D34, D36, D38,
                     D40, D42, D44, D46, D48,
                     D50, D51, D52, D53,                    
                     C20, C21.
*/

//////////////////////////////////////////////////////////////////////////////////////////

// *** Afficheur LCD
// On importe la bibliothèque
#include <LiquidCrystal.h>
// Initialisation de la bibliothèque avec les numéros de broches utilisées
LiquidCrystal lcd(2,3,4,5,6,7);   // Correspond sur l'afficheur à RS,Enable,DB4,DB5,DB6,DB7
const byte NB_LIGNES_LCD = 4;     // Nombre de lignes de l'écran
const byte NB_COLONNES_LCD = 20;  // Nombre de colonnes de l'écran
byte ligneLCD = 0;                // Numéro de la ligne
byte colonneLCD = 0;              // Numéro de la colonne
byte degre[8] = {   // Déclaration d’un tableau de 8 octets pour le caractère °.
  B00111,           // Définition de chaque octet au format binaire :
  B00101,           // 1 pour un pixel affiché – 0 pour un pixel éteint.
  B00111,           // Les 3 bits de poids forts sont ici inutiles.
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};
byte carre[8] = {    // Déclaration d’un tableau de 8 octets pour le caractère carré.
  B11100,            // Définition de chaque octet au format binaire :
  B00100,            // 1 pour un pixel affiché – 0 pour un pixel éteint.
  B11100,            // Les 3 bits de poids forts sont ici inutiles.
  B10000,
  B11100,
  B00000,
  B00000,
  B00000
};
byte flecheDroite[8] = {
  B00000,
  B00100,
  B00010,
  B11111,
  B00010,
  B00100,
  B00000,
  B00000
};
byte flecheHaut[8] = {
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00100,
  B00000
};
byte rondPointe[8] = {
  B01110,
  B10001,
  B10001,
  B10101,
  B10001,
  B10001,
  B01110,
  B00000
};
byte caractereDegre = 0;
byte caractereCarre = 1;
byte caractereFlecheDroite = 2;
byte caractereFlecheHaut = 3;
byte caractereRondPointe = 4;
char ligne[NB_COLONNES_LCD]; // Pour le formatage des nombres avec la fonction dtostrf()

//////////////////////////////////////////////////////////////////////////////////////////

// *** Buzzer
const byte BUZZER = 8;

//////////////////////////////////////////////////////////////////////////////////////////

// *** LED
const byte LED_ROUGE = 24;      // Fil orange
const byte LED_JAUNE = 26;      // Fil rouge
const byte LED_VERTE = 28;      // Fil marron

//////////////////////////////////////////////////////////////////////////////////////////

// *** Clavier
// On importe la bibliothèque
#include <Keypad.h>  // http://playground.arduino.cc/Code/Keypad
const byte ROWS = 4; // Nombre de rangées du clavier
const byte COLS = 3; // Nombre de colonnes du clavier
const char KEYS[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {48,46,44,42}; // Rangées R1,R2,R3,R4
byte colPins[COLS] = {40,38,36};    // Colonnes C1,C2,C3
Keypad keypad = Keypad(makeKeymap(KEYS), rowPins, colPins, ROWS, COLS);
// Supérieurs à 99 pour permettre de rentrer au clavier des nombres à 2 chiffres,
const byte TOUCHE_DIESE = 100;  // car le code ASCII de # est 35 
const byte TOUCHE_ETOILE = 101; // et celui de * est 42.
//const char NO_KEY = '\0'; // null ou code 0 en ASCII => intégré à la bibliothèque Keypad

//////////////////////////////////////////////////////////////////////////////////////////

// *** Horloge RTC
// On importe les bibliothèques
#include <Wire.h>
#include <RTClib.h>  // https://github.com/adafruit/RTClib
RTC_DS1307 RTC;
DateTime DateHeure;
String Date;
String Heure;

//////////////////////////////////////////////////////////////////////////////////////////

// *** Carte SD
// On importe la bibliothèque
#include <SD.h>
// On the Ethernet Shield, CHIP_SELECT (SS) is pin 4. Note that even if it's not
// used as the SS pin, the hardware SS pin (10 on most Arduino boards,
// 53 on the Mega) must be left as an output or the SD library
// functions will not work.
const byte CHIP_SELECT = 53; // Chip Select de la carte SD.
File dataFile;
String enteteFichier;
const String SEPARATEUR =";"; // Séparateur de données pour le tableur
                              // Ne pas choisir l'espace à cause de Date Heure

//////////////////////////////////////////////////////////////////////////////////////////

// *** Ecrans
const byte TAG_ECRAN_ACCUEIL = 0;
const byte TAG_MENU_PRINCIPAL = 1;
const byte TAG_REGLAGE_HORLOGE = 2;
const byte TAG_MENU_CADENCE = 3;
const byte TAG_MENU_CAPTEURS = 4;
const byte TAG_ENREGISTRER_FICHIER = 5;
const byte TAG_TRANSFERER_FICHIER = 6;
const byte TAG_MENU_BARO_ALTIMETRE = 7;
const byte TAG_MENU_BOUSSOLE = 8;
const byte TAG_MENU_ACCELEROMETRE = 9;
//const byte TAG_MENU_ENTREES_ANALOGIQUES = 10;
//const byte TAG_MENU_ENTREES_NUMERIQUES = 11;
byte ecran = TAG_ECRAN_ACCUEIL;
int defilement = 0;

//////////////////////////////////////////////////////////////////////////////////////////

// *** Gestion du temps
const byte MODE_MANUEL = 0;
const unsigned long CADENCE[9] = {
MODE_MANUEL, // MODE MANUEL
100,         // 100 MS
1000,        // 1 S
5000,        // 5 S
15000,       // 15 S
60000,       // 1 MIN
300000,      // 5 MIN
900000,      // 15 MIN
3600000      // 1 H
};
unsigned long cadenceDefaut = CADENCE[2]; // Valeur par défaut.
unsigned long deltaMesures = cadenceDefaut; // Intervalle entre 2 mesures (en ms).
unsigned long time = 0;
unsigned long timeOffset = 0;

//////////////////////////////////////////////////////////////////////////////////////////

// *** Autres
unsigned long numeroMesure = 0;
boolean recording = false;

//////////////////////////////////////////////////////////////////////////////////////////






//////////////////////////////////////////////////////////////////////////////////////////

// *** LES CAPTEURS (début)

//////////////////////////////////////////////////////////////////////////////////////////

#define AUCUN_CAPTEUR -1

// *** Entrées analogiques en façade
const byte EA0 = 0;  // Entrée blanche
const byte EA1 = 1;  // Entrée bleue
const byte EA2 = 2;  // Entrée jaune

// *** Entrées numériques en façade (Bornes vertes)
const byte EN32 = 32;           // Fil vert
const byte EN34 = 34;           // Fil vert
const byte DECLENCHEUR = EN34;  // Pour déclencher un enregistrement en mode manuel

// Le baromètre BMP180
#define ADRESSE_BMP180 0x77 // 119 en décimal

// La boussole CMPS10
#define ADRESSE_CMPS10 0x60 // 96 en décimal

// L'accéléromètre ADXL322
#define ADRESSE_ADXL322 EA8  // La première de 2 entrées analogiques nécessairement successives
const byte EA8 = 8;          // Axe X de l'accéléromètre
const byte EA9 = 9;          // Axe Y de l'accéléromètre

// *** Entrées disponibles
// Modifier en conséquence la procédure lectureCapteurs() juste ci-dessous
const byte NB_ENTREES_MAX = 8;
const byte ENTREE[NB_ENTREES_MAX] = {
  EA0,                   //   0  Entrée analogique A0
  EA1,                   //   1  Entrée analogique A1
  EA2,                   //   2  Entrée analogique A2
  EN32,                  //  32  Entrée numérique D32
  EN34,                  //  34  Entrée numérique D34
  ADRESSE_BMP180,        // 119  Adresse du baro-altimètre BMP180 sur le port I2C
  ADRESSE_CMPS10,        //  96  Adresse de la boussole CMPS10 sur le port I2C
  ADRESSE_ADXL322        //   8  Adresse de l'accéléromètre ADXL322
};
const String NOM_ENTREE[NB_ENTREES_MAX] = {
  "EA1 (BLANCHE)",       // EA0
  "EA2 (BLEUE)",         // EA1
  "EA3 (JAUNE)",         // EA2
  "EN1 (VERTE)",         // EN32
  "EN2 (VERTE)",         // EN34
  "BARO-ALTIMETRE",      // ADRESSE_BMP180
  "BOUSSOLE",            // ADRESSE_CMPS10
  "ACCELEROMETRE"        // ADRESSE_ADXL322
};
const byte POIDS_ENTREE[NB_ENTREES_MAX] = {
  1, // L'entrée blanche mesure 1 seule grandeur
  1, // L'entrée bleue mesure 1 seule grandeur
  1, // L'entrée jaune mesure 1 seule grandeur
  1, // L'entrée verte mesure 1 seule grandeur
  1, // L'entrée verte mesure 1 seule grandeur
  3, // Le baro-altimètre renvoie 3 grandeurs : La pression absolue, la pression relative, l'altitude
  3, // La boussole renvoie 3 grandeurs : L'azimut, le tangage, le roulis
  2  // L'accéléromètre renvoie 2 grandeurs : L'accélération en X et en Y
};
const byte NB_MESURES_MAX = 13; // La somme du poids des entrées (1+1+1+1+1+3+3+2)
const String NOM_MESURE[NB_MESURES_MAX] = {
  "EA1 (BLANCHE)",
  "EA2 (BLEUE)",
  "EA3 (JAUNE)",
  "EN1 (VERTE)",
  "EN2 (VERTE)",
  "PRESSION ABSOLUE (hPa)", "PRESSION RELATIVE (hPa)", "ALTITUDE (m)",
  "AZIMUT (°)", "ROULIS (°)", "TANGAGE (°)",
  "ACCELERATION X (dm/s2)", "ACCELERATION Y (dm/s2)"
};

// *** Variables
boolean selectionCapteur[NB_ENTREES_MAX];
int nbCapteurs;
int adresseCapteur[NB_ENTREES_MAX];
int nbMesures;
int mesureBrute[NB_MESURES_MAX];

// *** Baro-altimètre BMP180
float pressionAbsolue = 0;
float pressionRelative = 0;
float altitude = 0;
float temperature = 0;       // Température à l'intérieur de la centrale (non utilisée ici)
float altitudeReference = 0; // Altitude connue rentrée au clavier
float pressionReference = 0; // Pression à l'altitude de référence

// *** Boussole CMPS10
int azimut = 0;
int roulis = 0;
int tangage = 0;

// Accéléromètre ADXL322
float xRead; // Mesure brute (0-1023)
float yRead; // Mesure brute (0-1023)
float accelerationX = 0; // en m/s2
float accelerationY = 0; // en m/s2
int angleX; // Seulement pour controler le capteur. Préférer le roulis de la boussole
int angleY; // Seulement pour controler le capteur. Préférer le tangage de la boussole

//////////////////////////////////////////////////////////////////////////////////////////

void initCapteurs()
{
  // Initialisation des entrées
  nbCapteurs = 0;
  for (int i=0; i<NB_ENTREES_MAX; i++) {
    adresseCapteur[i] = AUCUN_CAPTEUR;
  }
  // Initialisation de l'entete du fichier
  enteteFichier = "MESURE" + SEPARATEUR + "DATE HEURE" + SEPARATEUR + "MILLISECONDES";    
}

//////////////////////////////////////////////////////////////////////////////////////

void lectureCapteurs()
{
  int j=0;
  for (int i=0; i<nbCapteurs ; i++) {
    if (adresseCapteur[i] == ENTREE[0]) {       // Entrée blanche
      mesureBrute[j] = analogRead(ENTREE[0]);
      j=j+POIDS_ENTREE[0];
    }
    else if (adresseCapteur[i] == ENTREE[1]) {  // Entrée bleue
      mesureBrute[j] = analogRead(ENTREE[1]);
      j=j+POIDS_ENTREE[1];
    }
    else if (adresseCapteur[i] == ENTREE[2]) {  // Entrée jaune
      mesureBrute[j] = analogRead(ENTREE[2]);
      j=j+POIDS_ENTREE[2];
    }
    else if (adresseCapteur[i] == ENTREE[3]) {  // Entrée verte 1
      mesureBrute[j] = digitalRead(ENTREE[3]);
      j=j+POIDS_ENTREE[3];
    }
    else if (adresseCapteur[i] == ENTREE[4]) {  // Entrée verte 2
      mesureBrute[j] = digitalRead(ENTREE[4]);
      j=j+POIDS_ENTREE[4];
    }    
    else if (adresseCapteur[i] == ENTREE[5]) {  // Baro-Altimètre BMP180
      barometreRead(ENTREE[5]);
      mesureBrute[j] = round(pressionAbsolue);
      mesureBrute[j+1] = round(pressionRelative);
      mesureBrute[j+2] = round(altitude);
      j=j+POIDS_ENTREE[5];
    }    
    else if (adresseCapteur[i] == ENTREE[6]) {  // Boussole CMPS10
      boussoleRead(ENTREE[6]);
      mesureBrute[j] = azimut;
      mesureBrute[j+1] = roulis;
      mesureBrute[j+2] = tangage;
      j=j+POIDS_ENTREE[6];
    }
    else if (adresseCapteur[i] == ENTREE[7]) {  // Accéléromètre
      accelerometreRead(ENTREE[7]);
      mesureBrute[j] = accelerationX * 10;      // Pour récupérer la première décimale
      mesureBrute[j+1] = accelerationY * 10;    // Pour récupérer la première décimale
      j=j+POIDS_ENTREE[7];
    }      
  }
  nbMesures=j;
}

//////////////////////////////////////////////////////////////////////////////////////////

// *** LES CAPTEURS (fin)

//////////////////////////////////////////////////////////////////////////////////////////






//////////////////////////////////////////////////////////////////////////////////////////
/*
    *** SETUP
*/
//////////////////////////////////////////////////////////////////////////////////////////

void setup()
{
  // Capteurs par défaut
  initCapteurs();

  Serial.begin(9600);  // Setup serial. Transfert des données à 9600 bauds.
  
  // Afficheur LCD
  lcd.begin(20,4);
  lcd.clear();
  
  // Buzzer
  pinMode(BUZZER, OUTPUT);
  bipOK();
  
  // LED
  pinMode(LED_ROUGE, OUTPUT);
  pinMode(LED_JAUNE, OUTPUT);
  pinMode(LED_VERTE, OUTPUT);
  digitalWrite(LED_ROUGE, LOW);  // LED éteinte.
  digitalWrite(LED_JAUNE, LOW);  // LED éteinte.
  digitalWrite(LED_VERTE, LOW);  // LED éteinte.
 
  // Initialisation de l'interface I2C.
  Wire.begin();
  initHorloge();  // Horloge RTC DS 1307 (I2C)

  // Carte SD
  // Make sure that the default chip select pin is set to output, even if you don't use it.
  pinMode(CHIP_SELECT, OUTPUT); // Chip Select en sortie.
  // Les 4 broches 10,11,12,13 du shield mémoire sont reroutées vers le bus SPI de la carte
  // Arduino Mega par 4 fils. Afin que les broches D10,D11,D12,D13 de la carte Arduino ne
  // perturbent pas ce reroutage, on les définit comme des entrées.
  pinMode(10,INPUT);
  pinMode(11,INPUT);
  pinMode(12,INPUT);
  pinMode(13,INPUT);
  // See if the card is present and can be initialized.
  if (!SD.begin(CHIP_SELECT)) {
    bipErreur();
    lcd.setCursor(0,1);
    lcd.print("PROBLEME DE CARTE SD");    
    delay(3000);
    lcd.clear();
  } else {
    bipOK();
  }
  
  // Bornes numériques
  pinMode(EN32, INPUT);
  pinMode(EN34, INPUT);
}

//////////////////////////////////////////////////////////////////////////////////////////






//////////////////////////////////////////////////////////////////////////////////////////
/*
    *** LOOP
*/
//////////////////////////////////////////////////////////////////////////////////////////

void loop()
{ 
  switch(ecran) {
    case TAG_ECRAN_ACCUEIL: afficherEcranAccueil(); break;
    case TAG_MENU_PRINCIPAL: afficherMenuPrincipal(); break;
    case TAG_REGLAGE_HORLOGE: afficherReglageHorloge(); break;
    case TAG_MENU_CADENCE: afficherMenuCadence(); break;
    case TAG_MENU_CAPTEURS: afficherMenuCapteurs(); break;
    case TAG_MENU_BARO_ALTIMETRE: afficherBaroAltimetre(ADRESSE_BMP180); break;
    case TAG_MENU_BOUSSOLE: afficherBoussole(ADRESSE_CMPS10); break;
    case TAG_MENU_ACCELEROMETRE: afficherAccelerometre(ADRESSE_ADXL322); break;
    case TAG_ENREGISTRER_FICHIER: enregistrerFichier(); break;
    case TAG_TRANSFERER_FICHIER: transfererFichier(); break;
  }
}

//////////////////////////////////////////////////////////////////////////////////////






//////////////////////////////////////////////////////////////////////////////////////
/*
   *** ECRAN ACCUEIL
*/
//////////////////////////////////////////////////////////////////////////////////////

void afficherEcranAccueil()
{    
  // Affichage
  afficherHorloge();
  lcd.setCursor(0,0);
  lcd.print("* MENU");
  lcd.setCursor(11,0);
  lcd.print("# HORLOGE");

  // Leds de couleur
  digitalWrite(LED_VERTE, HIGH); // LED verte allumée
  digitalWrite(LED_JAUNE, LOW);  // LED jaune éteinte.
  digitalWrite(LED_ROUGE, LOW);  // LED rouge éteinte.
  
  // Gestion du clavier
  switch(keypad.getKey()) {      
    case '*': selectMenu(TAG_MENU_PRINCIPAL); break;
    case '#': selectMenu(TAG_REGLAGE_HORLOGE); break;
  }
}

//////////////////////////////////////////////////////////////////////////////////////






//////////////////////////////////////////////////////////////////////////////////////
/*
   *** MENU PRINCIPAL
*/
//////////////////////////////////////////////////////////////////////////////////////

void afficherMenuPrincipal()
{
  // Affichage
  const byte NB_LIGNES_MENU = 11;
  String menu[] = {
  "0: ACCUEIL          ",
  "1: PARAMETRES       ",
  "2: ENREGISTRER      ",
  "3: TRANSFERT -> USB ",
  "--------------------",
  "4: BARO-ALTIMETRE   ",
  "5: BOUSSOLE         ",
  "6: ACCELEROMETRE    ",
  "--------------------",
  "    V." + VERSION,
  "--------------------"
  };
  afficheMenu(menu, NB_LIGNES_MENU);

  // Leds de couleur
  digitalWrite(LED_VERTE, HIGH); // LED verte allumée
  digitalWrite(LED_JAUNE, LOW);  // LED jaune éteinte.
  digitalWrite(LED_ROUGE, LOW);  // LED rouge éteinte.

  // Gestion du clavier
  switch(choixMenu(1)) { // On entre un nombre à 1 chiffre
    case 0: selectMenu(TAG_ECRAN_ACCUEIL); break;
    case 1: selectMenu(TAG_MENU_CADENCE); break;
    case 2: selectMenu(TAG_ENREGISTRER_FICHIER); break;
    case 3: selectMenu(TAG_TRANSFERER_FICHIER); break;
    case 4: selectMenu(TAG_MENU_BARO_ALTIMETRE); break;
    case 5: selectMenu(TAG_MENU_BOUSSOLE); break;
    case 6: selectMenu(TAG_MENU_ACCELEROMETRE); break;
    case TOUCHE_DIESE: defileMenu(NB_LIGNES_MENU); break;
  }
}

//////////////////////////////////////////////////////////////////////////////////////






//////////////////////////////////////////////////////////////////////////////////////
/*
   *** PARAMETRES D'ENREGISTREMENT
*/
//////////////////////////////////////////////////////////////////////////////////////

void afficherMenuCadence()
{
  // Affichage
  const byte NB_LIGNES_MENU = 9;
  String menu[] = {
    "1:     MANUEL",
    "2: 100 MILLISECONDES",
    "3:   1 SECONDE",
    "4:   5 SECONDES",
    "5:  15 SECONDES",
    "6:   1 MINUTE",
    "7:   5 MINUTES",
    "8:  15 MINUTES",
    "9:   1 HEURE" 
  };  
  afficheMenu(menu, NB_LIGNES_MENU);
  
  // Leds de couleur  
  digitalWrite(LED_JAUNE, HIGH); // LED jaune allumée
  digitalWrite(LED_VERTE, LOW);  // LED verte éteinte  
  
  // Gestion du clavier
  byte choix = choixMenu(1); // On entre un nombre à 1 chiffre
  switch(choix) {
    case TOUCHE_ETOILE: break;
    case 0: selectMenu(TAG_MENU_PRINCIPAL); break;
    case TOUCHE_DIESE: defileMenu(NB_LIGNES_MENU); break;
    default :
      if (choix <= NB_LIGNES_MENU) {
        deltaMesures = CADENCE[choix-1];
        int correction = 0;
        if (defilement > NB_LIGNES_MENU - NB_LIGNES_LCD) {
          if (choix <= defilement) {
          correction = NB_LIGNES_MENU;
          }
        }
        if ((choix+correction >= defilement+1) 
        && (choix+correction <= defilement+NB_LIGNES_LCD)) {
          lcd.clear();
          lcd.setCursor(0,(choix+NB_LIGNES_MENU-1-defilement)%NB_LIGNES_MENU);
          lcd.print(menu[choix-1]);
          delay(1000);
          selectMenu(TAG_MENU_CAPTEURS);
          // Initialisation des capteurs sélectionnés
          nbCapteurs = 0;
          for (int i=0; i<NB_ENTREES_MAX; i++) {
            adresseCapteur[i] = AUCUN_CAPTEUR;
            selectionCapteur[i] = false;
          }
          enteteFichier="MESURE";
        }  
      }
  }
}

//////////////////////////////////////////////////////////////////////////////////////

void afficherMenuCapteurs()
{ 
  // Affichage
  const byte NB_LIGNES_MENU = NB_ENTREES_MAX;
  String menu[NB_LIGNES_MENU];
  for (int i=0; i<NB_LIGNES_MENU; i++) {
    int nbEspaces = NB_COLONNES_LCD - NOM_ENTREE[i].length() - 4;
    /*if (i<9) {
      menu[i] = "0";
    }*/
    menu[i] += String(i+1,DEC) + ": " + NOM_ENTREE[i];
    for (int j=1; j<=nbEspaces; j++) {
      menu[i] += " ";
    }
    if (selectionCapteur[i]) {
      menu[i]=menu[i].substring(0,NB_COLONNES_LCD-1)+'*';
    }
  }
  afficheMenu(menu, NB_LIGNES_MENU);
  
  // Gestion du clavier
  byte choix = choixMenu(1); // On entre un nombre à 1 chiffre
  switch(choix) {
    case TOUCHE_ETOILE: // Validation
      enteteFichier += SEPARATEUR + "DATE HEURE" + SEPARATEUR + "MILLISECONDES";               
      // REGLAGE DE L'ALTIMETRE
      for (int i=0; i<nbCapteurs; i++) {
        if (adresseCapteur[i] == ADRESSE_BMP180) {
          initBaroAltimetre(adresseCapteur[i]);
          reglageBaroAltimetre(adresseCapteur[i]);
        }
      }
      selectMenu(TAG_MENU_PRINCIPAL);
      break;   
    case 0: selectMenu(TAG_MENU_PRINCIPAL); break;
    case TOUCHE_DIESE: defileMenu(NB_LIGNES_MENU); break;
    default :
      if (choix <= NB_LIGNES_MENU) {
        int correction = 0;
        if (defilement > NB_LIGNES_MENU - NB_LIGNES_LCD) {
          if (choix <= defilement) {
            correction = NB_LIGNES_MENU;
          }  
        }
        if ((choix+correction >= defilement+1) 
        && (choix+correction <= defilement+NB_LIGNES_LCD)) {
          if (selectionCapteur[choix-1] == true) {
            bipErreur();
          }
          else {
            selectionCapteur[choix-1] = true;
            adresseCapteur[nbCapteurs] = ENTREE[choix-1];       
            int index = 0;
            for (int i=0; i<choix-1; i++) {
              index += POIDS_ENTREE[i];
            }
            for (int i=0; i<POIDS_ENTREE[choix-1]; i++) {
              enteteFichier += SEPARATEUR + NOM_MESURE[index+i];
            }       
            nbCapteurs++;
            while (keypad.getKey() != NO_KEY); // On boucle tant que la touche pressée n'est pas relachée
          }
        }        
      }
  }
}

//////////////////////////////////////////////////////////////////////////////////////






//////////////////////////////////////////////////////////////////////////////////////
/*
   *** ENREGISTRER FICHIER
*/
//////////////////////////////////////////////////////////////////////////////////////

void enregistrerFichier()
{
  do {
    if (!recording) {
      nouveauFichier();
      recording = true;
      numeroMesure = 0;
      // Leds de couleur
      digitalWrite(LED_ROUGE, HIGH); // LED rouge allumée.
      digitalWrite(LED_VERTE, LOW);  // LED verte éteinte
    }
    else {
      // MODE MANUEL
      if (deltaMesures == MODE_MANUEL) {
        DateHeure = RTC.now();
        lectureCapteurs();
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("ATTENTE");      
        lcd.setCursor(0,1);
        lcd.print("NB MESURES: ");     
        lcd.print(numeroMesure+1);
        lcd.setCursor(0,2);
        lcd.print("#: ENREGISTRER");
        lcd.setCursor(0,3);
        lcd.print("*: STOP");        
        formaterDateHeure();
        
        // Vers la carte SD
        // Open the file. Note that only one file can be open at a time,
        // so you have to close this one before opening another.
        File dataFile = SD.open("data.txt", FILE_WRITE);
        // If the file is available, write to it.
        if (dataFile) {
          dataFile.print(numeroMesure);
          dataFile.print(SEPARATEUR);
          for (int i=0; i<(nbMesures); i++) {
            dataFile.print(mesureBrute[i]);
            dataFile.print(SEPARATEUR); 
          }
          dataFile.print(Date);
          dataFile.print(" ");
          dataFile.print(Heure);
          dataFile.println("");
          dataFile.close();
        }
        // If the file isn't open, pop up an error.
        else {
          Serial.println("Error opening datalog.txt");  
          bipErreur();
        }
        
        // Vers le port série
        Serial.print(numeroMesure);
        Serial.print(SEPARATEUR);
        for (int i=0; i<(nbMesures); i++) {
          Serial.print(mesureBrute[i]);
          Serial.print(SEPARATEUR);
        }
        Serial.print(Date);
        Serial.print(" ");
        Serial.print(Heure);
        Serial.println("");        
        
        boolean attente = true;
        do {
          // Lecture du clavier
          char key = keypad.getKey();
          switch (key) {
            case '*' :
              // Stoppe l'enregistrement
              recording = false;
              // Marqueur de fin d'enregistrement (pour Python)
              Serial.write("\r\n"); // Retour à la ligne + Saut de ligne
              selectMenu(TAG_MENU_PRINCIPAL);
              break;
            case '#' :
              attente = false;
              while (keypad.getKey() != NO_KEY); // On boucle tant que la touche pressée n'est pas relachée
              break;
          }
          // Lecture du déclencheur à distance
          if (digitalRead(DECLENCHEUR) == HIGH) {
            delay(50); // Delai supérieur à celui des rebondissements d'une éventuelle touche
            attente = false;
            while (digitalRead(DECLENCHEUR) == HIGH); // On boucle en attendant que le déclencheur passe à LOW          
          }
        } while (attente && recording);
        numeroMesure++;
      }
      // MODE AUTOMATIQUE
      else {
        time = millis();
        DateHeure = RTC.now();
        if (numeroMesure == 0) {
          timeOffset = time;
        }
        unsigned long duree = time - timeOffset;
        lectureCapteurs();
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("ECRITURE");      
        lcd.setCursor(0,1);
        lcd.print("NB MESURES: ");     
        lcd.print(numeroMesure+1);
        lcd.setCursor(0,2);
        lcd.print("DUREE: ");
        String dureeFormatee = "";
        int nbHeures = long(numeroMesure*deltaMesures/3600000);
        int nbMinutes = long(numeroMesure*deltaMesures/60000-nbHeures*60);
        int nbSecondes = long(numeroMesure*deltaMesures/1000-nbHeures*3600-nbMinutes*60);
        if (nbHeures < 10) {
          dureeFormatee = "0";
        }
        dureeFormatee += String(nbHeures, DEC);
        if (deltaMesures < 3600000) {
          dureeFormatee += ":";  
          if (nbMinutes < 10) {
            dureeFormatee += "0";
          }
          dureeFormatee += String(nbMinutes, DEC);
        }
        if (deltaMesures < 60000) {
          dureeFormatee += ":";      
          if (nbSecondes < 10) {
            dureeFormatee += "0";
          }      
          dureeFormatee += String(nbSecondes, DEC);
        }
        lcd.print(dureeFormatee);
        lcd.setCursor(0,3);
        lcd.print("*: STOP");        
        formaterDateHeure();
        
        // Vers la carte SD
        // Open the file. Note that only one file can be open at a time,
        // so you have to close this one before opening another.
        File dataFile = SD.open("data.txt", FILE_WRITE);
        // If the file is available, write to it.
        if (dataFile) {
          dataFile.print(numeroMesure);
          dataFile.print(SEPARATEUR);
          for (int i=0; i<(nbMesures); i++) {
            dataFile.print(mesureBrute[i]);
            dataFile.print(SEPARATEUR);
          }      
          dataFile.print(Date);
          dataFile.print(" ");
          dataFile.print(Heure);
          dataFile.print(SEPARATEUR);
          dataFile.print(duree);
          dataFile.println("");
          dataFile.close(); 
        }
        // If the file isn't open, pop up an error.
        else {
          Serial.println("Error opening datalog.txt");  
          bipErreur();
        }
        
        // Vers le port série
        Serial.print(numeroMesure);
        Serial.print(SEPARATEUR);
        for (int i=0; i<(nbMesures); i++) {
          Serial.print(mesureBrute[i]);
          Serial.print(SEPARATEUR);
        }
        Serial.print(Date);
        Serial.print(" ");
        Serial.print(Heure);
        Serial.print(SEPARATEUR);
        Serial.print(duree);
        Serial.println("");
        
        // Mécanisme de régulation.
        // On regarde où on en est au niveau temps parce qu'une boucle dure ici environ 33 ms
        time = millis();
        duree = time - timeOffset;
        long correction = duree-(numeroMesure*deltaMesures);
        long tempsPause = deltaMesures-correction; // Pour éviter de rares bugs lors de
        if (tempsPause < 0) {                      // la régulation : il peut arriver que
          tempsPause = 0;                          // correction dépasse 100 ms.
        }
        unsigned long tempsEcoule;
        do {
          char key = keypad.getKey(); // Lecture de la touche actionnée 
          if (key == '*') {
            // Stoppe l'enregistrement
            recording = false;
            // Marqueur de fin d'enregistrement (pour Python)
            Serial.write("\r\n"); // Retour à la ligne + Saut de ligne
            selectMenu(TAG_MENU_PRINCIPAL);
            break;  // On sort de la boucle do...while  
          }
          tempsEcoule = millis() - time;     
        } while (tempsEcoule < tempsPause);
        numeroMesure++;
      }
    }  
  } while (recording == true);
}

//////////////////////////////////////////////////////////////////////////////////////

void nouveauFichier()
{
  String entete = enteteFichier;
  if (deltaMesures == MODE_MANUEL) {
    // On enlève MILLISECONDES à la fin de l'entete du fichier d'où le -14
    entete = enteteFichier.substring(0,enteteFichier.length()-14);
  }
  
  // Sortie sur carte SD
  // Effacer le fichier data.txt précédent
  if (SD.exists("data.txt")) {
    SD.remove("data.txt");
  }
  // Créer le fichier data.txt et sa première ligne
  dataFile = SD.open("data.txt", FILE_WRITE);  
  // If the file is available, write to it.
  if (dataFile) {
    dataFile.println(entete);
    dataFile.close(); 
  }
  // If the file isn't open, pop up an error.
  else {
    Serial.println("Error opening data.txt");
    bipErreur();
  }

  // Sortie sur port série
  Serial.println(entete);
}

//////////////////////////////////////////////////////////////////////////////////////////






//////////////////////////////////////////////////////////////////////////////////////
/*
   *** TRANSFERER FICHIER
*/
//////////////////////////////////////////////////////////////////////////////////////

void transfererFichier()
{
  // Leds de couleur
  digitalWrite(LED_JAUNE, HIGH); // LED jaune allumée
  digitalWrite(LED_VERTE, LOW);  // LED verte éteinte  
  
  lcd.setCursor(0,0);
  lcd.print("TRANSFERT DU FICHIER");
  // Lecture du fichier data.txt sur le port série  
  dataFile = SD.open("data.txt");
  if (dataFile) {
    unsigned long tailleFichier = dataFile.size();
    // Read from the file until there's nothing else in it
    while (dataFile.available()) {
      Serial.write(dataFile.read());
      // Affichage du pourcentage de transfert
      lcd.setCursor(15,3);
      byte pourcentage = 100 * dataFile.position() / tailleFichier;
      dtostrf(pourcentage, 3, 0, ligne);
      lcd.print(ligne);
      lcd.print(" %");    
      // Pour abandonner le transfert
      char key = keypad.getKey(); // Lecture de la touche actionnée 
      if (key == '0') {
        break;  // On sort de la boucle while  
      }
    }
    // Marqueur de fin de fichier
    Serial.write("\r\n"); // Retour à la ligne + Saut de ligne
    // Close the file
    dataFile.close();
    delay(3000); // Le temps que le programme de récupération se ferme
    bipOK();
    selectMenu(TAG_MENU_PRINCIPAL);
  }
  // If the file didn't open, print an error
  else { 
    Serial.println("Error opening data.txt");
    bipErreur();
  }  
}

//////////////////////////////////////////////////////////////////////////////////////////
