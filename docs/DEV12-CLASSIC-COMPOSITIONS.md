# Dev12 — importer la composition classique « esaai blueprint tabel »

La dev12 reprend **le BTBP réel que vous avez reconstruit en jeu**, sans en
modifier un octet. Son composite contient **cinq objets et zéro voxel à
construire** : la table d'appoint, le bol, la bougie et deux sous-verres.
Il utilise désormais `bp_fill`, la file de coûts/commit et `bp_place_object`
du blueprint classique. Aucun second moteur de placement n'est ajouté.

**État de validation :** votre reconstruction classique et la réapparition des
cinq objets sont confirmées. Les tests automatiques de conversion, transformations,
file native et contrôles supplémentaires passent. La reconstruction **composite**,
les interactions et la persistance après rechargement demandent encore votre
test dans Enshrouded. Aucune sauvegarde n'a été ouverte ou modifiée par l'agent.
La DLL n'a pas été installée automatiquement. Le château reste inchangé.

## Fichier réellement analysé

Source : `BuildingTools-Blueprints/esaai blueprint tabel.btbp`, datée du
22 novembre 2026 à 16:18:22. Taille : **224 octets**. SHA-256 :

```text
7147654d0e59e4a69a99b8cb2338a8f4ada128651a654561b0074d6a636e956b
```

`Dev12_Table_Classique_Source.btbp` est une copie identique. Le manifeste seul
sélectionne les objets ; les 64 voxels de sol du fichier ne seront pas reconstruits.
Les fichiers d'analyse et de preuve sont livrés à côté du projet.

| Zone du fichier | Données |
|---|---|
| Octets 0–31 | BTBP v1, grille 0,5 m, dimensions 8 × 8 × 8, shape `0x766da1db`, une matière |
| Octets 32–47 | Palette : item `0x3c37b9d6`, matériau `0xc4`, 64 cellules/64 items |
| Octets 48–59 | Deux runs : 64 cellules de sol puis 448 cellules vides |
| Octets 60–63 | Nombre d'objets : 5 |
| Octets 64–223 | Cinq entrées de 32 octets, dans l'ordre ci-dessous |

Les positions ci-dessous sont en mètres, relatives au coin de capture.
Le JSON `Dev12_Table_Classique_Analysis.json` conserve leur précision complète.

| Ordre / offset | Objet / ID d'inventaire | Position locale | Quaternion x, y, z, w |
|---|---|---|---|
| 0 / 64 | Bol en bois `0x58bbb7d6` | 0,910644531 ; 1,199218750 ; 2,737792969 | 0 ; 0,965568542 ; 0 ; −0,260148853 |
| 1 / 96 | Bougie `0xaf3025e4` | 0,337890625 ; 1,199218750 ; 2,747314453 | 0 ; 0,999999046 ; 0 ; −0,001383771 |
| 2 / 128 | Petit sous-verre `0x301dfbe1` | 0,767089844 ; 1,199218750 ; 3,099609375 | 0 ; 0,866290152 ; 0 ; 0,499541193 |
| 3 / 160 | Petit sous-verre `0x301dfbe1` | 0,338378906 ; 1,199218750 ; 3,129638672 | 0 ; 0,866290152 ; 0 ; 0,499541193 |
| 4 / 192 | Table d'appoint en bois `0xc672cf0d` | 0,741699219 ; 0,449951172 ; 2,930419922 | 0 ; 0,999999046 ; 0 ; −0,001383771 |

Cette table est différente de la table brute `0xd109e828` du démonstrateur dev10.
La source contient bien **une composition**, et pas un modèle fusionné en un objet.
La table figure en dernier ; cet ordre est conservé dans le lot de reconstruction.

Le journal installé de votre session dev11 montre la capture de **38 175 entités
en 250 ms**, puis cinq objets sauvegardés à 16:18:22.596. À 16:18:39.600–601,
les cinq appels de placement sont acceptés, bougie comprise. Les 64 voxels et les
cinq objets donnent 69 écritures dans la reconstruction classique. Votre retour
visuel confirme leur présence. L'extrait et l'empreinte du journal sont dans
`Dev12_Classic_Test_Evidence.json`. Ces faits ne prouvent pas encore la persistance.

## Ce que le blueprint conserve réellement

Une entrée BTBP écrit **ID d'inventaire uint32 + position float32×3 + quaternion
float32×4**. Elle ne contient ni GUID de template ni ID d'entité du monde.
Le GUID 128 bits `EquipmentSetup.placedEntity` est retrouvé **au placement**, dans
le registre du jeu, avec le même ID d'inventaire. Le CSV n'intervient pas dans
cette reconstruction ; il fournit seulement des noms/catégories à la bibliothèque.

Le fichier ne sérialise pas l'échelle, le contenu des coffres, le carburant,
les liens logiques, le câblage, l'état des portes ou des propriétés personnalisées.
Les petits objets réapparaissent parce qu'ils ont chacun leur propre entrée et
leur transformation relative. La dev12 conserve ces données ; elle ne prétend
pas copier des propriétés absentes du format classique.

Le pont original fournit aux objets à logique son writer privé puis abandonne
l'événement de liaison au réseau de base. C'est aussi ce qui a permis votre
bougie dans le test classique. Ce comportement est conservé, avec ses limites.

## Comparaison des chemins

| Étape | Blueprint classique | Composite importé dev12 |
|---|---|---|
| Détection lors de la capture | `bp_scan_entities` dans le callback natif de confort ; `CurrentTransform` et découverte de `UsedItem` par le lecteur historique | Réutilise le BTBP déjà capturé ; le même lecteur sert au contrôle des objets existants |
| Lecture du fichier | `core::bp_decode` | **Même décodeur**, même source intacte |
| Transformation | Rotation du rectangle de capture, déplacement et quaternion tourné | Transformation du module puis du projet ; ancre/translation explicites, quaternions conservés |
| Résolution | Recherche exacte de l'item, prop natif et template | Même admission de prop en mode explicite `blueprint`, avec gardes ABI, ID, flags lisibles et bornes finies |
| Précontrôles | Pas de précontrôle explicite de chevauchement entre props dans la boucle classique du mod | Contrôles supplémentaires détaillés ci-dessous ; jamais de succès sur une lecture incomplète |
| Coût et file | `bp_fill` → entrée native de coût et index d'objet | **`bp_fill` également**, sans second writer de coûts dans le composite |
| Paiement | Mécanisme natif ; gratuité créative existante | Même mécanisme ; pas d'édition de l'inventaire |
| Commit | `bp_commit_object` → `bp_place_object` | Adaptateur de coordonnées composite → **même `bp_place_object`** |
| Fonction moteur | `g_placePropOrig`, cible `PlaceProp` `0x48deb0` sur l'exécutable vérifié | Identique, avec le même payload natif de 0x50 octets, undo et writer logique |

Le payload contient notamment le template à +0, la position à +0x10, le
quaternion à +0x1c et les bornes locales à +0x2c/+0x38. Le résultat natif non nul
est une acceptation d'appel. Il ne remplace pas un contrôle visuel ni un rechargement.

Les deux voies partageaient déjà le dernier pont natif avant dev12. Les blocages
provenaient surtout des étapes **ajoutées avant ce pont** : politique de flags,
nouvelle découverte de composants ECS et validation géométrique trop indépendante
du contexte capturé. Les échecs dev11 à 16:11, avant toute prop examinée, puis
votre réussite classique à 16:18 dans la même session, démontrent cette différence.
Le log ne nomme pas le composant exact qui bloquait la nouvelle découverte.
Un test synthétique reproduit notamment la différence avec un tag de taille zéro :
le lecteur historique l'ignore correctement, tandis qu'un contrôle générique de
stride ne doit pas l'interpréter comme un item corrompu.

## Mode explicite et protections

Le module utilise deux champs apparus en dev12 :

```json
{
  "file": "Dev12_Table_Classique_Source.btbp",
  "offset": [0, 0, 0],
  "rotation": 0,
  "category": "furniture",
  "content": "objects",
  "objectPlacement": "blueprint"
}
```

`content=objects` exclut toute construction des voxels source. Ceux-ci deviennent
un masque de **contacts capturés**, limité au module. `objectPlacement=blueprint`
choisit explicitement la voie historique et son traitement des objets à logique.
Il ne modifie pas les autres projets ni la politique générale du catalogue.
Les anciens manifests restent en mode contrôlé par défaut ; une ancienne DLL
refuse les nouveaux champs au lieu de construire le sol par erreur.

La bougie porte `0x80001`. Le filtre conservateur général la classe toujours
`special_flags`. L'import classique la traite comme `classic_blueprint_candidate`
parce que l'admission et le writer du mécanisme historique savent la reproduire.
Ce n'est pas une approbation générale de tous les luminaires ou de tous les flags.
Les bits inconnus de l'ABI vérifiée, les flags illisibles, IDs inconnus, templates
absents, autels et bornes invalides restent refusés.

Pour cette première étape, ces compositions forment un **projet indépendant
d'objets uniquement**. Elles ne se mélangent pas aux modules de structure,
pièces natives ou espaces d'escalier. Plusieurs compositions peuvent être ajoutées
avec leurs offsets/rotations. Les collisions entre modules restent bloquantes.

Les contrôles supplémentaires ont chacun une raison et des tests :

| Contrôle | Raison et comportement |
|---|---|
| Objets extérieurs | Le lecteur **historique** reçoit un visiteur ; il examine les props du monde et conserve celles dont l'AABB transformée coupe la cible. Il ne filtre pas uniquement le pivot, ce qui pourrait manquer un grand meuble. |
| Lecture complète | Deux lectures par le même lecteur historique, comparaison de l'occupation/identité des entités, des IDs et des poses, en-têtes avant/après et comptages. Budget commun de 5 s, limite de 512 props pertinents. Erreur, annulation ou changement détecté : aucun candidat partiel publié. Aucun appel au scanner de découverte dev11 pour ce mode. |
| Supports | Vérification des contacts avec meubles/voxels existants. Les décorations peuvent dépendre de leur table capturée même si celle-ci vient plus tard dans le fichier. |
| Voxels extérieurs | Un voxel dans une boîte d'objet bloque, **sauf** s'il correspond à une cellule solide enregistrée dans le même module source. Cela conserve uniquement les contacts présents dans le blueprint, pas une tolérance générale à travers les murs. |
| Autels | Couverture 3D par les zones natives, conservée pour chaque cellule contrôlée. |
| Données et identité | Résolution exacte à la lecture puis avant les lots ; aucun remplacement d'ID ni boîte de substitution. |
| Intégrité de composition | Ordre source et transforms conservés. Les modifications individuelles en mémoire imposent une nouvelle capture/import ; l'export refuse aussi une source modifiée sur disque. |
| Lot | Les cinq objets sont présentés ensemble à la file originale. Budget inférieur à cinq : refus avant ce lot, sans découpe silencieuse. |

Le bas de la boîte native de la table se trouve à y=0,449951 dans une couche de
sol source allant de 0 à 0,5 m. Cela représente environ **5 cm de contact de proxy
dans le sol**, pas une preuve de défaut visuel du mesh. Refuser tout voxel touché
rejetterait ce placement qui fonctionne déjà en classique. Le masque source
conserve ce contact précis ; un bloc ajouté au-dessus du sol reste refusé.

Les AABB sont les proxies natifs de placement, pas les meshes physiques exacts.
Le callback reprend le contexte de lecture de la capture originale ; le verrou
du mod ne devient pas un nouveau verrou du moteur. Stabilité et performance
réelles du contrôle dans votre monde restent à valider en jeu.
Les empreintes FNV64 de cohérence sont diagnostiques, pas cryptographiques.
Elles détectent aussi les mutations à effectif constant ; un changement d'ID,
de pose ou d'entrée provoque un refus `E_TABLE_CHANGED`, jamais un succès forcé.
Les deux passages sont journalisés (`passes=2`, `visited`, `verified`) et partagent
le budget de 5 s. Ils ne rendent pas le monde immuable après la lecture ; la
durée de validité de 250 ms et les gardes de commit restent appliquées.

Un **lot unique n'est pas une transaction avec rollback garanti** : si un appel
natif échoue après d'autres, les objets acceptés peuvent rester. Les compteurs
et le journal indiquent le préfixe écrit. Ne pas relancer automatiquement au même
endroit. L'undo garde les limites du mécanisme historique.

## Installation et premier test

1. Fermer le jeu. Installer manuellement le ZIP **Standalone 0.34.0-dev12** à côté
   d'`enshrouded.exe`, en gardant ses dossiers. Ne pas mélanger cette disposition
   avec celle de Shroudforge. Le ZIP contient la DLL Windows x64, les langues,
   catalogues et les fichiers du démonstrateur.
2. Ouvrir un monde de test local. Vérifier `Building Tools 0.34.0-dev12` et
   les gardes ABI valides dans `BuildingTools.log`. Prendre le vrai cube du
   marteau de **0,5 m** et ouvrir le sélecteur natif pour observer registre/autels.
3. Conserver **Objets : Construire** dans les options classiques. Dans le composite,
   activer les objets et le placement natif expérimental. En créatif, activer
   également les objets natifs créatifs ; sinon fournir table, bol, bougie et
   deux sous-verres. Le mode explicite du manifeste utilise le writer logique
   original sans changer le réglage général de compatibilité du catalogue.
4. Préparer un sol existant plat sous autel, libre de meubles. Un carré **4 × 4 m**
   reproduit la sélection source. L'origine du composite est son **coin** x/z,
   et y est la face supérieure du sol ; ce n'est pas son centre. L'ancre du projet
   `[0,0.5,0]` place les objets comme au-dessus du sol capturé.
5. Charger **`Dev12_Table_Classique.btproject`**. Attendu : un module, **cinq objets,
   zéro cellule à construire et zéro pièce native**. Garder le fichier
   `Dev12_Table_Classique_Source.btbp` à côté du manifeste.
6. Positionner la composition, rotation globale 0 pour commencer, puis cliquer
   **Vérifier** une fois. Attendre `classic composition SCAN_COMPLETE
   method=original_blueprint_reader`, puis le résultat de collision et la
   validation 5/5. Une lecture indisponible ne vaut pas une zone libre.
7. Cliquer **Construire** une fois. Le journal doit montrer
   `OBJECT_BATCH_BEGIN method=bp_fill/original_native_queue`, cinq objets mis en
   file par le blueprint, puis cinq résultats natifs : **placed=5, failed=0**.
   Vérifier visuellement table, bol, bougie et deux sous-verres, avec leurs poses
   relatives. Un succès d'appel sans les cinq objets visibles est un échec.
8. Refaire seulement **Vérifier** au même endroit : un doublon doit être refusé.
   Pour un autre essai, choisir une zone libre. Tester ensuite la rotation de
   l'ensemble, puis un obstacle volontaire dans la cible : l'obstacle doit bloquer.
9. Quitter normalement le monde puis le jeu. Relancer le même monde et vérifier
   présence, poses, interactions et lumière. Noter séparément apparition,
   interaction et persistance : aucune de ces dernières n'est déduite du compteur.

La table est centrée à environ `(0.741699, -0.050049, 2.930420)` par rapport
à cette origine de sol ; les décorations sont environ 0,749267 m plus haut.
Cette asymétrie vient de la sélection réellement capturée et n'est pas corrigée.

En cas d'échec, conserver `BuildingTools.log`, `BuildingTools.prev.log` si utile,
`BuildingTools-Projects.log`, le message et le dernier export de catalogue.
Les lignes `classic composition`, `native object TRACE/BOUNDS`, `REFUSED` et
`OBJECT_RESULT` distinguent source, métadonnées, contacts, lecture et placement.
Une limite ou une donnée invalide doit rester un refus explicite.

## Importer et réutiliser d'autres compositions

Dans la fenêtre de placement **classique**, charger un BTBP contenant des objets,
puis utiliser **Importer ce blueprint comme mobilier composite**. Le mod crée
de nouveaux fichiers dans `BuildingTools-Projects` : copie identique du BTBP et
manifeste d'objets uniquement. Il reconnaît une couche complète de sol pour
placer l'ancre à sa face supérieure ; sinon l'ancre reste au coin source.
Il ne construit rien pendant l'import. Le BP original et la sauvegarde restent intacts.

L'export standard ou **Uniquement le mobilier** d'une composition importée écrit
un nouveau manifeste référençant les sources originales. Il conserve ainsi les
contacts capturés et vérifie que ni les sources ni les objets n'ont été modifiés.
Pour déplacer ces fichiers ailleurs, copier aussi les BTBP référencés. L'ancre
et les transformations de modules sont conservées ; noter séparément l'origine
monde et la rotation globale de pose pour un futur alignement au château.

Le catalogue local `BuildingTools-Catalog/classic-compositions.json` commence
avec cette composition et ses quatre IDs uniques. Il distingue reconstruction
classique observée, composite en attente et persistance en attente. Le catalogue
général reste disponible pour rechercher les noms/IDs ; aucune catégorie entière
n'est déclarée compatible à partir de ce seul cas.

L'outil de fichiers permet de réutiliser un ensemble à des coordonnées précises :

```powershell
python tools/make_classic_composite.py --blueprint "chemin/esaai blueprint tabel.btbp" --outdir "build/deux-compositions" --prefix Tables --anchor 0 0.5 0 --instance 0 0 0 0 --instance 10 0 6 90
```

Les offsets des ensembles respectent la grille 0,5 m, les rotations de module
sont 0/90/180/270°, et les positions/quaternions internes gardent les flottants
du BTBP. Les fichiers existants ne sont pas écrasés. La première extension aux
lits, chaises, luminaires, statues ou autres décorations devra partir de captures
et de tests réels distincts. Aucun mobilier du château n'est généré dans cette étape.

## Preuves automatiques et limites

Les tests compilent les décodeurs, la file de coût native partagée, le pont de
placement, le lecteur historique, son visiteur, l'import et l'export de production.
Les appels moteur sont des espions/simulations, sans exécution d'Enshrouded.

Les contrôles couvrent la source réelle et son empreinte, zéro voxel construit,
64 cellules de contact, cinq entrées dans l'ordre, données natives identiques
entre classique et composite sur les quatre rotations, writer de la bougie,
absence d'écrasement, refus des edits/IDs inconnus/bornes invalides, parent placé
plus tard, sol manquant, obstacle ajouté, doublon et autels. Le lecteur est aussi
testé avec des tags de taille zéro, une transformation illisible, des mutations
de comptage mais aussi de pose/ID/entrée à effectif constant, une annulation et
un délai dépassé. Un objet qui entre dans une cible initialement vide pendant
la lecture doit provoquer un refus, pas une publication « zone libre ».
Une table synthétique de 1 048 576 emplacements avec 60 000 props et une seule
prop pertinente vérifie aussi les deux passages du lecteur historique, sans
saturation liée aux meubles éloignés. Plus de 512 props pertinents reste un refus.

Les dimensions du bol, de la table et des sous-verres utilisées en test viennent
de l'export natif observé. **Celles de la bougie sont synthétiques dans les tests** :
l'ancien export les omettait à cause de la politique des flags. La DLL les lit
réellement dans le registre, sans remplacement. La capture/reconstruction classique
réussie prouve son passage par le pont natif ; elle ne fournit pas leurs valeurs.

Commandes reproductibles : `bash build.sh test`,
`sh tools/test_native_scanner_sanitizers.sh`, `bash build.sh package`,
`python tools/verify_package.py`. Les fixtures et archives du château sont également
vérifiées sans les régénérer. **Le composite et la persistance attendent votre
validation réelle avant toute décoration du château.**
