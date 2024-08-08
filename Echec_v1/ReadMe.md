# Installation
Le code est destiné pour un ESP-32 et utilise l'IDE Arduino pour le téléversement. <br />
Il faut que le fichier Echec_v1.ino soit dans un dossier nommé Echec_v1 pour le téléverser. 

## Librairies
Pour opérer, le projet nécessite quelques librairies. Il est recommandé d'importer ces librairies dans le fichier globale de Arduino

- Case                    Pour gérer les actions 
- Adafruit_NeoPixel       Controle les DEL addressables de la série NeoPixel d'Adafruit  
- Adafruit_SSD1306        Controle l'écran OLED monochrome d'Adafruit utilisé sur le projet [https://www.adafruit.com/product/326]
- Adafruit_GFX_librairy   Dépendance de Adafruit_SSD1306
- Adafruit_BusIO          Dépendance de Adafruit_GFX_librair 

- Optionnel : arduino-timer  Cette librairie est non-compatible avec l'ESP32, mais permet de substituer le coeur 1 pour un arduino

## Fichier
Le fichier _Definition.h_ définie les branchements entre les éléments du circuit et l'ESP32. <br />
Il prend aussi en note les constantes liées à la partie physique du jeu.
