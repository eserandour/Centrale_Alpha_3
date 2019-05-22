//////////////////////////////////////////////////////////////////////////////////////////
/*
    BARO-ALTIMETRE BMP180
*/
//////////////////////////////////////////////////////////////////////////////////////////

void afficherBaroAltimetre(int adresseI2C)
{
  // Leds de couleur
  digitalWrite(LED_JAUNE, HIGH); // LED jaune allumée
  digitalWrite(LED_VERTE, LOW);  // LED verte éteinte    
  
  // Initialisation du Baro-Altimètre BMP180
  initBaroAltimetre(adresseI2C);
  
  // Réglage de l'alti-baromètre
  reglageBaroAltimetre(adresseI2C);
  
  // Affichage des mesures
  lcd.createChar(caractereDegre, degre); // Création du caractère personnalisé degré 
  boolean quitter = false;
  do {
    lcd.clear();
    byte codeErreur = barometreRead(adresseI2C);
    if (codeErreur != 0) {
      lcd.setCursor(0,0);
      lcd.print("BARO-ALTIMETRE");
      lcd.setCursor(0,1);
      lcd.print("ERREUR (CODE ");
      lcd.print(codeErreur);
      lcd.print(")");
      bipErreur();
      delay(1000);
      quitter = true;
    }
    else {
      lcd.setCursor(0,0);
      lcd.print("BARO-ALTIMETRE");
      /*dtostrf(temperature, 6, 1, ligne);
      lcd.print(ligne);
      lcd.write(caractereDegre);
      lcd.print("C");*/
      lcd.setCursor(0,1);
      lcd.print("Pa : ");
      dtostrf(pressionAbsolue, 6, 1, ligne);
      lcd.print(ligne);
      lcd.print(" hPa");
      lcd.setCursor(0,2);
      lcd.print("Pr : ");
      dtostrf(pressionRelative, 6, 1, ligne);
      lcd.print(ligne);
      lcd.print(" hPa");
      lcd.setCursor(0,3);
      lcd.print("H  : ");
      dtostrf(altitude, 6, 1, ligne);
      lcd.print(ligne);
      lcd.print(" m");
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

void reglageBaroAltimetre(int adresseI2C)
{
  altitudeReference = 0;
  int colonne = 15; // Position du point d'interrogation (voir pus bas)
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("REGLAGE");
  lcd.setCursor(0,1);
  lcd.print("ALTITUDE (m) : ?");
  lcd.setCursor(0,2);
  lcd.print("# : EFFACER");   
  lcd.setCursor(0,3);
  lcd.print("* : VALIDER");  
  boolean quitter = false;
  do {
    char key = keypad.getKey();
    if (key != NO_KEY) { // Si une touche est actionnée
      switch (key) {
        case '#': // Effacer
          lcd.setCursor(0,1);
          lcd.print("ALTITUDE (m) : ?    ");
          colonne = 15;
          altitudeReference = 0;
          break;
        case '*': // Valider
          barometreRead(adresseI2C);
          pressionReference = pressionAbsolue;
          quitter = true;
          break;
        default :
          if (colonne < NB_COLONNES_LCD-1) {
            lcd.setCursor(colonne,1);
            lcd.print(valeurTouche(key));
            altitudeReference = 10 * altitudeReference + valeurTouche(key);
            colonne++;
          }
      }
    }
  } while (quitter == false);  
}

//////////////////////////////////////////////////////////////////////////////////////////

int16_t  ac1, ac2, ac3, b1, b2, mb, mc, md; // Calibration coefficients
uint16_t ac4, ac5, ac6;                     // Calibration coefficients
// Ultra low power       : oss = 0, osd =  5 ms
// Standard              : oss = 1, osd =  8 ms
// High resolution       : oss = 2, osd = 14 ms
// Ultra high resolution : oss = 3, osd = 26 ms
const uint8_t OSS = 3;     // Set oversampling setting
const uint8_t OSD = 26;    // with corresponding oversampling delay

//////////////////////////////////////////////////////////////////////////////////////////

void initBaroAltimetre(int adresseI2C) // Voir le Data sheet du BMP180, à la page 15.
{
  // Read calibration data from the EEPROM of the BMP180
  ac1 = readRegister16(adresseI2C, 0xAA);
  ac2 = readRegister16(adresseI2C, 0xAC);
  ac3 = readRegister16(adresseI2C, 0xAE);
  ac4 = readRegister16(adresseI2C, 0xB0);
  ac5 = readRegister16(adresseI2C, 0xB2);
  ac6 = readRegister16(adresseI2C, 0xB4);
  b1  = readRegister16(adresseI2C, 0xB6);
  b2  = readRegister16(adresseI2C, 0xB8);
  mb  = readRegister16(adresseI2C, 0xBA);
  mc  = readRegister16(adresseI2C, 0xBC);
  md  = readRegister16(adresseI2C, 0xBE);
/*  
  Serial.println("Sensor calibration data :");
  Serial.print("AC1 = "); Serial.println(ac1);
  Serial.print("AC2 = "); Serial.println(ac2);
  Serial.print("AC3 = "); Serial.println(ac3);
  Serial.print("AC4 = "); Serial.println(ac4);
  Serial.print("AC5 = "); Serial.println(ac5);
  Serial.print("AC6 = "); Serial.println(ac6);
  Serial.print("B1 = ");  Serial.println(b1);
  Serial.print("B2 = ");  Serial.println(b2);
  Serial.print("MB = ");  Serial.println(mb);
  Serial.print("MC = ");  Serial.println(mc);
  Serial.print("MD = ");  Serial.println(md);
*/
}

//////////////////////////////////////////////////////////////////////////////////////////

uint16_t readRegister16(int adresseI2C, uint8_t code)
{
  uint16_t value = 0;
  Wire.beginTransmission(adresseI2C);         // Start transmission to device 
  Wire.write(code);                           // Sends register address to read from
  byte error = Wire.endTransmission();        // End transmission
  if (error == 0) {
    Wire.requestFrom(adresseI2C, 2);          // Request 2 bytes from device
    while (Wire.available() < 2);             // Wait until bytes are ready
    value = (Wire.read() << 8) + Wire.read();
  }
  return value;
}

//////////////////////////////////////////////////////////////////////////////////////////

byte barometreRead(int adresseI2C) // Voir le Data sheet du BMP180, à la page 15.
{
  int32_t x1, x2, x3, b3, b5, b6, ut, up, t, p;
  uint32_t b4, b7;
  int16_t msb, lsb, xlsb;
  byte error = 0;
  
  // Read uncompensated temperature value (ut)
  Wire.beginTransmission(adresseI2C);          // Start transmission to device
  Wire.write(0xf4);                            // Sends register address
  Wire.write(0x2e);                            // Write data
  error = Wire.endTransmission();              // End transmission 
  if (error == 0) { // On continue
    delay(5);                                  // Data sheet suggests 4.5 ms 
    
    Wire.beginTransmission(adresseI2C);        // Start transmission to device
    Wire.write(0xf6);                          // Sends register address to read from
    error = Wire.endTransmission();            // End transmission
    if (error == 0) { // On continue
      Wire.requestFrom(adresseI2C, 2);         // Request 2 bytes (0xf6, 0xf7)
      while (Wire.available() < 2);            // Wait until bytes are ready
      msb = Wire.read();
      lsb = Wire.read();
  
      ut = ((int32_t)msb << 8) + (int32_t)lsb;
  
      // Read uncompensated pressure value (up)
      Wire.beginTransmission(adresseI2C);      // Start transmission to device
      Wire.write(0xf4);                        // Sends register address
      Wire.write(0x34 + (OSS << 6));           // Write data
      error = Wire.endTransmission();          // End transmission
      if (error == 0) { // On continue
        delay(OSD);                            // Oversampling setting delay
  
        Wire.beginTransmission(adresseI2C);    // Start transmission to device
        Wire.write(0xf6);                      // Sends register address to read from
        error = Wire.endTransmission();        // End transmission
        if (error == 0) { // On continue
          Wire.requestFrom(adresseI2C, 3);     // Request 3 bytes (0xf6, 0xf7, 0xf8)
          while (Wire.available() < 3);        // Wait until bytes are ready
          msb = Wire.read();
          lsb = Wire.read();
          xlsb = Wire.read();
  
          up = (((int32_t)msb << 16) + ((int32_t)lsb << 8) + ((int32_t)xlsb)) >> (8 - OSS);
  
          // Calculate true temperature
          x1 = (ut - (int32_t)ac6) * (int32_t)ac5 >> 15;
          x2 = ((int32_t)mc << 11) / (x1 + (int32_t)md);
          b5 = x1 + x2;
          t = (b5 + 8) >> 4;
          temperature = t / 10.0f;  // temperature in celsius                         
  
          // Calculate true pressure
          // On étend la taille de certaines variables pour éviter des dépassements.
          // Par exemple, dans la 2ème ligne, x1 est int32_t, b2 est int16_t
          // et b6 est int_32t d'où le (int32_t)b2.
          // Pour gagner en vitesse de calcul, on utilise << ou >> :
          // << : un décalage de 1 bit vers la gauche revient à multiplier par 2
          // >> : un décalage de 1 bit vers la droite revient à diviser par 2
          b6 = b5 - 4000;          
          x1 = ((int32_t)b2 * (b6 * b6 >> 12)) >> 11;          
          x2 = (int32_t)ac2 * b6 >> 11;          
          x3 = x1 + x2;          
          b3 = ((((int32_t)ac1 * 4 + x3) << OSS) + 2) >> 2;          
          x1 = (int32_t)ac3 * b6 >> 13;          
          x2 = ((int32_t)b1 * (b6 * b6 >> 12)) >> 16;
          x3 = ((x1 + x2) + 2) >> 2;
          b4 = ((uint32_t)ac4 * (uint32_t)(x3 + 32768)) >> 15;
          b7 = ((uint32_t)up - (uint32_t)b3) * (uint32_t)(50000 >> OSS);
          if (b7 < 0x80000000) { p = (b7 << 1) / b4; }
          else { p = (b7 / b4) << 1; }
          x1 = (p >> 8) * (p >> 8);
          x1 = (x1 * 3038) >> 16;
          x2 = (-7357 * p) >> 16;
          p = p + ((x1 + x2 + 3791) >> 4);
          pressionAbsolue = p / 100.0f;  // pression in hPa
  
          // Calculate pressure at sea level
          pressionRelative = pressionAbsolue / pow((1.0f - (altitudeReference / 44330.0f)), 5.255f);
          
          // Calculate absolute altitude
          float pression0 = pressionReference / pow((1.0f - (altitudeReference / 44330.0f)), 5.255f);
          altitude = 44330.0f * (1 - pow(pressionAbsolue / pression0, (1 / 5.255f)));
        }
      }
    }
  }
  return error;
}

//////////////////////////////////////////////////////////////////////////////////////////
