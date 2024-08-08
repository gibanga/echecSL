# Description
Cette librairie permet de controler le comportement des cases sur un jeu d'échecs

## Téléchargement
Déposer dans le dossier de librairie globale de l'IDE Arduino

## Utilisation
Exemple d'utilisation des fonctions publiques. Les exemples se suivent.
- Case(...) &emsp; Constructeur : Permet la création d'une variable de type Case. Si il n'y a rien dans la parenthèse, une case par défaut est créée.
```C
// Case(char nom[2], char piece, short joueur, short rangee, short colonne, short led);
Case echec = Case("a2", 'P', 1, 1, 0, 14); 
```
- get...()&emsp; Getter : Permet d'aller chercher de l'information dans les variables de type Case
```C
echec.getLED(); // retourne 14
```
- set...()&emsp;Setter : Permet d'écrire de l'information dans les variables de type Case
```C
echec.setName("a3"); //Le nom passe de a2 à a3
```
- readCase()&emsp;Permet de lire toutes les informations enregistrées dans les variables de type Case
```C
echec.readCase(); // retourne "a3 P blanc 2 1;
```
- isVide()&emsp;Vérifie si une pièce est sur une case
```C
echec.isVide(); // retourne false, car un pion blanc est sur la case
```
- bougerPiece()&emsp;Vérifie quels déplacements une pièce peut faire  
```C
echec.bougerPiece(); // retourne la case en (2,0) et en (3,0), car c'est un pion sur sa case de départ
```
