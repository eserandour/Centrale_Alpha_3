//////////////////////////////////////////////////////////////////////////////////////////
/*
    ACCELEROMETRE 2D ADXL322
*/
//////////////////////////////////////////////////////////////////////////////////////////

void afficherAccelerometre(int adresseEA)
{
  // Leds de couleur
  digitalWrite(LED_JAUNE, HIGH); // LED jaune allumée
  digitalWrite(LED_VERTE, LOW);  // LED verte éteinte     
  
  // Création de caractères personnalisés
  lcd.createChar(caractereCarre, carre);
  lcd.createChar(caractereFlecheDroite, flecheDroite);
  lcd.createChar(caractereFlecheHaut, flecheHaut);
  lcd.createChar(caractereRondPointe, rondPointe);
  lcd.createChar(caractereDegre, degre);  
  // Affichage des mesures
  boolean quitter = false;
  do {
    lcd.clear();
    accelerometreRead(adresseEA);    
    lcd.setCursor(0,0);
    lcd.print("ACCELEROMETRE");
    // Axe X
    lcd.setCursor(0,1);
    lcd.print("X ");
    lcd.write(caractereFlecheDroite);
    lcd.print(" : ");
    dtostrf(accelerationX, 5, 1, ligne);
    lcd.print(ligne);
    lcd.print(" m/s");
    lcd.write(caractereCarre);
    // Axe Y
    lcd.setCursor(0,2);
    lcd.print("Y ");
    lcd.write(caractereFlecheHaut);
    lcd.print(" : ");
    dtostrf(accelerationY, 5, 1, ligne);
    lcd.print(ligne);
    lcd.print(" m/s");
    lcd.write(caractereCarre);
  
    // Axe Z (non implémenté : accéléromètre 2D)
    lcd.setCursor(0,3);
    lcd.print("Z ");
    lcd.write(caractereRondPointe);
    lcd.print(" : ");
    /*dtostrf(accelerationZ, 5, 1, ligne);
    lcd.print(ligne);
    lcd.print(" m/s");
    lcd.write(caractereCarre);*/
    
/*   
    // Pour tester le capteur
    lcd.setCursor(16,0);
    dtostrf(xRead, 4, 0, ligne);
    lcd.print(ligne);
    lcd.setCursor(16,1);
    dtostrf(yRead, 4, 0, ligne);
    lcd.print(ligne);  
    lcd.setCursor(16,2);
    dtostrf(angleX, 3, 0, ligne);
    lcd.print(ligne);
    lcd.write(caractereDegre);
    lcd.setCursor(16,3);
    dtostrf(angleY, 3, 0, ligne);
    lcd.print(ligne);
    lcd.write(caractereDegre);  
*/    
    for (int i=0; i<100; i++) {
      delay(1);
      char key = keypad.getKey();
      if (key == '0') {
        quitter = true;
        break;  
      }        
    }
  } while (quitter == false);   
  selectMenu(TAG_MENU_PRINCIPAL);
}

//////////////////////////////////////////////////////////////////////////////////////////

void accelerometreRead(int adresseEA)
{
  // Constantes obtenues expérimentalement à partir de l'étude de mon capteur ADXL322
  // Voir http://serandour.com/etude-d-un-accelerometre-2d.htm  
  const float X0 = 515.16;            // pour 0g
  const float Y0 = 502.84;            // Pour 0g
  const float SENSIBILITE_X = 723.42; // Donnée constructeur : 750 mV/g +/-5%
  const float SENSIBILITE_Y = 721.57; // Donnée constructeur : 750 mV/g +/-5%

  #define NBREADINGS 5
  const float G = 9.81;
  
  // accelerationX
  long total = 0;
  for(int i=0; i<NBREADINGS; i++) {
    total = total + analogRead(adresseEA);
    delay(2); // 2 ms car l'accéléromètre ADXL322 est à 500 Hz.
  }
  xRead = float(total) / NBREADINGS; // On calcule la moyenne
  accelerationX = (5000.0f/SENSIBILITE_X) * (xRead-X0)/1023.0f * G;
  accelerationX = - accelerationX; // A cause du positionnement du capteur dans le boitier
  
  // accelerationY    
  total = 0;
  for(int i=0; i<NBREADINGS; i++) {
    total = total + analogRead(adresseEA+1);
    delay(2); // 2 ms car l'accéléromètre ADXL322 est à 500 Hz.
  }
  yRead = float(total) / NBREADINGS; // On calcule la moyenne
  accelerationY = (5000.0f/SENSIBILITE_Y) * (yRead-Y0)/1023.0f * G;
  
  // Pour calculer l'inclinaison, on peut utiliser les fonctions qui suivent
  // mais les mesures s'écartent de la réalité dès que les angles dépassent 78°
  angleX = int((180.0f / PI) * asin(accelerationX / G));
  angleY = int((180.0f / PI) * asin(accelerationY / G));
}

//////////////////////////////////////////////////////////////////////////////////////////
