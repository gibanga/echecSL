/*
Case.h - une librairie pour generer des case pour un jeu d'echec
Permet de gerer une variable de facon similaire a un INT, CHAR, FLOAT, etc.
Dans l'objet sont ecrit les regles qui regissent son fonctionnement
Ce qui est PUBLIC peut etre acceder dans le code et ce qui est PRIVATE peut seulement etre acceder par Case.h et Case.cpp

Cree par William Walsh, 5 mars 2024
Derniere mise a jour : 15 mai 2024
*/

#ifndef Case_h

#define Case_h
#define TAILLE 8 // Taille de la grille. 8x8 aux echecs

#include <Arduino.h>

// Stucture. Decrit un deplacement d'une piece
struct Move
{
  short fromRow;  // Rangee d'origine
  short fromCol;  // Colonne d'origne
  short toRow;    // Rangee destination
  short toCol;    // Colonne destination
};

// Objet. Contient toute l'information d'une case ainsi que ses interactions possibles
class Case
{
public:
  Case(char nom[2], char piece, short joueur, short rangee, short colonne, short led);
  Case();

  const char *getNom() const;
  int getLed();

  char getPiece();
  void setPiece(char piece);

  short getJoueur();
  String getCouleur();
  void setJoueur(short joueur);

  bool getABouger();
  void setABouger();

  bool getVulnerable();
  void setVulnerable(bool vulnerable);

  short getRangee();

  short getColonne();

  String readCase();
  bool isVide();

  int bougerPiece(Case echiquier[8][8], Move *tableauAction);
  bool inbounds(short rangee, short colonne);
  bool echec(Case echiquier[8][8], bool chercheMouvement);

private:
// Un _ devant une variable indique que celle-ci est propre a une instance de type Case
// La valeur de _led sera donc differente si on regarde la Case a1 ou la Case g5

  bool _aBouger = false;    // Indique si cette case a ete modifiee par le deplacement d'une piece
  bool _vulnerable = false; // Indique si cette case est vulnerable a une prise au passage
  char _nom[3];             // Indique le nom de cette case. a1, a2, ...
  char _piece;              // Indique quel piece est sur cette case
  short _joueur;            // Indique quel joueur a une piece sur cette case
  short _rangee;            // Indique la rangee de cette case
  short _colonne;           // Indique la colonne de cette case
  int _led;                 // Indique l'adresse de la DEL sous cette case

  

  int Pion(Case echiquier[8][8], Move *actionPossible);
  int Tour(Case echiquier[8][8], Move *actionPossible);
  int Cavalier(Case echiquier[8][8], Move *actionPossible);
  int Fou(Case echiquier[8][8], Move *actionPossible);
  int Reine(Case echiquier[8][8], Move *actionPossible);
  int Roi(Case echiquier[8][8], Move *actionPossible);

  char majuscule(char symbole);

  
};
#endif