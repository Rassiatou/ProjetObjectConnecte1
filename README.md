# 🏠 Projet Objet Connecté – ESP32

Ce projet consiste à concevoir un objet connecté basé sur un ESP32 permettant de contrôler un système d’éclairage intelligent et de surveiller des données en temps réel via une interface locale et une interface web.

---

## 🚀 Objectifs du projet

- Utiliser un microcontrôleur ESP32
- Lire un capteur analogique (potentiomètre)
- Contrôler une LED avec PWM
- Afficher des informations sur un écran OLED
- Interagir via un clavier matriciel
- Mettre en place un serveur web embarqué

---

## ⚙️ Fonctionnalités principales

- 🔆 Contrôle de la LED (ON / OFF / AUTO)
- 🎛️ Variation de la luminosité via potentiomètre
- 📺 Affichage dynamique sur écran OLED
- 🎮 Contrôle local avec clavier matriciel
- 🌐 Interface web interactive avec boutons
- 🔄 Rafraîchissement automatique des données

---

## 🧰 Technologies utilisées

- ESP32 (microcontrôleur)
- C++ (Arduino / PlatformIO)
- Wi-Fi (serveur HTTP)
- PWM (contrôle LED)
- I2C (écran OLED)
- HTML / CSS (interface web)
- Clavier matriciel (entrée utilisateur)

---

## 🔌 Matériel utilisé

- ESP32
- LED + résistance
- Potentiomètre
- Écran OLED SSD1306
- Clavier matriciel 4x4
- Fils de connexion

---

## 🔧 Connexions

### OLED (I2C)
- VCC → 3.3V  
- GND → GND  
- SDA → GPIO 21  
- SCL → GPIO 22  

### LED
- GPIO 2 → LED → résistance → GND  

### Potentiomètre
- 3.3V → côté gauche  
- GND → côté droit  
- GPIO 34 → milieu  

### Clavier matriciel
- C4 → GPIO 13  
- C3 → GPIO 12  
- C2 → GPIO 14  
- C1 → GPIO 27  
- R1 → GPIO 26  
- R2 → GPIO 25  
- R3 → GPIO 33  
- R4 → GPIO 32  

---

## 🌐 Interface Web

Accessible via l’adresse IP de l’ESP32 :
http://172.20.10.9


### Fonctionnalités web :
- Affichage des données en temps réel
- Boutons interactifs :
  - LED ON
  - LED OFF
  - MODE AUTO
  - CHANGER ÉCRAN OLED

---

## 🎮 Commandes

### Clavier

| Touche | Fonction |
|------|--------|
| 1 | Allumer la LED |
| 2 | Éteindre la LED |
| 3 | Mode automatique |
| A | Changer écran OLED |

---

## 📺 Écran OLED

### Page 1 : Informations
- État Wi-Fi
- Valeur du potentiomètre
- Luminosité LED
- Mode (AUTO / MANUEL)

### Page 2 : Commandes
- Instructions pour l’utilisateur

---

## 🧠 Fonctionnement du système

Le programme fonctionne en boucle continue :

1. Lecture du clavier
2. Lecture du potentiomètre
3. Calcul de la luminosité
4. Mise à jour de la LED (PWM)
5. Affichage OLED
6. Gestion du serveur web

---

## 🔗 Routes API

| Route | Description |
|------|------------|
| /led/on | Allumer la LED |
| /led/off | Éteindre la LED |
| /mode/auto | Activer mode automatique |
| /oled/change | Changer l’écran OLED |

---

## 💡 Concepts utilisés

- PWM : contrôle de luminosité
- I2C : communication avec l’OLED
- HTTP : serveur web embarqué
- Clavier matriciel : gestion des entrées utilisateur

---

## 📈 Améliorations possibles

- Ajouter un slider web pour la luminosité
- Ajouter un capteur réel (température, humidité)
- Ajouter une application mobile
- Stockage des données

---

## 👨‍💻 Auteur

- Woundé
- Rassiatou Coulibaly
- Lucresse Pascale
- Daryl Saah
- Leonel NKam
- Alhousseini 

---

## 📜 Licence

Projet académique – usage éducatif
