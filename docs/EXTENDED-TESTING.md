# Building Tools Extended 0.34.0-dev2 — installation et recette

Version de développement du fork, basée sur Building Tools 0.33.2.
La compilation et les tests automatisés ne remplacent pas votre test dans le jeu.
**Le placement composite en jeu reste à valider personnellement.**

## Installation manuelle

Fermer le jeu. Utiliser l'archive locale
`dist/EnshroudedBuildingTools-0.34.0-dev2.zip`, ou l'archive Shroudforge si ce
chargeur est déjà utilisé. Les deux archives contiennent la même DLL.

1. Conserver une copie de votre DLL de mod précédente si vous souhaitez revenir
   à la version d'origine. Ne pas écraser les fichiers d'un autre chargeur.
2. Extraire l'archive standard à côté de `enshrouded.exe`. Elle contient
   `dinput8.dll`, la licence MIT, les langues, la documentation et le dossier
   `BuildingTools-Projects` avec les trois fichiers `.btbp` et leur manifeste.
3. Avec Shroudforge, extraire l'archive correspondante : la DLL est dans
   `mods/building-tools/BuildingTools.dll`. Conserver une seule copie de Building
   Tools chargée. Les projets restent à côté de l'exécutable du jeu.
4. Démarrer un **monde local de test** et vérifier la bannière 0.34.0-dev2 et
   `BuildingTools.log`. Aucun test ci-dessous ne doit utiliser votre monde principal.

Le développement n'installe rien dans le jeu. Aucun fichier `.ksd`, sauvegarde
Steam ou monde existant n'est modifié par les scripts de compilation et de test.
Avant tout essai de construction, conserver hors du répertoire du jeu une copie
de votre monde de test, jeu arrêté ; la DLL ne réalise pas de sauvegarde automatique.

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

Les essais complémentaires de dev2 sont décrits dans
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
```

Les signatures contrôlées sur l'exécutable local passent au 17 novembre 2026.
Cela vérifie les correspondances binaires, pas le comportement du mod en cours
de jeu. Le test personnel des étapes ci-dessus reste le critère d'acceptation final.
