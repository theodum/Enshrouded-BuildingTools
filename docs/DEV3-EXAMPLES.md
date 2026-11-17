# Exemples installables de dev3

Les deux ZIP placent les manifests et leurs sources dans `BuildingTools-Projects`,
à côté de l'exécutable du jeu. Home → Options → Projets composites les ouvre dans
la fenêtre native ; le chemin classique Blueprint → Placer reste disponible.
Prendre le cube de 0,5 m. Aucun de ces exemples n'a été placé en jeu par l'agent.

| Projet | Contenu | Emprise relative | Cellules natives |
|---|---|---|---|
| `Trois_modules_test` | Exemple dev1 conservé, trois sources et grilles mélangées | Environ 9×3×8 m | 312 |
| `Dev3_Trois_batiments_relies` | Trois maisons à 0/180/90°, deux passages, ouvertures sur les quatre faces | 20×4×4 m | 800 |
| `Dev3_Batiment_etages` | Trois planchers, murs par niveau, trémies 2×2 m alignées et toiture | 8×15×8 m ; planchers Y=0/4/8 | 1 444 |
| `Dev3_Grille_vide_Y120` | Deux maisons reliées, placées par un plan virtuel | 12×4×4 m | 528 |
| `Dev3_Village_36_batiments` | 36 maisons, 60 passages, 96 modules ; sources à 1 m et 0,5 m | 68×8×68 m | 75 648 |

Les dimensions suivent les blocs occupés ; l'enveloppe des fichiers sources peut
inclure des cellules vides. Les passages du grand village sont décalés de 0,5 m
en Y afin d'aligner leur dessus sur les planchers des maisons à grille de 1 m.
Les tests vérifient la continuité du petit ensemble et les comptes des cinq projets.

Le matériau est repris du vrai matériau déjà utilisé dans l'exemple dev1 ; les
identifiants ne sont pas inventés. La DLL les résout à nouveau dans le registre du
jeu. Les prix des nouvelles sources sont marqués inconnus et recalculés nativement.

## Essai de grille virtuelle

1. Home → Options → Grille : activer Free Placement et la grille, choisir Y=120,
   un pas de 0,5 m et afficher les axes. L'altitude de la caméra peut servir de point
   de départ. Enregistrer un des huit niveaux pour retrouver cette hauteur.
2. Revenir aux projets et charger `Dev3_Grille_vide_Y120`. Viser le plan vers le
   bas ou le haut : le rayon doit le rencontrer à moins de 512 m. Une visée parallèle
   ne donne pas de placement valide.
3. Fixer le point puis utiliser Vérifier sans construire. Les zones d'autels doivent
   couvrir le projet en trois dimensions, même si le sol est absent.
4. Construire seulement après ce contrôle. La grille reste un affichage et une
   référence de curseur ; aucun support invisible n'est créé.

## Exemple de véritables escaliers entre deux niveaux

Ce test est préparé **dans le jeu** avec ses vrais identifiants. Le pack ne contient
pas de faux ID d'escalier ni de manifeste invalide à compléter à la main. Les données
du registre ne sont disponibles qu'à l'exécution ; un nom ou une boîte ne prouve pas
qu'un modèle monte correctement.

1. Dans un monde de test, ouvrir le sélecteur natif de matériaux puis la bibliothèque
   Extended. Choisir un matériau de construction et, dans Pièces natives, un véritable
   escalier dont vous reconnaissez le modèle et le nom. Une fondation reste une
   fondation : le générateur ne transforme pas un autre modèle en escalier.
2. Ouvrir Composants architecturaux → Escaliers. Régler **Orientation du modèle**
   pour que sa montée locale suive +Z. Ce réglage est distinct de **Rotation**, qui
   tourne l'ensemble du tracé et tous les paliers. Lire les dimensions mesurées.
3. Commencer par une seule volée et régler sa montée réelle. La hauteur de la boîte
   englobante est une estimation, pas une preuve de hauteur de marche. En l'absence
   de résultat cohérent, conserver le placement manuel original.
4. Pour le test complet, choisir **Préparer un exemple d'escalier natif entre deux
   niveaux**. Cela remplace le brouillon en mémoire : enregistrer auparavant tout
   travail à conserver. Le mod utilise les dimensions natives pour préparer une
   montée d'au moins 4 m avec deux planchers et des réservations ; aucune marche
   n'est fabriquée en cubes. Les planchers/paliers sont des surfaces de blocs.
5. Activer **Placement natif expérimental** dans Home → Options → Créatif/Objets
   natifs. Il est désactivé par défaut. Une ABI incompatible ou une pièce trop grande
   interdit l'automatisation avec un message. Les projets de blocs restent utilisables.
6. Reprendre le brouillon, inspecter puis construire dans une zone libre couverte.
   Vérifier montée sans saut, sortie sur le palier, hauteur libre et réservations.
   Refaire le test à 90/180/270°, puis tester quart tournant, impérial et spirale
   carrée avec leurs paliers. Une spirale arrondie n'est pas proposée.
7. Enregistrer le projet : de nouveaux BTBP et un `.btproject` conservent les vrais
   ID de pièces/matériaux, pivots, orientations et dégagement. Recharger ce manifeste
   exporté, comparer et vérifier la persistance après rechargement du monde de test.

Le contrôle des limites est conservateur ; un volume natif inclut parfois du vide.
Il peut donc refuser un assemblage physiquement possible. Les points de sortie,
la montée réelle et l'usage des objets restent des critères de la recette en jeu.

## Objets, portes et éclairages

Pour placer un compatible sans le posséder, activer le créatif original, l'option
Objets natifs créatifs et l'activation expérimentale, puis le choisir dans la
bibliothèque et préparer sa position/rotation. Les meubles et lumières ordinaires
ne sont proposés automatiquement que si leurs drapeaux natifs le permettent.

Les objets à connaissances/PIDE, plans/outils spéciaux, PNJ/animaux/appâts, usines,
récompenses Twitch, objets de commerce et drapeaux inconnus sont exclus. Un objet
portant IsLogicEntity est refusé dans un composite, même s'il ressemble à une porte,
fenêtre ou lampe ordinaire. La catégorie en lecture seule explique ce refus.
Le placement manuel original garde son contexte et ses événements ; l'exemption
créative y reste également soumise aux autres drapeaux de sécurité.

Aucun état de coffre, contenu d'inventaire, câblage, progression ou quête n'est
copié. Tester les usages et lumières, puis sauvegarder/recharger le monde de test.
Voir [EXTENDED-TESTING.md](EXTENDED-TESTING.md) pour la campagne complète.

## Reproduire les fixtures

```sh
python3 tools/make_dev3_examples.py --material-from examples/three-modules/hall.btbp --output NEW-DIRECTORY
python3 -m unittest discover -s tests -p test_project_example.py
```

Le générateur lit seulement la table d'un BTBP, refuse un dossier de sortie existant
et n'accède à aucun monde ou fichier `.ksd`.
