# Livraison de la forteresse spacieuse V2 — 19 novembre 2026

La V1 a été testée en jeu par le joueur : extérieur apprécié, intérieurs trop
serrés. La V2 a été produite, vérifiée et copiée dans le dossier de projets du
jeu. **Son confort et ses marches n'ont pas encore été testés dans Enshrouded.**

## Résultat final

| Mesure | Résultat |
|---|---|
| Projet | `Forteresse_Royale_Spacieuse_V2.btproject` |
| Nom affiché | Forteresse Royale V2 — Grands intérieurs |
| Cellules / BTBP | 156 888 / 30 |
| Dimensions X × Y × Z | 77 × 32,5 × 72 m |
| Volume à valider avec air | 77 × 34 × 72 m |
| Tours / donjon | 16 m de diamètre / 18 × 18 m |
| Sols principaux | 0 / 5 / 10 m |
| Escaliers | 27 : 1 monumental, 2 L, 24 U |
| Largeur utile / palier / air réservé | 2,5 m / 3 m / 3,5 m |
| Volée centrale impériale | 5 m |
| Réservations natives / colonnes | 431 / 7 986 |
| Pièces et destinations reliées | 62 / 62 |
| Éléments intérieurs / allées protégées | 142 / 32 |
| Graphe des surfaces / composante d'entrée | 46 918 / 46 918 |
| Empreinte du second contrôle | 1,5 × 1,5 m, 3 m d'air |
| Chevauchement physique entre composants | 0, même matériau inclus |
| Intersection entre corps d'escaliers / soutien découpé | 0 / 0 |
| Lots théoriques de 128 cellules | 1 226 |

Les 52 composantes de sols larges utilisées pour les pièces et les paliers sont
raccordées. Les deux petites composantes supplémentaires sont les intérieurs
des bassins décoratifs, explicitement exclus des chemins larges. Aucun palier
ni aucune pièce n'est exclu. Les niches de murs, appuis, corniches et toitures
ne servent pas de raccourcis au graphe de circulation.

## Contrôles exécutés

Commande : `bash tools/test_royal_castle.sh`, avec l'exécutable local fourni en
lecture seule via `ENSHROUDED_EXE`. Journal : `build/royal-castle-v2-tests.log`.

- Core, langues, 40 signatures uniques et contrôles de réflexion/ABI : réussis.
- Composites : 145 203 assertions ; actions : 209 ; matériaux : 209 ; étages :
  28 220 ; escaliers voxels : 130 749 ; Extended : 2 457 ; configuration : 16.
  Aucun échec. Export C++ : aucun échec.
- Fixtures dev7 : 59 fichiers reproduits, 8 068 cellules. Exemples Python :
  4 tests ; château V1 : 8 tests ; château V2 : 11 tests. Tous réussis.
- V2 : reproduction exacte des 30 BTBP et du manifeste, grille 0,5 m,
  dépendances complètes, budgets et relecture par `core::bp_decode`,
  `project::add/finish`, `spaces::validate` : réussis.
- Les quatre rotations, lots 1/32/128/256 et interruptions de la machine d'état
  sont contrôlés par l'hôte C++. Aucun paiement ou placement du monde n'est émulé.
- L'hôte V2 compilé avec AddressSanitizer et UndefinedBehaviorSanitizer relit
  le projet sans diagnostic. Plan SVG : XML valide.
- Archive finale : CRC, SHA-256, disposition, références et octets comparés
  aux sources. Les dernières modifications de vues n'ont pas changé la géométrie.

Les onze tests V2 couvrent le dimensionnement, chaque accès de pièce, le refus
de composants identiques superposés, l'interdiction du mobilier dans les cages
vides, les volumes de tous les éléments intérieurs, une allée obstruée, une
marche retirée, un plafond trop bas, le virage de terrasse trop étroit, une
palette de quatre rôles distincts, le non-écrasement et les octets exportés.

## Installation réalisée

Les 30 BTBP `FR2_*.btbp`, puis le manifeste, ont été copiés directement dans :

```text
C:\Program Files (x86)\Steam\steamapps\common\Enshrouded\BuildingTools-Projects
```

Les 31 fichiers ont été vérifiés par SHA-256. La V1 reste présente. Le reçu
local est `build/royal-castle-v2-installation.json`. Aucun fichier de sauvegarde
n'a été lu ou modifié et aucune construction n'a été déclenchée dans le monde.
La DLL dev7 reste inchangée, SHA-256 :
`aa7109f61a59d0c814b0a06357b594a736eb083da312875ac5acf2f40380fb59`.

Archive : `dist/Forteresse_Royale_Spacieuse_V2-dev7.zip`.
SHA-256 final :

```text
0fa61a2c139f695ff325d25d66f9af8c8a57497e1295156ce0d118446a8faa9a
```

## Recette restant à faire

Suivre `examples/royal-castle-v2/Placement.md`, avec une zone libre et couverte
d'environ 80 × 75 m et 36 m de hauteur. Contrôler les autels et les ressources
avec **Vérifier sans construire** avant le chantier. Parcourir les 27 escaliers,
les paliers, toutes les pièces et les couronnements, puis sauvegarder/recharger
le monde de test et recommencer. La capsule, les sauts nécessaires sur les
marches de 0,5 m, la stabilité et les collisions réelles restent hors du modèle.

La Pierre des Anciens reste la substitution explicite disponible. Le mobilier
en voxels n'est pas présenté comme un lit, un atelier ou un objet natif utilisable.
La reprise automatique et le rollback complet restent indisponibles dans dev7.
