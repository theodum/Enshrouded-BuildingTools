# Dev9 — catalogue réel et projets d'ameublement

**Bilan historique.** Les essais utilisateur de dev9 ont depuis révélé un refus
du lecteur du monde sur un semis de riz et un refus distinct des flags de la
bougie. Pour la correction dev10 et les essais à un puis cinq objets, suivre
[DEV10-NATIVE-PLACEMENT-FIX.md](DEV10-NATIVE-PLACEMENT-FIX.md). La DLL installée
est désormais la dev9 ; les mentions de dev7 ci-dessous décrivent l'état avant
son installation par l'utilisateur.

Livraison du 22 novembre 2026, `0.34.0-dev9`. Le château V2 de 156 888 cellules reste inchangé. Cette livraison n'installe pas la DLL et ne modifie aucune sauvegarde. Elle ajoute des données réelles à la mécanique de mobilier dev8, sans second moteur de placement.

## Données et distinction des identifiants

Le [CSV fourni](https://enshroudedsaveeditor.com/items/item-ids.csv) contient 3 526 IDs uniques, dont 1 303 `BuildTools`. L'index local conserve nom, hex/décimal, catégorie source, taille de pile, empreinte du CSV, catégorie de navigation et état de résolution. Les catégories déduites des noms ne décident jamais si l'objet est autorisé.

Le [lecteur d'Emberforge audité](https://github.com/evoniq-bit/Emberforge/blob/639bd73ba7152cce68d0de2fc071d467329bdaf4/Emberforge-v0.2.15-Source/Quellcode/GameCatalog.cs) indexe des GUID 128 bits de templates/modèles. Son moteur d'override utilise aussi le modèle de prévisualisation et une source de placement. Ce n'est pas le même identifiant que l'item d'inventaire 32 bits. Le lecteur attend KFC3 ; l'archive installée porte KFC4. Il n'a pas été exécuté, et son code n'a pas été recopié dans le mod.

[enshrouded.gg](https://enshrouded.gg/about/) indique un snapshot du 15 août 2026, des tables provenant du serveur et des noms provenant du client. Ses catégories confort/décoration/portes sont utiles comme comparaison. Aucun ID, volume ni niveau de confort n'a été fusionné entre bases uniquement sur la base du nom.

Dans Building Tools, l'ID 32 bits est directement donné à `g_bpItemById`. La définition doit être retrouvée avec **le même ID**, être un prop `ITEM_OTHER`, posséder un `EquipmentSetup.placedEntity` non nul à `+0x3c0` et ne pas réclamer de slot d'autel (`+0x427`). L'ABI, les drapeaux `+0x5d8`, la politique des objets à logique et les bornes natives doivent également être valides. Le GUID 128 bits et les volumes sont ensuite lus dans la définition réelle : **le CSV n'a pas à les inventer**. `bp_place_object` réutilise le véritable `PlaceProp` d'origine, son paiement et son undo. La seule extension de cette fonction est la comptabilisation de son retour natif.

Les anciens journaux locaux apportent une vérification supplémentaire du format d'ID : six items ont été tenus en main par le joueur, dont **cinq correspondent exactement au CSV**, notamment `bad60643` (Boreal Wood Banquet Table), `b8fb7c73` (Silver Candleholder) et `1ce48f02` (Large Iron Window Frame). Le sixième, `d1c3d410`, est absent du CSV ; aucun nom ne lui a été inventé. Les observations natives contiennent un template et des drapeaux ; elles sont conservées séparément dans `prior-session-observations.json`. **Tenir un objet ne prouve pas son placement ni ses interactions**, et ces traces anciennes ne rendent aucune entrée du catalogue actuel compatible. Par exemple le Silver Candleholder observé porte `0x80001` : le nom « meuble » ne suffit pas à annuler le refus conservateur des drapeaux spéciaux.

## Utilisation dans le mod

1. Installer manuellement la dev9, jeu fermé, avec votre chargeur habituel. Les archives contiennent aussi `BuildingTools-Catalog` et le démonstrateur dans `BuildingTools-Projects`.
2. Dans le jeu, ouvrir le sélecteur natif des matériaux et le véritable cube de marteau de **0,5 m** afin d'observer le registre et les autels. Ouvrir la bibliothèque native et actualiser.
3. Cliquer sur **Recherche nom ou ID** pour saisir un nom anglais du CSV, un ID décimal ou hexadécimal ; Entrée termine la saisie. Choisir la catégorie importée si utile. Le filtrage est insensible à la casse ASCII. Les noms localisés déjà mis en cache sont également recherchés. La recherche ne déclenche jamais Construire par Entrée.
4. Les objets résolus passent dans les compatibles. Les autres restent visibles en consultation, avec le motif : `unknown_id`, `not_prop`, `altar_slot`, `special_flags`, `logic_disabled`, `invalid_bounds`, `abi_unavailable` ou `registry_unobserved`. `policy_compatible` signifie que les métadonnées natives satisfont la politique ; cela ne certifie pas le placement, les interactions ou la sauvegarde persistante.
5. Les portes et certains luminaires réclament l'activation expérimentale de la **copie logique originale**. Le writer du blueprint original est réutilisé ; il ne rétablit ni liens de logique ni câblage. Les objets spéciaux/progression restent refusés.
6. Exporter le catalogue observé. `NativeCatalog_*.json` contient aussi les IDs non résolus, la catégorie, le motif, le GUID `placedEntity` si disponible, les bornes, le contexte PE et les appels natifs acceptés/échoués de cette session. `interactionValidated` reste `false`. Si l'historique des appels dépasse sa capacité, `callHistoryTruncated` le signale. Les fichiers `.partial` ne sont pas des exports valides.

Le chargement d'un composite comportant un ID non résolu ou refusé **échoue immédiatement**, avec ID et motif. Il n'autorise plus un aperçu incomplet en attendant Construire. Pour charger des modèles à logique, activer d'abord l'option correspondante dans la bibliothèque. Le registre doit déjà avoir été observé. La vérification est répétée à la préparation et avant les lots, indépendamment des états d'un JSON externe.

## Démonstrateur fourni

`Dev9_Demo_Mobilier_A_VALIDER.btproject` référence un BTBP contenant **11 objets natifs et zéro voxel / pièce de structure** :

| Type | Nom réel du CSV | ID hexadécimal | Quantité |
|---|---|---|---|
| Lit | Crude Wooden Bed | `566c8dfe` | 1 |
| Table | Crude Wooden Table | `d109e828` | 1 |
| Chaises | Wooden Chair | `dd5f8bb9` | 2 |
| Flambeaux muraux | Wall Torch | `66f057f6` | 2 |
| Tableau | "Flower Vase" | `500ddd97` | 1 |
| Coffre | Tiny Chest | `02f09172` | 1 |
| Décorations sur table | Candle | `af3025e4` | 2 |
| Décoration au sol | Clay Pot I | `d76bd513` | 1 |

**Ce fichier est un brouillon avec IDs réels, pas un placement validé.** Les positions sont des pivots d'auteur ; la taille exacte et le point d'appui des modèles ne sont pas déduits du CSV. Son chargement peut refuser un modèle absent/à logique désactivée ; sa vérification doit refuser un support absent ou une collision. Ne pas contourner ce refus pour le construire.

Pour le test réel recommandé, le bouton **Sélectionner les huit identifiants exacts du témoin CSV** remplit les huit rôles sans substitution. Choisir aussi un matériau réel, autoriser la copie logique si nécessaire puis **Préparer la pièce témoin meublée**. Le mod résout ces IDs, mesure leurs volumes, recale les appuis et génère la pièce de 12 × 10 m avec ses 11 objets. Si un modèle ne tient pas, la génération refuse en conservant le brouillon précédent. Il faut alors sélectionner explicitement un autre modèle ; aucun remplacement automatique n'est effectué.

Exporter puis recharger le projet mesuré et exécuter **Vérifier sans construire**, dans une zone libre et couverte par un autel. Construire une fois, attendre 11 retours positifs et vérifier réellement les appuis, le sens des modèles, les interactions, la lumière et la persistance après rechargement. Tester ensuite la sauvegarde de cette pièce avec le blueprint d'origine, comme indiqué dans [DEV8-NATIVE-OBJECTS.md](DEV8-NATIVE-OBJECTS.md). Cette validation réelle reste à effectuer ; les tests hôte utilisent un registre simulé.

## Créer un projet séparé et précis

Depuis un projet contenant du mobilier, **Exporter uniquement le mobilier** utilise l'exporteur existant en omettant tous les voxels, pièces natives et volumes de passage. L'ancre et les positions/quaternions des objets sont conservés. Le brouillon et sa structure restent en mémoire sans modification. Cette option refuse les projets sans objets ; elle ne transforme pas le mobilier voxel du château en vrais objets.

Pour écrire une disposition réutilisable, `tools/make_furnishing_project.py` prend une recette JSON avec `item`, `position` en mètres et soit `rotation: [x,y,z,w]`, soit `yawDegrees`. Les positions n'ont pas à suivre la grille de 0,5 m ; les quaternions inclinés sont conservés. Les propriétés non prises en charge et les rotations invalides sont refusées. La fabrique utilise le codec et l'exporteur C++ de production, jamais un moteur de placement séparé.

```sh
c++ -std=c++17 -O1 tools/native_furniture_host.cpp -o build/native_furniture_host
python3 tools/native_item_catalog.py merge --catalog catalog/native-items.json --observed /chemin/NativeCatalog_REEL.json --out /chemin/catalogue_observe.json
python3 tools/make_furnishing_project.py --catalog /chemin/catalogue_observe.json --recipe exemples/chambre.recipe.json --outdir /chemin/nouveau-projet
```

La commande refuse les IDs non observés/refusés. `--unverified-draft` permet seulement d'écrire un brouillon à partir d'IDs présents dans le CSV, sans prétendre les valider ; il ne contourne pas une incompatibilité connue ni les contrôles du mod. Les sorties incluent une provenance avec `voxelCells: 0`, `nativePieces: 0`, `inGameValidated: false`. Les dossiers existants ne sont pas écrasés.

## Application au château de 156 888 cellules

La fabrique accepte `--reference CHEMIN/Forteresse_Royale_Spacieuse_V2.btproject --world-origin X Y Z --world-turn 0`. Elle reprend exactement l'ancre du manifeste et conserve l'origine mondiale/la rotation **que vous indiquez depuis le journal du château construit**. Elle refuse de déduire cette position de l'ancre JSON seule. Une empreinte de chaque fichier structurel référencé accompagne le projet et les fichiers sources sont vérifiés inchangés après export.

Les coordonnées des meubles doivent être exprimées dans le repère local du château, avant rotation globale. Le projet de décoration doit être placé avec l'origine mondiale et la rotation enregistrées. Il ne peut émettre aucun voxel ni pièce native et ne reconstruit aucun mur, plancher, escalier ou tour. Les contrôles natifs vérifient les supports existants, les collisions et les autels. Les anciens meubles voxel restent des obstacles ; aucune suppression automatique n'est faite.

La présente livraison ne génère pas encore un ameublement complet du château : la validation en jeu du démonstrateur, demandée précédemment, n'a pas été fournie. Les ensembles chambre, banquet, trône, bibliothèque et atelier pourront utiliser le même format de recette et le même export uniquement mobilier après cette étape.

## Mise à jour et limites de validation

Le catalogue livré est une photographie de la source, pas une autorisation persistante. Télécharger/importer un nouveau CSV vers de nouveaux fichiers, examiner les différences, remplacer le snapshot/index/header ensemble puis recompiler. Les commandes et la provenance sont détaillées dans `BuildingTools-Catalog/README.md` dans les archives, ou `catalog/README.md` dans le dépôt. Après toute mise à jour du jeu, actualiser/exporter les observations ; la fusion exige le même SHA d'index et ne conserve aucune ancienne approbation absente du nouveau rapport.

Les contrôles géométriques de dev8 restent conservateurs : boîtes englobantes, contact de 1 cm, cinq sondes sur une face ou appui sur meuble, couverture native 3D des autels, maximum 2048 objets par opération. Ils ne prouvent ni la collision précise du personnage ni les règles propres à chaque fixation. Les contenus de coffres, propriétaires et réseaux logiques ne sont pas copiés par le format original. Le créatif réutilise les interceptions de paiement existantes ; son absence de consommation reste à tester en jeu.

## Vérifications exécutées pour dev9

| Contrôle | Résultat / portée |
|---|---|
| Import et reproduction du snapshot | 3 526 IDs uniques, cohérence hex/décimal, index/header reproduits exactement |
| Résolution et pipeline de mobilier C++ | 3 843 assertions, dont l'intégrité de l'index ; registre simulé, fonction de résolution réelle, refus des IDs substitués/inconnus et des drapeaux interdits avant appel des bornes, original `PlaceProp` remplacé par un callback de test |
| Prévalidation de mobilier C++ | 59 contrôles, monde/voxel/autels simulés ; code de prévalidation de production |
| ASan + UBSan sur ces deux suites | 3 902 contrôles, aucun diagnostic mémoire ni fuite |
| Fabriques Python | 9 nouveaux tests catalogue/recettes/alignement + 3 tests de pièce mesurée ; tous réussis |
| Export mobilier depuis un projet avec structure | Test de l'exporteur réel : zéro voxel/pièce/espace en sortie, ancre et objets conservés, source intacte |
| Core, langues, commandes, coûts, multiétages, escaliers, configuration et export | Suites historiques réussies ; 40 signatures historiques et 3 vérifications de réflexion ABI sur l'exécutable installé, lu sans exécution |
| Châteaux et fixtures | 4 + 8 + 11 tests Python réussis ; archives V1/V2, dépendances, empreintes et relecture native vérifiées, données architecturales inchangées |
| Archives de livraison | Produites par `build.sh package`, vérifiables par `tools/verify_package.py` ; empreintes dans `dist/SHA256SUMS-0.34.0-dev9.txt` |

**Aucun objet n'a été placé dans Enshrouded pendant ces tests.** La capture réelle d'une pièce meublée, les appuis natifs, les interactions, la lumière, le paiement et la persistance restent à valider par la procédure ci-dessus. Les cinq correspondances CSV trouvées dans les journaux antérieurs attestent uniquement des objets tenus en main, pas de placements dev9 réussis. La DLL installée dans le jeu conserve la version dev7 ; la dev9 est livrée en archives séparées.
