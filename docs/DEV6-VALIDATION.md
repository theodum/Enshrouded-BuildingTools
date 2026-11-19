# Dev6 — bilan de livraison et portée des tests

Version : `0.34.0-dev6`, Standalone Windows x64.
Référence de non-régression : dev5 `591090f`.

Le retour utilisateur de dev5 valide les composites testés (dont 312 cellules),
les bâtiments reliés et leurs passages, matériaux et limites d'autels. Il ne
valide pas les nouvelles fonctions natives multiétages de dev6.

Audit : [DEV6-AUDIT.md](DEV6-AUDIT.md).
Plan d'évolution : [ARCHITECTURE-ROADMAP.md](ARCHITECTURE-ROADMAP.md).
Recette : [DEV6-MULTISTOREY.md](DEV6-MULTISTOREY.md).

## Livrables

- `build/dinput8.dll` : DLL Windows PE32+ x64.
- `dist/EnshroudedBuildingTools-0.34.0-dev6.zip` : installation Standalone.
- `dist/EnshroudedBuildingTools-Shroudforge-0.34.0-dev6.zip` : autre chargeur, même DLL.
- `dist/SHA256SUMS-0.34.0-dev6.txt` : empreintes des deux ZIP.
- Fabrique du projet multiétages dans Composants architecturaux → Escaliers ;
  export BTBP/manifeste une fois les vrais IDs et la montée choisis dans le jeu.

SHA-256 de la DLL :
`c8cd9f44f5a2481e1dbd467dd75c709bfb77ae4ee097058e3632dc7c021b7961`.
Les anciens ZIP, la licence MIT et les cinq exemples statiques sont conservés.
Le ZIP inclut les nouveaux audit, feuille de route et protocole, ainsi que les
bilans dev3/dev4/dev5 identifiés comme historiques.

## Contrôles exécutés le 19 novembre 2026

| Contrôle | Résultat |
|---|---|
| Compilation freestanding Windows x64 sous WSL, clang/lld | Réussie |
| Suite historique core, langues et vérifications du binaire local | Réussie ; 40 signatures uniques et 3 contrôles de réflexion ABI |
| `test_project` | 145 203 assertions, 0 échec |
| `test_project_actions` | 209 assertions, 0 échec |
| `test_materials` | 209 assertions, 0 échec |
| `test_storeys` | 28 220 assertions, 0 échec |
| `test_extended` | 2 457 assertions, 0 échec |
| `test_extended_config` | 16 assertions, 0 échec |
| `test_project_export` | 0 échec |
| ASan + UBSan, matériaux et nouvel adaptateur multiétages | Réussis, aucun diagnostic mémoire |
| Fixtures Python, Windows / WSL | 4 tests réussis dans chaque environnement |
| Sources des exemples dev5 | 15 fichiers BTBP/manifeste inchangés octet pour octet |
| Archives Standalone / Shroudforge | 49 / 52 entrées ; PE x64/proxy/imports, CRC, sources, langues, licence, guides/liens et chargeur vérifiés |
| Recompilation des mêmes sources | DLL strictement identique ; horodatage PE nul |

Le nouveau test compile le **vrai `ArchitectureEdit` et l'adaptateur de démo**,
avec des allocations hôtes et des fonctions natives contrôlées. Il vérifie les
volumes réservés, planchers, paliers, embrasures, toiture, quatre orientations,
IDs et matériaux, refus de modèle/montée/budget, et la restauration du brouillon
après un échec tardif ou d'allocation. Il exporte le bâtiment complet avec le
**vrai exporteur**, relit tous ses BTBP/manifeste et compare coordonnées,
catégories, matériaux et pièces. L'écrasement est refusé. Les IDs de cette suite
sont des doubles de test, pas des IDs livrés comme escaliers du jeu.

Les tests de diagnostic vérifient progression, devis incomplet/complet, échec et
invalidation sur génération, position, rotation, mode créatif ou option objets.
Les tests de limites flottantes distinguent l'erreur microscopique de rotation
d'un réel débordement ; les contrôles de construction restent actifs.

Le modèle contrôlé de 2 × 2 × 3 m avec montée de 2 m donne 16 modules,
4 660 cellules, 4 pièces et 9 × 14,5 × 22 m. Un modèle différent produit un autre
projet. Ce résultat de géométrie/export ne prouve pas une marche sur une pièce
native, le paiement, une interaction ou un état persistant du monde.

Binaire du jeu lu pour les contrôles statiques : SHA-256
`5d9ef2f140b8ed49e30dbee655841b89d08baec20f799a1817e171a18a850c77`.
Traces locales : `build/dev6-package.log`, `build/dev6-sanitizers.log`,
`build/dev6-fixtures-wsl.log`, `build/dev6-artifacts.log` et `build/dev6-rebuild.log`.
Builds/ZIP/traces sont ignorés par Git ; code, tests et documentation sont versionnés.

**Aucun test de cette livraison n'écrit dans le jeu ni n'accède aux sauvegardes.**
Les tests de fichiers écrivent seulement dans de nouveaux dossiers de build.

## À valider dans le jeu

La non-régression des cas dev5 sur la DLL dev6, la sélection/calibration native,
l'affichage des nouveaux diagnostics, le parcours des trois niveaux aux quatre
rotations, l'export/rechargement dans l'interface, les coûts réels en mode normal
et la persistance après rechargement du monde sont **en attente**.

Les portes interactives, collisions générales d'entités, rollback intégral,
autels automatiques et génération complète de châteaux/villages ne sont pas
livrés. Le placement natif reste expérimental et désactivé par défaut.
Appliquer le [protocole détaillé](DEV6-MULTISTOREY.md) avant de déclarer ces nouveaux
comportements fonctionnels.
