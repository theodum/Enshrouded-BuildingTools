# Dev8 — audit et validation des objets natifs

Version `0.34.0-dev8`, base `c93d6ec` (châteaux V1/V2, dev7). Ce document distingue le code livré, les tests sur hôte et la validation réelle encore requise.

## Audit de l'implémentation d'origine

| Besoin | Mécanisme conservé / évolution dev8 |
|---|---|
| Capturer une pièce meublée | `src/bpobjects.h`, `bp_scan_entities` lit la table d'entités depuis le callback confort, le composant item et `CurrentTransform`. `src/blueprint.h`, `bp_save_write` convertit ces positions depuis le coin bas de la capture. Dev8 extrait seulement cette conversion dans `props::capture` et refuse une capture dépassant 512 objets. |
| Sérialiser | `src/core.h`, BTBP version 1, `BpObject` : ID u32, position float3, quaternion float4, 32 octets. Codec conservé ; pas de nouvelles propriétés inventées. |
| Charger dans un composite | `src/project_runtime.h`, chargement des BTBP, `extended::transform_object`, transformations du module, puis `extended::object_world` pour l'ancre/rotation globales. |
| Créer un objet natif | `bp_place_object`, extrait sans modification de son corps dans `src/blueprint_object_place.h`. Copie le modèle natif depuis l'item, la position, le quaternion et les bornes locales puis appelle l'original `g_placePropOrig`. Aucun moteur concurrent. |
| Logique des portes/lumières | L'original dispose déjà d'un writer privé quand le contexte blueprint ne possède pas de writer logique. Il supprime l'événement de liaison. Dev8 expose une activation expérimentale explicite, désactivée au lancement. Ce contournement ne devient pas une restauration de réseau logique. |
| Créatif / paiement | Interceptions `pay_body` dans `src/main.cpp`, `g_creativeProps`, `creative_on` et procédure native de facturation des blueprints existantes. Dev8 ne fabrique pas d'objets dans l'inventaire. |
| Catalogue | Itérateur de groupes d'items vérifié, déjà utilisé par `src/extended_catalog.h`, et registre natif observé. Dev8 ajoute l'export JSON dans `src/native_catalog_export.h`. Limite 4096 références ; l'export refuse une traversée incomplète. |
| Contrôles et ordre | `src/native_props.h`, `src/project_props_runtime.h` : boîtes mesurées, refus d'intersections, supports, ordre des parents avant décorations, coût et couverture par le contrôleur d'autels existant. La lecture des objets existants réutilise le lecteur original avec des tranches de travail. |
| Pièce témoin | `src/native_furniture.h` fournit la disposition commune au bouton du mod et à la fabrique hors jeu ; `src/native_furniture_runtime.h` ne modifie que le brouillon. Export complet par `project_export`. |
| Suivi | Compteurs `attempted/placed/failed`, lignes `OBJECT_RESULT` avec ID et retour de l'appel natif, rapport et interface. Un retour positif n'est pas un audit de sauvegarde persistante. |

Le dossier du jeu et ses projets ont été lus pour l'audit. Les 194 BTBP trouvés avaient tous zéro objet. Quelques IDs apparaissent dans les anciens journaux, sans nom ni volume : ils n'ont pas été attribués à des lits/tables supposés. Vous avez confirmé ne pas posséder de blueprint meublé. Le catalogue final et les identifiants du démonstrateur doivent donc être exportés dans une session réelle.

## Fonctionnalités implémentées

Les neuf points demandés disposent d'un chemin de code : catalogue réel, objets via BTBP dans les composites, conservation ID/transform, traitement après structure, supports sol/meuble/mur, contrôles géométriques et couverture d'autels, créatif existant, refus explicites et compteurs. Les contraintes natives de fixation, interactions et états non sérialisés ne sont pas exposés comme des fonctionnalités validées. Voir les limites précises et la procédure dans [DEV8-NATIVE-OBJECTS.md](DEV8-NATIVE-OBJECTS.md).

La recette livrée `native-furniture-room.recipe.json` contient huit rôles sans identifiant, pas un faux projet meublé. Le générateur dans le mod et la fabrique hors jeu produisent le composite complet à partir de sélections réelles. Aucun projet de décoration du château n'est livré avant la validation réelle demandée.

## Tests automatisés

Les tests de meubles utilisent des **identifiants synthétiques explicitement désignés comme tels**, confinés au code de test et à des dossiers temporaires/build. Ils ne sont pas livrés comme catalogue du jeu.

| Contrôle | Périmètre |
|---|---|
| `test_native_props` | Disposition des 11 objets, pivots décentrés, passage, supports sur table et mur, refus de volumes invalides/chevauchements, exporteur réel, rechargement des BTBP/composite, conservation des identifiants/quaternions, aide de capture originale, codec original et véritable fonction `bp_place_object` compilée avec un callback natif simulé. Writer logique installé/restauré, retours positifs/négatifs comptés, refus d'un ID inconnu sans substitution. Export JSON réel et publication atomique, échecs d'écriture/flush/catalogue incomplet. Export de mobilier avec zéro voxel. |
| `test_native_prop_checks` | Véritable prévalidation composite compilée avec requêtes voxels en lecture seule simulées : collision prévue/existante, manque de support, mur et sol existants pour un projet uniquement mobilier, limites, ID inconnu, couverture d'autel refusée, coût de chaque objet, créatif désactivé, attente/timeout/échec de lecture. |
| `test_native_furniture.py` | Catalogue incomplet ou sélection inconnue/refusée/logique non autorisée, bornes invalides, doublons et matériau absent ; génération complète par le code de production, vérification des 1692 voxels et 11 IDs exacts dans les fichiers, provenance non validée en jeu et refus d'écraser un dossier existant. |
| Suites historiques | Codec/langues/signatures, projets, commandes, coûts, multiétages, escaliers voxels, configuration et export. Vérification de la reproductibilité des exemples et des châteaux inchangés. |

Résultats exécutés sous WSL Ubuntu 24.04, le 19 novembre 2026 :

| Suite | Résultat |
|---|---|
| Compilation Windows x64 freestanding clang/lld | Réussie, sans avertissement |
| Core, 15 langues et exécutable installé lu sans exécution | Réussis ; 40 signatures historiques uniques et 3 contrôles de réflexion ABI |
| Projets | 145 203 contrôles, 0 échec |
| Commandes et matériaux | 209 + 209 contrôles, 0 échec |
| Multiétages et escaliers voxels | 28 220 + 130 749 contrôles, 0 échec |
| Extended et configuration | 2 457 + 16 contrôles, 0 échec |
| Export historique | 0 échec |
| Nouveaux objets et prévalidation | 299 + 59 contrôles, 0 échec |
| ASan + UBSan, deux nouvelles suites C++ | 358 contrôles, aucun diagnostic mémoire ni fuite |
| Fabrique de mobilier Python | 3 tests réussis |
| Fixtures historiques Python | 4 tests réussis |
| Château V1 / V2 | 8 + 11 tests réussis ; vérification des fichiers, archives et relecture native réussie |
| Exemple dev7 | 59 fichiers reproduits à l'identique, 8 068 cellules, 22 passages |
| Fichiers architecturaux V1 / V2 | Aucun changement dans Git |

SHA-256 de `build/dinput8.dll` :
`4f6ee588fc8910bca2385866ac8064eaf4b9a6c101264f56f394c0f4d2747784`.

La DLL installée dans le dossier du jeu conserve l'empreinte dev7
`aa7109f61a59d0c814b0a06357b594a736eb083da312875ac5acf2f40380fb59` : cette livraison ne l'a pas remplacée.
Les archives `dist/EnshroudedBuildingTools-0.34.0-dev8.zip` et
`dist/EnshroudedBuildingTools-Shroudforge-0.34.0-dev8.zip` sont conditionnées par
`build.sh package` ; `tools/verify_package.py` compare leurs DLL, sources, langues,
recette, documentation et chargeurs, vérifie CRC/PE/liens et produit
`dist/SHA256SUMS-0.34.0-dev8.txt`. Les logs de test restent dans `build/dev8-*.log`.

## Validation réelle encore nécessaire

**Non testés dans Enshrouded par l'agent** : disponibilité des huit types exacts dans votre registre, sélection des véritables modèles muraux, lecture de la scène par le nouveau contrôle, construction native des 11 objets, interactions et éclairage, paiement/absence de consommation, sauvegarde puis chargement d'une vraie pièce meublée via l'original, persistance après rechargement. Les tests hôte ne remplacent aucun de ces points.

Le feu vert pour décorer le château exige cette recette réelle réussie, ses objets comptés et ses interactions vérifiées. Les données du château ne sont pas modifiées pour préparer ce test.
