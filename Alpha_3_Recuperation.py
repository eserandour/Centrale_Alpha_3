# -*- coding: utf-8 -*-
"""
########################################################################
#
#  La Valise / Centrale Alpha 3 :
#  Récupération des données brutes (version 2019.06.11)
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

########################################################################

# Indiquer le port sélectionné dans le menu Arduino (Outils >  Port) :
# Sous Linux : /dev/ttyACM suivi d'un numéro (0,1,...)
# Sous Windows : COM suivi d'un numéro (1,2,...)
PORT = "/dev/ttyACM0"
VITESSE = 9600  # Vitesse en bauds
serialPort = serial.Serial(port = PORT, baudrate = VITESSE)

# Réinitialisation du microcontrôleur via la broche DTR
serialPort.setDTR(False)
time.sleep(0.1)
serialPort.setDTR(True)

# On vide le tampon (buffer)
serialPort.flushInput()

########################################################################

FICHIER = "données.csv"
file = open(FICHIER, "wb")  # Ecriture en mode binaire

########################################################################

# Lecture puis écriture des données
finFichier = False
while not finFichier:
    ligne = serialPort.readline()
    print(ligne)
    if ligne == b'\r\n':
        finFichier = True
    else:
        file.write(ligne)

# Fermeture du port série
serialPort.close()

########################################################################
