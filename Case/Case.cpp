#include <Arduino.h>
#include <Case.h>

// Constructeur. Creer une instance de Case avec des parametres definis
Case::Case(char nom[2], char piece, short joueur, short rangee, short colonne, short led)
{
  strncpy(_nom, nom, 2);
  _nom[2] = '\0';
  setPiece(piece);
  setJoueur(joueur);
  _rangee = rangee;
  _colonne = colonne;
  _led = led;
}

// Constructeur par defaut. Creer une instance de Case avec des parametres predefinis
Case::Case()
{
  _nom[0] = '\0';
  _piece = '-';
  _joueur = 0;
  _rangee = -1;
  _colonne = -1;
  _led = -1;
}

//****** Getters - Retourne des valeurs dans la case ******//

// Retourne le nom de la case. Ex :A1, B5, H8, ...
const char *Case::getNom() const
{
  return _nom;
}

// Retourne l'adresse de la DEL associee a la case
int Case::getLed()
{
  return _led;
}

// Retourne le nom de la piece en notation anglaise. Ex: R pour fou.
// Exception, le pion prend la notation P.
char Case::getPiece()
{
  return majuscule(_piece);
}

// Retourne la valeur du joueur.
// 1 pour blanc, -1 pour noir ou 0 si aucun joueur
short Case::getJoueur()
{
  return _joueur;
}

// Retourne la couleur du joueur. Blanc, noir ou  case vide
String Case::getCouleur()
{
  if (_joueur == 1)
  {
    return "Joueur blanc";
  }
  else if (_joueur == -1)
  {
    return "Joueur noir";
  }
  else
  {
    return "Case vide";
  }
}

// Retourne la coordonnee verticale de la case
short Case::getRangee()
{
  return _rangee;
}

// Retourne la coordonne horizontale de la case
short Case::getColonne()
{
  return _colonne;
}

// Retourne si une piece a bouge ou non sur cette case
bool Case::getABouger()
{
  return _aBouger;
}

// Retourne si la piece sur la case est vulnerable a une prise en passant ou non
bool Case::getVulnerable()
{
  return _vulnerable;
}

//****** Setters - Ecrit des valeurs dans la case ******//

// Assigne une nouvelle piece sur la case.
// Si il y a une erreur, la piece sera la valeur par defaut
void Case::setPiece(char piece)
{
  piece = majuscule(piece);

  switch (piece)
  {
  case 'R':
  case 'N':
  case 'B':
  case 'Q':
  case 'K':
  case 'P':
  case ' ':
    _piece = piece;
    break;
  default:
    _piece = '-';
    break;
  }
}

// Assigne un nouveau joueur a un case
// 1 pour blanc, -1 pour noir ou 0 si aucun joueur
void Case::setJoueur(short joueur)
{
  _joueur = joueur;

  // Si la valeur entree est hors bornes,
  // elle est ajuste a la borne la plus pres
  if (joueur > 0)
    _joueur = 1;
  if (joueur < 0)
    _joueur = -1;
}

// Change la valeur de _aBouger pour indiquer qu'une piece a bouger sur cette case
// Aucun parametre, car cet etat peut seulement devenir vrai (faux a l'initialisation)
void Case::setABouger()
{
  _aBouger = true;
}

// Assigne un etat de vulnaribilite d'une piece a un prise en passant
void Case::setVulnerable(bool vulnerable)
{
  _vulnerable = vulnerable;
}

//****** Fonctions utilitaires - Fonctions servant a gerer differents parametres dans une case ******//

// Retourne une string pour qui indique chaque valeur dans la case
// TODO ajouter les parametres manquants : aBouger, vulnerable, LED, etc.
String Case::readCase()
{
  String piece;
  String joueur;
  String x, y;

  switch (_joueur)
  {
  case 0:
    joueur = " vide";
    break;
  case 1:
    joueur = " blanc";
    break;
  case -1:
    joueur = " noir";
    break;
  default:
    joueur = " joueur";
    break;
  }

  switch (_piece)
  {
  case 'R':
    piece = " tour";
    break;
  case 'N':
    piece = " cavalier";
    break;
  case 'B':
    piece = " fou";
    break;
  case 'Q':
    piece = " reine";
    break;
  case 'K':
    piece = " roi";
    break;
  case 'p':
    piece = " pion";
    break;
  case ' ':
    piece = " case";
    break;
  default:
    piece = " piece";
    break;
  }

  return _nom + piece + joueur + " " + String(_rangee) + " " + String(_colonne);
}

// Verifie si une case est vide. Ie. si il n'y a pas de piece et de joueur dessus
bool Case::isVide()
{
  if (_piece == ' ' && _joueur == 0)
  {
    return true;
  }

  return false;
}

// Verifie si une case est dans le tableau
// rangee : nombre de rangee du tableau
// colonne :  nombre de colonne du tableau
bool Case::inbounds(short rangee, short colonne)
{
  if (rangee >= 0 && rangee < TAILLE && colonne >= 0 && colonne < TAILLE)
    return true;
  return false;
}

// Convertit une lettre minuscule en une lettre majuscule
char Case::majuscule(char symbole)
{
  if (symbole >= 'a' && symbole <= 'z')
  {
    symbole = symbole + 'A' - 'a';
  }
  return symbole;
}

// TODO Convertir une lettre majuscule en une lettre minuscule



//****** Déplacement - Indique les déplacements qu'une piece en action peut effectuer ******//

// Identifie la pièce en action et retourne le nombre de case où elle peut bouger
// echiquier[8][8] : copie de l'échiquier de jeu
// *actionPossible : pointeur vers le tableau contenant les actions possibles
int Case::bougerPiece(Case echiquier[8][8], Move *actionPossible)
{
  int actions = 0; // indique le nombre de case sur l'échiquier où la pièce peut être bougé

  actionPossible[0] = {_rangee, _colonne, _rangee, _colonne}; // action de déposer la pièce à sa position de départ

  // aller chercher les actions/mouvements/déplacements possibles selon la pièce
  switch (_piece)
  {
  case 'R':
    actions = Tour(echiquier, actionPossible);
    break;
  case 'N':
    actions = Cavalier(echiquier, actionPossible);
    break;
  case 'B':
    actions = Fou(echiquier, actionPossible);
    break;
  case 'Q':
    actions = Reine(echiquier, actionPossible);
    break;
  case 'K':
    actions = Roi(echiquier, actionPossible);
    break;
  case 'P':
    actions = Pion(echiquier, actionPossible);
    break;
  default:
    Serial.print("Erreur : Piece inconnue : "); // TODO verifier si le port seriel est actif
    Serial.println(_piece);
    break;
  }

  return actions;
}

// Retourne le nombre d'action qu'un pion peut faire
// Ajoute les actions que le pion peut faire au tableau actionPossible
int Case::Pion(Case echiquier[8][8], Move *actionPossible)
{
  // La rangée destination du pion
  short newX;
  // La colonne destination du pion
  short newY;
  // Indique le nombre de case sur l'échiquier où la pièce peut être bougée.
  // Commence a 1 parce que redeposer a deja ete pris en compte
  int actions = 1;
  Case destination; // Destination testee par un pion

  // Bouger 1 case
  newX = _rangee + _joueur;
  newY = _colonne;
  destination = echiquier[newX][newY];
  // Si la destination est libre, le pion peut s'y déplacer
  if (destination.isVide())
  {
    actionPossible[actions] = {_rangee, _colonne, newX, newY};
    actions++;
  }

  // Bouger 2 cases
  newX += _joueur;
  newY = _colonne;
  // Verifie si le pion peut se déplacer de deux cases
  if ((_rangee == 1 && _joueur == 1) || (_rangee == 6 && _joueur == -1))
  {
    destination = echiquier[newX][newY];
    // S'il n'y a pas d'obstacle entre le pion et la case, il peut s'y déplacer
    if (destination.isVide() && echiquier[newX - _joueur][newY].isVide())
    {
      actionPossible[actions] = {_rangee, _colonne, newX, newY};
      actions++;
    }
  }

  // Capture
  for (int i = -1; i <= 1; i += 2)
  {
    newX = _rangee + _joueur;
    newY = _colonne + i;

    // La nouvelle colonne est hors de l'échiquier
    if (!inbounds(newX, newY))
    {
      continue; // Passe immediatement a la prochaine etape dans la boucle
    }

    destination = echiquier[newX][newY];
    // Vérifie si la destination est occupée par une pièce qui n'est pas celle du joueur
    if (!destination.isVide() && destination.getJoueur() != _joueur)
    {
      actionPossible[actions] = {_rangee, _colonne, newX, newY};
      actions++;
    }
  }

  // TODO faire de quoi ---> Ceci est un commentaire dont la signification a été oubliée. Il est possible qu'il soit désuet

  // au passage   ----> Ne fonctionne pas au moment de la remise de V1
  for (int i = -1; i <= 1; i += 2)
  {
    newX = _rangee + _joueur;
    newY = _colonne + i;

    // La nouvelle colonne est hors de l'échiquier
    if (!inbounds(newX, newY))
    {
      continue;
    }

    // TODO Erreur : destination.isVide(), pas !destination().isVide()
    // Ajoute la destination aux actions possibles si valide
    destination = echiquier[newX][newY];
    if (!destination.isVide() && destination.getJoueur() != _joueur && destination.getVulnerable())
    {
      actionPossible[actions] = {_rangee, _colonne, newX, newY};
      actions++;
    }
  }

  return actions;
}

// Retourne le nombre d'action qu'une tour peut faire
// Ajoute les actions que la tour peut faire au tableau actionPossible
int Case::Tour(Case echiquier[8][8], Move *actionPossible)
{
  // La rangée destination de la tour
  short newX;
  // La colonne destination du tour
  short newY;
  // Indique le nombre de case sur l'échiquier où la pièce peut être bougé.
  // Commence a 1 parce que redeposer a deja ete pris en compte
  int actions = 1;
  Case destination; // Destination testee par une tour

  // Bouger horizontalement
  for (short direction = -1; direction <= 1; direction += 2)
  {
    for (short i = 1; i < TAILLE; i++)
    {
      newX = _rangee + direction * i;
      newY = _colonne;

      // Si la case est hors jeu, on arrete le traitement de la boucle,
      // car les cases suivantes seraient aussi hors-jeu
      if (!inbounds(newX, newY))
      {
        break;
      }

      destination = echiquier[newX][newY];
      // Si la case est vide, on l'ajoute a la liste des positions posibles
      if (destination.isVide())
      {
        actionPossible[actions] = {_rangee, _colonne, newX, newY};
        actions++;
      }
      // Sinon, si la case est occupée par une piece adverse, on l'ajoute a la liste et on arrête de traiter la direction
      else if (destination.getJoueur() != _joueur)
      {
        actionPossible[actions] = {_rangee, _colonne, newX, newY};
        actions++;
        break;
      }
      // Sinon, si la case est occupée par une piece amicale, on ne l'ajoute pas a la liste et on arrête de traiter la direction
      else
      {
        break;
      }
    }
  }

  // Bouger verticalement
  for (short direction = -1; direction <= 1; direction += 2)
  {
    for (short i = 1; i < TAILLE; i++)
    {
      newX = _rangee;
      newY = _colonne + direction * i;

      // Si la case est hors jeu, on arrete le traitement de la boucle,
      // car les cases suivantes seraient aussi hors jeu
      if (!inbounds(newX, newY))
      {
        break;
      }

      destination = echiquier[newX][newY];
      // Si la case est vide, on l'ajoute a la liste des positions posibles
      if (destination.isVide())
      {
        actionPossible[actions] = {_rangee, _colonne, newX, newY};
        actions++;
      }
      // Sinon, si la case est occuper par une piece adverse, on l'ajoute a la liste et on arrete de traiter la direction
      else if (destination.getJoueur() != _joueur)
      {
        actionPossible[actions] = {_rangee, _colonne, newX, newY};
        actions++;
        break;
      }
      // Sinon, si la case est occuper par une piece amicale, on arrete de traiter la direction et on ne l'ajoute pas a la liste
      else
      {
        break;
      }
    }
  }

  return actions;
}

// Retourne le nombre d'action qu'un cavalier peut faire
// Ajoute les actions que le cavalier peut faire au tableau actionPossible
int Case::Cavalier(Case echiquier[8][8], Move *actionPossible)
{
  // La rangee destination du cavalier
  short newX;
  // La colonne destination du cavalier
  short newY;
  // Matrice contenant les deplacements possibles du cavalier {rangee, colonne}
  short deplacement[8][2] = {{-2, 1}, {-1, 2}, {1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}};
  // indique le nombre de case sur l'échiquier où la pièce peut être bougé.
  // Commence a 1 parce que redeposer a déjà été pris en compte
  int actions = 1;
  // Destination testée par le cavalier
  Case destination;


  for (short i = 0; i < 8; i++)
  {
    newX = _rangee + deplacement[i][0]; // { * , }
    newY = _colonne + deplacement[i][1]; // { , * }

    // Si une position est hors-jeu, on évite de la traiter en passant à la prochaine itération de la boucle
    if (!inbounds(newX, newY)) 
    {
      continue;
    }

    destination = echiquier[newX][newY];
    // Si la position est libre ou n'appartient pas au joueur actif, on l'ajoute à la liste des position possible
    if (destination.isVide() || destination.getJoueur() != _joueur)
    {
      actionPossible[actions] = {_rangee, _colonne, newX, newY};
      actions++;
    }
  }

  return actions;
}

// Retourne le nombre d'action qu'un fou peut faire
// Ajoute les actions que le fou peut faire au tableau actionPossible
int Case::Fou(Case echiquier[8][8], Move *actionPossible)
{
  // La rangée destination du fou
  short newX; 
  // La colonne destination du fou
  short newY; 
  // indique le nombre de case sur l'échiquier où la pièce peut être bougé.
  // Commence a 1 parce que redéposer a déjà été pris en compte
  int actions = 1;
  // Destination testée par le fou
  Case destination;

  for (short horizon = -1; horizon <= 1; horizon += 2) // Gauche ou droite
  {
    for (short vertical = -1; vertical <= 1; vertical += 2) // Bas ou haut
    {
      for (short i = 1; i < TAILLE; i++)
      {
        newX = _rangee + horizon * i;
        newY = _colonne + vertical * i;

        // Si la case est hors jeu, on arrete le traitement de la boucle,
        // car les cases suivantes seraient aussi hors jeu
        if (!inbounds(newX, newY))
        {
          break;
        }

        destination = echiquier[newX][newY];
        // Si la case est vide, on l'ajoute a la liste des positions posibles 
        if (destination.isVide()) 
        {
          actionPossible[actions] = {_rangee, _colonne, newX, newY};
          actions++;
        }
        // Sinon, si la case est occuper par une piece adverse, on l'ajoute a la liste et on arrete de traiter la direction
        else if (destination.getJoueur() != _joueur) 
        {
          actionPossible[actions] = {_rangee, _colonne, newX, newY};
          actions++;
          break;
        }
        // Sinon, si la case est occuper par une piece amicale, on arrete de traiter la direction et on ne l'ajoute pas a la liste
        else
        {
          break;
        }
      }
    }
  }

  return actions;
}

// Retourne le nombre d'action qu'une reine peut faire
// Ajoute les actions que la reine peut faire au tableau actionPossible
int Case::Reine(Case echiquier[8][8], Move *actionPossible)
{
 // La rangee destination de la reine
  short newX;
  // La colonne destination du reine
  short newY;
  // Indique le nombre de case sur l'échiquier où la pièce peut être bougé.
  // Commence a 1 parce que redeposer a deja ete pris en compte
  int actions = 1;
  Case destination; // Destination testee par la reine

  // Bouger horizontalement
  for (int direction = -1; direction <= 1; direction += 2)
  {
    for (int i = 1; i < TAILLE; i++)
    {
      newX = _rangee + direction * i;
      newY = _colonne;

      // Si la case est hors jeu, on arrete le traitement de la boucle,
      // car les cases suivantes seraient aussi hors jeu
      if (!inbounds(newX, newY))
      {
        break;
      }

      destination = echiquier[newX][newY];
      // Si la case est vide, on l'ajoute a la liste des positions posibles
      if (destination.isVide()) 
      {
        actionPossible[actions] = {_rangee, _colonne, newX, newY};
        actions++;
      }
      // Sinon, si la case est occuper par une piece adverse, on l'ajoute a la liste et on arrete de traiter la direction 
      else if (destination.getJoueur() != _joueur)
      {
        actionPossible[actions] = {_rangee, _colonne, newX, newY};
        actions++;
        break;
      }
      // Sinon, si la case est occuper par une piece amicale, on arrete de traiter la direction et on ne l'ajoute pas a la liste
      else
      {
        break;
      }
    }
  }

  // Bouger verticalement
  for (int direction = -1; direction <= 1; direction += 2)
  {
    for (int i = 1; i < TAILLE; i++)
    {
      newX = _rangee;
      newY = _colonne + direction * i;

      // Si la case est hors jeu, on arrete le traitement de la boucle,
      // car les cases suivantes seraient aussi hors jeu
      if (!inbounds(newX, newY))
      {
        break;
      }

      destination = echiquier[newX][newY];
      // Si la case est vide, on l'ajoute a la liste des positions posibles
      if (destination.isVide()) 
      {
        actionPossible[actions] = {_rangee, _colonne, newX, newY};
        actions++;
      }
      // Sinon, si la case est occuper par une piece adverse, on l'ajoute a la liste et on arrete de traiter la direction  
      else if (destination.getJoueur() != _joueur)
      {
        actionPossible[actions] = {_rangee, _colonne, newX, newY};
        actions++;
        break;
      }
      // Sinon, si la case est occuper par une piece amicale, on arrete de traiter la direction et on ne l'ajoute pas a la liste
      else 
      {
        break;
      }
    }
  }

  // Bouger diagonalement
  for (int horizon = -1; horizon <= 1; horizon += 2)
  {
    for (int vertical = -1; vertical <= 1; vertical += 2)
    {
      for (int i = 1; i < TAILLE; i++)
      {
        newX = _rangee + horizon * i;
        newY = _colonne + vertical * i;

        // Si la case est hors jeu, on arrete le traitement de la boucle,
        // car les cases suivantes seraient aussi hors jeu
        if (!inbounds(newX, newY)) 
        {
          break;
        }

        destination = echiquier[newX][newY];
        // Si la case est vide, on l'ajoute a la liste des positions posibles
        if (destination.isVide()) 
        {
          actionPossible[actions] = {_rangee, _colonne, newX, newY};
          actions++;
        }
        // Sinon, si la case est occuper par une piece adverse, on l'ajoute a la liste et on arrete de traiter la direction
        else if (destination.getJoueur() != _joueur)
        {
          actionPossible[actions] = {_rangee, _colonne, newX, newY};
          actions++;
          break;
        }
        // Sinon, si la case est occuper par une piece amicale, on arrete de traiter la direction et on ne l'ajoute pas a la liste
        else
        {
          break;
        }
      }
    }
  }

  return actions;
}

// Retourne le nombre d'action qu'un roi peut faire
// Ajoute les actions que le roi peut faire au tableau actionPossible
int Case::Roi(Case echiquier[8][8], Move *actionPossible)
{
  // La rangee destination du roi
  short newX;
  // La colonne destination du roi
  short newY;
  // Matrice contenant les deplacements possibles du roi {rangee, colonne}
  short deplacement[8][2] = {{1, 1}, {1, 0}, {1, -1}, {0, 1}, {0, -1}, {-1, 1}, {-1, 0}, {-1, -1}};
  // Indique le nombre de case sur l'échiquier où la pièce peut être bougé.
  // Commence a 1 parce que redeposer a deja ete pris en compte
  int actions = 1;
  // Destination testee par un roi
  Case destination;

  for (short i = 0; i < 8; i++)
  {
    Serial.print("Roi deplacement : ");
    Serial.println(i);
    newX = _rangee + deplacement[i][0];
    newY = _colonne + deplacement[i][1];

    // Si une position est hors-jeu, on évite de la traiter en passant à la prochaine itération de la boucle
    if (!inbounds(newX, newY))
    {
      continue;
    }

    destination = echiquier[newX][newY];
    // Si la case est vide, on l'ajoute a la liste des positions possibles
    if (destination.isVide()) 
    {
      // On vérifie si le déplacement ne met pas le roi en échec  (****Attention : Permet de se deplacer sur une piece amie****)
      bool echecDestination = destination.echec(echiquier, true);
      if (!echecDestination)
      { 
        actionPossible[actions] = {_rangee, _colonne, newX, newY};
        actions++;
      }
    }
    // Si la destination n'est pas occupée par une pièce amie, on ajoute la destination à la liste
    else if (destination.getJoueur() != _joueur) // Capture
    {
      actionPossible[actions] = {_rangee, _colonne, newX, newY};
      actions++;
    }
  }

  // Si le roi a déjà bougé, on arrête le traitement avant de considérer les roques
  if (_aBouger)
  {
    return actions;
  }

  //TODO **** La logique des roques contient des erreurs ****

  // Case générique. Utiliée pour effectuer des tests
  Case c;
  // État de la tour. A-t-elle bougé?
  bool tourPasBouge;
  // État du tableau. Les cases entre le roi et la tour sont-elles vides?
  bool passageLibre;

  // grand roque
  while (1) // Les multiples IF pourraient utiliser une condition booléenne plutôt qu'un WHILE(1) avec plusieurs BREAK
  {
    // Aller chercher si la tour du joueur actuel ont bougé
    c = echiquier[_rangee][0];
    tourPasBouge = (c.getPiece() == 'R') && (c.getJoueur() == _joueur);

    // Vérifie si la tour a bougé. Arrête si elle a déjà bougé
    if (!tourPasBouge)
    { 
      break;
    }

    // Vérifier qu'il n'y a pas des pièces entre le roi et la tour
    passageLibre = true;
    for (short i = 1; i < 4; i++)
    {
      c = echiquier[_rangee][i];
      
      // Si le chemin a un obstacle, on arrête de vérifier le passage
      if (!c.isVide())
      { 
        passageLibre = false;
        break;
      }
    }

    // Si le passage est occupé, le roque ne peut avoir lieu, on arrête de le traiter
    if (!passageLibre)
    {
      break;
    }

    // Le roque ne peut avoir lieur si le roi est en échec
    if (echec(echiquier, false))
    {
      break;
    }

    // Si une pièce adverse menace une case sur le chemin entre la position actuelle et la destination du roi, le roque ne peut avoir lieu
    c = echiquier[_rangee][3];
    if (c.echec(echiquier, true))
    {
      break;
    }

    // Vérifie que le roi ne termine pas sur une case en échec
    c = echiquier[_rangee][2];
    if (c.echec(echiquier, true))
    {
      break;
    }

    // Rendu ici, toute les conditions de grand roque devrait être respectées
    // Ajout du mouvement à la liste d'actions possibles
    actionPossible[actions] = {_rangee, _colonne, _rangee, 2};
    actions++;
    break;
  }

  // petit roque
  while (1) // Les multiples IF pourraient utiliser une condition booléenne plutôt qu'un WHILE(1) avec plusieurs BREAK
  {
    // Aller chercher si les tours du joueur actuel ont bougé
    c = echiquier[_rangee][7];
    tourPasBouge = (c.getPiece() == 'R') && (c.getJoueur() == _joueur);

    // Si la tour a bougé, on arrête le traitement
    if (!tourPasBouge)
    {
      break;
    }
    
    // Vérifier qu'il n'y a pas des pièces entre le roi et la tour
    passageLibre = true;
    for (short i = 5; i < 7; i++)
    {
      // Si le chemin a un obstacle, on arrête de vérifier le passage
      c = echiquier[_rangee][i];
      if (!c.isVide())
      { 
        passageLibre = false;
        break;
      }
    }

    // Si le passage est occupé, le roque ne peut avoir lieu, on arrête de le traiter
    if (!passageLibre)
    {
      break;
    }

    // Le roque ne peut avoir lieur si le roi est en échec
    if (echec(echiquier, false))
    {
      break;
    }

    // Si une pièce adverse menace une case sur le chemin entre la position actuelle et la destination du roi, le roque ne peut avoir lieu
    c = echiquier[_rangee][5];
    if (c.echec(echiquier, true))
    {
      break;
    }

    // Vérifie que le roi ne termine pas sur une case en échec
    c = echiquier[_rangee][6];
    if (c.echec(echiquier, true))
    {
      break;
    }

    // Rendu ici, toute les conditions de petit roque devrait être respectées
    // Ajout du mouvement à la liste d'actions possibles
    actionPossible[actions] = {_rangee, _colonne, _rangee, 6};
    actions++;
    break;
  }

  return actions;
}

// chercheMouvement = indique si l'echec est calculer pour un deplacement du roi ou non
// return true si le roi/case est en échec, false sinon
bool Case::echec(Case echiquier[8][8], bool chercheMouvement)
{
  short menaceX; // la rangé potentielle de la pièce qui menace d'un échec
  short menaceY; // la colonne potentielle de la pièce qui menace d'un échec
  Case menace;   // la case verifiee pour un echec sur le roi du joueur

  // vérifier les cases qui menace à 1 de distance autour de celle d'échec pour un pion ou un roi
  Serial.print("Echec deplacement 1 case :");
  short deplacement1[8][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}, {1, 0}, {0, 1}, {0, -1}, {-1, 0}};

  for (short i = 0; i < 4; i++)
  { 
    // les 4 première options sont les cases en diagonale à 1 de distance
    Serial.println(i);
    menaceX = _rangee + deplacement1[i][0];
    menaceY = _colonne + deplacement1[i][1];

    // Evite de traiter une case hors-jeu
    if (!inbounds(menaceX, menaceY)) 
    {
      continue;
    }

    // pièce du joueur adverse
    menace = echiquier[menaceX][menaceY];
    if (menace.getJoueur() == (_joueur * -1)) 
    {
      // le roi de l'adversaire
      if (menace.getPiece() == 'K')
      { 
        return true;
      }
      // un pion de l'adversaire
      else if (menace.getPiece() == 'P')
      { 

        // est-il en position de capture?
        if (menaceX == (_rangee + _joueur) &&
            (menaceY == (_colonne + 1) || menaceY == (_colonne - 1)))
        {
          return true;
        }
      }
    }
  }

  // Les 4 dernière options sont les cases de meme rangé ou colonne à 1 de distance
  for (short i = 4; i < 8; i++)
  { 
    Serial.println("i");
    menaceX = _rangee + deplacement1[i][0];
    menaceY = _colonne + deplacement1[i][1];

    // Évite de traiter une case hors-jeu
    if (!inbounds(menaceX, menaceY)) 
    {
      continue;
    }

    // le roi de l'adversaire est sur cette case
    menace = echiquier[menaceX][menaceY];
    if (menace.getJoueur() == (_joueur * -1) && menace.getPiece() == 'K') 
    {
      return true;
    }
  }

  // Vérifie les cases qui menace au position du cavalier
  Serial.print("Echec deplacement cavalier : ");
  short deplacement2[8][2] = {{-2, 1}, {-1, 2}, {1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}};

  for (short i = 0; i < 8; i++)
  {
    Serial.println(i);
    menaceX = _rangee + deplacement2[i][0];
    menaceY = _colonne + deplacement2[i][1];

    // Évite de traiter une case hors-jeu
    if (!inbounds(menaceX, menaceY)) 
    {
      continue;
    }

    // un cavalier du joueur adverse est sur cette case
    menace = echiquier[menaceX][menaceY];
    if (menace.getJoueur() == (_joueur * -1) && menace.getPiece() == 'N') 
    {
      return true;
    }
  }

  // Vérifie les cases qui menaces les diagonales partant de celle d'échec pour un fou ou une reine de l'adversaire
  // TODO Manque le pion
  Serial.print("Echec deplacement diago : ");
  for (short horizon = -1; horizon <= 1; horizon += 2)
  {
    for (short vertical = -1; vertical <= 1; vertical += 2)
    {
      for (short i = 1; i < TAILLE; i++)
      {
        Serial.print(horizon);
        Serial.print(vertical);
        Serial.println(i);
        menaceX = _rangee + horizon * i;
        menaceY = _colonne + vertical * i;
        
        // La case de menace est hors-jeu
        if (!inbounds(menaceX, menaceY)) 
        {
          break;
        }

        // Une pièce de l'adversaire menace potentiellement le roi
        menace = echiquier[menaceX][menaceY];
        if (menace.getJoueur() == (_joueur * -1)) 
        {
          // un fou ou une reine de l'adversaire a le roi en echec
          if (menace.getPiece() == 'B' || menace.getPiece() == 'Q')
          { 
            return true;
          }
          // Une piece adverse bloque le chemin menant a l'echec
          else
          {
            break; 
          }
        }
        // Une piece du joueur bloque le chemin
        // Si echec est vérifier pour une analyse de mouvement du roi du joueur actuelle, 
        // est-ce que la pièce qui block est ce roi? Si oui, continue à regarder la diagonale, 
        // car le roi aura été déplacer à la case en échec et donc ne bloquera plus la diagonale. 
        // Sinon une autre pièce ami block le chemin.
        else if (menace.getJoueur() == _joueur)
        { 
          // pièce ami block le chemin
          if (chercheMouvement && (menace.getPiece() != 'K'))
          {
            break; 
          }
          // Sinon, c'est le roi lui-meme et on continue à chercher
        }
      }
    }
  }

  // vérifier les cases qui menaces de la rangé et colonne de celle d'échec

  // Menace horizontal
  Serial.print("Echec deplacement horizontal ");
  for (short direction = -1; direction <= 1; direction += 2)
  {
    for (short i = 1; i < TAILLE; i++)
    {
      Serial.print(direction);
      Serial.println(i);
      menaceX = _rangee + direction * i;
      menaceY = _colonne;
      
      // La case de menace est hors-jeu
      if (!inbounds(menaceX, menaceY)) 
      {
        break;
      }
      
      // Une pièce de l'adversaire menace le roi
      menace = echiquier[menaceX][menaceY];
      if (menace.getJoueur() == (_joueur * -1)) 
      {
        // une tour ou une reine de l'adversaire a le roi en echec
        if (menace.getPiece() == 'R' || menace.getPiece() == 'Q')
        { 
          return true;
        }
        // Une piece adverse bloque le chemin menant a l'echec
        else
        {
          break; 
        }
      }

      else if (menace.getJoueur() == _joueur)
      { 
        // Une piece du joueur bloque le chemin
        // Si echec est vérifier pour une analyse de mouvement du roi du joueur actuelle,
        // est-ce que la pièce qui block est ce roi? Si oui, continue à regarder la diagonale,
        // car le roi aura été déplacer à la case en échec et donc ne bloquera plus la diagonale.
        // Sinon une autre pièce ami block le chemin.
        if (chercheMouvement && (menace.getPiece() != 'K'))
        {
          break;
        }
        // Sinon, c'est le roi lui-meme et on continue à chercher
      }
    }
  }

  // Menace vertical
  Serial.print("Echec deplacement vertical ");
  for (short direction = -1; direction <= 1; direction += 2)
  {
    for (short i = 1; i < TAILLE; i++)
    {
      Serial.print(direction);
      Serial.print(i);
      menaceX = _rangee;
      menaceY = _colonne + direction * i;

      // La case de menace est hors-jeu
      if (!inbounds(menaceX, menaceY)) 
      {
        break;
      }

      // Une pièce de l'adversaire menace potentiellement le roi
      menace = echiquier[menaceX][menaceY];
      if (menace.getJoueur() == (_joueur * -1)) 
      {
        // une tour ou une reine de l'adversaire a le roi en echec
        if (menace.getPiece() == 'R' || menace.getPiece() == 'Q')
        { 
          return true;
        }
        // Une piece adverse bloque le chemin menant a l'echec
        else
        {
          break; 
        }
      }
      else if (menace.getJoueur() == _joueur)
      { 
        // Une piece du joueur bloque le chemin
        // Si echec est vérifier pour une analyse de mouvement du roi du joueur actuelle,
        // est-ce que la pièce qui block est ce roi? Si oui, continue à regarder la diagonale,
        // car le roi aura été déplacer à la case en échec et donc ne bloquera plus la diagonale.
        // Sinon une autre pièce ami block le chemin.
        if (chercheMouvement && (menace.getPiece() != 'K'))
        {
          break;
        }
        // Sinon, c'est le roi lui-meme et on continue à chercher
      }
    }
  }

  return false;
}