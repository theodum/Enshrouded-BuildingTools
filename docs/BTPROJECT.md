# Format `.btproject` — version 1

Dev12 ajoute deux champs de module optionnels : `content` (`all` par défaut,
ou `objects`) et `objectPlacement` (`checked` par défaut, ou `blueprint`).
Le mode `blueprint` exige `content=objects` et un projet indépendant constitué
uniquement de ces modules, sans pièces natives ni espaces de structure.
Le BTBP source reste intact : ses voxels servent aux contacts capturés mais ne
sont pas construits. Les anciens projets gardent leurs règles par défaut ;
une DLL antérieure à dev12 refuse les nouveaux champs. Voir
[DEV12-CLASSIC-COMPOSITIONS.md](DEV12-CLASSIC-COMPOSITIONS.md) pour les garanties,
limites de propriétés, admission native et export par références originales.

Dev10 conserve ce format et les entrées d'objets BTBP v1. Un fichier séparé
`.btcalibration` garde les mesures natives d'une capture classique pour préparer
un nouveau brouillon ; il ne se substitue ni au module BTBP ni à la résolution
du registre actuel. L'échelle non unitaire reste refusée, le BTBP ne la stockant
pas. Voir [DEV10-NATIVE-PLACEMENT-FIX.md](DEV10-NATIVE-PLACEMENT-FIX.md).

Dev9 vérifie chaque ID d'objet dans le registre actuel **au chargement** et
refuse les références non résolues avec leur motif. L'export uniquement mobilier
conserve l'ancre et les transforms, sans voxel, pièce native ni espace structurel.
Les recettes d'auteur acceptent les positions décimales et quaternions précis ;
voir [DEV9-CATALOG.md](DEV9-CATALOG.md). Aucun changement de format BTBP n'est requis.

Dev8 réutilise les objets des BTBP existants : aucun champ JSON `objects` et
aucun nouveau format binaire. Un module BTBP peut contenir uniquement des objets
(toutes les cellules vides, palette vide) ; le composite peut donc placer du
mobilier avec **zéro voxel et zéro pièce native**. Identifiant, position en mètres
et quaternion sont conservés. Les voxels sont traités avant les objets et les
supports meubles avant les décorations. Voir [DEV8-NATIVE-OBJECTS.md](DEV8-NATIVE-OBJECTS.md)
pour le catalogue réel, les contraintes et les limites de propriétés copiées.

Manifeste JSON UTF-8 sans BOM. Les anciens `.btbp` ne changent pas de format.
Tous les fichiers référencés sont des `.btbp` placés dans le même répertoire
`BuildingTools-Projects`, à côté de `enshrouded.exe`.

Depuis dev7, un champ optionnel `spaces` conserve les dégagements des escaliers
voxels. Les anciens projets sans ce champ restent compatibles avec dev7. Les
anciennes DLL au lecteur strict refusent les nouveaux fichiers qui le contiennent.
Ne pas retirer ce champ pour contourner un contrôle de passage.

```json
"spaces": [
  {"low": [0, 0.5, 0], "high": [2, 2.5, 1]}
]
```

Les bornes sont locales au projet, avant rotation globale, en mètres et sur la
grille de 0,5 m. Le volume est semi-ouvert. Sa hauteur libre doit être comprise
entre 2 et 8 m, son empreinte au plus 4 096 colonnes de 0,5 m. Limites : 512
volumes et 65 536 colonnes cumulées. Le volume doit être libre dans le plan et
chaque colonne doit avoir un voxel de passage immédiatement dessous. Le moteur
vérifie ensuite l'air du monde par cette surface, sans créer ni facturer l'air.
L'export conserve ces données ; il omet le champ si aucun espace n'est réservé.
Voir [DEV7-VOXEL-STAIRS.md](DEV7-VOXEL-STAIRS.md).

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
- Maximum 128 modules, manifeste de 512 Kio, nom UTF-8 de 191 octets,
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
comporte au plus `min(MaxShapes, ProjectBatchCells)` voxels : 128 par défaut,
réglable jusqu'à 256. Dev3 réduit ce plafond pendant l'opération si un contrôle
dépasse 50 ms ; la géométrie et l'ordre ne changent pas. Les confirmations ne
sont pas répétées ; le mod réutilise les clics natifs synthétiques d'Exact mode.

Sur échec, aucune nouvelle tâche n'est envoyée ; `BuildingTools-Projects.log`
enregistre ancrage, rotation, progression et résultat des lots. Le journal est
diagnostique, pas un fichier de reprise automatique. Une construction commencée
est également arrêtée si le journal ne peut pas être écrit. Une opération commencée
peut rester partielle ; les matériaux d'un lot déjà payé peuvent avoir été
consommés même si son écriture échoue. Pas de rollback ni de remboursement
supplémentaire inventé. L'annulation native conserve ses limites d'origine.

## Extensions facultatives de dev2

Un module peut avoir une `category` : `general`, `wall`, `floor`, `roof`, `tower`,
`stairs` ou `furniture`. Elle permet de cibler un remplacement de matériau.
Les anciens manifests sans cette propriété restent lisibles.

Le tableau facultatif `pieces` ajoute jusqu'à 2 048 éléments natifs. Chaque entrée :

| Champ | Signification |
|---|---|
| `item` | Identifiant entier uint32 non nul, lu dans la bibliothèque native ; obligatoire |
| `offset` | Position du pivot natif en mètres, trois axes ; obligatoire |
| `rotation` | 0, 90, 180 ou 270 degrés ; 0 par défaut |
| `material` | Identifiant d'un vrai matériau de construction ; sinon matériau du marteau capturé au début du contrôle |
| `clearance` | Dégagement vertical à vérifier, 0 à 8 m ; 0 par défaut, 2 m pour les escaliers générés |

Le pivot d'une pièce native est celui de son modèle, différent du coin inférieur
d'un module BTBP. Le générateur mesure les limites réelles du modèle et calcule
le décalage nécessaire. Les manifests ne peuvent pas inventer une géométrie : le
registre doit reconnaître un élément de construction et les appels natifs doivent
accepter son coût. Les autels sont exclus. Aucun exemple d'identifiant d'escalier
n'est inventé ; sélectionner l'élément réel puis exporter fournit les bons nombres.
Un projet composé uniquement de `pieces` peut avoir `modules: []`.

Les objets présents dans les BTBP sont transformés avec leur quaternion d'origine.
Maximum 65 536 objets chargés. L'option Inclure les objets décide de leur placement.
Leurs modèles et initialisations passent par PlaceProp ; les objets spéciaux,
PNJ/animaux, objets à déverrouillages, PIDE, usines et objets liés à la logique
sont refusés dans les composites. Les stocks des objets ne sont pas dans le cache
des blocs : leur paiement natif est contrôlé par lot, sans garantie globale de stock.

L'ordre de construction est blocs, pièces natives, puis objets compatibles.
Les lots sont limités à 256 voxels, une pièce native ou quatre objets, et à MaxShapes
si cette valeur est inférieure. Le plafond voxel par défaut est 128. La couverture des objets utilise leurs limites
natives tournées. Les pièces natives utilisent leurs volumes et leur dégagement ;
les chevauchements avec les blocs du projet sont refusés avant la première écriture.
Les cellules et pièces identiques sont dédupliquées. Le compteur de progression
compte des placements, pas les voxels intérieurs d'une pièce native.

Dev3 désactive par défaut l'automatisation des pièces et objets. L'activation
expérimentale se fait dans Home, avec explication des limites. La réflexion
ItemInfo/EquipmentSetup et tous les drapeaux natifs sont contrôlés avant d'utiliser
ces voies. Les volumes sont bornés à 4 096 sondes voxel par pièce (dégagement inclus)
et 512 par objet. Les intersections avec les blocs du plan utilisent un index
Y/Z/X ; les limites englobantes rendent ces contrôles conservateurs. La physique
de toutes les entités n'est pas interrogée.

## Aperçu, analyse et export

L'aperçu réutilise Vulkan : cellules translucides pour un volume englobant d'au
plus 40 000 voxels, volumes de modules au-delà, limites des pièces natives et
repères d'objets. Pas de textures réelles ni de rendu du modèle complet des meubles.
Les lignes sont regroupées en instances GPU, avec un budget de 4 096 segments.
Les filtres de visibilité et d'altitude concernent seulement l'affichage ; tous
les modules seront construits. Vérifier sans construire effectue la passe native
et revient à l'aperçu, avec le premier voxel problématique marqué en rouge.

L'assistance aux autels analyse les zones et sous-zones natives, en 3D, puis
propose jusqu'à 64 positions modifiables à partir des dimensions observées.
Les blocs sont tous examinés ; neuf échantillons par pièce/objet inclus servent
à estimer leur couverture. Ce calcul progressif ne remplace pas la validation
native avant construction.
Ces zones théoriques ne vérifient pas progression, terrain, quota ou permission
de créer un nouvel autel. Aucun autel n'est placé ni supprimé.

Enregistrer le projet écrit uniquement de nouveaux fichiers. Le plan est découpé
en tuiles BTBP de 32³ cellules (16 m de côté), par module et catégorie. Les objets
sont répartis en fichiers de 512 objets. La limite de 128 fichiers sources reste
appliquée. Les coûts indicatifs des tuiles exportées peuvent être inconnus ; le
projet recalcule ses prix natifs avant construction. Le manifeste garde les pièces
natives et est publié en dernier par renommage sans remplacement. Un échec peut
laisser de nouveaux fichiers incomplets sans publier de projet terminé ; aucun
fichier existant n'est écrasé. L'export n'enregistre aucun état de coffre ou de quête.

Le journal dev3 identifie chaque opération et son empreinte FNV64 diagnostique,
sans valeur cryptographique. Diagnostics permet d'écrire un nouveau rapport TXT
avec ancrage/rotation, budgets et préfixe acquitté. L'export peut répartir un module
en plusieurs tuiles/catégories et regrouper les objets dans des sources distinctes ;
positions, matériaux, pivots et rotations sont conservés, pas les indices de modules.

Uniquement dans un monde hébergé localement, clavier/souris. Les objets disposent
de contrôles conservateurs utilisant les AABB natives de placement ; les meshes
physiques exacts ne sont pas testés. Les fantômes texturés, une spirale arrondie
vérifiée, la reprise de chantier et le rollback complet restent indisponibles. Le moteur peut encore
refuser un placement après contrôle. Voir `EXTENDED-STATUS.md` pour les limites
et le protocole de validation en jeu.
