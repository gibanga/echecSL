/* 
  Jeu d'echec
  
  Jeu d'echec utilisant des senseurs dans les cases pour detecter la presence d'une piece sur sa surface
  Ces senseurs sont relier a des multiplexeurs pour gerer leur lecture
  Des DEL sous la surface de jeu permettent d'indiquer la couleur des cases ainsi que des actions 
  ou des erreurs par les joueurs
  Un ecran et deux boutons permettent de controler quelle piece deviendra un pion apres une promotion

  Cree par William Walsh, 5 mars 2024
  Derniere mise a jour : 17 mai 2024
*/

#include "Definition.h"
// Ecran
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// DEL
#include <Adafruit_NeoPixel.h>
// Sur mesure
#include <Case.h>

// Representation hexadecimale des 16 premieres et 16 dernieres cases
// d'un tableau d'echec activees
#define GAMESTART 0xFFFF00000000FFFF

#define SCREEN_WIDTH 128    // Largeur de l'ecran OLED en pixels
#define SCREEN_HEIGHT 64    // Hauteur de l'ecran OLED en pixels
#define OLED_RESET 0        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // Adresse de l'ecran. Verifiez dans la datasheet pour la bonne adresse si changee
#define TESTREEL true       // Active le mode reel sur un 'true' ou le mode test sur un 'false'
#define DEBUG false         // Active les commentaire de debugage

// Creation d'une instance pour un ecran
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
const int epd_bitmap_allArray_LEN = 1;
const unsigned char *epd_bitmap_allArray[1] = {
    epd_bitmap_logo};

// Declaration de quelques fonctions. Voir sous 'Loop()' pour leur fonctionnement
void initialiseGrille(Case (&echiquier)[8][8]);
uint64_t virtuelleToBits(Case (&echiquier)[8][8]);
void afficheTableauPiece(Case (&echiquier)[8][8]);

// Initialisation de quelques varibles globales
Adafruit_NeoPixel ledStrip(LEDCOUNT, LED, NEO_GRB + NEO_KHZ800); // Initialisation des DEL adressables. (nombre de DEL, broche IO, type de DEL)
Move actionPossible[64];                                         // Liste des deplacements possibles que peut prendre une piece. Chaque position est un deplacement unique
Case echiquier[TAILLE][TAILLE];                                  // Matrice des cases du jeu d'echec
uint64_t tableau = 0;                                            // Etat actuelle du tableau
const char promotion[] = {'R', 'N', 'B', 'Q'};                   // Liste des pieces disponibles possibles lors de la promotion d'un pion

TaskHandle_t Task0;                                             // Creer une tache qui pourra etre executer par un coeur du ESP32
static portMUX_TYPE my_spinlock = portMUX_INITIALIZER_UNLOCKED; // Empeche que deux coeurs accedent a une meme variable en meme temps

#if TESTREEL
// Passe par les 16 canaux des 4 multiplexeurs pour lire les 64 case du tableau de jeu
void LectureTableau(void *pvParameters)
{
  uint64_t lecture; // Store la derniere lecture du tableau sous la forme de 64 bits
  uint64_t courant; // Garde en memoire le dernier etat stable du tableau sous le forme de 64 bits
  while (true)
  {
#if 0 
     Serial.println("Lecture commence");
#endif

    lecture = 0;
    for (int i = 0; i < 64; i++)
    {
      int multiplexeur = i / 16; // Choix du multiplexeur
      int canal = i % 16;        // Choix du canal des multiplexeurs

      digitalWrite(MUX_S0, bitRead(canal, 0)); // Lit le canal du multiplexeur sous forme binaire
      digitalWrite(MUX_S1, bitRead(canal, 1)); // Ex: 11 en decimal equivaut a 1011 en binaire
      digitalWrite(MUX_S2, bitRead(canal, 2)); // Chaque entre des mux se voit ecrire le premier, deuxieme
      digitalWrite(MUX_S3, bitRead(canal, 3)); // troisieme et quatrieme bit de 'canal' respectivement

      // Active le multiplexeur actif et desactive les autres. Actif sur un LOW
      switch (multiplexeur)
      {
      case 0:
        digitalWrite(MUX1_E, LOW);
        digitalWrite(MUX2_E, HIGH);
        digitalWrite(MUX3_E, HIGH);
        digitalWrite(MUX4_E, HIGH);
        break;
      case 1:
        digitalWrite(MUX1_E, HIGH);
        digitalWrite(MUX2_E, LOW);
        digitalWrite(MUX3_E, HIGH);
        digitalWrite(MUX4_E, HIGH);
        break;
      case 2:
        digitalWrite(MUX1_E, HIGH);
        digitalWrite(MUX2_E, HIGH);
        digitalWrite(MUX3_E, LOW);
        digitalWrite(MUX4_E, HIGH);
        break;
      case 3:
        digitalWrite(MUX1_E, HIGH);
        digitalWrite(MUX2_E, HIGH);
        digitalWrite(MUX3_E, HIGH);
        digitalWrite(MUX4_E, LOW);
        break;
      }

      lecture = lecture << 1; // Pousse les bits de lecture de 1 vers la gauche
      delay(5);               // Delai pour laisser le temps aux multiplexeurs de se stabiliser. Voir Datasheet
      lecture += digitalRead(RS_DATA);
    }

    // Si l'etat du jeu change, on le met a jour
    if (courant != lecture)
    {
      courant = lecture;
      // On donne la permission exclusive au coeur pour la lecture et l'ecrire
      // dans la memoire. Il faut garder se bloque tres court
      taskENTER_CRITICAL(&my_spinlock);
      tableau = lecture;
      taskEXIT_CRITICAL(&my_spinlock);

      print64BIN(tableau);
    }

    if (digitalRead(CHANGER) && !digitalRead(CONFIRME))
    {
      logo();
    }
    else if (!digitalRead(CHANGER) && digitalRead(CONFIRME))
    {
      titre();
    }

    delay(100); // Les coeurs ont besoin d'un petit delai sinon ils peuvent tomber en erreurs
  }
}
#else
bool utiliseTest = false;
// Simule une serie d'action prise par des joueurs
void jeuVirtuel(void *pvParameters)
{
  int delaie = 1500; // Temps entre les action
  Case test[8][8];   // Creer un echiquier pour effectuer des tests
  uint64_t virtuel;  // Representation du tableau sur 64 bits
  Move list[64];     // Cree une list d'action possible
  int i = 0;         // Index representant le nombre d'elements actifs dans 'list'

  // Liste de deplacement. De (x1, y1) a (x2, y2)
  list[0] = {1, 1, 2, 1};
  list[1] = {6, 7, 5, 7};
  list[2] = {2, 1, 3, 1};
  list[3] = {5, 7, 4, 7};
  list[4] = {3, 1, 4, 1};
  list[5] = {4, 7, 3, 7};
  list[6] = {4, 1, 5, 1};
  list[7] = {3, 7, 2, 7};
  list[8] = {5, 1, 6, 2};
  list[9] = {2, 7, 1, 6};
  list[10] = {6, 2, 7, 3};
  list[11] = {1, 6, 0, 5};

  // Initialisation du jeu
  virtuel = virtuelleToBits(test);
  initialiseGrille(test);
  tableau = GAMESTART;

  delay(delaie);

  while (true) // Boucle pour rouler la partie demo
  {
    if (utiliseTest && i < 12)
    {
      Move a = list[i]; // Action qui sera faite pendant le tour

      Case temp = test[a.fromRow][a.fromCol]; // pour substitution

      // La piece est soulevee du jeu
      Serial.print("leve ");
      test[a.fromRow][a.fromCol].setJoueur(0);
      test[a.fromRow][a.fromCol].setPiece(' ');

      // On met a jour l'etat du jeu sur 64 bits
      virtuel = virtuelleToBits(test);
      taskENTER_CRITICAL(&my_spinlock);
      tableau = virtuel;
      taskEXIT_CRITICAL(&my_spinlock);

      Serial.print("Automove Move ");
      print64BIN(tableau);

      delay(delaie);

      // On verifie s'il y a une capture
      if (test[a.toRow][a.toCol].getJoueur() == -1 * temp.getJoueur())
      {
        // On retire la piece adverse du tableau
        Serial.print("eat! ");
        test[a.toRow][a.toCol].setJoueur(0);
        test[a.toRow][a.toCol].setPiece(' ');

        // Mise a jour
        virtuel = virtuelleToBits(test);
        taskENTER_CRITICAL(&my_spinlock);
        tableau = virtuel;
        taskEXIT_CRITICAL(&my_spinlock);

        delay(delaie);
      }

      // La piece est redepose sur le jeu
      Serial.print("Drop ");
      test[a.toRow][a.toCol] = temp;

      // Mise a jour
      virtuel = virtuelleToBits(test);
      taskENTER_CRITICAL(&my_spinlock);
      tableau = virtuel;
      taskEXIT_CRITICAL(&my_spinlock);

      // Incrementation pour passer au prochain tour
      i++;

      delay(delaie);
    }
    delay(delaie);
  }
}
#endif

void setup()
{
  // Le baud rate pour le ESP32 est 115200
  Serial.begin(115200); 
  Serial.println("Connexion serielle etablie");

  // Pour verifier sur quel coeur roule le setup
  Serial.print("setup() in core ");
  Serial.println(xPortGetCoreID());

  pinMode(MUX_S0, OUTPUT);
  digitalWrite(MUX_S0, LOW);
  pinMode(MUX_S1, OUTPUT);
  digitalWrite(MUX_S1, LOW);
  pinMode(MUX_S2, OUTPUT);
  digitalWrite(MUX_S2, LOW);
  pinMode(MUX_S3, OUTPUT);
  digitalWrite(MUX_S3, LOW);

  pinMode(MUX1_E, OUTPUT);
  digitalWrite(MUX1_E, HIGH);
  pinMode(MUX2_E, OUTPUT);
  digitalWrite(MUX2_E, HIGH);
  pinMode(MUX3_E, OUTPUT);
  digitalWrite(MUX3_E, HIGH);
  pinMode(MUX4_E, OUTPUT);
  digitalWrite(MUX4_E, HIGH);

  pinMode(RS_DATA, INPUT);
  digitalWrite(RS_DATA, LOW);
  Serial.println("Pin allouee");

  pinMode(CONFIRME, INPUT);
  digitalWrite(CONFIRME, LOW);
  pinMode(CHANGER, INPUT);
  digitalWrite(CHANGER, LOW);
  Serial.println("Boutons actifs");

  ledStrip.begin();
  ledStrip.show();
  ledStrip.setBrightness(200); // sur 255
  Serial.println("Strip active");

  initialiseGrille(echiquier);
  Serial.println("Grille virtuelle initialisee");

  oled.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  Serial.println("OLED initialisee");

#if TESTREEL
  xTaskCreatePinnedToCore(
      LectureTableau,
      "task0",
      10000,
      NULL,
      1,
      &Task0,
      0);
  delay(10);
#else
  xTaskCreatePinnedToCore(
      jeuVirtuel,
      "task0",
      10000,
      NULL,
      1,
      &Task0,
      0);
  delay(10);
#endif
  Serial.println("Tache creee");

#if 1
  InitialiseLED();
#else
  ledEchiquier();
#endif
}

void loop()
{
  bool enJeu = true;                                     // indique si une partie est en cours
  bool pieceValide = false;                              // indique si la position est valide 
  bool actionValide = false;                             // indique si l'action est valide et si l'on peut passer a la prochaine etape
  bool promo = false;                                    // indique si un pion est pret pour une promotion
  char piece = ' ';                                      // nom de la piece (R, N, B, Q, K, P)
  short joueur = 1;                                      // joueur blanc au debut de la partie
  short rangee = -1, colonne = -1;                       // actuelle
  short vieilleRangee = -1, vieilleColonne = -1;         // position de ou la piece a ete leve
  short rangeeDestination = -1, colonneDestination = -1; // position de ou la piece a ete deposer
  int positions = -1;                                    // nombre de mouvement/action/deplacement possible
  int promouvoir = 0;                                    // promotion actuellement afficher au joueur
  uint64_t tableauInterim = 0;                           // etat du tableau quand la piece est en l'air en binaire
  uint64_t tableauDebutTour = 0;                         // etat du tableau au debut du tour en binaire
  uint64_t tableauCapture = 0;                           // etat du tableau au debut de la capture en binaire
  Case changement = Case();                              // la case de l'echiquier qui a ete modifier lors de la lecture du tableau
  Case erreur = Case();                                  // la case de l'echiquier presentement en erreur
  Case capture = Case();                                 // La case de l'echiquier sur laquelle se deroule une capture

  // Pour verifier sur quel coeur roule le setup
  Serial.print("loop() in core ");
  Serial.println(xPortGetCoreID());

  titre();

  // Attend que les pieces soit placer adequatement sur l'echiquier
  while (true)
  {
    tableauDebutTour = getTableau();
    if (tableauDebutTour == GAMESTART)
    {
      break;
    }
  }

#if 1
  utiliseTest = true;
  Serial.println("Started");
#endif

  // Boucle d'un tour de jeu
  while (enJeu)
  {
    // sauvegarde du tableau au debut du tour
    tableauDebutTour = getTableau();
    afficheTableauPiece(echiquier);


  // Retirer vulnerabilite sur les pieces du joueur actif (prise en passant)
  #if 0
    for (int i = 0; i < TAILLE; i++) {
      for (int j = 0; j < TAILLE; j++) {
        // Verifie si la case est occuper par un pion du joueur. Seul les pions peuvent etre pris au passage
        if (echiquier[i][j].getJoueur() == joueur && echiquier[i][j].getPiece() == 'P')
        {
          // La case precedent est vulnerable, car c'est sur elle que l'adversaire 
          // doit placer son pion pour effectuer la capture
          echiquier[i - joueur][j].setVulnerable(false);
          
        }
      }
    }
  #endif

    // Premiere partie du tour : On souleve une piece valide
    actionValide = false;
    while (!actionValide)
    {
      // Verifie si une piece a ete bougee
      if (getTableau() != tableauDebutTour)
      {
        afficheTableauPiece(echiquier);

        // On copie la case qui a ete modifiee
        changement = difference(tableauDebutTour, getTableau(), echiquier);

        // Une piece du joueur actif a ete souleve
        if (changement.inbounds(changement.getRangee(), changement.getColonne()) && getTableau() < tableauDebutTour && changement.getJoueur() == joueur)
        {
          // Sauvegarde l'etat du tableau quand une piece est soulevee
          tableauInterim = getTableau();

          piece = changement.getPiece();
          actionValide = true;
        }
        // Une piece est placee sur l'echiquier ou une piece du joueur inactif est soulevee
        else
        {
          ledErreur(changement, tableauDebutTour);
        }
      }
    }

    // Fonction debug : Indique quel joueur a souleve quelle piece sur quelle case
    Serial.print(changement.getCouleur());
    Serial.print(" a soulever la piece ");
    Serial.print(changement.getPiece());
    Serial.print(" en ");
    Serial.println(changement.getNom());

    // Affiche les deplacements possibles
    positions = changement.bougerPiece(echiquier, actionPossible);
    ledAction(positions);

    // Deuxieme partie du tour : la piece revient sur le jeu
    actionValide = false;
    while (!actionValide)
    {
      // Une piece a ete bouger
      if (getTableau() != tableauInterim)
      {
        // On copie la case modifiee et on note ses coordonnees
        changement = difference(tableauInterim, getTableau(), echiquier);
        rangee = changement.getRangee();
        colonne = changement.getColonne();

        // Une piece est ajoutee au sur l'echiquier
        if (getTableau() > tableauInterim)
        {
          // On compare la case modifiee avec les deplacements possibles
          for (int i = 0; i < positions; i++)
          {
            rangeeDestination = actionPossible[i].toRow;
            colonneDestination = actionPossible[i].toCol;

            // Si la case modifiee est parmis les cases possibles, on met a jours les cases pour refleter le changement
            if (rangee == rangeeDestination && colonne == colonneDestination)
            {
              actionValide = true;

              vieilleRangee = actionPossible[i].fromRow;
              vieilleColonne = actionPossible[i].fromCol;

              echiquier[vieilleRangee][vieilleColonne].setJoueur(0);
              echiquier[vieilleRangee][vieilleColonne].setPiece(' ');

              echiquier[rangee][colonne].setJoueur(joueur);
              echiquier[rangee][colonne].setPiece(piece);

              // A RETRAVAILLER (avec pion dans Case.cpp)
              // Verifie si la piece qui a bouger est un pion et qu'elle a avancee de deux case
              bool bouge2case = abs(vieilleRangee - rangee) % 2 == 0;
              if (piece == 'P' && bouge2case)
              {
                // Le pion devient vulnerable a un prise en Passant
                echiquier[vieilleRangee - joueur][colonne].setVulnerable(true);
              }

              // TODO Corriger la demarche pour effectuer un roque
              if (piece == 'K')
              {
                bool roque = false;       // Indique si un roque peut etre fait
                bool petitRoque = false;  // Indique si un petit roque peut etre fait
                bool grandRoque = false;  // Indique si un grand roque peut etre fait
                Case tourPR;              // Case sur laquelle la tour pour un petit roque devrait etre
                Case tourGR;              // Case sur laquelle la tour pour un grand roque devrait etre
                
                // Verifie si le joueur blanc peut faire un roque
                if (joueur == 1)
                { 
                  tourPR = echiquier[0][7];
                  tourGR = echiquier[0][0];
                  petitRoque = (tourPR.getPiece() == 'R') && (tourPR.getJoueur() == joueur) && !tourPR.getABouger() && (rangee == 0) && (colonne == 6);
                  grandRoque = (tourGR.getPiece() == 'R') && (tourGR.getJoueur() == joueur) && !tourGR.getABouger() && (rangee == 0) && (colonne == 2);
                  roque = petitRoque && grandRoque; // Possiblement un OU plutot qu'un ET
                }
                // Verifie si le joueur noir peut faire un roque
                else
                { 
                  tourPR = echiquier[7][7];
                  tourGR = echiquier[7][0];
                  petitRoque = (tourPR.getPiece() == 'R') && (tourPR.getJoueur() == joueur) && !tourPR.getABouger() && (rangee == 7) && (colonne == 6);
                  grandRoque = (tourGR.getPiece() == 'R') && (tourGR.getJoueur() == joueur) && !tourGR.getABouger() && (rangee == 7) && (colonne == 2);
                  roque = petitRoque && grandRoque; // Possiblement un OU plutot qu'un ET
                }


                if (roque)
                {
                  Case mettreRoque;
                  echiquier[rangee][colonne].setABouger();

                  if (grandRoque)
                  {
                    mettreRoque = echiquier[rangee][0];
                    deplaceTourRoque(tourGR, mettreRoque, echiquier);
                  }
                  // petit roque
                  else
                  { 
                    mettreRoque = echiquier[rangee][0];
                    deplaceTourRoque(tourPR, mettreRoque, echiquier);
                  }
                  mettreRoque.setABouger();
                }
              }
              break;
            }
          }

          // La piece deposee n'est pas sur une lumiere
          if (!actionValide)
          {
            Serial.print("Depose ");
            ledErreur(changement, tableauInterim);
          }
        }
        // Une piece est retiree
        else
        {
          // Capture une piece adverse
          for (int i = 0; i < positions; i++)
          {
            rangeeDestination = actionPossible[i].toRow;
            colonneDestination = actionPossible[i].toCol;
            
            // Ne considere pas prise au passage
            if (rangee == rangeeDestination && colonne == colonneDestination && changement.getJoueur() == -1 * joueur) 
            {
              actionValide = true; // La piece retiré est sur une bonne case

              vieilleRangee = actionPossible[i].fromRow;
              vieilleColonne = actionPossible[i].fromCol;

              echiquier[vieilleRangee][vieilleColonne].setJoueur(0);
              echiquier[vieilleRangee][vieilleColonne].setPiece(' ');

              echiquier[rangee][colonne].setJoueur(joueur);
              echiquier[rangee][colonne].setPiece(piece);

              capture = changement;
              tableauCapture = getTableau();

              break;
            }
            // C'est une prise au passage
            else if (rangee == actionPossible[i].fromRow && colonne == colonneDestination && changement.getJoueur() == -1 * joueur) 
            {
              actionValide = true; // La piece retiré est sur une bonne case

              vieilleRangee = actionPossible[i].fromRow;
              vieilleColonne = actionPossible[i].fromCol;

              // ancienne position de la pièce du joueur
              echiquier[vieilleRangee][vieilleColonne].setJoueur(0);
              echiquier[vieilleRangee][vieilleColonne].setPiece(' ');

              // nouvelle position de la pièce du joueur
              echiquier[rangee][colonne].setJoueur(joueur);
              echiquier[rangee][colonne].setPiece(piece);
              echiquier[rangee][colonne].setVulnerable(false);

              // position du pion adverse capturé avec la prise au passage
              echiquier[vieilleRangee][colonne].setJoueur(0);
              echiquier[vieilleRangee][colonne].setPiece(' ');

              break;
            }
          }
          // depose piece ou l'autre a ete mangee
          while (actionValide && getTableau() != tableauInterim)
          {
            if (getTableau() != tableauCapture)
            {
              // On verifie si la piece est deposee sur la bonne case
              changement = difference(tableauCapture, getTableau(), echiquier);
              if (changement.getLed() != capture.getLed())
              {
                ledErreur(changement, tableauCapture);
              }
            }
          }

          // Piece en erreur
          if (!actionValide)
          {
            Serial.print("Eat take "); // Message d'erreur. Rendre plus significatif
            ledErreur(changement, tableauInterim);
          }
        }
      }
    }

   
    // Verifie echec

    // Promotion
    // Observe les extremites pour des pions 
    if (piece == 'P')
    {
      if ((joueur == 1 && rangee == 7) || (joueur == -1 && rangee == 0))
      {
        promo = true;
      }
    }

    Serial.println("debut promotion");
    if (promo == true)
    {
      // Indicateur de la case  (La couleur ne change pas. Voir pourquoi)
      ledStrip.setPixelColor(echiquier[rangee][colonne].getLed(), ledStrip.Color(0, 255, 255));
      ledStrip.show();

      // Choix de la promotion
      promouvoir = 0;

      Serial.print(echiquier[rangee][colonne].getNom()); // Indique le pion a promouvoir
      Serial.print(" a ");

#if 0 // Promotion seulement a Reine pour l'instant
      while (digitalRead(CONFIRME) == LOW)       // Attente de la confirmation du choix
      {
        if (digitalRead(CHANGER) == HIGH)  // Change le choix
        {
          promouvoir++;
          promouvoir %= 4;
          // TODO : Dessiner piece sur ecran
        }
      }
#endif
      echiquier[rangee][colonne].setPiece(promotion[3]); // Reine jusqu'a implementation graphique
      echangePromotion(echiquier[rangee][colonne], echiquier);
      Serial.println(echiquier[rangee][colonne].getPiece());

      promo = false;
    }

    Serial.println("Fin promotion");

    // Vide les actions possibles
    clearAction();
    // Joueur redepose sa piece d'ou il l'a prise, son tour recommence
    ledEchiquier();
    if (getTableau() == tableauDebutTour)
    {
      continue;
    }
    else
    { // Pour les mouvement de roque
      if (piece == 'K' || piece == 'R')
      {
        echiquier[rangee][colonne].setABouger();
      }
    }

    bool trouverRoi;
    for (int i = 0; i < TAILLE; i++)
    {
      for (int j = 0; j < TAILLE; j++)
      {
        if (echiquier[i][j].getJoueur() == -1 * joueur && echiquier[i][j].getPiece() == 'R')
        {
          trouverRoi = true;
        }
      }
    }
    if (trouverRoi == false)
    {
      enJeu = false;
      if (joueur == 1)
      {
        for (int i = 32; i < 64; i++)
        {
          ledStrip.setPixelColor(i, ledStrip.Color(0, 255, 0));
        }
        for (int i = 0; i < 32; i++)
        {
          ledStrip.setPixelColor(i, ledStrip.Color(255, 0, 0));
        }
      }
      else
      {
        for (int i = 32; i < 64; i++)
        {
          ledStrip.setPixelColor(i, ledStrip.Color(255, 0, 0));
        }
        for (int i = 0; i < 32; i++)
        {
          ledStrip.setPixelColor(i, ledStrip.Color(0, 255, 0));
        }
      }
    }

    if (digitalRead(CONFIRME) && digitalRead(CHANGER))
    {
      ecranReset();
      enJeu = false;
      delay(5000);
      initialiseGrille(echiquier);
    }

    // prochain tour
    joueur *= -1;
  }
}

// ------------------------------------Fonctions tableau ---------------------------------------------------------

// Initialisation de la partie
void initialiseGrille(Case (&echiquier)[8][8])
{
  int joueur; // Numero du joueur. Blanc = 1, Noir = 0
  int led;    // Adresse de la DEL dans la case

  for (short i = 0; i < TAILLE; i++)
  {
    for (short j = 0; j < TAILLE; j++)
    {
      char nom[2] = {'A' + j, '1' + i}; // Le nom de la case devrait etre en minuscule (notation d'echec)

      // Les rangees 0 et 1 appartiennent au joueur blanc
      if (i <= 1)
      {
        joueur = 1;
      }
      // Les rangees 6 et 7 appartiennet au joueur noir
      else if (i >= 6)
      {
        joueur = -1;
      }
      // Les autres rangees n'appartiennent pas a un joueur
      else
      {
        joueur = 0;
      }

      // Genere l'adresse des DEL adressables a une case
      if (i % 2 == 0)
      {
        led = i * 8 + j;
        // led = 63 - ((i + 1) * 8 - j - 1);
      }
      // La direction des DEL change a chaque rangee
      else
      {
        led = (i + 1) * 8 - j - 1;
        // led = 63 - (i * 8 + j);
      }

      // Creer une case avec son nom, la lettre du pion, la rangee, la colonne et l'adresse de sa DEL
      if (i == 0 || i == TAILLE - 1)
      {
        switch (j)
        {
        case 0:
        case 7:
          echiquier[i][j] = Case(nom, 'R', joueur, i, j, led);
          break;
        case 1:
        case 6:
          echiquier[i][j] = Case(nom, 'N', joueur, i, j, led);
          break;
        case 2:
        case 5:
          echiquier[i][j] = Case(nom, 'B', joueur, i, j, led);
          break;
        case 3:
          echiquier[i][j] = Case(nom, 'Q', joueur, i, j, led);
          break;
        case 4:
          echiquier[i][j] = Case(nom, 'K', joueur, i, j, led);
          break;
        }
      }
      else if (i == 1 || i == 6)
      {
        echiquier[i][j] = Case(nom, 'P', joueur, i, j, led);
      }
      else
      {
        echiquier[i][j] = Case(nom, ' ', 0, i, j, led);
      }
    }
  }
}

// Attend que les pieces soient placees pour commencer une partie
void setupPartie(uint64_t tableauCourant)
{
  // Affiche le jeu actuel et le tableau de depart.
  // 1 indique qu'un pion occupe la case, 0 indique que la case n'est pas occupee 
  print64BIN(tableauCourant);
  print64BIN(GAMESTART);
#if 1
  // Tant que le jeu actuel ne represente pas le jeu au depart, le jeu ne commence pas
  while ((tableauCourant != GAMESTART))
    ;
#endif

  Serial.println("C'est un depart");
}

// compare deux tableaux representes sur 64 bits pour identifier la case differente
// Retourne le contenue de la case affectee
Case difference(uint64_t vieuxTableau, uint64_t tableauCourant, Case echiquier[8][8])
{
  uint64_t changement = vieuxTableau ^ tableauCourant; // Valeur 64-bit. Comparaison par un OU-EXCLUSIF : Si deux bits different a la meme position, le resultat aura un 1 a cette position
  int position = -1; // Position a verifier (1 x 64)
  int rangee = -1, colonne = -1; // Rangee et colonne de la position (8 x 8)

#if 1 // Pour debug
  Serial.print("Difference : ");
  print64BIN(changement);
#endif

  // On cherche la position qui a change. Si changement = 0, on a trouver la position
  while (changement != 0)
  {
    // On tasse les bits de 1 position vers la droite sur 64 bits (ULL). Le plus petit bit est retirer et un 0 est insere au plus haut bit
    changement = changement >> 1ULL;
    position++;
  }

  rangee = position / 8;  // TODO Verifier si x et y ne sont pas inverses.
  colonne = position % 8;

  if (rangee >= 0 && colonne >= 0)
  {
    return echiquier[rangee][colonne];
  }

  return Case(); // Si ce RETURN est utilise, une erreur a eu lieu
}

// Est-ce pertinent?
void clearAction() 
{
  // Change l'espace memoire commencant au premier espace d'actionPossible a la valeur 0 pour tout l'espace occupe par actionPossible
  memset(actionPossible, 0, sizeof(actionPossible));
}

void deplaceTourRoque(Case dep, Case arriv, Case echiquier[8][8])
{
  uint64_t avant;     // Etat du tableau avant toute modification
  uint64_t pendant;   // Etat du tableau pendant les modifications
  bool actionValide;  // Indique si la modification est valide
  Case changement;    // Case qui a change d'etat

  // Affiche la case de depart et la case d'arrivee
  ledStrip.setPixelColor(dep.getLed(), ledStrip.Color(0, 255, 255));
  ledStrip.setPixelColor(arriv.getLed(), ledStrip.Color(0, 255, 0));
  ledStrip.show();

  // Avant la transition
  avant = getTableau();

  // On retire le pion
  actionValide = false;
  while (!actionValide)
  {
    // Une piece est retiree
    if (getTableau() < avant)
    {
      // On trouve quelle piece a ete retiree
      changement = difference(avant, getTableau(), echiquier);
      if (changement.inbounds(changement.getRangee(), changement.getColonne()) && changement.getLed() == dep.getLed()) // La led peut servir d'identifiant a une case
      {
        actionValide = true;
        // On enregistre le tableau pendant la transition
        pendant = getTableau(); 
      }
      else
      {
        ledErreur(changement, avant);
      }
    }
  }

  // On ajoute une piece
  actionValide = false;
  while (!actionValide)
  {
    // Une piece est ajoutee
    if (getTableau() > pendant)
    {
      // On trouve quelle piece a ete ajoutee
      changement = difference(pendant, getTableau(), echiquier);
      if (changement.inbounds(changement.getRangee(), changement.getColonne()) && changement.getLed() == arriv.getLed()) // La led peut servir d'identifiant a une case
      {
        actionValide = true;
      }
      else
      {
        ledErreur(changement, pendant);
      }
    }
  }
}

void echangePromotion(Case piece, Case echiquier[8][8])
{
  uint64_t avant, pendant;
  bool actionValide;
  Case changement;

  // Avant la transition
  avant = getTableau();

  // On retire le pion
  actionValide = false;
  while (!actionValide)
  {
    // Une piece est retiree
    if (getTableau() < avant)
    {
      // Quelle piece a ete retiree?
      changement = difference(avant, getTableau(), echiquier);
      if (changement.inbounds(changement.getRangee(), changement.getColonne()) && changement.getLed() == piece.getLed()) // La led peut servir d'identifiant a une case
      {
        actionValide = true;
        pendant = getTableau(); // pendant la transition
      }
      else
      {
        ledErreur(changement, avant);
      }
    }
  }

  // On ajoute une piece
  actionValide = false;
  while (!actionValide)
  {
    // Une piece est ajoutee
    if (getTableau() > pendant)
    {
      // Quelle piece a ete ajoutee?
      changement = difference(pendant, getTableau(), echiquier);
      if (changement.inbounds(changement.getRangee(), changement.getColonne()) && changement.getLed() == piece.getLed()) // La led peut servir d'identifiant a une case
      {
        actionValide = true;
      }
      else
      {
        ledErreur(changement, pendant);
      }
    }
  }
}

uint64_t getTableau()
{
  uint64_t buffer = 0;

  taskENTER_CRITICAL(&my_spinlock);
  buffer = tableau;
  taskEXIT_CRITICAL(&my_spinlock);

#if 0
  Serial.print("GetTableau tableau : ");
  print64BIN(tableau);
  Serial.print("GetTableau buffer : ");
  print64BIN(buffer);
#endif

  return buffer;
}

uint64_t virtuelleToBits(Case (&echiquier)[8][8])
{
  uint64_t bitfield = 0;

  for (int rangee = 0; rangee < 8; rangee++)
  {
    for (int colonne = 0; colonne < 8; colonne++)
    {
      if (echiquier[rangee][colonne].getJoueur() != 0)
      {
        uint64_t target = 1ULL << ((rangee * 8) + colonne);
        bitfield |= target;
      }
    }
  }
#if 1
  Serial.print("Virtual to bits : ");
  print64BIN(bitfield);
#endif
  return bitfield;
}

// ------------------------------------Fonctions DEL ---------------------------------------------------------

// Serpentine pour tester les DEL
void InitialiseLED()
{
  for (int i = 0; i < LEDCOUNT + 8; i++)
  {
    if (i < LEDCOUNT)
    {
      ledStrip.setPixelColor(i, ledStrip.Color(255, 255, 255));
    }
    if (i >= 8)
    {
      ledStrip.setPixelColor(i - 8, ledStrip.Color(255, 0, 0));
    }

    ledStrip.show();
    delay(75);
  }

  delay(200);
  ledStrip.clear();
  ledEchiquier();
}

// Genere l'eclairage d'un echiquier (Cases blanches et noires)
void ledEchiquier()
{
  Case carre;
  ledStrip.clear();
  for (int rangee = 0; rangee < 8; rangee++)
  {
    for (int colonne = 0; colonne < 8; colonne++)
    {
      carre = echiquier[rangee][colonne];
      if (carre.getLed() % 2 != 0)
      {
        ledStrip.setPixelColor(carre.getLed(), ledStrip.Color(255, 255, 255));
        ledStrip.show();
      }
    }
  }
}

void ledAction(int positions)
{
  int led, colonne, rangee;
  colonne = actionPossible[0].fromCol;
  rangee = actionPossible[0].fromRow;
  led = echiquier[rangee][colonne].getLed();
  ledStrip.setPixelColor(led, ledStrip.Color(0, 255, 255)); // Cyan

  for (int i = 1; i < positions; i++)
  {
    colonne = actionPossible[i].toCol;
    rangee = actionPossible[i].toRow;
    led = echiquier[rangee][colonne].getLed();              // On va chercher l'adresse de la DEL a cette case
    ledStrip.setPixelColor(led, ledStrip.Color(0, 255, 0)); // vert
  }

  ledStrip.show();
}

void ledErreur(Case erreur, uint64_t tableauPrecedent)
{
  Serial.print("Erreur a ");
  Serial.println(erreur.getNom());

#if 0
  afficheTableauDetail();
#endif

  if (erreur.inbounds(erreur.getRangee(), erreur.getColonne()))
  {
    ledStrip.setPixelColor(erreur.getLed(), ledStrip.Color(255, 0, 0));
    ledStrip.show();
  }

  while (getTableau() != tableauPrecedent)
    ;
  if (erreur.getLed() % 2 == 0)
  {
    ledStrip.setPixelColor(erreur.getLed(), ledStrip.Color(0, 0, 0));
  }
  else
  {
    ledStrip.setPixelColor(erreur.getLed(), ledStrip.Color(255, 255, 255));
  }
  ledStrip.show();
}

// ------------------------------------Fonctions Ecran ---------------------------------------------------------

// Genere le logo sur l'ecran
void logo()
{
  oled.clearDisplay();
  oled.drawBitmap(0, 0, epd_bitmap_logo, 128, 64, 1);
  oled.display();
}

// Genere le titre du projet et les auteurs sur l'ecran
void titre()
{
  oled.clearDisplay();              // Efface l'ecran
  oled.setTextSize(2);              // Taille du texte
  oled.setTextColor(SSD1306_WHITE); // Couleur du texte
  oled.setCursor(35, 15);           // Position du curseur
  oled.print("Echec");
  oled.setCursor(30, 30);
  oled.print("Samuel");
  oled.setCursor(24, 45);
  oled.print("William");
  oled.display();                   // Affiche le texte
}

void ecranReset()
{
  oled.clearDisplay();
  oled.setTextSize(3);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(30, 15);
  oled.print("RESET");
  oled.display();
}

// ------------------------------------Fonctions debug ---------------------------------------------------------

// Affiche la position des pieces
// TODO Changer k et j pour rangee et colonne pour clarifier
void afficheTableauPiece(Case (&echiquier)[8][8])
{
  int k = 0; // Numero de la rangee du jeu

  for (int i = 0; i < 17; i++)
  {
    // Dessine une ligne pour diviser les rangees de la grille de jeu
    if (i % 2 == 0)
    {
      Serial.println("-----------------");
    }
    else
    {
      for (int j = 0; j < 8; j++)
      {
        Serial.print("|"); // Separe les colonnes de la grille de jeu
        Serial.print(echiquier[k][j].getPiece());
      }
      Serial.println("|");
      k++;
    }
  }
  // Serial.println("fin\n\n");
}

// Affiche si une case est occupee ou non
void afficheTableauPresence()
{
  uint64_t valeur = getTableau(); // representation 64-bit du tableau. TODO Trouver un meilleur nom?

  for (int i = 0; i < 64; i++)
  {
    // Retour a la ligne apres 8 characteres
    if (i % 8 == 0)
      Serial.println();

    Serial.print(bitRead(valeur, 0)); // Imprime la valeur du plus petit bit de valeur
    Serial.print(" ");
    valeur >> 1ULL; // Tasse 64 bits vers la droite. Le plus petit bit est retirer et 0 est ajoute comme plus grand bit
  }
  Serial.println("\n -------------------");
}

// Affiche le retour d'un multiplexeur
void afficheMultiplexeur()
{
  int i = 7;
  digitalWrite(MUX1_E, LOW);
  digitalWrite(MUX2_E, HIGH);
  digitalWrite(MUX3_E, HIGH);
  digitalWrite(MUX4_E, HIGH);

  digitalWrite(MUX_S0, bitRead(i, 0));
  digitalWrite(MUX_S1, bitRead(i, 1));
  digitalWrite(MUX_S2, bitRead(i, 2));
  digitalWrite(MUX_S3, bitRead(i, 3));

  Serial.print(digitalRead(RS_DATA));
  Serial.print(" ");

  Serial.println();
}

// Affiche quel joueur occupe les cases
// TODO Changer k et j pour rangee et colonne pour clarifier
void afficheTableauJoueur()
{
  int k = 0; // Numero de la rangee du jeu

  for (int i = 0; i < 17; i++)
  {
    // Dessine une ligne pour diviser les rangees de la grille de jeu
    if (i % 2 == 0)
    {
      Serial.println("-----------------");
    }
    else
    {
      for (int j = 0; j < 8; j++)
      {
        Serial.print("|"); // Separe les colonnes de la grille de jeu
        Serial.print(echiquier[k][j].getJoueur());
      }
      Serial.println("|");
      k++;
    }
  }
  Serial.println("fin\n\n");
}

// Affiche le nom des cases
// TODO Changer k et j pour rangee et colonne pour clarifier
void afficheTableauNom()
{
  int k = 0; // Numero de la rangee du jeu

  // Dessine une ligne pour diviser les rangees de la grille de jeu
  for (int i = 0; i < 17; i++)
  {
    if (i % 2 == 0)
    {
      Serial.println("-------------------------");
    }
    else
    {
      for (int j = 0; j < 8; j++)
      {
        Serial.print("|"); // Separe les colonnes de la grille de jeu
        Serial.print(echiquier[k][j].getNom());
      }
      Serial.println("|");
      k++;
    }
  }
  Serial.println("fin\n\n");
}

// Affiche en detail le contenue des cases
void afficheTableauDetail()
{
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      Serial.print(echiquier[i][j].readCase() + "\t");
    }
    Serial.println();
  }
  Serial.println("fin\n\n");
}

// Permet d'imprimer 64 bits d'information.
// L'ESP32 est limiter a une impression normale de 32 bits
void print64BIN(uint64_t valeur)
{
  for (int i = 63; i >= 0; i--)
  {

    // Si le bit est un 1, on imprime un 1, sinon on imprime 0
    Serial.print((valeur & (1ULL << i)) ? '1' : '0');
    
    // Creer un espace entre chaque 4 bits pour lisibilite
    if (i % 4 == 0)
    {
      Serial.print(" ");
    }
  }
  Serial.println();
}