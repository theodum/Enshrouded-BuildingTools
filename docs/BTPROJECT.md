# Format `.btproject` — version 1

Manifeste JSON UTF-8 sans BOM. Les anciens `.btbp` ne changent pas de format.
Tous les fichiers référencés sont des `.btbp` placés dans le même répertoire
`BuildingTools-Projects`, à côté de `enshrouded.exe`.

```json
{
  "format": "BuildingToolsExtendedProject",
  "version": 1,
  "name": "Citadelle des Cimes",
  "gridSize": 0.5,
  "anchor": [0, 0, 0],
  "modules": [
    { "file": "hall.btbp", "offset": [0, 0, 0], "rotation": 0 },
    { "file": "tower.btbp", "offset": [8, 1, 0], "rotation": 90 },
    { "file": "bridge.btbp", "offset": [2, 2, 5], "rotation": 270 }
  ]
}
```

## Coordonnées et rotations

X et Z sont les axes horizontaux ; **Y est l'altitude**. Tous les nombres de
position sont exprimés en mètres, jamais en nombre de cellules. `gridSize`
décrit la grille d'édition du projet (0,5 à 4 m, multiple de 0,5) et ne multiplie
pas les offsets. Chaque `.btbp` conserve sa propre grille, également multiple
de 0,5 m, entre 0,5 et 4 m par axe. Les cellules sources plus grandes sont
subdivisées en voxels cubiques natifs de 0,5 m ; pas de rééchantillonnage flottant.

`offset` désigne le **coin inférieur du volume du module après sa rotation
locale**, dans les axes du projet. Les rotations locales suivent `bp_turn` :
un quart de tour envoie un point `(x,z)` du module vers `(profondeur-z,x)`.
Le module reste donc dans un volume local positif, puis son offset est ajouté.

`anchor` est le pivot du projet dans ces mêmes coordonnées locales. La rotation
globale se fait autour de ce pivot, sans recentrage automatique sur la boîte :

```text
centre monde = ancrage monde + R_global(centre module tourné + offset - anchor)
R_90(x,y,z)  = (-z,y,x)
```

Dans la fenêtre native, l'ancrage monde est le coin inférieur de la cellule
visée, déplacé par les réglages de position (pas de 0,5 m). Le bouton « Placer ici »
le fixe ; les flèches de déplacement et la hauteur déplacent tout le projet.
L'orientation de la caméra n'affecte que le sens des boutons droite/avant,
jamais les offsets X/Y/Z du manifeste. Pas de miroir composite dans cette version.

Le plan stocke des seizièmes de mètre entiers ; la position finale utilise les
entiers fixes 32.32 déjà employés par le mod. Pas d'accumulation d'erreurs entre
modules ou lots. Les écritures natives utilisent ensuite les floats du moteur,
comme le placement original.

## Validation et limites

- Tous les champs de l'exemple sont requis ; `rotation` est optionnelle dans
  un module (0 par défaut). Champs inconnus, doublons JSON et versions futures
  sont refusés. Rotations : 0, 90, 180 ou 270 degrés.
- Maximum 128 modules, manifeste de 128 Kio, nom UTF-8 de 191 octets,
  nom de fichier de 191 octets, chemin Windows complet inférieur à MAX_PATH.
- Seulement des noms de fichiers `.btbp` simples : pas de sous-dossier,
  chemin absolu, `..`, lecteur ou flux NTFS.
- Chaque blueprint source reste limité à 65 536 cellules et 64 matériaux.
  Le projet accepte 1 048 576 cellules occupées après subdivision, avant
  élimination des doublons, et 64 couples item/matériau sauvegardé distincts.
- Offsets et pivot : multiples de 0,5 m. Décimaux ordinaires, pas d'exposant.
  Limite locale de ±65 536 m, vérifiée aussi après expansion ; les centres
  monde doivent rester dans ±65 536 m (garde propre à cette version).
- Des cellules superposées du même matériau sont comptées une seule fois ;
  des matériaux différents au même emplacement bloquent tout le chargement.
- Ordre déterministe Y, Z, X du plan ; les modules conservent leur origine et
  rotation globales identiques pour chaque lot.

Le plan est figé en mémoire au chargement. Modifier un fichier pendant le
placement ne modifie pas l'opération en cours ; recharger pour prendre en compte
les modifications. Aucun manifeste ne peut exécuter du code ou écrire une sauvegarde.

## Exécution native

Deux passes automatiques : vérifier toutes les cellules sans écriture, puis
construire. Chaque lot est revérifié juste avant sa tarification. La couverture
utilise la fonction native `BaseAt` sur les huit coins intérieurs de chaque voxel,
en trois dimensions, à partir du système de bases lu par le jeu. Les collisions
de blocs existants sont détectées par le retrait à blanc déjà utilisé pour sauver
les blueprints. Les prix viennent du `PlaceShape` natif et le paiement reste natif.

La première passe refuse les ressources manquantes ou inconnues pour le projet
entier. Le mode créatif existant ignore les coûts comme auparavant. Chaque lot
comporte au plus `min(MaxShapes, 256)` voxels. Les confirmations utilisateur ne
sont pas répétées ; le mod réutilise les clics natifs synthétiques d'Exact mode.

Sur échec, aucune nouvelle tâche n'est envoyée ; `BuildingTools-Projects.log`
enregistre ancrage, rotation, progression et résultat des lots. Le journal est
diagnostique, pas un fichier de reprise automatique. Une construction commencée
est également arrêtée si le journal ne peut pas être écrit. Une opération commencée
peut rester partielle ; les matériaux d'un lot déjà payé peuvent avoir été
consommés même si son écriture échoue. Pas de rollback ni de remboursement
supplémentaire inventé. L'annulation native conserve ses limites d'origine.

## Limites de la première intégration

Blocs échantillonnés uniquement ; objets natifs des composites omis et signalés,
sans modifier le placement d'objets des blueprints classiques. La subdivision
ne recrée pas des escaliers, toitures ou objets natifs absents du contenu voxel.
Collisions d'entités/meubles et prévisualisation des matériaux non implémentées.
Le moteur peut encore refuser une cellule malgré le précontrôle.

L'aperçu global réutilise Vulkan : cellules translucides pour un volume englobant
d'au plus 40 000 voxels, boîte englobante au-delà. Pas encore de maillage simplifié
par module, de surbrillance des collisions ou de carte des zones d'autels.
Uniquement dans un monde hébergé localement, avec clavier/souris ; pas de serveur
dédié ni de placement automatique d'autels. La grille virtuelle à altitude fixe,
le créatif étendu aux objets et les composants architecturaux sont des étapes
ultérieures ; Free Placement existant permet déjà de viser dans le vide.
