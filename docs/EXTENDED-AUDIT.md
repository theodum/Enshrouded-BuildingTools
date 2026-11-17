# Audit du fork — 17 novembre 2026

Point de départ : `6af1321` (Building Tools 0.33.2), branche `main`, arbre propre.
Remote existant : `git@github.com:theodum/Enshrouded-BuildingTools.git`.
Branche de travail : `feature/composite-projects`. Licence MIT et attribution
d'Aleksander Wojtal conservées. Aucun fichier de jeu ni sauvegarde modifié.

| Système | Code et constat |
|---|---|
| Format `.btbp` | `src/core.h`, `BpHeader`, `bp_encode`, `bp_decode`. BTBP v1 binaire little endian, cellules RLE, table de 64 matériaux, extension optionnelle de 512 objets. Index X + nx × (Z + nz × Y). |
| Limite de sauvegarde | `src/blueprint.h`, `kBpMaxCells = 65536`. Limite du tampon du mod ; une longueur RLE u16 ne limite pas le volume total du format. |
| Chargement | `bp_list_files`, `bp_load` dans `src/blueprint.h`. Répertoire `BuildingTools-Blueprints`. |
| Placement des blocs | `bp_fill` prépare les coûts via `price_shape` / `g_placeOrig`. Le jeu paie, puis `place_commit_hook_body` appelle `g_placeCommitOrig` pour chaque cellule avec le matériau résolu. |
| Matériaux | Registre natif `g_bpItemById`, `bp_block_item`, repli contrôlé pour les anciens identifiants de remboursement (#61). Ne pas inventer des identifiants. |
| Objets | `src/bpobjects.h` : requêtes du système de confort, `bp_place_object` réutilise `g_placePropOrig`. Objets câblés : lien logique non restauré. La copie n'est pas une sauvegarde de l'état des coffres ou des quêtes. |
| Créatif | `pay_hook` ignore coûts et remboursements des appels de construction reconnus ; catalogue de matériaux étendu. Ne couvre pas arbitrairement tous les objets du jeu. |
| Free Placement | `src/main.cpp`, curseur positionné devant la caméra et aligné à 0,5 m. Réutilisable pour viser le projet dans le vide. |
| Aperçu | `bp_box`, `bp_ghost_bits`, `area_box_live`, `src/vkdraw.h`. Cellules en instances Vulkan/masque, jusqu'à 40 000 cellules dans le volume englobant ; au-delà, volume seulement. Aucun placement pour dessiner. |
| Interface | `draw_mod_menu` (Home), `src/blueprint_ui.h` (fenêtres Save/Place natives, écran 52), roue Caps Lock et mode Blueprint. |
| Autels | PlayerBases : jusqu'à 16 enregistrements, boîtes et sous-boîtes ; `base_at` appelle la fonction native de couverture, signatures et structure vérifiées à l'installation. Aucun placement automatique d'autel fiable déjà exposé. |
| File et taille d'action | `kMaxPending = 20000`, `MaxShapes` configurable 1..20000 (2000 par défaut). Pas de transaction multi-frame existante. |
| Annulation | `UndoSplit` limite les enregistrements natifs et ajuste leurs coûts. L'annulation ne couvre qu'une partie des grosses actions. Pas de rollback complet garanti. |
| Multijoueur | Blueprints réservés aux mondes hébergés localement ; pas de transfert des fichiers au serveur dédié. Conserver cette restriction. |

## Compilation initiale

WSL Ubuntu 24.04, clang 18 et lld, en-têtes/imports MinGW-w64 et Vulkan,
`bash build.sh test`. Les dépendances lld, mingw-w64-x86-64-dev et libvulkan-dev
ont été installées dans WSL. La copie Windows de `build.sh` avait des fins de
ligne CRLF : conversion LF pour l'exécution initiale, puis attribut Git dédié.

Résultat avant changement fonctionnel : DLL Windows x86-64 compilée, tests
`test_core` réussis. Copie locale conservée dans `build/baseline/` (ignorée par Git).
Sans exécutable du jeu fourni, les vérifications des signatures sont sautées.
Ce résultat ne constitue pas une validation en jeu.

## Décision pour la première livraison

Ajouter un manifeste JSON `.btproject`, sans fusionner les modules dans un
`.btbp` géant. Plan occupé séparé, calculs entiers, raffinement des grilles
compatibles en cellules natives de 0,5 m, détection des chevauchements.
Conserver les appels natifs de prix, paiement, écriture et annulation, par lots.
Validation progressive avant la première écriture et revalidation avant chaque
lot ; arrêt explicite, journal conservé en cas d'échec. Pas de reprise aveugle ni
de promesse d'annulation totale. Les objets des composites restent exclus de
cette première version ; les blueprints originaux conservent leurs objets.

## Vérifications natives supplémentaires pour dev2

Analyse statique en lecture seule de l'exécutable local dont le SHA-256 est
consigné dans `EXTENDED-VALIDATION.md`. Les adresses ci-dessous sont propres
à ce binaire ; elles ne remplacent pas les signatures lors du chargement.

- `BaseAt`, RVA `0x470410` : le masque de zones à un bit utilise le volume
  global ; plusieurs bits sélectionnent les intersections avec les sous-volumes.
  Les propositions d'autels utilisent ces zones observées sans les modifier.
- `PlaceCommit`, RVA `0x48e6f0` : à `+0x8c`, le chargement de `[r15+0x1c]`
  alimente la recherche du matériau dans le registre. Le champ est un identifiant
  de matériau, pas un identifiant de joueur. La disposition du commit est conservée.
- `PlaceShape`, RVA `0x491a10` : à `+0x56`, le prix lit le matériau à `+0x454`
  sur son argument matériau. Les nouvelles pièces conservent séparément forme
  et matériau, sans changer la voie classique. Les deux instructions sont
  vérifiées avant d'autoriser les pièces natives, ainsi que dans `test_core`.
- Réflexion `keen::ItemInfo` : `flags` à `0x5d8`. L'énumération `keen::ItemFlag`
  confirme `IsLogicEntity` au bit 19 et `IsFactoryEntity` au bit 20 ; les drapeaux
  de connaissances, PIDE, PNJ et animaux sont également exclus des composites.
- Le problème du writer `LogicEntityPlacedEvent` décrit dans `bpobjects.h`
  demeure : aucune propriété sûre d'un writer extérieur au système Blueprint
  n'a été établie. Les composites refusent ces objets. Une trace native optionnelle
  en lecture seule prépare une investigation ultérieure en jeu, sans injection.

Les API de quota/progression d'autel et de rollback complet ne sont pas exposées
par les fonctions vérifiées. Les replis et les fonctions encore non livrées sont
détaillés dans `EXTENDED-STATUS.md`. Aucun accès aux sauvegardes n'a été nécessaire.
