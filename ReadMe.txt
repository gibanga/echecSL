Avant de téléverser echec.ino dans un ESP32, veuillez avoir toutes les librairies à importer dans les libraires globales.

Pour compatibilité avec un Arduino, une librairie qui créer des interruptions basés sur le temps (ex: arduino-timer)
peut être employé pour créer des tâches comme on assigne aux coeurs du ESP32

Le loop est sur le coeur 0 du ESP32
La fonction print() sur l'ESP32 ne supporte que 32 bits a à la fois


Problèmes à régler:
-Le roi peut se déplacer sur une case qui le metterait en échec
-Le grand roque et le petit roque arrètent avant de permutter la tour
-La prise au passage ne fonctionne pas
-Le choix de promotion est presentement désactivée

Ajout possible:
-L'écran pourrait afficher la dernière action jouée
-Autres possibilités

Librairies externes:
-Adafruit_BusIO
-Adafruit_GFX_librairy
-Adafruit_NeoPixel
-Adafruit_SSD1306
-BluetoothSerial

