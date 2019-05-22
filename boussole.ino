//////////////////////////////////////////////////////////////////////////////////////////
/*
    BOUSSOLE CMPS10
*/
//////////////////////////////////////////////////////////////////////////////////////////

void afficherBoussole(int adresseI2C) // Boussole CMPS10
{  
  // Leds de couleur
  digitalWrite(LED_JAUNE, HIGH); // LED jaune allumée
  digitalWrite(LED_VERTE, LOW);  // LED verte éteinte  
  
  lcd.createChar(caractereDegre, degre); // création du caractère personnalisé degré 
  boolean quitter = false;
  do {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("BOUSSOLE");
    byte codeErreur = boussoleRead(adresseI2C);    
    if (codeErreur != 0) {
      lcd.setCursor(0,1);
      lcd.print("ERREUR (CODE ");
      lcd.print(codeErreur);
      lcd.print(")");
      bipErreur();
      delay(1000);
      quitter = true;
    }
    else {    
      lcd.setCursor(0,1);
      lcd.print("AZIMUT  : ");
      dtostrf(azimut, 3, 0, ligne);
      lcd.print(ligne);
      lcd.write(caractereDegre);
      lcd.print(" (");
      if (azimut > 315) { lcd.print("NNO"); }
      else if (azimut == 315) { lcd.print("NO"); }
      else if (azimut > 270) { lcd.print("ONO"); }
      else if (azimut == 270) { lcd.print("O"); }
      else if (azimut > 225) { lcd.print("OSO"); }
      else if (azimut == 225) { lcd.print("SO"); }
      else if (azimut > 180) { lcd.print("SSO"); }
      else if (azimut == 180) { lcd.print("S"); }
      else if (azimut > 135) { lcd.print("SSE"); }
      else if (azimut == 135) { lcd.print("SE"); }
      else if (azimut > 90) { lcd.print("ESE"); }
      else if (azimut == 90) { lcd.print("E"); }
      else if (azimut > 45) { lcd.print("ENE"); }
      else if (azimut == 45) { lcd.print("NE"); }
      else if (azimut > 0) { lcd.print("NNE"); }
      else if (azimut == 0) { lcd.print("N"); }    
      lcd.print(")");
      lcd.setCursor(0,2);
      lcd.print("ROULIS  : ");
      dtostrf(roulis, 3, 0, ligne);
      lcd.print(ligne);
      lcd.write(caractereDegre);
      lcd.setCursor(0,3);
      lcd.print("TANGAGE : ");
      dtostrf(tangage, 3, 0, ligne);
      lcd.print(ligne);
      lcd.write(caractereDegre);
      for (int i=0; i<100; i++) {
        delay(1);
        char key = keypad.getKey();
        if (key == '0') {
          quitter = true;
          break;  
        }        
      }
    }
  } while (quitter == false);
  selectMenu(TAG_MENU_PRINCIPAL);
}

//////////////////////////////////////////////////////////////////////////////////////////

byte boussoleRead(int adresseI2C)
{
  // Correctif de bearing : la détection du champ magnétique terrestre est perturbé par
  // la présence d'acier et de courants électriques à proximité du capteur dans le boitier
  // de la centrale. On compare les mesures du capteur avec celles d'une vraie boussole et
  // on corrige. Quand la centrale est orientée au nord, on doit ajouter 14° à la mesure
  // du capteur. Mais attention, si la centrale est orientée différemment, on a une autre
  // correction à adopter :
  // N : +14°
  // E : +32°
  // S : -36°
  // O : -19°
  // Il faudra donc étudier l'azimut mesuré en fonction de l'azimut réel et essayer de
  // modéliser une correction à apporter.
  // Azimut réel           :   0°   45°   90°  135°  180°  225°  270°  315°  360°
  // Azimut mesuré         : 346°   18°   58°  127°  216°  258°  289°  317°  346°
  // Correction à apporter : +14°  +27°  +32°   +8°  -36°  -33°  -19°   -2°  +14°
  // On affine la chose en effectuant un relevé tous les 22,5° et on obtient : 
  const int NB_RELEVES = 17;
  const float AZIMUT_REEL[NB_RELEVES] = 
    {0, 22.5, 45, 67.5, 90, 112.5, 135, 157.5, 180, 202.5, 225, 247.5, 270, 292.5, 315, 337.5, 360};  
  const int AZIMUT_MESURE[NB_RELEVES] =
  // -14 car 346-360 = -14 ; Les azimuts vont toujours en augmentant
    {-14, 1, 18, 37, 58, 86, 127, 179, 216, 240, 258, 274, 289, 303, 317, 331, 346};
  float decalage[NB_RELEVES];
  int azimutMin;
  int azimutMax;  
  float decalageMin;
  float decalageMax;
  int correction = 0;  
  for (int i=0; i<NB_RELEVES; i++) {
    decalage[i] = AZIMUT_REEL[i] - float(AZIMUT_MESURE[i]);  
  }  
  // Lecture des données de la boussole sur le bus I2C
  // Initialisation du pointeur de registre
  Wire.beginTransmission(adresseI2C);
  Wire.write(2); // On pointe vers le registre 2 (voir doc du CMPS10)
  byte error = Wire.endTransmission();
  // On utilise la valeur de retour de Wire.endTransmission pour voir si la boussole
  // est reconnue à l'adresse I2C indiquée. Cela permet de savoir si la boussole est
  // connectée ou non.
  if (error == 0) {
    // Lecture de 4 octets (registres 2, 3, 4, 5)
    Wire.requestFrom(adresseI2C,4);
    while (Wire.available() < 4);
    byte highByte = Wire.read(); // highByte and lowByte store high and
    byte lowByte = Wire.read();  // low bytes of the bearing.
    // Compass Bearing as a word, i.e. 0-3599 for a full circle,
    // representing 0-359.9 degrees.
    int bearing = ((highByte<<8)+lowByte)/10.0; // Calculate full bearing
    // Calcul de la correction liée aux perturbations électromagnétiques dans le boitier
    if (bearing < AZIMUT_MESURE[0]) {
      azimutMin = AZIMUT_MESURE[NB_RELEVES-2]-360;
      azimutMax = AZIMUT_MESURE[0];          
      decalageMin = decalage[NB_RELEVES-2];
      decalageMax = decalage[0];       
    } else if (bearing >= AZIMUT_MESURE[NB_RELEVES-1]) {
      azimutMin = AZIMUT_MESURE[NB_RELEVES-1];
      azimutMax = AZIMUT_MESURE[1]+360;          
      decalageMin = decalage[NB_RELEVES-1];
      decalageMax = decalage[1];      
    } else {
      for (int i = 0; i < NB_RELEVES-1; i++) {
        if (bearing >= AZIMUT_MESURE[i] && bearing < AZIMUT_MESURE[i+1]) {
          azimutMin = AZIMUT_MESURE[i];
          azimutMax = AZIMUT_MESURE[i+1];          
          decalageMin = decalage[i];
          decalageMax = decalage[i+1];
        }
      }
    }       
    // fonction affine du type y = ax + b
    float a = (decalageMax - decalageMin) / float(azimutMax - azimutMin);
    float b = (decalageMin) - float(azimutMin) * (decalageMax - decalageMin) / float(azimutMax - azimutMin);
    correction = int(a * bearing + b);      
    bearing = bearing + correction;
    if (bearing < 0) {
      bearing = bearing + 360;
    } else if (bearing >= 360) {
      bearing = bearing - 360; 
    }
    char pitch = Wire.read();
    char roll = Wire.read();
    azimut = bearing;
    tangage = pitch;
    roulis = roll;    
  }
  return error;
}

//////////////////////////////////////////////////////////////////////////////////////////
