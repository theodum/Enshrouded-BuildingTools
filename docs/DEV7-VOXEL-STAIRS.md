# Dev7 — escaliers en voxels et recette dans Enshrouded

Version : `0.34.0-dev7`. Cette alternative n'utilise que des cellules composites
de 0,5 m. **Aucune pièce native expérimentale n'est nécessaire.** Les fonctions
natives ordinaires de prix, couverture et construction restent utilisées comme
pour les projets de blocs validés en dev5.

## Capacités et limites réelles

| Paramètre | Comportement |
|---|---|
| Tracé droit | Deux portions séparées par un palier intermédiaire ; palier de départ et d'arrivée |
| En L | Quart de tour sur un palier ; le groupe entier est orientable |
| En U | Volée de retour parallèle ; séparation de 0,5 m entre les deux volées |
| Monumental | Volée centrale deux fois plus large que le paramètre largeur, grand palier, deux branches symétriques |
| Hauteur totale / nombre de contremarches | La montée vaut exactement hauteur / nombre ; même nombre de contremarches sur chaque parcours monumental |
| Largeur / profondeur des marches | Multiples de 0,5 m ; pas d'arrondi silencieux |
| Paliers | Profondeur au moins égale à la largeur d'une branche/volée ; départ et arrivée exactement aux altitudes prévues |
| Soutiens | Remplissage plein, ou tabliers de 0,5 m avec murs latéraux ; les paliers ont aussi des soutiens |
| Hauteur libre | 2 m par défaut, réglable jusqu'à 8 m ; conservée dans le manifeste |
| Matériau | Bloc réel choisi dans la bibliothèque ; aucune fabrication d'ID |
| Rotation / décalage | Rotation locale par quarts de tour et décalage sur la grille ; rotation globale du composite conservée |

La voie composite actuelle exige le cube tenu de 0,5 m. Une montée de marche
de 0,5 m, 1 m ou un autre multiple exact est représentable. **Une montée de 0,25 m
avec ces cubes est refusée.** Le fait que le moteur accepte des positions ou
d'autres formes ne prouve pas qu'un cube entier placé avec un décalage plus fin
produirait des marches fiables dans ce plan.

La montée n'est pas fixée implicitement à 0,5 m : le couple hauteur/nombre de
contremarches la détermine, puis la capacité du backend est contrôlée. Le nombre
indiqué correspond aux changements d'altitude, pas au nombre de cubes ni de
marches visibles dans les branches cumulées.

**Représentable ne signifie pas franchissable.** Un personnage peut devoir sauter
une contremarche de 0,5 m ou davantage. Ni la boîte de dégagement ni le graphe de
raccords ne prouvent la navigation du joueur. Cette version ne prétend pas livrer
un escalier utilisable sans saut avant le test dans le jeu.

## Conception et garanties géométriques

`voxel_stairs.h` calcule les empreintes de marches et paliers, leurs altitudes et
les volumes de soutien, en seizièmes de mètre entiers. Toutes les cellules émises
appartiennent à la grille de 0,5 m. Les surfaces adjacentes se touchent sur une
arête complète ; les paliers partagent exactement l'altitude de sortie précédente.
Les rotations portent sur les volumes des cellules et sur leurs dégagements.

`architecture_voxels.h` ajoute le résultat au plan ordinaire et à sa palette.
Une transaction de brouillon restaure l'état précédent si un budget, matériau,
chevauchement ou dégagement est refusé. Un chevauchement de cellules de matériaux
différents est refusé ; les doublons identiques suivent la politique composite
existante. Aucune découpe destructive n'est faite dans un bâtiment existant pour
forcer l'ajout d'un escalier : ses trémies doivent être suffisantes.

Pour un ajout, placer le départ à la surface du plancher et choisir une hauteur
égale à la différence exacte entre les surfaces des deux planchers. Le générateur
ne devine pas l'étage cible et ne corrige pas une hauteur incompatible. La démo
calcule ensemble ces altitudes et les trémies. Si le projet contient déjà des
pièces natives expérimentales, leur activation reste requise pour ces pièces ;
ajouter des marches voxels ne contourne pas cette protection.

Les espaces réservés sont vérifiés contre les voxels du projet et les enveloppes
connues de ses pièces/objets actifs. Ils doivent disposer d'une surface complète
en dessous. La prévalidation et la revérification des lots interrogent ensuite la
couverture et les blocs du monde au-dessus de chaque surface. L'air n'est ni
construit, ni facturé, ni supprimé. Une erreur donne sa position/cellule/module
dans l'interface et dans le journal.

Ce contrôle ne reproduit pas les collisions complètes des entités, du terrain,
de la physique ou la navigation. Il ne certifie pas la stabilité d'un soutien.
Il reste nécessaire d'inspecter le site et de parcourir le résultat dans le jeu.

## Exemple installable à trois niveaux

Projet : **`Dev7_Batiment_trois_niveaux_voxels.btproject`**, nom affiché
« Dev7 - trois niveaux et escaliers voxels ».

- Trois planchers dont les surfaces sont Y=0, 4 et 8 m ; murs, fenêtres,
  embrasures de portes et toiture.
- Deux parcours droits de huit contremarches : montée de 0,5 m, marches de
  1 m de profondeur, largeur de 2 m, paliers de 2 m, soutien plein.
- Les deux parcours sont décalés pour conserver les dégagements ; les trémies
  sont calculées à partir des marches, soutiens et 2 m d'air. Aucun plancher ne
  coupe le passage prévu. Les paliers relient exactement les surfaces des étages.
- **8 068 cellules, 22 volumes de dégagement, zéro pièce native et zéro objet**.
  Les 18 modules de conception sont exportés en 58 tuiles BTBP ; il s'agit du
  même bâtiment, pas de 58 bâtiments. Les prix sont recalculés dans le jeu.
- Matériau `0x7ad0559c` / `0x9d`, repris de l'exemple dont l'identité native a été
  confirmée lors du diagnostic dev5. Le jeu doit encore le résoudre à chaque
  construction. Les fichiers personnels ne sont pas modifiés.

Le ZIP installe les 58 BTBP et leur manifeste dans `BuildingTools-Projects`.
Il suffit de charger le manifeste dans la liste habituelle. Une autre démo peut
être préparée depuis l'interface avec les paramètres choisis ; elle emploie des
volées droites et exige au moins 4 m par niveau, puis remplace le brouillon en
mémoire. La forme L/U/monumentale choisie pour le générateur individuel ne change
pas ce tracé de démonstration. Les portes interactives restent à poser manuellement.

## Export et compatibilité

Les BTBP v1 et les anciens champs du manifeste ne changent pas. Dev7 lit les
projets précédents ; sans dégagement explicite, leurs contrôles existants sont
conservés. Les projets conçus avec ces escaliers ajoutent le champ optionnel
`spaces`, décrit dans [BTPROJECT.md](BTPROJECT.md).

Ces fichiers exigent **dev7 ou ultérieure compatible** : une ancienne DLL au
lecteur strict refuse ce champ. Ce refus évite de perdre les contrôles de passage.
Ne pas retirer le champ pour contourner ce refus. L'export dev7 le conserve ; les
projets dépourvus d'espaces n'obtiennent pas de champ nouveau.

## Protocole en jeu

1. Fermer le jeu, installer le ZIP Standalone dev7 avec ses langues et tous ses
   exemples. Garder une seule DLL de Building Tools chargée. Utiliser un monde
   **local de test** ; rien n'a été installé automatiquement ni écrit dans vos
   sauvegardes par les scripts de cette livraison.
2. Vérifier la bannière dev7. Ouvrir le sélecteur de matériaux dans une zone
   d'autel et prendre le cube de **0,5 m**. Laisser le placement natif expérimental
   **désactivé**. Rejouer les trois modules (312 cellules) et les bâtiments reliés
   validés en dev5 avant de poursuivre en cas de régression.
3. Bibliothèque → choisir un matériau. Composants architecturaux → **Escaliers en
   voxels**. Commencer par un petit tracé droit : hauteur 1 m, deux contremarches,
   largeur 2 m, profondeur 1 m et paliers 2 m. Générer un nouveau brouillon.
4. Inspecter les marches et soutiens dans l'aperçu et les tranches. Vérifier sans
   construire ; aucun bloc ne doit apparaître. Construire sur un site libre et
   couvert. Tester la montée/descente, sans saut puis avec saut si nécessaire.
   Consigner ce résultat ; ne pas conclure à la franchissabilité d'un autre profil.
5. Essayer hauteur 1 m / quatre contremarches : la montée de 0,25 m doit être
   refusée explicitement, sans modification du brouillon ni du monde.
6. Tester séparément L, U et monumental, les deux soutiens, les rotations
   0/90/180/270° et un décalage. Contrôler les paliers et les deux branches.
   En ajout à un projet, vérifier les raccords aux planchers et les réservations.
7. Charger l'exemple de trois niveaux. Vérifier 8 068 cellules, 22 dégagements et
   **zéro pièce native**. Contrôler les trémies, paliers et altitudes avec l'aperçu.
   L'option expérimentale désactivée ne doit pas empêcher ce projet de blocs.
8. Analyser la couverture des autels, puis Vérifier sans construire. Une zone
   non couverte dans les marches **ou leur dégagement** doit être refusée. Un devis
   partiel ou périmé ne doit pas être présenté comme une validation complète.
9. Dans une copie de test, ajouter un bloc au-dessus d'une marche ou un plafond
   trop bas ; Vérifier doit signaler la collision de dégagement. Tester également
   un ajout de mur/plancher dans le passage du brouillon : la génération doit
   échouer. Aucun obstacle n'est supprimé automatiquement.
10. Exporter le projet, le recharger depuis la liste et répéter l'inspection :
    cellules, matériaux, positions et 22 dégagements doivent être conservés.
    Vérifier que le plafond/bloc bloquant est encore détecté après rechargement.
11. Construire l'exemple sur une zone libre distincte et attendre les acquittements
    jusqu'au total prévu. Parcourir sol → niveau 1 → niveau 2 puis redescendre.
    Noter les sauts nécessaires, éventuels accrochages, chutes ou passages bouchés.
12. Sauvegarder/recharger le monde de test et reprendre le parcours. Refaire un
    petit cas en mode normal avec un stock suffisant puis insuffisant ; vérifier
    le débit réel et le refus de manque de matériaux. Tester Échap/perte de focus
    comme sur les autres composites ; les lots déjà construits restent à inspecter.

Conserver version du jeu/mod, paramètres exacts, mode créatif, résultat à pied,
rotation, matériau, coûts, `BuildingTools.log`, `BuildingTools-Projects.log` et le
rapport exporté. Les logs `voxel stairs PREPARED/REFUSED`, `project space REFUSED`
et `project spatial REJECT` isolent génération, cohérence interne et monde réel.

**Ces étapes sont des résultats attendus, pas des observations de cette livraison.**
Le bilan automatique est dans [DEV7-VALIDATION.md](DEV7-VALIDATION.md).

## Variantes inclinées et demi-blocs : étape ultérieure

La limite `Cube=8` du générateur exprime la capacité actuelle, pas une affirmation
sur toutes les formes du jeu. Une variante plus fine demandera un profil prouvé :
ID de forme, dimensions réelles, montée/sortie, représentation du plan, aperçu,
tarification/commit et compatibilité de la couverture. Une forme inclinée n'est
pas équivalente à une suite de cubes décalés.

Il faudra un backend de formes distinct, une sérialisation adaptée et des tests
de raccords, prix, collision, marche et persistance sur le modèle réel. Réduire
une constante de grille ou désactiver le contrôle du cube de 0,5 m ne suffit pas.
La sélection de profils non certifiés n'est pas exposée comme fonction utilisable.
