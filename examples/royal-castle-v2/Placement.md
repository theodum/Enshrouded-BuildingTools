# Placement de la forteresse spacieuse V2

Projet à sélectionner : **Forteresse_Royale_Spacieuse_V2**.
Compatible avec la DLL **0.34.0-dev7** existante ; aucune nouvelle DLL nécessaire.

1. Installer tous les `FR2_*.btbp` et `Forteresse_Royale_Spacieuse_V2.btproject` directement dans
   `BuildingTools-Projects`, à côté de `enshrouded.exe`. Le ZIP possède déjà cette
   arborescence. La V1 et ses fichiers `FR_*.btbp` peuvent être conservés.
2. Prévoir un site de test libre d'environ **80 × 75 m**, avec **36 m** de
   hauteur libre et couverte. Emprise construite : 77 × 72 m ; volume d'air à
   valider : 77 × 34 × 72 m (X × Y × Z). Pivot au sol central, entrée vers -Z.
   Bornes occupées : [[-38.5, -0.5, -34.5], [38.5, 32.0, 37.5]]. Avec l'air :
   [[-38.5, -0.5, -34.5], [38.5, 33.5, 37.5]]. Le sol inférieur est à Y=-0,5 m.
3. Prendre le cube de **0,5 m**, ouvrir le sélecteur de matériaux dans la base,
   puis Blueprint → Placer → Projets composites. Choisir le projet V2 et
   positionner l'ensemble. Le placement natif expérimental reste inutile.
4. Vérifier les compteurs : **156888 cellules**, 30
   BTBP, 431 dégagements, zéro pièce native et zéro objet.
   Examiner `Plans.svg`, les coupes du hall et des tours et `Pieces.csv`.
5. Analyser la couverture des autels, notamment jusqu'à Y=33,5 m au-dessus du
   pivot pour les dégagements du donjon. Les contraintes natives sont conservées.
   Le nombre exact d'autels dépend de leurs volumes réellement disponibles.
6. Cliquer **Vérifier sans construire** et attendre la fin. Résoudre tout refus
   de collision, couverture, matériau ou ressources avant la construction.
7. Cliquer **Construire une seule fois**. Tous les secteurs sont assemblés
   automatiquement ; à 128 cellules, 1226
   lots théoriques. Le moteur peut réduire la taille des lots.

## Recette intérieure prioritaire

Tester les deux branches impériales, les L et U des ailes, puis chaque tour.
Vérifier les 2,5 m entre garde-corps, les paliers de 3 m, les arrivées à chaque
sol et la possibilité de tourner sans se coincer. Parcourir chaque pièce des
trois niveaux, la bibliothèque, les chambres, les trophées et les deux côtés
de l'estrade. Rejoindre les terrasses, les couronnes et le sommet du donjon.
Noter les sauts nécessaires, accrochages, angles étroits et éventuels obstacles.
Le confort réel de cette V2 reste à confirmer par ce test.

Échap, perte de focus ou erreur peuvent laisser un chantier partiel. Les lots
acceptés restent ; aucune reprise automatique garantie ni restauration totale
n'est fournie. Conserver les journaux et utiliser une copie de monde de test.
Le générateur et les tests hors jeu ne modifient aucune sauvegarde.
