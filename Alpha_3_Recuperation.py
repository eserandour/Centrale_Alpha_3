# -*- coding: utf-8 -*-
"""
########################################################################
#
#  La Valise / Centrale Alpha 3 :
#  Récupération des données brutes (version 2019.06.12)
#
#  Copyright 2019 - Eric Sérandour
#  http://3615.entropie.org
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License as
#  published by the Free Software Foundation; either version 3 of
#  the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public
#  License along with this program; if not, write to the Free
#  Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
#  Boston, MA 02110-1301, USA.
#
########################################################################
"""
#!/usr/bin/env python

import serial
import time
import csv
import matplotlib.pyplot as plt  # Pour faire des graphiques

########################################################################

# Indiquer le port sélectionné dans le menu Arduino (Outils >  Port) :
# Sous Linux : /dev/ttyACM suivi d'un numéro (0,1,...)
# Sous Windows : COM suivi d'un numéro (1,2,...)
PORT = "/dev/ttyACM0"                                                   # A modifier éventuellement
VITESSE = 9600  # Vitesse en bauds                                      # A modifier éventuellement
serialPort = serial.Serial(port = PORT, baudrate = VITESSE)

# Réinitialisation du microcontrôleur via la broche DTR
serialPort.setDTR(False)
time.sleep(0.1)
serialPort.setDTR(True)

# On vide le tampon (buffer)
serialPort.flushInput()

########################################################################
#  RECUPERATION DES DONNEES BRUTES DEPUIS LE PORT SERIE => FICHIER CSV
########################################################################

print("Données brutes :")
print()

# Lecture des données puis écriture dans le fichier CSV
FICHIER = "données.csv"
file = open(FICHIER, "wb")  # Ecriture en mode binaire
finFichier = False
while not finFichier:
    ligne = serialPort.readline()
    print(ligne)
    if ligne == b'\r\n':
        finFichier = True
    else:
        file.write(ligne)
file.close();

# Fermeture du port série
serialPort.close()

print()

########################################################################
#  EXTRACTION DES DONNEES DU FICHIER CSV
########################################################################

def readColCSV(numCol):
    """Lit une colonne du fichier CSV (la numérotation commence à 0)"""
    file = open(FICHIER, "r")
    reader = csv.reader(file, delimiter =";")
    colonne = []  # Création de la liste "colonne" (vide)
    for row in reader:  # On balaye toutes les lignes du fichier CSV
        try:
            # Remplissage de la liste "colonne" avec des entiers
            colonne.append(int(row[numCol]))
        except:
            pass
    file.close()
    return colonne

########################################################################

print("Données extraites :")
print()

x = readColCSV(0)  # Colonne 0 du fichier CSV                           # A modifier éventuellement
y = readColCSV(2)  # Colonne 2 du fichier CSV                           # A modifier éventuellement

print("Abscisses :")
print(x)
print("Ordonnées :")
print(y)
print()

########################################################################
#  AFFICHAGE DU GRAPHIQUE
########################################################################

plt.plot(x,y)
plt.xlabel("Abscisses")                                                 # A modifier éventuellement
plt.ylabel("Ordonnées")                                                 # A modifier éventuellement
plt.title("Titre")                                                      # A modifier éventuellement
plt.savefig("graphique.png")  # Sauvegarde du graphique
plt.show()

########################################################################
