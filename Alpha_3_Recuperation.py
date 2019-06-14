# -*- coding: utf-8 -*-
"""
########################################################################
#
#  La Valise / Centrale Alpha 3 :
#  Récupération des données brutes (version 2019.06.14d)
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
import numpy
import scipy.optimize  # Routines d'optimisation
import scipy.fftpack  # Routines de FFT

########################################################################






########################################################################
#  FONCTION DE RECUPERATION DES DONNEES DU PORT SERIE => FICHIER CSV
########################################################################

def enregistrerDonnees(nomPort, vitessePort, nomFichier):
    """Enregistrement des données dans un fichier CSV"""
    # Ouverture du port série
    serialPort = serial.Serial(port = nomPort, baudrate = vitessePort)
    # Réinitialisation du microcontrôleur via la broche DTR
    serialPort.setDTR(False)
    time.sleep(0.1)
    serialPort.setDTR(True)
    # On vide le tampon (buffer)
    serialPort.flushInput()
    # -----------------------------------------------------------------
    # Enregistrement dans le fichier CSV
    file = open(nomFichier, "wb")  # Ecriture en mode binaire
    finFichier = False
    data = False
    while not finFichier:
        # Lecture du port série
        ligne = serialPort.readline()
        # Affichage des données en provenance d'Arduino dans la console
        if data == False:
            print("Données brutes :")
            data = True
        print(ligne)
        if ligne == b'\r\n':
            finFichier = True
        else:
            # Ecriture dans le fichier CSV
            file.write(ligne)
    file.close();
    # -----------------------------------------------------------------
    # Fermeture du port série
    serialPort.close()

########################################################################






########################################################################
#  FONCTION D'EXTRACTION DES DONNEES (NOMBRES) DEPUIS LE .CSV
########################################################################

def readColCSV(nomFichier, numCol):
    """Lit une colonne du fichier CSV (la numérotation commence à 0)"""
    file = open(nomFichier, "r")
    reader = csv.reader(file, delimiter =";")
    colonne = []  # Création de la liste "colonne" (vide)
    for row in reader:  # On balaye toutes les lignes du fichier CSV
        try:
            # On remplace les virgules éventuelles par des points 
            notationPoint = row[numCol].replace(",", ".")
            # Remplissage de la liste "colonne" avec des réels
            colonne.append(float(notationPoint))
        except:
            pass
    file.close()
    return colonne

########################################################################

def extraireDonnees(nomFichier, colX, colY):
    """Extraction des données depuis le fichier CSV"""
    listeX = readColCSV(nomFichier, colX)  # Colonne choisie pour x
    listeY = readColCSV(nomFichier, colY)  # Colonne choisie pour y
    x = numpy.array(listeX)    # Liste => Tableau
    y = numpy.array(listeY)    # Liste => Tableau
    return numpy.array([x,y])

########################################################################






########################################################################
#  AFFICHAGE DES DONNEES EXTRAITES ET TRAITEES
########################################################################

def afficherDonnees(message, x, y):
    """Affichage des données"""
    print(message)
    print("Abscisses :", x)
    print("Ordonnées :", y)
    print("---------------------------------------------------------")

########################################################################






########################################################################
#  DEFINITIONS DES FONCTIONS POUR LA REGRESSION
########################################################################
"""
Quelques fonctions mathématiques :
x**n               : puissance
numpy.sqrt(x)      : racine carré
numpy.exp(x)       : exponentielle
numpy.log(x)       : logarithme népérien
numpy.log10(x)     : logarithme décimal
numpy.sin(x)       : sinus
numpy.cos(x)       : cosinus
numpy.tan(x)       : tangente
numpy.arcsin(x)    : arcsinus
numpy.arccos(x)    : arccosinus
numpy.arctan(x)    : arctangente
"""

def lineaire(x, a, b):
    # Linéaire : y = a.x + b
    return a*x + b

def quadratique(x, a, b, c):
    # Quadratique : y = a.x^2 + b.x + c
    return a*x**2 + b*x + c

def cubique(x, a, b, c, d):
    # Cubique : y = a.x^3 + b.x^2 + c.x + d
    return a*x**3 + b*x**2 + c*x + d

def quartique(x, a, b, c, d, e):
    # Quartique : y = a.x^4 + b.x^3 + c.x^2 + d.x + e
    return a*x**4 + b*x**3 + c*x**2 + d*x + e

def exponentielle(x, a, b):
    # Exponentielle : y = a.e^(b.x)
    return a*numpy.exp(b*x)

def logarithmique(x, a, b):
    # Logarithmique : y = a.ln(x) + b
    return a*numpy.log(x) + b

def puissance(x, a, b):
    # Puissance : y = a.x^b
    return a*x**b

def trigonometrique(x, a, b, c, d):
    # Trigonométrique : y = a.sin(b.x + c) + d
    return a*numpy.sin(b*x + c) + d

########################################################################






########################################################################
#  CHOIX DE LA REGRESSION
########################################################################

def choixRegression(choix):
    """Choix de la régression"""
    if choix == 1:
        regressionChoisie = lineaire
        print("Régression linéaire : y = a.x + b")
    elif choix == 2:
        regressionChoisie = quadratique
        print("Régression quadratique : y = a.x^2 + b.x + c")
    elif choix == 3:
        regressionChoisie = cubique
        print("Régression cubique : y = a.x^3 + b.x^2 + c.x + d")
    elif choix == 4:
        regressionChoisie = quartique
        print("Régression quartique : y = a.x^4 + b.x^3 + c.x^2 + d.x + e")
    elif choix == 5:
        regressionChoisie = exponentielle
        print("Régression exponentielle : y = a.e^(b.x)")
    elif choix == 6:
        regressionChoisie = logarithmique
        print("Régression logarithmique : y = a.ln(x) + b")
    elif choix == 7:
        regressionChoisie = puissance
        print("Régression puissance : y = a.x^b")
    elif choix == 8:
        regressionChoisie = trigonometrique
        print("Régression trigonométrique : y = a.sin(b.x + c) + d")
    return regressionChoisie

########################################################################






########################################################################
#  REGRESSION D'UNE FONCTION
########################################################################

def regressionFonction(x, y, regression):
    """Régression d'une fonction"""
    # Régression
    if regression == trigonometrique:
        # Transformée de Fourier rapide (FFT)
        FFT = abs(scipy.fft(y)) / (y.size / 2)  # Amplitudes
        freqs = scipy.fftpack.fftfreq(y.size, x[1]-x[0])  # Fréquences
        FFT = FFT[0:len(FFT)//2]  # On supprime les fréquences négatives
        freqs = freqs[0:len(freqs)//2]  # Idem
        freqPicMax = freqs[numpy.argmax(FFT[1:])+1]  # 1: et +1 => Exclusion du pic à 0 Hz qui correspond à un d non nul
        # Valeurs d'initialisation
        a = numpy.std(y)*2**0.5  # Ecart type * racine carré de 2
        b = 2 * numpy.pi * freqPicMax
        c = 0
        d = numpy.mean(y)  # Moyenne de l'échantillon
        p0 = numpy.array([a, b, c, d])
        coefReg, pcov = scipy.optimize.curve_fit(regression, x, y, p0)
    else:
        coefReg, pcov = scipy.optimize.curve_fit(regression, x, y)
    # Coordonnées de points de la fonction de régression
    NB_POINTS = 1000
    xReg = numpy.linspace(min(x), max(x), NB_POINTS)
    yReg = regression(xReg, *coefReg)
    return numpy.array([coefReg, xReg, yReg])
    
########################################################################

def afficheCoefReg(nbCoef, regression):
    for i in range(nbCoef):
        print(chr(97+i), "=", coefReg[i])

########################################################################









"""
########################################################################
#  DEBUT DU PROGRAMME
########################################################################
"""






########################################################################
#  TRAITEMENT DES DONNEES BRUTES
########################################################################

# Enregistrement des données Arduino dans un fichier CSV
print ("En attente de données...")
print ()
# Indiquer le port sélectionné dans le menu Arduino (Outils >  Port) :
# Sous Linux : /dev/ttyACM suivi d'un numéro (0,1,...)
# Sous Windows : COM suivi d'un numéro (1,2,...)
PORT = "/dev/ttyACM0"                                                   # A modifier éventuellement
VITESSE = 9600  # Vitesse en bauds                                      
FICHIER_CSV = "données.csv"                                             # A modifier éventuellement
enregistrerDonnees(PORT, VITESSE, FICHIER_CSV)
print("---------------------------------------------------------")

# Extraction du fichier CSV
COLONNE_X = 0                                                           # A modifier éventuellement
COLONNE_Y = 1                                                           # A modifier éventuellement
x, y = extraireDonnees(FICHIER_CSV, COLONNE_X, COLONNE_Y)
"""afficherDonnees("Données extraites :", x, y)"""

# Conversion des données
# x : Temps en secondes
# Pour une temporisation de 100 ms, choisir 0.1 s
temporisation = 0.1  # en s, min, ou h                                  # A modifier éventuellement
x = x * temporisation
# y : Tension en volts
#y = 5.0 * y / 1023                                                      # A modifier éventuellement
"""afficherDonnees("Données converties :", x, y)"""

########################################################################






########################################################################
#  ANALYSE DES DONNEES : REGRESSION
########################################################################
"""
Choix du type de régression (définies plus haut) :
    1 : lineaire : y = a.x + b
    2 : quadratique : y = a.x^2 + b.x + c
    3 : cubique : y = a.x^3 + b.x^2 + c.x + d
    4 : quartique : y = a.x^4 + b.x^3 + c.x^2 + d.x + e
    5 : exponentielle : y = a.e^(b.x)
    6 : logarithmique : y = a.ln(x) + b
    7 : puissance : y = a.x^b
    8 : trigonometrique : y = a.sin(b.x + c) + d
"""
choix = 1                                                               # A modifier éventuellement
 
regressionChoisie = choixRegression(choix)
coefReg, xReg, yReg = regressionFonction(x, y, regressionChoisie)
afficheCoefReg(numpy.size(coefReg), regressionChoisie)
print("---------------------------------------------------------")

# Calcul de la période et de la fréquence pour une régression trigonométrique
# Attention : La base de temps doit être en secondes !
if regressionChoisie == trigonometrique:
    periode = (2 * numpy.pi) / coefReg[1]
    frequence = coefReg[1] / (2 * numpy.pi)
    print("Période   =", periode, "s")
    print("Fréquence =", frequence, "Hz")
    print("---------------------------------------------------------")

########################################################################






########################################################################
#  AUTRES CALCULS
########################################################################

"""
Commentaires
"""

########################################################################






########################################################################
#  AFFICHAGE DU GRAPHIQUE
########################################################################

plt.title("")                                                           # A modifier (Titre)
plt.xlabel("Temps (s)")                                                 # A modifier éventuellement (Abscisses)
plt.ylabel("Tension (V)")                                               # A modifier éventuellement (Ordonnées)

#plt.plot(x, y, ".r")  # Les points ne sont pas reliés (r : rouge)
plt.plot(x,y)          # Les points sont reliés
plt.plot(xReg,yReg)    # Courbe de régression

plt.grid(True)         # Grille
plt.savefig("graphique.png")  # Sauvegarde du graphique au format PNG
plt.show()

########################################################################
