# Forteresse Royale V2 — grands intérieurs

La V1 a été parcourue en jeu par le joueur : l'extérieur est apprécié, mais les
tours, les escaliers et les circulations intérieures sont trop serrés. Cette V2
reprend la composition royale et la redessine pièce par pièce. **La V2 n'a pas
encore été testée en jeu.** Tous les blocs restent des cubes de **0,5 m**.

## Dimensions et confort prévu

| Élément | V1 | V2 |
|---|---|---|
| Emprise extérieure | 45 × 42,5 m | 77 × 72 m |
| Corps royal et ailes | 36 × 30 m | 56 × 48 m |
| Tours d'angle octogonales | 8 m | 16 m |
| Donjon | 10 × 10 m | 18 × 18 m |
| Hall central | 18 × 12 m | 28 × 20 m |
| Salle du trône | 18 × 10 m | 28 × 14 m |
| Hauteur d'étage | 4,5 m | 5 m |
| Largeur des volées secondaires | 1,5 m | 2,5 m utiles, garde-corps à l'extérieur |
| Profondeur des paliers | 1,5 m | 3 m, avec approches de 2,5 m |
| Hauteur libre réservée aux escaliers | 2,5 m | 3,5 m |

Dimensions construites X × Y × Z : **77 × 32.5 × 72 m**. Les sols principaux sont
à 0 / 5 / 10 m. Les galeries de toiture sont à 15 m, les couronnes des tours à
20 m, et le donjon possède des sols jusqu'à 30 m. Son couronnement monte à 32 m.
Le volume à contrôler, air compris, mesure **77 × 34 × 72 m**.

## Implantation des pièces

Le hall à double hauteur reçoit la volée impériale de 5 m, ses deux branches de
2,5 m et une galerie large sur les côtés. Les arcades et colonnes sont placées
hors des débouchés. La réception et le trône occupent les grandes salles arrière.
La salle du trône conserve une nef libre de 6 m, des colonnes séparées de
l'estrade et trois degrés de 1,5 m de giron avant le plateau. Deux accès latéraux
au plateau sont contrôlés.

Les ailes possèdent des galeries de circulation avec **3 m d'allée protégée**.
Les pièces avant et arrière mesurent 9,5 × 16 m nominaux ; les vestibules
d'escalier centraux mesurent 14 × 16 m. Les portes ordinaires font 3 m, les
principales 4 à 6 m. Chaque salle a son axe d'accès, ses zones d'aménagement et
un point de contrôle situé dans son propre volume. Les bibliothèques gardent
leurs rayonnages sur les côtés, les salles de conseil et de banquet placent
leurs tables après l'allée transversale, et les trophées restent en périphérie.
Les appartements disposent d'alcôves et de cabinets en retrait.

Les cages des tours sont décalées vers l'extérieur du corps royal : leurs volées
n'occupent pas les salles des ailes. Les tours possèdent des pièces sur le côté,
des banquettes et des paliers transversaux. Le donjon sépare sa cage de sa grande
allée de 3 m et de ses banquettes. La terrasse a été avancée pour dégager le
passage devant le donjon. La cour conserve son axe central entre deux bassins secs.

## Structure des escaliers

27 parcours : un monumental, deux L, vingt-quatre U. Les profils
proviennent de `voxel::prepare` en production : dix contremarches de 0,5 m,
giron de 0,5 m. Les volées sont des bandes de pierre de 1 m d'épaisseur, reliées
aux paliers ; les U ont un noyau porteur continu et des garde-corps extérieurs.
Les soutiens pleins empilés de la V1 ont été remplacés. **Aucun soutien n'est
découpé et aucun tablier n'est supprimé pour loger une autre volée.** Les cages,
les corps physiques des escaliers et leurs dégagements sont vérifiés séparément.

Les marches de 0,5 m peuvent toujours demander des sauts dans Enshrouded. La
grille, les dégagements et les graphes ne certifient pas la marche du personnage.

## Aménagement et limites

142 éléments intérieurs sont construits en voxels : bases,
colonnes, chapiteaux, rayonnages, tables, alcôves, banquettes, présentoirs, estrade
et trône. Ils sont décoratifs : aucun lit utilisable, atelier fonctionnel, feu,
livre, trophée natif ou porte interactive n'est prétendu placé. Le mobilier en
blocs peut servir de base à un ameublement ultérieur dans le jeu.

La Pierre des Anciens confirmée localement reste la substitution explicite pour
la maçonnerie, la taille, les toitures et le décor sombre. Une vraie palette peut
être fournie via `--palette` et des petits BTBP exportés du jeu. Les couleurs
des vues distinguent les rôles architecturaux, elles ne simulent pas les textures.

## Inventaire vérifié pièce par pièce

La surface libre ci-dessous est l'aire des colonnes disposant d'un sol et de 3 m
d'air ; elle ne constitue pas une mesure de surface habitable réglementaire.
Tours et donjon incluent leurs circulations ; l'estrade est une destination dans
la salle du trône. `Pieces.csv` et `Circulation.json` donnent les accès et éléments.

| N° | Pièce / destination | Sol (m) | Surface nominale (m²) | Sol libre avec 3 m d'air (m²) | Éléments |
|---|---|---|---|---|---|
| 1 | Parvis royal | 0 | 162 | 143 | 0 |
| 2 | Grand hall impérial | 0 | 560 | 453.5 | 12 |
| 3 | Réception et antichambre | 0 | 392 | 350.5 | 0 |
| 4 | Cour des arcades | 0 | 302 | 290 | 0 |
| 5 | Galerie impériale ouest | 5 | 120 | 93.75 | 6 |
| 6 | Galerie impériale est | 5 | 120 | 93.75 | 6 |
| 7 | Grande salle du trône | 5 | 392 | 283.5 | 19 |
| 8 | Estrade royale | 6.5 | 14 | 9.5 | 4 |
| 9 | Grande bibliothèque des archives | 10 | 560 | 497 | 24 |
| 10 | Appartement royal ouest | 10 | 189 | 156.75 | 2 |
| 11 | Appartement royal est | 10 | 189 | 156.75 | 2 |
| 12 | Salle des gardes | 0 | 152 | 130 | 3 |
| 13 | Salle des banquets | 0 | 152 | 116.75 | 3 |
| 14 | Grand vestibule escalier -1/0 | 0 | 224 | 181.25 | 0 |
| 15 | Salle des armes | 0 | 152 | 130 | 3 |
| 16 | Cuisine et offices | 0 | 152 | 120.75 | 3 |
| 17 | Grand vestibule escalier 1/0 | 0 | 224 | 169.75 | 0 |
| 18 | Salon noble | 5 | 152 | 124 | 3 |
| 19 | Salle du conseil | 5 | 152 | 116.75 | 3 |
| 20 | Grand vestibule escalier -1/5 | 5 | 224 | 177.75 | 0 |
| 21 | Salon des hôtes | 5 | 152 | 124 | 3 |
| 22 | Galerie des portraits | 5 | 152 | 125.75 | 2 |
| 23 | Grand vestibule escalier 1/5 | 5 | 224 | 166.75 | 0 |
| 24 | Chambre des hôtes ouest | 10 | 152 | 121 | 3 |
| 25 | Bibliothèque de lecture | 10 | 152 | 125.75 | 8 |
| 26 | Grand vestibule escalier -1/10 | 10 | 224 | 180 | 0 |
| 27 | Chambre des hôtes est | 10 | 152 | 121 | 3 |
| 28 | Salle des trophées | 10 | 152 | 116.75 | 6 |
| 29 | Grand vestibule escalier 1/10 | 10 | 224 | 168.5 | 0 |
| 30 | Tour SO — salle 0 m | 0 | 199 | 122.75 | 1 |
| 31 | Tour SO — salle 5 m | 5 | 199 | 119.75 | 1 |
| 32 | Tour SO — salle 10 m | 10 | 199 | 119.75 | 1 |
| 33 | Tour SO — salle 15 m | 15 | 199 | 119.75 | 1 |
| 34 | Tour SO — salle 20 m | 20 | 199 | 135.75 | 0 |
| 35 | Tour SE — salle 0 m | 0 | 199 | 122.75 | 1 |
| 36 | Tour SE — salle 5 m | 5 | 199 | 119.75 | 1 |
| 37 | Tour SE — salle 10 m | 10 | 199 | 119.75 | 1 |
| 38 | Tour SE — salle 15 m | 15 | 199 | 119.75 | 1 |
| 39 | Tour SE — salle 20 m | 20 | 199 | 135.75 | 0 |
| 40 | Tour NO — salle 0 m | 0 | 199 | 122.75 | 1 |
| 41 | Tour NO — salle 5 m | 5 | 199 | 119.75 | 1 |
| 42 | Tour NO — salle 10 m | 10 | 199 | 119.75 | 1 |
| 43 | Tour NO — salle 15 m | 15 | 199 | 119.75 | 1 |
| 44 | Tour NO — salle 20 m | 20 | 199 | 152 | 0 |
| 45 | Tour NE — salle 0 m | 0 | 199 | 122.75 | 1 |
| 46 | Tour NE — salle 5 m | 5 | 199 | 119.75 | 1 |
| 47 | Tour NE — salle 10 m | 10 | 199 | 119.75 | 1 |
| 48 | Tour NE — salle 15 m | 15 | 199 | 119.75 | 1 |
| 49 | Tour NE — salle 20 m | 20 | 199 | 152 | 0 |
| 50 | Donjon — chambre 0 m | 0 | 324 | 220 | 2 |
| 51 | Donjon — chambre 5 m | 5 | 324 | 213 | 2 |
| 52 | Donjon — chambre 10 m | 10 | 324 | 217 | 2 |
| 53 | Donjon — chambre 15 m | 15 | 324 | 217 | 2 |
| 54 | Donjon — chambre 20 m | 20 | 324 | 213 | 2 |
| 55 | Donjon — chambre 25 m | 25 | 324 | 213 | 2 |
| 56 | Donjon — chambre 30 m | 30 | 324 | 262 | 0 |
| 57 | Terrasse arrière 10 m | 10 | 330 | 266.5 | 7 |
| 58 | Terrasse arrière 15 m | 15 | 330 | 281 | 0 |
| 59 | Chemin de ronde -1 | 15 | 240 | 197.75 | 0 |
| 60 | Chemin de ronde 1 | 15 | 240 | 197.75 | 0 |
| 61 | Loggia royale 5 m | 5 | 60 | 40 | 0 |
| 62 | Loggia royale 10 m | 10 | 60 | 49.5 | 0 |
