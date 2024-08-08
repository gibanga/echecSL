<p align="center">
<img src="Images/echec_et_robotique.png" alt="Logo Échecs et Robotique" class="center">
</p>

# Échecs magnétiques

Ce jeu d'échecs assistent les joueurs en présentant les cases où peuvent se déplacer les pièces soulevées. 

<p align="center">
<img src="Images/overview.jpg" width="300" class="center">
</p>

## Indicateurs

En cours de jeu, les cases vont changer de couleur en fonction des actions prises par les joueurs.
- Les cases vertes indiquent les déplacements valides d'une pièces.
- La case bleue indique la case d'origine d'une pièce soulevée. 
- Une case rouge indique qu'une pièce soulevée ne peut être déplacée, soit parce qu'elle n'appartient pas au joueur actif ou qu'une autre pièce est déjà soulevée.
- Une case jaune indique que la pièce sur la case à une action spéciale tel qu'un roque ou une promotion.
</div>

<p align="center">
<img src="Images/action.png" width="400">
</p>

## Sous la surface

Sous chaque case se trouvent un interrupteur reed et une DEL adressable d'un ruban de DEL. Ceux-ci permettent l'échange d'informations entre l'ESP-32 et les joueurs lorsqu'une pièce magnétisée est manipulée.

<img src="Images/squelette_echec.png" >

Avec ses deux coeurs, l'ESP-32 peut lire l'état des interrupteurs via des multiplexeurs via son premier coeur et faire rouler le jeu avec son deuxième coeur.

## Expansions

Deux boutons et un écran sont intégrés près de la surface de jeu et pourront permettre l'ajout de fonctionnalités supplémentaires


## Remerciements

Merci à Samuel, mon partenaire, pour avoir conçu et assemblé la structure et la partie électronique. <br />
Merci au département d'Internet et Robotique du cégep de Saint-Laurent de nous avoir donné l'opportunité de travailler sur ce projet. <br />
Finalement, merci à nos proches et à nos mentors pour leur support en tout genre.


## Notes

Le projet, jeu d'échecs V1, a été réalisé dans le cadre des cours 243-556-SL et 243-658-SL du cégep de Saint-Laurent.
