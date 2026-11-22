# Dev10 — diagnostic du placement natif et recette en jeu

**Bilan historique :** le test utilisateur dev10 résout la table mais refuse
la lecture sur `E_TABLE_CHANGED`. La collecte a depuis été remplacée en dev11.
Suivre en priorité [DEV11-TABLE-CHANGED.md](DEV11-TABLE-CHANGED.md), uniquement
avec la table A ; les budgets et le parcours décrits ci-dessous sont ceux de dev10.

Livraison du 22 novembre 2026, `0.34.0-dev10`, Windows x64 Standalone.
La correction est implémentée et testée automatiquement. **Le placement réel,
les interactions et la persistance restent à tester dans Enshrouded par vous.**
Aucune sauvegarde n'a été ouverte ou modifiée ; aucune DLL n'a été installée
automatiquement. L'architecture et les fichiers du château restent inchangés.

## Ce qui explique les refus dev9

Le fichier joint à la demande contient le cahier des charges, sans pièce jointe
`BuildingTools(5).log`. Le journal installé `BuildingTools.log` contient néanmoins
les quatre refus décrits, le 22 novembre à **12:23:06.453, 12:23:10.517,
12:23:13.567 et 12:23:31.872** : trois Vérifier puis un Construire. La session
charge bien la dev9, trouve les signatures et valide l'ABI des items.
Les empreintes et un extrait des métadonnées réellement exportées sont livrés
dans `BuildingTools-Projects/Dev10_Failure_Evidence.json`.

Le projet réellement exporté, `Extended_20261122_122323_701357656.btproject`,
contient **La carte des dragons**, ID `0xd01eb1b8`, un objet, zéro cellule et
zéro pièce native. Le refus cite **un autre item**, `0xd1d489cb`, **Semis de riz**.
Ce n'est donc pas la résolution de l'objet du projet qui a déclenché ce refus.

Le chemin concerné était `project_prop_scan_ready` → `bp_snapshot_request` →
`bp_snapshot_step` dans `bpobjects.h`, puis le message commun
`S_EXT_PROP_SCAN_FAILED`. Le lecteur parcourait les props du monde et exigeait
une boîte strictement positive **avant** de filtrer leur intersection avec la
zone cible. Un prop dont la boîte native est dégénérée pouvait donc interrompre
une opération sans rapport avec lui. `index=-1` est la **sentinelle volontaire
de lecture du monde**, et non un accès à l'objet numéro −1 du composite.

L'export réel `NativeCatalog_20261122_701307234.json` confirme :

| Item | Résolution observée | Conclusion permise |
|---|---|---|
| Carte `0xd01eb1b8` | `policy_compatible`, flags 0, boîte 0,6 × 0,6 × 0,1 m | Métadonnées autorisées ; aucun placement prouvé |
| Semis de riz `0xd1d489cb` | `invalid_bounds`, flags 0, template présent | Boîte inutilisable pour le contrôle strict ; erreur de lecture, pas collision confirmée |
| Bougie `0xaf3025e4` | `special_flags`, flags **0x80001** | Refus de politique distinct ; activer la copie logique seule ne suffit pas |

**Limite du diagnostic historique :** la dev9 n'enregistrait ni les valeurs
brutes des bornes invalides ni la position du semis en cause. On peut identifier
le mauvais item et la famille de refus, mais pas reconstruire le sous-cas
numérique exact — boîte nulle, plate, non finie, etc. — ni affirmer que le semis
était éloigné. Un dépassement du plafond d'objets utilisait aussi ce chemin
générique. Les nouvelles traces lèvent cette ambiguïté au prochain essai.

L'autre message, « Identifiant natif ou dimensions mesurées indisponibles »,
provenait notamment du générateur de pièce : les huit rôles sont renseignés
dans l'export, mais la bougie est explicitement interdite. Dev10 affiche
maintenant le **rôle, l'ID et `special_flags`**, sans remplacer la bougie ou
assouplir les flags. Le premier essai utilise seulement une table autorisée.

## Audit des données et du mécanisme d'origine

| Donnée | Source utilisée et portée |
|---|---|
| ID d'inventaire 32 bits | CSV puis recherche exacte `g_bpItemById` dans le registre actuel ; identité retournée vérifiée |
| Template placé 128 bits | `EquipmentSetup.placedEntity`, item `+0x3c0` ; indispensable à PlaceProp |
| ID d'entité du monde 32 bits | Entité capturée `+0x10` ; identité de cette instance, pas ID d'inventaire |
| Composant de l'item placé | Découvert par le lecteur original ; **610** dans la session observée, pas fixé à 610 dans le code |
| Transformation du monde | `CurrentTransform`/`WorldTransform` **56 octets** : position int64×3 à 0, quaternion float×4 à 24, échelle float×3 à 40 |
| Volume local | Fonction native issue de `BpLocalBounds`, **AABB de placement** ; ce n'est pas une mesure du mesh ou de toute la physique |
| Autels/support/coût | Lectures de zones et requêtes natives existantes ; contrôles répétés dans le pipeline composite |
| Modèle, liens, contenu de coffre | Le template est confié au moteur ; ces propriétés ne sont pas déduites du CSV ni sérialisées dans le BTBP |

Le CSV fournit des noms et IDs d'inventaire, **pas** les GUID, pivots, boîtes,
échelles ni règles d'attachement. Les catégories restent des aides de recherche.
La bibliothèque locale ne transforme jamais une référence CSV en autorisation.

L'exécutable installé est lu sans être exécuté : PE `TimeDateStamp=1793791052`,
`SizeOfImage=44298240`. Les 40 signatures historiques sont uniques. Audit des
fonctions de cette version, adresses relatives à la base du module :

| Élément | Observation sur le binaire installé |
|---|---|
| `BpComfortRead` | Site `0x253f52` ; callback original puis lecture dans le contexte du système serveur local |
| Recherche d'entité | Fonction `0x9b25f0`, monde `+0x30`, map `+0x148`, valeurs `+0x188` |
| `BpLocalBounds` | Site `0x2daade`, cible `0x494210` ; lit notamment item `+0x3d0/+0x3dc`, `placementAABBmin/max` |
| `PlaceProp` original | Site `0x2dad03`, appel à +0x11, cible **0x48deb0** ; ne pas reprendre les anciennes adresses en commentaire |

Les descripteurs réfléchis d'`ItemInfo`, `EquipmentSetup`, des flags et des
transforms, ainsi que le code de recherche d'entité, sont vérifiés. Un décalage
d'ABI désactive la fonction concernée au lieu de deviner les nouveaux offsets.

Le blueprint original capture les entités dont le pivot est dans la sélection,
lit item/position/quaternion et les inscrit en BTBP v1. Chaque entrée fait
**32 octets : ID uint32 + position float×3 + quaternion float×4**. La position
est relative au coin de capture. Pas d'échelle, de contenu, de carburant ni
d'état de réseau dans ce format. La capture réelle reste à valider ; les tests
de sérialisation n'en constituent pas une preuve.

À la reconstruction, `bp_place_object` retrouve l'item, récupère son template
et ses bornes, prépare le payload natif de 0x50 octets et appelle
`g_placePropOrig(ctx, info, item)`. Les composites utilisent **ce même pont**,
le même paiement, le même writer des objets à logique et le même undo partiel.
La nouvelle erreur arrivait pendant leur précontrôle supplémentaire, avant cet
appel. Un retour natif positif signifie « appel accepté », pas « instance
retrouvée après rechargement ».

## Corrections et protections

Le lecteur progressif est isolé dans `native_snapshot_runtime.h` et testé avec
de la mémoire ECS bornée. Il prend désormais en compte l'échelle réelle et les
huit coins transformés de la boîte. Une boîte native **finie, ordonnée mais
plate ou ponctuelle** peut être écartée si son volume transformé est hors de la
zone de contrôle. Aucun rayon de pivot ni dimension de remplacement n'est utilisé.
Si elle intersecte la zone, l'opération est refusée comme **lecture non résolue**.
Une boîte absente, non finie ou corrompue reste bloquante, même éloignée : on ne
peut pas établir son exclusion. Ce contrôle porte sur les proxies de placement,
pas sur les collisions exactes des meshes ; cette limite reste à vérifier en jeu.

Le lecteur contrôle pointeurs, composant, stride, ligne, quaternion, échelle,
capacité et changement de table. Budget : 512 slots ou environ 2 ms par passage,
espacement minimal 8 ms, 512 objets pertinents conservés, deux redémarrages
autorisés si la table change, délai maximal de demande 30 s. Un monde très
chargé peut encore dépasser ces budgets ; cela provoque un refus explicite.
Les captures et lectures partagent le verrou d'état du mod. Le lecteur s'exécute
dans le callback natif, pas dans un scan de mémoire depuis l'interface.

Les composites restent réservés au **monde hébergé localement**. Le mode client
d'un serveur distant et le serveur dédié restent refusés pour les composites.
La lecture native doit avoir été observée récemment (moins de 3 s). La stabilité
des pointeurs et du nombre d'entités ne constitue pas une preuve d'immobilité
de tous les objets entre deux callbacks ; les contrôles sont repris par lot.

Le cube de substitution de 0,25 m de l'ancien pont est supprimé : une pose sans
bornes natives valides échoue avant PlaceProp. Les vérifications d'IDs inconnus,
flags, templates, autels, voxels, contacts et chevauchements restent actives.
Un objet existant du même item chevauchant la cible provoque un arrêt explicite
afin d'éviter une duplication ; il n'est pas déclaré identique ou sauté sans preuve.
Il n'y a pas de reprise automatique des objets déjà traités.

Les traces par objet donnent nom, ID, GUID, source, limites/dimensions,
position/quaternion, repère local ou candidat monde, état de scan, étape et
résultat natif. Les compteurs « placed/success » existants désignent toujours
les appels acceptés. `OBJECT_RESULT` et les erreurs sont conservés dans les journaux.

| Nouveau code de lecture | Sens et réponse au test |
|---|---|
| `E_COMFORT_INACTIVE` | Lecteur serveur pas observé récemment ; ouvrir le sélecteur natif, rester dans la base et réessayer |
| `E_WORLD_ABI_UNSUPPORTED` | Disposition native différente ; conserver le log, ne pas désactiver la garde |
| `E_TABLE_CHANGED` | Table modifiée plus de deux fois pendant la demande ; refaire l'essai au calme |
| `E_COMPONENT_UNREADABLE`, `E_ENTITY_UNREADABLE` | Donnée indisponible ; aucune collision n'est affirmée |
| `E_WORLD_PLACEMENT_BOUNDS_MISSING/CORRUPT` | Bornes impossibles à utiliser ; lire `native scan DATA`, ne pas inventer de dimensions |
| `E_DEGENERATE_PROXY_NEAR_TARGET` | Proxy plat/nul proche ; identifier l'entité et choisir une autre zone de test libre |
| `E_WORLD_OBJECT_CAPACITY`, `E_WORLD_READ_TIMEOUT` | Budget 512 objets pertinents ou 30 s dépassé |

Les exports dev10 ajoutent `boundsDiagnostic` aux entrées de bornes invalides.
Les valeurs brutes sont du texte, y compris `nan/inf`, pour conserver un JSON valide.

## Calibration native sans saisie d'ID

Dans la fenêtre **Sauvegarder un blueprint**, activer
**Calibrer un objet posé manuellement pendant cette capture**. Le lecteur
classique doit trouver exactement **un** meuble complet. La capture produit le
BTBP habituel et un fichier `Calibration_*.btcalibration` neuf dans
`BuildingTools-Projects`. Ce fichier conserve ID, GUID réellement porté par
l'entité, ID d'instance observée, position monde, position locale, quaternion,
échelle, bornes natives et identité du build du jeu. Aucun monde n'est modifié.

Le GUID de l'entité doit correspondre au template placé de l'item résolu.
L'échelle doit être unitaire : le format BTBP ne permet pas d'en restaurer une
autre. Les refus sont explicites, notamment `E_CAPTURE_NEEDS_EXACTLY_ONE`,
`E_CALIBRATION_SCALE_UNSUPPORTED`, `E_CALIBRATION_TEMPLATE_CHANGED` et
`E_CALIBRATION_GAME_CHANGED_RECAPTURE`. La calibration ne rend pas compatibles
les objets spéciaux ou les bornes invalides.

Dans **Bibliothèque native**, cliquer **Charger une calibration enregistrée**,
puis le fichier. Cela prépare un brouillon neuf d'un seul objet et zéro structure,
sans le construire. La rotation est conservée ; le bas-centre de la boîte
mesurée devient l'origine du nouvel exemplaire. La pose capturée d'origine reste
dans la définition pour diagnostic, elle ne devient pas automatiquement la
destination du duplicata. Exporter **Uniquement le mobilier**, puis recharger le
nouveau `.btproject`. La calibration est revalidée contre le registre et le
build actuels. Après mise à jour du jeu, recalibrer au lieu de réutiliser une
ancienne approbation. Les fichiers incomplets `.part` ne sont pas chargeables.

## Installation pour le test

1. Fermer Enshrouded. Conserver votre DLL précédente pour revenir à la dev9.
2. Extraire **`EnshroudedBuildingTools-0.34.0-dev10.zip`** à côté de
   `enshrouded.exe`, en conservant l'arborescence. Il contient `dinput8.dll`,
   les 15 langues, le catalogue, les projets et ce guide. Aucun runtime externe
   supplémentaire n'est nécessaire : les imports DLL sont uniquement
   KERNEL32, USER32 et GDI32. Utiliser l'autre archive uniquement si Shroudforge
   est déjà votre chargeur, sans installer les deux dispositions ensemble.
3. Vérifier le début de `BuildingTools.log` : version **0.34.0-dev10**,
   `native world reader ABI: 1`, ABI native des objets valide. Le fichier du
   journal précédent devient `BuildingTools.prev.log` au démarrage suivant.
4. Ouvrir votre monde **de test local**, prendre le marteau, sélectionner son
   vrai cube de **0,5 m** et ouvrir le sélecteur natif des matériaux pour observer
   registre/autels. Activer les objets natifs expérimentaux dans Extended.
   Pour le test gratuit, activer le créatif et **Objets natifs créatifs** ; sinon
   posséder les meubles requis. Ces options utilisent les mécanismes existants.
5. Pour A, préparer un sol existant plat d'au moins **5 × 4 m**, en zone d'autel,
   sans meuble dessus. L'origine visée est au centre du sol, **sur sa face
   supérieure**, pas au centre d'un cube. Le mode libre F10 et le plan virtuel
   permettent de fixer cette altitude. Conserver origine et rotation dans vos notes.

## Étapes A à E — à exécuter dans cet ordre

### A. Un seul meuble natif vérifié dans votre registre

Charger **`Dev10_A_Table_Unique.btproject`** depuis Projets composites.
Son module `Dev10_A_Table_Unique.btbp` doit rester à côté. Le bilan attendu est
**1 module, 0 cellule, 0 pièce, 1 objet**. C'est la **Table en bois brut**,
`0xd109e828`, template `ddae6400a2e77345961e551beb051aa0`, flags 0, dont les
bornes réellement exportées sont `[-1.240024,0,-0.650846]` à
`[1.247342,0.9,0.649154]`. Pivot local compensé :
`[-0.003659,0,0.000846]`, quaternion `[0,0,0,1]`.
Cela vérifie les métadonnées, pas la pose.

Le bouton **Préparer une seule table en bois brut vérifiée (zéro structure)**
dans la bibliothèque offre la même préparation à partir du registre de la
session actuelle, notamment si les bornes ont changé. Il ne constitue pas une
calibration de capture et le journal le précise.

1. Positionner l'origine sur le sol préparé, rotation globale 0.
2. Cliquer **Vérifier**. Attendre la fin, sans relancer plusieurs commandes.
   Attendu : `native scan COMPLETE code=OK`, contrôle **1/1 valide** et aucun
   `OBJECT_RESULT` de construction. Une erreur de lecture doit afficher son code.
3. Cliquer **Construire** une seule fois. Attendre la fin. Rechercher
   `native object TRACE stage=native_accepted code=NATIVE_CALL_ACCEPTED`, puis
   `OBJECT_RESULT ... success=1 placed=1 failed=0` dans le journal de lots.
4. Vérifier visuellement qu'une seule table est là, à bonne hauteur et orientation,
   sans enfoncement ni chevauchement. Tester son comportement de meuble.
   Un compteur positif sans objet visible est un **échec**, à signaler.
5. Refaire **Vérifier** à la même origine après apparition : refus d'objet existant
   attendu. Ne pas reconstruire au même endroit pour poursuivre la recette.

Si la lecture bloque encore sur le semis, conserver les lignes `native scan
REFUSED` et `native scan DATA` : elles donneront les valeurs absentes du log dev9.
La correction ne prétend pas résoudre un proxy non fini ou absent en le sautant.

### B. Capture et reconstruction par le blueprint classique

1. Dans un emplacement séparé, poser **manuellement la même table** avec le jeu.
2. Utiliser **Blueprint → Sauvegarder**, nommer par exemple `Dev10_B_Table`.
   Choisir une sélection englobant cette table et aucun autre meuble. Placer le
   bas de la sélection **exactement sur la face supérieure du sol** pour cette
   table : son pivot y=0 doit rester inclus. Ne pas monter le bas au-dessus du
   pivot. Un volume de 5 × 4 m sur 1,5 m de haut, centré horizontalement sur la
   table, convient si aucun autre meuble n'est dedans. Si votre sélection inclut
   des voxels du sol, B les conserve ; l'export mobilier de C les exclura.
3. Activer l'option de calibration pendant cette capture et sauvegarder.
   Attendu dans le log : `blueprint objects: ... 1 objects`,
   `calibration SAVED code=OK ... source=classic_native_capture`, puis
   `blueprint saved: ... 1 objects`. Conserver **les deux fichiers** créés :
   le BTBP dans `BuildingTools-Blueprints` et la définition dans
   `BuildingTools-Projects`. Le message final de sauvegarde BTBP peut remplacer
   le message fugitif de calibration ; les fichiers et le log font foi.
4. Charger **ce BTBP classique**, hors de la liste des composites, et reconstruire
   sur un autre support libre sous autel. Vérifier présence et orientation.
   Noter séparément si B réussit alors que A échoue, ou l'inverse.

### C. Le même meuble depuis un composite sans voxel

1. Charger la calibration de B dans la bibliothèque. Attendu :
   `calibration DRAFT code=OK objects=1 cells=0 pieces=0`.
2. Exporter **Uniquement le mobilier**. Charger le nouveau manifest dans les
   composites : **0 cellule, 0 pièce, 1 objet** doit être affiché.
3. Choisir encore une zone libre, appliquer Vérifier puis Construire une fois.
   Contrôler compteur 1/1, présence, hauteur, orientation et interaction.
4. Conserver manifest, module BTBP, définition de calibration et logs. C doit
   réussir avec le même item que B ; le GUID ne doit pas être remplacé.

### D. Cinq meubles, uniquement après réussite de A, B et C

Le suivi **`Dev10_D_Cinq_Meubles.btproject`** est fourni séparément : un lit,
une table, deux chaises et un flambeau mural ; zéro structure. Les quatre types
ont tous `policy_compatible` dans votre export : flags 0 pour les meubles,
**0x80000 pour le flambeau**. Activer explicitement **Copie logique originale
expérimentale** dans la bibliothèque **avant de charger D** ; sinon le refus
`logic_disabled` est attendu. Cela n'autorise pas les flags spéciaux 0x80001
de la bougie et ne restaure aucun lien logique. La recette et les
mesures sont dans `Dev10_D.recipe.json` et `Dev10_D_Measured.json` ; aucune pose
en jeu n'y est déclarée validée. Il ne contient pas la bougie refusée.

Préparer un **sol existant de 10 × 6 m**, centré sur l'origine du projet,
face supérieure y=0. Préparer un petit **mur existant** de 1 m de large,
3 m de haut et 0,5 m d'épaisseur : x de −0,5 à +0,5 m, y de 0 à 3 m,
z de **+2 à +2,5 m** par rapport à cette origine. Le meuble mural reste du
côté z<2. Rotation globale **0** pour ce premier essai. Tout le volume doit
être sous autel. Le projet n'ajoute ni sol ni mur.

Les bas-centres du lit/table/chaises sont respectivement `(-3,0,0)`, `(2,0,0)`,
`(2,0,-1.5)` et `(2,0,1.5)`. La seconde chaise tourne de 180° ; leur pivot
vertical est compensé de −0,0171 m. Le flambeau a son centre vertical à 2 m
et sa face arrière de boîte au plan z=2. Les contacts de boîte passent sur un
monde simulé ; la fixation, l'orientation visuelle, la lumière et le carburant
du flambeau demandent un test natif réel.

Vérifier puis Construire une fois : attendre **5/5 appels acceptés, 0 échec**,
puis inspecter chaque meuble et le flambeau. Tout placement partiel doit être
rapporté comme tel ; ne pas relancer automatiquement le groupe à la même origine.
Le mod refuse les chevauchements déjà présents, il n'a pas de reprise par
identifiant persistant d'instance. Un objet posé sur table sera un essai ultérieur
avec un petit objet réellement autorisé, après ce groupe et sans forcer la bougie.

### E. Persistance — critère principal restant

1. Noter les positions et les nombres réels observés après A/C puis D si testé.
2. Quitter normalement le monde pour laisser le jeu sauvegarder ; quitter le jeu.
3. Conserver les deux journaux avant un autre démarrage, puis relancer le même monde.
4. Vérifier que tous les meubles testés sont toujours présents aux mêmes poses.
   Tester le lit, les chaises et le flambeau, selon leurs interactions natives.
5. Rapporter séparément **visible après pose**, **interactif** et
   **présent après rechargement**. Un retour positif de PlaceProp ne remplace
   aucune de ces observations.

Pour un échec, fournir la phase A/B/C/D/E, le message affiché, `BuildingTools.log`,
`BuildingTools.prev.log` si nécessaire, `BuildingTools-Projects.log`, le projet
testé et le dernier `NativeCatalog_*.json`. Les lignes `TRACE`, `REFUSED`,
`DATA` et `OBJECT_RESULT` permettent de localiser la cause.

## Vérification automatique et portée

| Contrôle | Preuve automatique, distincte de l'essai natif |
|---|---|
| Compilation | DLL PE32+ Windows x64, Standalone et disposition Shroudforge |
| Exécutable installé | 40 signatures, réflexion items/flags/transforms, code de recherche d'entité ; mutations négatives d'ABI refusées |
| Lecteur du monde | 32 contrôles du code réel avec ECS synthétique : proxy nul distant/proche, grand objet hors pivot mais dans la zone, échelle, NaN, composant invalide, redémarrages et plafond 512 |
| Calibration | 27 contrôles du code réel de fichiers/brouillon avec registre simulé : capture/relecture, GUID/build/échelle/bornes incompatibles, JSON invalide, allocation échouée et absence d'écrasement |
| Placement original | Suite de 3 845 assertions avec registre/espion natif simulés ; aucun cube de remplacement si bornes invalides ; résultat et compteurs |
| Démonstrateurs | 7 tests Python dev10 : sérialisation et relecture BTBP, pivots, IDs, rotations, zéro structure ; 85 contrôles du précontrôle natif sur monde simulé, dont les fichiers A et D sur sol/mur existants et leurs refus sans support/avec doublon |
| Construction existante | Suites composites/matériaux/multiétages/escaliers/export et vérification des fichiers/archives du château |
| Livraison | Contenu ZIP, CRC, imports/exports PE, DLL identiques dans les deux dispositions, traductions, liens et SHA-256 |

Ces tests n'ont pas lancé le jeu, placé d'objet natif ni testé de sauvegarde.
Les quatre suites de lecture, calibration, objets et précontrôle passent aussi
avec **AddressSanitizer et UndefinedBehaviorSanitizer**, détection de fuites activée.
Les résultats détaillés sont produits par `bash build.sh test`, les régressions
du château par `sh tools/test_royal_castle.sh`, et le paquet par
`python tools/verify_package.py`. **Le problème ne sera déclaré résolu qu'après
la validation réelle du placement et de la persistance.**

## Château et mises à jour

Aucun overlay du château n'est appliqué ni généré à partir d'une origine supposée.
Après A–E, conserver son **origine monde exacte et sa rotation globale**, puis
exporter des groupes d'objets par pièce avec la même ancre. L'export mobilier
ne contient aucun voxel, escalier, toit ou pièce native de structure. Pour les
recettes externes, `make_furnishing_project.py --reference ... --world-origin ...
--world-turn ...` exige ces informations et conserve les empreintes des sources.

La procédure de rafraîchissement CSV et fusion d'un export natif neuf reste
décrite dans [DEV9-CATALOG.md](DEV9-CATALOG.md). Une mise à jour du CSV ne valide
pas le nouveau moteur. L'ABI, les GUID et les bornes doivent être à nouveau
observés ; la calibration refuse un build de jeu différent.
