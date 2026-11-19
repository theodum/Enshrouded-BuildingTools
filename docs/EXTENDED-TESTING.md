# Building Tools Extended 0.34.0-dev7 — installation et recette

Version de développement du fork, basée sur Building Tools 0.33.2.
La compilation et les tests automatisés ne remplacent pas votre test dans le jeu.
Les petits composites, matériaux et limites d'autels ont été validés par
l'utilisateur avec dev5. Les nouvelles fonctions dev6 et les cas non cités
demandent encore une validation personnelle.

## Installation manuelle

Fermer le jeu. Utiliser l'archive locale
`dist/EnshroudedBuildingTools-0.34.0-dev7.zip`, ou l'archive Shroudforge si ce
chargeur est déjà utilisé. Les deux archives contiennent la même DLL.

1. Conserver une copie de votre DLL de mod précédente si vous souhaitez revenir
   à la version d'origine. Ne pas écraser les fichiers d'un autre chargeur.
2. Extraire l'archive standard à côté de `enshrouded.exe`. Elle contient
   `dinput8.dll`, la licence MIT, les langues, la documentation et le dossier
   `BuildingTools-Projects` avec 68 fichiers `.btbp` et six manifestes.
3. Avec Shroudforge, extraire l'archive correspondante : la DLL est dans
   `mods/building-tools/BuildingTools.dll`. Conserver une seule copie de Building
   Tools chargée. Les projets restent à côté de l'exécutable du jeu.
4. Démarrer un **monde local de test** et vérifier la bannière 0.34.0-dev7 et
   `BuildingTools.log`. Aucun test ci-dessous ne doit utiliser votre monde principal.

Le développement n'installe rien dans le jeu. Aucun fichier `.ksd`, sauvegarde
Steam ou monde existant n'est modifié par les scripts de compilation et de test.
Avant tout essai de construction, conserver hors du répertoire du jeu une copie
de votre monde de test, jeu arrêté ; la DLL ne réalise pas de sauvegarde automatique.

**Retest prioritaire de dev7 :** suivre [DEV7-VOXEL-STAIRS.md](DEV7-VOXEL-STAIRS.md).
Le projet `Dev7_Batiment_trois_niveaux_voxels` ne demande pas l'option native
expérimentale. Vérifier notamment les contremarches, les dégagements persistants
et les refus de hauteur non représentable. La recette native reste disponible :
suivre [DEV6-MULTISTOREY.md](DEV6-MULTISTOREY.md) :
non-régression dev5, calibration d'une volée, génération, export/rechargement et
parcours des trois niveaux. L'audit est dans [DEV6-AUDIT.md](DEV6-AUDIT.md).
Le diagnostic historique de matériaux est dans [DEV5-MATERIALS-FIX.md](DEV5-MATERIALS-FIX.md).
Installer aussi les BTBP d'exemple corrigés : changer seulement la DLL conserve
leurs anciens identifiants invalides. Les actions dev4 ont démarré en jeu ; leur
protocole reste dans [DEV4-ACTIONS-FIX.md](DEV4-ACTIONS-FIX.md).
Le cube **tenu dans le marteau** doit faire 0,5 m :
cela n'interdit pas les sources BTBP à 1/2/4 m. Une mauvaise forme doit maintenant
produire une raison et un log, en conservant la fenêtre ouverte.

Le ZIP fournit `BuildingTools-Extended.ini.example`, pas un INI actif susceptible
d'écraser vos réglages. Les valeurs par défaut sont intégrées à la DLL. Les changer
dans Home → Options → Section Extended ; l'extension ne réécrit que ses propres clés.
Avec Shroudforge, conserver son `dinput8.dll` et une seule entrée `building-tools`.
Vérifier aussi l'absence d'une seconde copie dans `dinput8-chain.ini` ou un ancien
dossier de plugin. Une ligne « second copy » dans le journal indique un doublon.

## Campagne personnelle dev3 : douze étapes

Cette campagne reste applicable à dev6, après le protocole ciblé. Le premier
journal réel de dev3 confirme le chargement/UI/Vulkan et les projets chargés,
pas le placement : aucune opération composite n'y avait démarré.

**Tous ces essais restent à effectuer dans Enshrouded.** Le tableau décrit les
résultats attendus, pas des résultats observés. Utiliser des emplacements distincts
et libres pour chaque construction. Tenir le marteau ; ouvrir une fois son sélecteur
de matériaux pour observer le registre et les bases, puis le fermer.

Home → Options → Section Extended donne accès aux projets, grille, aperçu,
matériaux, créatif, objets natifs, autels et diagnostics. Les raccourcis Home/F5/F6/F10
ci-dessous désignent les valeurs par défaut ; utiliser vos touches si elles ont été
reliées autrement. L'ouverture depuis Home initialise la fenêtre avec un clic natif
sans construction ; si elle ne s'ouvre pas, consulter le journal puis utiliser le
chemin classique Blueprint → Placer → premier clic.

| Étape | Action | Résultat attendu | Informations à relever en cas d'échec |
|---|---|---|---|
| 1. Chargement et Home | Démarrer la copie du monde, ouvrir Home, choisir Options puis chaque section Extended | Bannière dev3, une seule DLL active, contrôles français lisibles ; projets et diagnostics s'ouvrent avec le marteau | Début de `BuildingTools.log`, bannière, écran et section concernés, chargeur et liste des autres mods |
| 2. Outils originaux | Sur quelques blocs de test, vérifier F5, Area/Replace, Free Placement, BTBP classique et annulation native | Fonctions originales disponibles, touches personnalisées conservées ; aucune nouvelle option ne s'active seule | Mode, touche réellement configurée, forme/matériau, journal et captures avant/après |
| 3. Composite simple | Cube de 0,5 m, charger `Trois_modules_test`, fixer l'ancrage et cliquer Construire une fois | Passe de contrôle sans écriture, puis 312 placements aux positions du tableau ci-dessous | Ancrage, rotation, journal complet de l'opération et photo des trois volumes |
| 4. Rotations et hauteur | Recommencer sur une autre zone à 90/180/270°, puis décaler Y | Un pivot commun pour tous les modules ; décalages relatifs inchangés, Y commun | Ancrage et angle affichés, module décalé, coordonnées attendues/observées ; aucun deuxième clic d'alignement nécessaire |
| 5. Vide et grille | Section Grille : activer Free Placement et le plan, régler Y=120 ; charger `Dev3_Grille_vide_Y120` | Visée du plan sans support, grille transparente, pas 0,5/1 m ; origine et niveaux conservés après redémarrage ; aucune plateforme invisible | Y/origine/pas, direction de visée, couverture des autels, `BuildingTools-Extended.ini` |
| 6. Escaliers réels | Suivre l'exemple natif de `DEV3-EXAMPLES.md`, d'abord une petite volée puis les deux niveaux | Vrai modèle choisi dans la bibliothèque, montée sans sauter, orientation et paliers cohérents à chaque rotation ; arrêt explicite si incompatible | ID et nom exacts, dimensions, orientation du modèle, montée réglée, type de tracé, image du débouché et diagnostic ABI |
| 7. Objets gratuits | Activer Créatif original, Objets natifs créatifs et Placement natif expérimental ; choisir un petit meuble/éclairage compatible absent du stock | Aperçu par repère, coût nul en créatif, modèle natif, usage/lumière à vérifier après sauvegarde et rechargement ; objets spéciaux/logiques refusés | ID/nom, trois options créatives, résultat de paiement, fonctionnement et journal après rechargement |
| 8. Plusieurs étages | Charger `Dev3_Batiment_etages`, puis `Dev3_Trois_batiments_relies` sur deux zones distinctes | Planchers Y=0/4/8 relativement à A, trémies alignées de 2×2 m ; trois maisons et passages de même niveau | Ancrage, niveau décalé, cellule manquante, matériaux et éventuelles réservations |
| 9. Aperçu | Déplacer, tourner, afficher une tranche et masquer un module ; remplacer un matériau puis Vérifier sans construire | Emprise et modules visibles ; repères des natifs, coûts dans Matériaux ; filtres purement visuels ; fin d'inspection sans `BATCH_BEGIN` | Capture avant/après, section/filtres, taille du projet, ID d'opération et trace `INSPECTED` |
| 10. Autels et collisions | Déborder d'une zone en X/Z puis Y ; analyser et déplacer une proposition ; viser ensuite un bloc existant | Recommandations manuelles, volumes or/verts, échantillons non couverts signalés ; contrôle de construction refuse zone/collision avant premier lot | Zones observées dans Diagnostics, ancrage, premier repère rouge, options d'objets et journaux ; absence d'autel créé automatiquement |
| 11. Grand projet | Charger `Dev3_Village_36_batiments`, inspecter puis construire seulement si toute la zone est couverte | 96 modules, 75 648 cellules, emprise 68×8×68 m ; lots progressifs et interface réactive ; durée potentiellement longue | Progression CHECK/BATCH_RESULT, durée maximale de contrôle et limite adaptative dans Diagnostics, captures et ID d'opération |
| 12. Erreurs/interruption | Annuler une inspection puis un petit chantier ; tester une copie invalide du manifeste et un export vers un emplacement non accessible | Aucun lot après arrêt ; inspection sans écriture ; lots acceptés conservés ; fichier invalide refusé ; export incomplet non publié | `BuildingTools.log`, `BuildingTools-Projects.log`, rapport via Diagnostics, message exact et dernier lot acquitté |

Les grandes actions n'ont pas d'annulation totale. Le rapport donne l'identifiant,
l'empreinte du plan, l'ancrage, la rotation et le préfixe acquitté par le moteur.
Ce n'est ni une preuve du contenu actuel de la sauvegarde ni une autorisation de
supprimer/rejouer ces cellules : un lot payé peut être incomplet et le monde peut
avoir changé. Restaurer la copie du monde de test si nécessaire, jeu arrêté.

La couverture d'assistance des pièces/objets utilise neuf échantillons de leurs
limites ; le contrôle avant placement reste obligatoire. Les collisions de toutes
les entités, l'usage des objets et la circulation ne sont pas validés par cet aperçu.

## Retour à la version précédente

1. Fermer le jeu. Conserver les journaux et projets de test pour le diagnostic.
2. Installation standard : remplacer uniquement la DLL Extended par la copie de
   votre DLL précédente. Avec Shroudforge : restaurer la précédente
   `mods/building-tools/BuildingTools.dll`, en conservant son chargeur à la racine.
3. Conserver vos `BuildingTools.ini`, blueprints et projets ; ne supprimer aucun
   monde. L'ancien mod ignore `BuildingTools-Extended.ini` et les `.btproject`.
4. Vérifier qu'une seule version est chargée. Pour supprimer toute construction
   de test, restaurer votre propre copie du monde de test ; changer de DLL ne retire
   pas les lots déjà construits.

## Premier test : trois modules, une validation

1. Dans une zone libre couverte par un autel, prendre le marteau et sélectionner
   le **cube de 0,5 m**. Prévoir un espace d'environ 10 × 5 × 9 m, dégagé et couvert
   sur toute sa hauteur. Monter l'aperçu pour éviter les blocs déjà présents.
2. Ouvrir une fois le sélecteur de matériaux dans cette base, puis le fermer.
   Cela permet au jeu de fournir ses zones et stocks au mod. Pour le premier
   essai, le mode créatif original peut être activé dans Home afin de tester
   l'alignement sans préparer les ressources ; il n'active pas les objets composites.
3. Passer en mode Blueprint (F6 : Prefab → Area → Blueprint, ou la roue Caps Lock).
   Choisir l'action **Placer**, puis cliquer une fois pour ouvrir la fenêtre.
4. Cliquer **Projets composites**, puis **Trois_modules_test**.
5. Vérifier l'aperçu des trois volumes, la liste des matériaux, « 312 cellules »
   et les trois coordonnées d'ancrage affichées. Maintenir le bouton droit pour
   masquer la fenêtre et regarder autour. « Placer ici » fixe l'ancrage ; les
   réglages de déplacement règlent l'ensemble, y compris sa hauteur.
6. Garder la rotation globale à 0°. Cliquer **Construire une seule fois** et
   ne pas changer d'outil, ouvrir de menu ou quitter la fenêtre du jeu pendant
   l'opération. Le mod contrôle tous les lots sans écrire, puis les construit.
7. Attendre « Projet terminé : 312 placements effectués ». Aucun clic manuel pour
   aligner ou confirmer les modules suivants ne doit être nécessaire.

Par rapport à l'ancrage monde A affiché :

| Module | Coin inférieur attendu | Dimensions X × Y × Z |
|---|---|---|
| Hall : sol et mur arrière | A + (0,0,0) | 6 × 3 × 4 m |
| Petit pilier de repérage | A + (8,1,0) | 1 × 1,5 × 1 m |
| Plateforme en hauteur | A + (2,2,5) | 1 × 0,5 × 3 m |

Le journal doit montrer `BEGIN`, la passe `CHECK` / `CHECKED`, deux
`BATCH_BEGIN` / `BATCH_RESULT` (256 puis 56 cellules avec MaxShapes ≥ 256),
puis `COMPLETE`, toujours au même ancrage et à la même rotation globale.
Les 312 cellules sont vérifiées par les tests C++ à partir des fichiers livrés.

## Rotations, hauteur et stabilité

Dans une autre zone vide, répéter avec une rotation globale de 90°. Les coins
inférieurs deviennent respectivement A + (-4,0,0), A + (-1,1,8), A + (-8,2,2).
Les rotations 180° et 270° suivent `R90(x,y,z)=(-z,y,x)` autour du même pivot.
Répéter après un décalage vertical de 0,5 m : tous les modules doivent monter
exactement de cette hauteur. Ne pas superposer les essais.

Puis vérifier les fonctionnalités originales sur le monde de test : chargement
et placement d'un `.btbp` classique, Save, Replace, Area, Free Placement, objets
classiques et Undo. Les tests automatiques couvrent leur logique existante,
mais cette recette de non-régression en jeu reste nécessaire.

## Refus et interruptions à vérifier

- **Annuler avant Construire** : Échap ou Annuler ferme l'aperçu sans changement
  du monde et sans entrée `BATCH_BEGIN`.
- **Collision** : viser un emplacement contenant un bloc de construction dans
  un des modules. La première passe doit arrêter tout le projet avant le premier lot.
- **Autel** : déplacer un module hors couverture, y compris en hauteur. Refus
  avant toute construction. Aucune création ou suppression d'autel.
- **Ressources** : désactiver le créatif avant d'ouvrir le projet et disposer
  de trop peu de matériau. Refus global si le stock est connu et insuffisant ;
  un stock inconnu demande d'ouvrir le sélecteur.
- **Interruption** : appuyer sur Échap pendant la vérification ; aucune écriture.
  Pendant la construction, Échap arrête les prochains lots ; ceux déjà terminés
  restent. Une perte de focus ou un changement d'outil arrête également l'opération.
- **Fichier incorrect** : dupliquer l'exemple hors des fichiers livrés, référencer
  un fichier absent ou une rotation de 45°. Refus au chargement, sans action monde.
- **Doublon** : répéter un module au même offset/matériau dans une copie du manifeste.
  Les cellules dupliquées sont éliminées et signalées.

Après une erreur partielle, ne pas relancer aveuglément au même endroit. Le journal
indique le dernier lot et le nombre de cellules effectivement signalées écrites
par le moteur. Il n'existe pas de reprise automatique, de rollback complet ou
d'annulation totale garantie. Les coûts d'un lot payé peuvent rester consommés.

## Compilation depuis Windows / WSL

Les exemples et les placements natifs de dev3 sont décrits dans
[DEV3-EXAMPLES.md](DEV3-EXAMPLES.md). L'état détaillé reste dans
[EXTENDED-STATUS.md](EXTENDED-STATUS.md). Commencer par les trois modules ci-dessus
avant d'essayer les objets ou les escaliers dans ce même monde de test.

Dans PowerShell, depuis la racine du dépôt, avec WSL Ubuntu 24.04 et les
dépendances décrites dans `docs/BUILDING.md` du dépôt installées :

```powershell
wsl -d Ubuntu-24.04 --cd "$PWD" -- bash build.sh test
wsl -d Ubuntu-24.04 --cd "$PWD" -- bash build.sh package
```

Les sorties sont `build/dinput8.dll` et les deux archives `dist/*.zip`.
Le script conserve l'architecture freestanding clang/lld du projet original,
sans bibliothèque C/C++ à installer dans le jeu. Il ne déploie pas la DLL.

Les dépendances WSL peuvent être installées avec :

```sh
sudo apt install clang lld mingw-w64-x86-64-dev libvulkan-dev zip
```

Pour les contrôles statiques optionnels de l'exécutable, utiliser dans WSL :

```sh
ENSHROUDED_EXE='/mnt/c/chemin/vers/enshrouded.exe' ./build.sh test
```

Pour vérifier le parseur et le planificateur avec les sanitizers :

```sh
c++ -std=c++17 -g -fsanitize=address,undefined tests/test_project.cpp -o build/test_project-sanitize
./build/test_project-sanitize
c++ -std=c++17 -g -fsanitize=address,undefined tests/test_extended.cpp -o build/test_extended-sanitize
./build/test_extended-sanitize
c++ -std=c++17 -g -fsanitize=address,undefined tests/test_project_export.cpp -o build/test_project_export-sanitize
./build/test_project_export-sanitize
c++ -std=c++17 -g -fsanitize=address,undefined tests/test_extended_config.cpp -o build/test_config-sanitize
./build/test_config-sanitize
```

Les signatures contrôlées sur l'exécutable local passent au 17 novembre 2026.
Cela vérifie les correspondances binaires, pas le comportement du mod en cours
de jeu. Le test personnel des étapes ci-dessus reste le critère d'acceptation final.
