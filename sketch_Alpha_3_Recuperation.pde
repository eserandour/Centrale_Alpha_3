//////////////////////////////////////////////////////////////////////////////////////////
/*
   La valise : Récupération des données brutes (version 2019.05.12)
   Copyright 2013, 2014, 2015, 2016, 2019 - Eric Sérandour
   http://3615.entropie.org
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
//////////////////////////////////////////////////////////////////////////////////////////
/*
  Compile sous Processing 3.5.3
*/
//////////////////////////////////////////////////////////////////////////////////////////

int width = 500;                        // Largeur de la fenêtre
int height = 250;                       // Hauteur de la fenêtre
int xmax = width-1;
int ymax = height-1;

import processing.serial.*;             // Charge la bibliothèque serial.

Serial myPort;                          // Création de l'objet myPort (classe Serial).
int baudrate = 9600;                    // Vitesse de transfert des données (en bauds).
int valPort = 0;                        // Données reçues depuis le port série.

String buffer = "";                     // Un petit tampon pour récupérer la dernière valeur  
                                        // mesurée sous la forme d'une chaine de caractères.
String ligne = "";

PrintWriter output;                     // Création d'un objet PrintWriter
int finFichier = 0;                     // Teste si on a lu tout le fichier

//////////////////////////////////////////////////////////////////////////////////////////

void settings() {
  size(width, height);
}

//////////////////////////////////////////////////////////////////////////////////////////

void setup() 
{ 
  println("Ports séries disponibles :");
  println(Serial.list());
  // Sur mon ordinateur, sous Debian, la carte Arduino est connectée au port 
  // /dev/ttyACM0, le premier dans la liste, d'où le 0 dans Serial.list()[0].  
  String portName = Serial.list()[0];
  myPort = new Serial(this, portName, baudrate);
  
  // Créé un fichier données.csv dans le répertoire sketch (lisible par tout tableur).
  // Le séparateur de données est le point virgule.
  output = createWriter("données.csv");
}

//////////////////////////////////////////////////////////////////////////////////////////

void draw()
{
  background(0);  // Fond noir
  stroke(255);    // Texte en blanc
  while (myPort.available() > 0) {
    // Pour transmettre la valeur mesurée codée sur 10 bits (2^10 = 1024),
    // soit un nombre compris entre 0 et 1023,
    // valPort prend successivement des valeurs entre 48 et 57,
    // ce qui correspond en code ASCII aux caractères 0 à 9.
    // Quand la valeur à transmettre (0 à 1023) l'est, valPort prend les valeurs 
    // 13 (retour chariot en code ASCII), puis 10 (saut à la ligne en code ASCII).
    valPort = myPort.read();
    serialEvent(valPort);
    finFichier = millis();
  }
  if ((valPort == 10) && (millis()-finFichier > 3000)) { // 3000 ms pour être bien sûr
    fin();                                      // que l'on a atteint la fin du fichier.
  }
  
  textAlign(CENTER, CENTER);  
  textFont(loadFont("DejaVuSans-24.vlw"), 24);
  if (ligne=="") {
    text("Connexion en cours...", width/2, height/2 -12);      
  }
  else {
    text("Transfert en cours...", width/2, height/2 -12);
  }
  textFont(loadFont("DejaVuSans-12.vlw"), 12);
  text("appuyez sur Echap pour interrompre ou finir.", width/2, height/2 +24);
}

//////////////////////////////////////////////////////////////////////////////////////////

void serialEvent(int serial)  // Méthode de la classe Serial.
{
  if (serial != 10) {         // 10 équivaut à un saut à la ligne en code ASCII.
    buffer += char(serial);   // Store all the characters on the line.
    } 
  else {
    // The end of each line is marked by two characters, a carriage return (13) 
    // and a newline (10). We're here because we've gotten a newline,
    // but we still need to strip off the carriage return.
    buffer = buffer.substring(0, buffer.length()-1);
    ligne = buffer;
    output.println(ligne);
    println(ligne);
    // Clear the value of "buffer"  
    buffer = "";
  }
}

//////////////////////////////////////////////////////////////////////////////////////////

void fin() {
  output.flush();  // Writes the remaining data to the file
  output.close();  // Finishes the file
  exit();          // Stops the program
}

//////////////////////////////////////////////////////////////////////////////////////////

void keyPressed() {
  if (key == ESC) {
    fin();
  }
}

//////////////////////////////////////////////////////////////////////////////////////////
