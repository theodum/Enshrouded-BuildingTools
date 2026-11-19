# Couronne des Anciens — rapport architectural

Forteresse compacte de **45 × 25 × 42.5 m** (X × Y × Z), corps principal nominal
36 × 30 m, avec quatre tours octogonales de 8 m et un donjon de 10 × 10 m.
Le nord du plan correspond à +Z ; l'entrée est au sud (-Z). Le pivot du projet
est le sol du hall : X=0, Y=0, Z=0. Une cellule vaut 0,5 m.

## Composition et silhouette

Le hall central mesure 18 × 12 m et la salle arrière 18 × 10 m. Les ailes
secondaires de 9 m encadrent le corps royal et une cour ouverte de 18 × 8 m,
dont le donjon occupe partiellement l'extrémité. Les tours sont intégrées à
l'union des murs ; leurs jonctions intérieures restent ouvertes. Le parvis,
le portail en saillie et les loggias donnent de la profondeur à la façade sud.

Le grand comble à deux pans culmine à 18 m environ. Deux combles secondaires
laissent des galeries latérales de 2 m de largeur utile. Les tours nord portent
des couronnes crénelées ; les tours sud des belvédères couverts au-dessus de
leurs terrasses. Le donjon termine la silhouette par une couronne panoramique.
Tous les toits sont des peaux de voxels fermées, à redents de 0,5 m. Les combles
ne sont pas présentés comme des étages supplémentaires habitables.

## Niveaux

| Surface du sol | Programme |
|---|---|
| 0 m | Parvis, entrée traversante, grand hall, réception, offices, gardes, banquets, cour et accès au donjon |
| 4,5 m | Galerie partielle sur le hall, grande salle du trône, salons nobles, conseil, portraits, loggia et tours |
| 9 m | Archives, appartements communicants, bibliothèque ouest, trophées est, chambres, terrasse arrière et donjon |
| 13,5 m | Chemins de ronde des ailes, terrasse arrière haute, niveaux des quatre tours et donjon |
| 18 m | Sommet des quatre tours et niveau haut du donjon |
| 22,5 m | Terrasse panoramique du donjon |

## Escaliers et raccords

Le hall reçoit un escalier impérial : volée centrale de 3 m, palier de 6 m,
deux branches de 1,5 m, arrivée sur les deux côtés de la galerie à 4,5 m.
L'aile ouest possède deux escaliers en L (4,5→9→13,5 m), l'aile est un U
(4,5→9 m). Chaque tour possède quatre U superposés ; le donjon cinq.
Total : **25 escaliers**, 22 U, 2 L et un monumental.
Les U font 3,5 × 5,5 m, les L 5,5 × 5 m. Les marches montent de 0,5 m et ont
0,5 m de giron. Les paliers ont au moins 1,5 m de profondeur. Le profil exact
vient de `src/voxel_stairs.h`, compilé dans l'outil hors jeu.

La montée par cubes de 0,5 m peut demander des sauts : **la franchissabilité
reste à tester en jeu**. Aucun profil à 0,25 m ni forme native non prouvée
n'est prétendu disponible. Les trémies sont découpées sur les volumes d'air
réels, avec 2,5 m de hauteur libre pour les escaliers. Les soutiens massifs superposés sont ajourés
uniquement sous les tabliers, là où ils rencontreraient un autre dégagement.
Aucune marche ni aucun palier n'est supprimé pour faire passer un contrôle.

## Façades et intérieurs

Lancettes réellement ouvertes, meurtrières dans les tours, oculi de pignons,
contreforts à retraits, corniches, piédroits, archivolte et blason minéral.
Les sols ne bouchent ni les trémies ni les baies. La salle du trône conserve
un axe central dégagé, quatre colonnes à bases et chapiteaux, une estrade à
trois degrés et un trône construit en pierre. La bibliothèque possède des
rayonnages de blocs, à meubler si souhaité. Le bassin de cour est sec.
Les portes sont des ouvertures architecturales ; aucune porte interactive,
lumière, tapisserie, eau ou meuble natif n'est inclus.

## Matériaux

Les rôles maçonnerie, pierre de taille, toiture et décor du trône sont séparés
dans le générateur. Par défaut ils utilisent tous le **Bloc de pierre des Anciens**,
ID `0x7ad0559c`, octet `0x9d`, reconnu dans le journal natif dev7 local et déjà
construit par les exemples validés. Bloc de château, pierre raffinée et
obsidienne restent **non identifiés**, donc substitués explicitement.
L'aperçu coloré distingue les rôles, il ne représente pas les textures du jeu.
`Materiaux.json` donne les choix effectivement exportés. Pour une vraie palette,
fournir au générateur `--palette CONFIG.json`, avec pour chaque rôle voulu
un `blueprint` BTBP exporté du jeu et son `name`. Voir la configuration d'exemple
dans le dépôt. Le jeu revalide toujours les ID avant toute construction.
