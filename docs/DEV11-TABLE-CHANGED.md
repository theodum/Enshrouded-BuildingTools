# Dev11 — stabiliser la vérification d'une seule table

**Bilan historique.** Le test utilisateur a depuis confirmé la reconstruction
classique d'une table et de ses décorations. La priorité actuelle est de réutiliser
ce BTBP par la voie classique dans les composites : suivre
[DEV12-CLASSIC-COMPOSITIONS.md](DEV12-CLASSIC-COMPOSITIONS.md). Ce nouveau mode
n'emprunte pas le scanner de découverte de composants présenté ci-dessous.

La `0.34.0-dev11` remplace le scanner dev10 étalé sur de nombreux callbacks
par une collecte complète, bornée et contrôlée **dans un seul callback serveur**.
Les mutations constatées pendant une collecte, les données invalides et les
lectures incomplètes restent bloquantes. Le projet d'essai reste exactement
**`Dev10_A_Table_Unique.btproject`**, un objet et zéro structure.

**Compilation et tests automatiques ne prouvent pas une pose en jeu.** Le
placement réel, l'apparition et la persistance après rechargement sont à valider
par vous. Aucun monde n'a été ouvert, aucune sauvegarde modifiée et aucune DLL
installée automatiquement. Le château et ses fichiers restent inchangés.

## Diagnostic fondé sur votre session dev10

Le journal installé `BuildingTools.log`, démarré à 13:52:39 le 22 novembre 2026,
contient les événements cités dans la demande. Son SHA-256 et les observations
utiles sont conservés dans `BuildingTools-Projects/Dev11_Scanner_Evidence.json`.
Il confirme la résolution de la table `0xd109e828`, de son GUID et de ses bornes,
la bonne ABI des transforms et les deux actions. Aucun appel de placement natif
n'a été atteint dans ces deux opérations.

| Action | Heure du refus | Dernier slot | Compteurs du journal |
|---|---|---|---|
| Vérifier | 13:53:36.980 | 599 / 1 048 576 | 34 entités examinées, 3 props, 0 retenu |
| Construire | 13:53:44.078 | 978 / 1 048 576 | 48 entités examinées, 7 props, 0 retenu |

Les compteurs dev10 s'accumulaient sur les redémarrages ; ce ne sont pas les
nombres totaux d'entités du monde. Les bornes nulles exclues à distance sur
l'item `0xebc4d37a` ne déclenchent pas `E_TABLE_CHANGED`.

Le chemin de refus est `project_prop_scan_ready` → `bp_snapshot_step` →
`bp_snapshot_fail(TableChanged)`. Le test dev10 était cette disjonction, entre
**deux appels** au callback :

```text
world différent OU pointeur de bits différent OU pointeur de valeurs différent
OU capacité différente OU nombre global d'entités différent
```

Après deux redémarrages, une troisième différence arrêtait l'opération. Le
verrou `g_stateLock` protège l'état du mod ; il **ne fige pas le monde natif**
entre ces appels. Les créations/destructions d'entités sont normales, même loin
de l'emplacement où la table doit être posée. Un changement de contexte monde
entre callbacks pouvait également déclencher cette même condition.

Le défaut de stratégie est établi : la boucle limitait le travail à **512 slots**,
y compris vides, ou environ 2 ms, puis attendait un autre callback de confort.
Les événements de cette session sont espacés d'environ une seconde. Un million
de slots exige au moins **2 048 callbacks**, donc potentiellement des dizaines
de minutes, tout en exigeant la stabilité globale. Le délai maximal de 30 s ne
permettait même pas ce parcours sans mutation à cette cadence.

**Ce que le log dev10 ne permet pas d'affirmer :** il ne consignait pas le champ
différent ni ses valeurs avant/après. Il ne permet donc pas d'attribuer ce refus
précis à un compteur, à un pointeur, ou à un autre monde, ni de prouver un faux
positif numérique. La correction ne suppose pas que la collection soit immuable.

## Structure native et ABI vérifiées

L'exécutable installé a été lu sans exécution. SHA-256 :
`5d9ef2f140b8ed49e30dbee655841b89d08baec20f799a1817e171a18a850c77`.
Les signatures historiques sont vérifiées par la suite core.

| Élément | Preuve dans le binaire installé |
|---|---|
| Recherche d'entité `0x9b25f0` | Monde `[[sys]+0x30]`, map à `+0x148`, valeurs à `+0x188` |
| Recherche dans la map `0x9b4dc0` | Test du nombre à map `+0x54`, bits `+0x08`, capacité `+0x10`, clés uint32 `+0x28` ; ces offsets sont bien utilisés par le jeu |
| `CurrentTransform` / `WorldTransform` | Réflexion de taille **56** ; position int64×3 à 0, quaternion float×4 à 24, échelle float×3 à 40 |
| `UsedItem` | Réflexion de taille **4**, champ `itemId` à 0 |
| Index de composant transform | **133 dans votre session**, lu depuis le handle natif ; aucune constante 133 dans le scanner |
| Index du composant item | Découvert comme par la capture classique ; aucune constante 610 imposée |

Dev11 vérifie aussi le code de recherche dans la map, et pas seulement celui
du getter. Des tests modifiant les offsets/types réfléchis rendent ces gardes
fausses. Cela n'invente pas de garantie de stabilité entre callbacks : nombre
d'entités et génération de collection sont deux notions distinctes.

## Choix de lecture

| Méthode évaluée | Décision |
|---|---|
| Requête spatiale native | Les fonctions existantes demandent des composants enregistrés, limitent les résultats et ne couvrent pas tous les props ; un type non enregistré est dangereux. Aucune requête inconnue n'est envoyée. |
| Requêtes de confort / interaction / échelles | Utiles pour ces catégories ; une réponse vide ne prouve pas l'absence de fenêtres, clôtures ou décorations sans ces composants. |
| Ancien parcours incrémental global | Inadapté à la cadence observée ; sa stabilité sur plusieurs secondes n'est pas une condition réaliste. Un nombre identique ne détecterait pas non plus un remplacement à effectif constant. |
| Capture blueprint classique | Elle parcourt la collection dans le callback serveur, sans attendre des centaines de callbacks. Dev11 réutilise ce contexte et ces composants, en ajoutant contrôles de cohérence et budgets. |

Un index spatial complet éviterait un inventaire global ; les interfaces
actuellement identifiées ne donnent pas cette preuve de couverture. Dev11 garde
donc une collecte exhaustive des **entrées occupées**, sans visite individuelle
des cases vides. Pour une capacité de 1 048 576, il lit **16 384 mots de bits**
et examine seulement les entités marquées. Les calculs de collision conservent
uniquement les props dont la boîte transformée intersecte la zone cible élargie.
Un grand objet dont le pivot est extérieur mais la boîte intersecte reste retenu.

## Collecte et contrôle de cohérence

Dans un appel au callback natif de confort :

1. Lire l'identité du monde et de la table, les pointeurs des bits/clés/valeurs,
   la capacité et le nombre d'entrées. Vérifier ABI, limites et lisibilité.
2. Copier le masque d'occupation dans la mémoire du mod. Énumérer ses bits à 1.
   Avant chaque entrée, revérifier l'en-tête et son mot d'occupation afin de ne
   pas continuer dans un ancien tableau réalloué. Vérifier ensuite clé ↔ ID
   d'entité, composants et ligne de données.
3. Lire les IDs et transforms des props, puis leurs bornes natives de placement.
   Conserver les candidats dans un **tampon de travail non publié**.
4. Relire l'en-tête ; vérifier l'effectif réellement énuméré et le masque complet.
5. Vérifier à nouveau les clés/pointeurs, l'archetype, le bloc de données, la ligne,
   le composant item et les 56 octets de transformation utilisés pour chaque prop.
   Un effectif identique n'autorise pas un remplacement ou un déplacement invisible.
6. Relire encore l'en-tête, le masque et le handle transform. Publier les candidats
   uniquement après réussite de tous ces contrôles et du budget temporel.

Des changements entre **deux demandes terminées** sont admis : chaque demande
repart de la collection actuelle. Aucun pointeur d'entité d'un essai précédent
n'est réutilisé pour poursuivre une collecte. Les adresses gardées dans les traces
servent au diagnostic ; les résultats publiés contiennent ID d'item, ID d'entité
et boîte copiée. L'état du mod reste verrouillé pendant la capture.

La collecte utilise le contexte de lecture déjà employé par le blueprint
classique. Les contrôles avant/après détectent les incohérences observables ;
ils ne constituent pas un nouveau verrou du moteur. La stabilité et la durée
réelles de ce contexte restent à éprouver en jeu. Une garde ne sera pas retirée
pour faire disparaître un nouveau refus.

Budget : **200 ms par callback**, vérifié régulièrement et après chaque lecture
native de bornes, 131 072 entrées occupées,
16 777 216 slots et 512 props pertinents. Un dépassement refuse l'opération,
sans reprendre une moitié de collecte comme si elle était complète. Il s'agit
d'un seuil d'abandon, pas d'une préemption d'un appel natif en cours. Une vraie
invalidation pendant le callback autorise deux nouvelles tentatives complètes ;
la troisième invalidation produit toujours `E_TABLE_CHANGED`.

La vérification consommatrice refuse un snapshot vieux de plus de **250 ms**,
au début et à la fin du contrôle d'un lot. L'annulation, le chargement d'un projet
ou une nouvelle opération invalident les résultats précédents. Les lots gardent
leurs vérifications de support, voxels, prix natif et autels. Le pont PlaceProp,
le paiement et la sérialisation BTBP ne changent pas dans cette livraison.

Les boîtes restent des **AABB natives de placement**, pas une représentation
exacte des meshes physiques. Les bornes absentes/non finies et les proxies
dégénérés près de la cible restent non résolus et bloquants. Un ID natif inconnu
dans un composant item n'est pas considéré comme une absence d'objet.

## Journaux attendus et interprétation

La méthode est nommée `classic_ecs_bitset_snapshot`. Les lignes suivantes séparent
la lecture et le résultat de collision :

```text
native scan REQUEST method=classic_ecs_bitset_snapshot ...
native scan COHERENCE ... phase=before world=... table=... bits=... keys=... values=... slots=... count=...
native scan COHERENCE ... phase=after ...
native scan COMPLETE ... code=OK visited=... props=... relevant=... published=1 words=... restarts=... attempts=... elapsed_ms=...
native collision CHECK method=native_placement_aabb result=CLEAR ...
```

`COMPLETE ... collision=NOT_CHECKED` signifie uniquement une lecture cohérente.
Il faut ensuite le résultat du contrôle spatial, des appuis et des autels.
En cas de chevauchement : `result=COLLISION`, ID et index de l'objet prévu,
ID d'entité et ID d'item existants. Les doublons gardent leur refus explicite.

Une invalidation donne `W_TABLE_CHANGED change=entity_count`, `world`, `capacity`,
`occupancy_content`, `entity_entry`, `prop_transform`, etc., avec les deux
en-têtes. Une différence de contenu donne aussi `native scan CONTENT field=...`
et les valeurs attendue/observée. Pour un transform, l'index d'octet différent
et ses deux valeurs sont consignés. Les compteurs de l'essai courant sont
distincts du nombre de redémarrages. Une lecture abandonnée a `published=0`.

Les vérifications de mémoire utilisent les régions réellement validées par
`VirtualQuery`, mises en cache uniquement pendant le callback. Le cache est vidé
à chaque collecte ; il n'autorise ni pointeur deviné ni ancienne permission de
session. `memory_queries` et `memory_cache_hits` permettent d'en vérifier le coût.

| Code / résultat | Sens |
|---|---|
| `E_TABLE_CHANGED` | Invalidation réelle observée pendant trois collectes ; lecture instable, aucune zone déclarée libre |
| `E_WORLD_TABLE_INVALID` | Effectif et occupation incompatibles, ou slot hors capacité |
| `E_WORLD_READ_CALLBACK_BUDGET` | Collecte trop lente pour la limite de 200 ms ; garder `elapsed_ms` et les compteurs |
| `E_WORLD_ENTITY_BUDGET`, `E_WORLD_OBJECT_CAPACITY` | Limite d'entrées ou de candidats dépassée |
| `E_WORLD_SNAPSHOT_STALE` | Résultat devenu trop ancien ; refaire Vérifier une fois, sans forcer le placement |
| `E_ENTITY_UNREADABLE`, `E_COMPONENT_UNREADABLE`, `E_WORLD_ABI_UNSUPPORTED` | Lecture invalide ou structure incompatible ; aucune collision affirmée |
| `E_ITEM_COMPONENT_UNOBSERVED` | Aucun composant item fiable découvert ; faire une capture classique d'un meuble connu pour initialiser cette découverte |
| `E_WORLD_ITEM_UNRESOLVED`, `E_DEGENERATE_PROXY_NEAR_TARGET` | Objet ou dimensions non résolus ; conserver son ID et les données du log |
| `result=COLLISION` | Chevauchement confirmé des boîtes de placement, pas une simple panne de lecture |
| `E_WORLD_READ_CANCELLED` | Résultat annulé et jeté ; aucune reprise de ce résultat |

## Test en jeu — uniquement une table

1. Fermer Enshrouded. Installer manuellement le ZIP **Standalone dev11** à côté
   d'`enshrouded.exe`, en gardant les dossiers. Conserver la DLL dev10 pour retour
   arrière. Les dépendances nécessaires, langues, catalogue et module BTBP sont
   dans le ZIP ; les imports natifs n'exigent aucun runtime supplémentaire.
2. Démarrer un monde **de test local**. Vérifier `Building Tools 0.34.0-dev11`
   et `native world reader ABI: 1` dans `BuildingTools.log`. Le journal dev10
   précédent devient `BuildingTools.prev.log` au démarrage : en garder une copie.
3. Prendre le vrai cube du marteau de 0,5 m ; ouvrir son sélecteur natif pour
   observer le registre et les autels. Activer le placement natif expérimental.
   En créatif, activer aussi l'option des objets natifs créatifs ; sinon posséder
   la table requise. La table n'exige pas de copie logique.
4. Préparer un sol existant plat de **5 × 4 m** sous autel. Mettre l'origine
   au centre du sol sur sa face supérieure, rotation globale 0. Noter cette
   origine exacte. Charger **Dev10_A_Table_Unique**, pas le groupe de cinq meubles.
   Bilan attendu : **1 objet, 0 cellule, 0 pièce**.
5. Cliquer **Vérifier** une fois. Attendre : `native scan COMPLETE`,
   `native collision CHECK ... result=CLEAR`, puis validation **1/1**. Conserver
   `visited`, `words`, `count`, `relevant`, `elapsed_ms` et les adresses avant/après.
   Un refus est un résultat à analyser ; ne pas désactiver sa garde.
6. Cliquer **Construire** une fois et attendre. Le scan est refait dans le pipeline.
   Attendu ensuite : `NATIVE_CALL_ACCEPTED` et `OBJECT_RESULT ... success=1
   placed=1 failed=0`. Vérifier qu'une seule vraie table apparaît, à bonne hauteur
   et orientation. Un appel accepté sans table visible est un échec.
7. Après apparition, refaire seulement **Vérifier** à la même origine : la table
   existante doit être détectée et empêcher le doublon. Ne pas relancer Construire.
8. Quitter normalement le monde puis le jeu. Relancer le même monde et vérifier
   présence, position, rotation et comportement de la table. C'est la validation
   de persistance ; les compteurs précédents ne la prouvent pas.

Les essais complémentaires utilisent toujours ce **même projet à une table** :

| Situation | Résultat à vérifier |
|---|---|
| Zone vide sur sol existant | Lecture complète, pas de collision de prop, appui et autel valides |
| Meubles loin de la cible | Ils sont examinés mais exclus des candidats ; la zone libre reste utilisable |
| Meuble proche sans chevauchement | Il peut être candidat du voisinage ; le contrôle exact des boîtes reste libre |
| Meuble à l'emplacement cible | Collision / doublon explicite, aucun appel de pose |
| Objet aux dimensions nulles | Distant et excluable : compté ; près de la cible : refus non résolu, aucune fausse collision |
| Activité du monde entre deux Vérifier | Nouvelle collecte complète, sans exiger la stabilité de la demande précédente |
| Échap après Vérifier/pendant l'attente | Annulation sans réutilisation des candidats ; nouvelle demande nécessaire |
| Monde très chargé | Terminaison avec compteurs et durée, ou refus de budget explicite ; jamais succès partiel |

Pour tout échec, conserver `BuildingTools.log`, `BuildingTools.prev.log` si utile,
`BuildingTools-Projects.log`, le message affiché et le dernier export natif.
Les lignes COHERENCE/CONTENT/REFUSED/COMPLETE et OBJECT_RESULT suffisent à
identifier la phase et le champ en cause. Aucun ameublement du château n'est
préparé dans cette livraison.

## Validation automatique et limites

La suite `test_native_snapshot.cpp` compile le **lecteur de production** avec
des entités synthétiques et des accès mémoire bornés. Elle couvre zone vide,
meubles éloignés/proches, chevauchement, pivot extérieur, échelle, bornes nulles
et NaN, ID inconnu, composant corrompu, ajout d'entité, remplacement à effectif
constant, masque modifié, item/transform modifié, monde changé, tampon de valeurs
réalloué avec libération effective de l'ancien sous ASan, redémarrages
épuisés, budgets et annulations avant/pendant/après lecture.
Les tests couvrent aussi les limites de régions mémoire, le débordement d'adresse
et l'invalidation du cache de permissions entre deux callbacks.

Le cas volumineux comporte **1 048 576 slots, 60 000 entrées occupées et un seul
candidat** ; un appel au callback le termine. Le temps hôte imprimé est celui
de fonctions natives simulées, **pas une prédiction du temps en jeu**.
Les tests du précontrôle spatial vérifient aussi le refus d'un résultat périmé,
les collisions, appuis, autels et le fichier réellement livré de la table.

Les suites historiques de construction, matériaux, multiétages, escaliers,
blueprints, calibration et exports sont conservées. Les vérifications de DLL/ZIP
contrôlent PE x64, imports, CRC, identité des fichiers, traductions et liens.
Les tests mémoire utilisent AddressSanitizer et UndefinedBehaviorSanitizer.
Les commandes reproductibles sont `bash build.sh test`,
`sh tools/test_native_scanner_sanitizers.sh`, puis `bash build.sh package` et
`python tools/verify_package.py` pour les archives.

**Restent à valider en jeu :** durée sur votre monde, absence de nouveau refus
de cohérence, pose native, présence réelle, interactions et persistance.
Les anciennes étapes de calibration restent documentées dans
[DEV10-NATIVE-PLACEMENT-FIX.md](DEV10-NATIVE-PLACEMENT-FIX.md) ; le protocole
prioritaire actuel se limite à la table ci-dessus.
