# Catalogue local des IDs Enshrouded

Snapshot récupéré le **22 novembre 2026** depuis le [CSV Enshrouded Save Editor](https://enshroudedsaveeditor.com/items/item-ids.csv), présenté sur sa [page des identifiants](https://enshroudedsaveeditor.com/items/item-ids). Les données sont attribuées à cette source ; aucune affiliation avec son auteur ou Keen Games n'est revendiquée.

`item-ids.source.csv` conserve exactement les octets reçus. `native-items.json` est l'index normalisé : **3 526 IDs uniques**, hexadécimal et décimal cohérents ; **1 303 BuildTools** et 2 223 autres entrées. Les noms homonymes conservent leurs IDs distincts. L'empreinte SHA-256 du CSV figure dans l'index et dans `src/native_item_index.h`.

Les 14 catégories sont des **indices de navigation déduits des noms**, pas des types de moteur. Un nom de PNJ peut apparaître dans « decorations » : il restera refusé s'il ne satisfait pas les contraintes natives. Le classement source `BuildTools` ne prouve aucune compatibilité. Toutes les entrées livrées sont `source_only_unverified`, sans dimensions inventées ni appel de placement revendiqué.

`prior-session-observations.json` conserve séparément six IDs observés dans les anciens journaux natifs comme objets tenus en main : cinq sont présents dans ce CSV et un (`d1c3d410`) en est absent. Cela confirme le format d'ID d'inventaire utilisé par le mod et montre que le CSV n'est pas exhaustif, sans prouver un placement, des volumes ou une disponibilité dans la session actuelle. Ces traces ne sont pas fusionnées comme approbations.

Recherche depuis la racine du dépôt :

```powershell
python tools/native_item_catalog.py query bed
python tools/native_item_catalog.py query torch --category wall_lights
python tools/native_item_catalog.py query 566c8dfe
python tools/native_item_catalog.py query --status policy_compatible --catalog CHEMIN/catalogue_observe.json
```

Le mod dev9 inclut également recherche nom/ID, catégories et résolution native. Les noms du CSV sont en anglais ; les noms localisés déjà observés par l'interface sont aussi recherchables. Les contrôles de placement utilisent uniquement le registre du jeu, jamais la catégorie importée.

## Actualiser après une mise à jour

1. Télécharger un **nouveau** fichier : `python tools/native_item_catalog.py download --out NOUVEAU/item-ids.csv`.
2. Importer vers de nouveaux fichiers : `python tools/native_item_catalog.py import --csv NOUVEAU/item-ids.csv --out NOUVEAU/native-items.json --header NOUVEAU/native_item_index.h`.
3. Examiner les différences ; remplacer ensemble le snapshot/index/header du dépôt puis recompiler la DLL. L'import refuse les doublons, schémas modifiés et discordances hex/décimal. Le nouvel index repart sans aucune validation native.
4. Dans le jeu mis à jour, ouvrir le sélecteur natif, actualiser la bibliothèque puis exporter un nouveau `NativeCatalog_*.json`. Le mod résout à nouveau tous les IDs et note le contexte PE de l'exécutable. Une modification d'ABI désactive la compatibilité des composites.
5. Fusionner les observations : `python tools/native_item_catalog.py merge --catalog NOUVEAU/native-items.json --observed CHEMIN/NativeCatalog_REEL.json --out NOUVEAU/catalogue_observe.json`.

La fusion exige le même SHA-256 d'index. Elle remplace les observations précédentes, remet les lignes non observées à « non vérifié » et inclut les nouveaux items découverts dans le registre. Elle ne réutilise pas les autorisations d'une ancienne session. Les JSON servent à l'auteur des projets ; le moteur vérifie encore chaque ID au chargement et avant les lots. Les compteurs d'appels natifs sont ceux de la session d'export, pas un certificat d'interaction ou de persistance.

## Ressources examinées

- [Emberforge, commit 639bd73ba7152cce68d0de2fc071d467329bdaf4](https://github.com/evoniq-bit/Emberforge/tree/639bd73ba7152cce68d0de2fc071d467329bdaf4) : son lecteur `GameCatalog.cs` indexe des GUID de templates/modèles de 128 bits et attend KFC3 ; l'override utilise une source de placement et un modèle de prévisualisation. Le fichier installé ici commence par **KFC4** (`4b464334`). Son lecteur n'est donc pas présenté comme compatible avec cette installation. Aucun code d'Emberforge, trainer ou DLL tierce n'a été exécuté ou intégré.
- [enshrouded.gg — provenance](https://enshrouded.gg/about/) : données extraites du serveur et noms du client, snapshot annoncé au 15 août 2026. Ses [catégories de construction](https://enshrouded.gg/building/) aident à comparer mobilier, décoration et portes. Elles ne remplacent ni les IDs 32 bits du CSV ni les volumes mesurés par le moteur. Aucun confort ni volume n'a été attribué par simple correspondance de nom entre bases.

La procédure du mod et l'ameublement séparé sont décrits dans `docs/DEV9-CATALOG.md`.
