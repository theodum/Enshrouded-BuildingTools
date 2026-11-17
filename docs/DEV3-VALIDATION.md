# Livraison de développement 0.34.0-dev3 — 17 novembre 2026

**Prête pour une première campagne personnelle de tests, pas validée en jeu.**
Branche `feature/composite-projects`, base upstream `6af1321` / 0.33.2.
Aucune installation, aucun lancement du jeu, aucune sauvegarde lue ou modifiée.

## Fichiers à utiliser

Chemins depuis la racine `C:\Users\User\enshrouded\Enshrouded-BuildingTools` :

| Livrable | Chemin |
|---|---|
| DLL Windows x64 | `build/dinput8.dll` |
| ZIP standard | `dist/EnshroudedBuildingTools-0.34.0-dev3.zip` |
| ZIP Shroudforge | `dist/EnshroudedBuildingTools-Shroudforge-0.34.0-dev3.zip` |
| Dossier standard prêt à copier | `dist/BuildingToolsExtended-0.34.0-dev3/` |
| Empreintes des ZIP | `dist/SHA256SUMS-0.34.0-dev3.txt` |
| Compilation/tests finaux | `build/dev3-validation.log` |
| Contrôles des archives | `build/dev3-artifacts.log` |
| Sanitizers | `build/dev3-sanitizers.log`, complément final `build/dev3-config-sanitizers.log` |

Les artefacts restent ignorés par Git ; ils sont disponibles dans le workspace.
Les ZIP dev1/dev2 et les blueprints préexistants sont conservés. Chaque ZIP
contient la même DLL, les 15 langues, la licence MIT inchangée, les cinq exemples
composites et leurs dix sources, un modèle de configuration non actif et les
guides installation/test, format, exemples, audit, bilan et changelog.

## Contrôles automatisés

| Contrôle | Résultat |
|---|---|
| Compilation freestanding clang/lld 18.1.3 / WSL Ubuntu 24.04 | DLL PE32+ x86-64, sans runtime C/C++ à installer |
| Suite historique `test_core`, langues incluses | Réussie |
| Exécutable local en lecture seule | 40 signatures uniques aux RVA attendues, prologues/sites de patch et instructions prix/commit vérifiés |
| Garde de réflexion utilisée dans la DLL | ItemInfo/EquipmentSetup et les 22 valeurs ItemFlag correspondent ; modifications de descripteurs simulées refusées |
| `test_project` | 145 203 assertions, 0 échec : BTBP anciens, JSON, UTF-8, chemins, grilles 0,5/1/2/4 m, rotations, dédoublonnage, lots, interruptions, cinq fixtures et grand plan |
| `test_extended` | 2 457 assertions, 0 échec : grille, horloge, drapeaux, objets, composants, escaliers/paliers tournés et collisions comparées à une recherche exhaustive |
| `test_extended_config` | 16 assertions, 0 échec : valeurs par défaut/invalides, aller-retour, autres clés conservées, refus d'écriture signalé |
| `test_project_export` | 0 échec : vrai exporteur avec adaptateurs de fichiers, relecture, positions/matériaux/pièces, non-écrasement, échecs d'écriture et de vidage disque |
| Python Windows et WSL | 3 tests réussis dans chacun des deux environnements : fixtures reproductibles, source lue seulement et sorties existantes refusées ; manifests LF explicites |
| ASan + UBSan | Suites projet, composants, export et configuration réussies ; configuration rejouée après le dernier correctif |
| SPIR-V de lignes | Les deux en-têtes réellement embarqués passent `spirv-val` |
| Recompilation lors du packaging | Empreinte DLL identique à la compilation de tests ; horodatages PE nuls |
| ZIP standard/Shroudforge | CRC, liste exacte des fichiers, DLL/PE/imports/export proxy, JSON du chargeur, langues, MIT, fixtures, docs et liens internes contrôlés par `tools/verify_package.py` |

Les comptes d'assertions incluent des boucles de comparaison de cellules ; ils
ne correspondent pas à autant de scénarios distincts. Le profil Windows est
testé avec un magasin isolé en mémoire ; l'export avec des adaptateurs de fichiers.
Ces suites ne simulent ni le moteur ni les événements d'entités ni les entrées ou
le rendu. Aucun résultat de FPS, de circulation ou d'usage natif n'est revendiqué.

SHA-256 du seul exécutable local lu pour les contrôles statiques :
`5d9ef2f140b8ed49e30dbee655841b89d08baec20f799a1817e171a18a850c77`.
Il n'est pas inclus dans le dépôt ou les archives.

## État fonctionnel livré

**Implémenté dans la DLL :** projets BTBP composites, ancrage/hauteur/rotation
globaux, deux passes et une confirmation, grille virtuelle et huit niveaux,
prévisualisation simplifiée, matériaux, catalogue natif, murs/sols/toits/tours,
contrôles de blocs et de zones, aide aux autels, export sans écrasement, navigation
Home française, réglages persistants et diagnostics d'opération. Les outils
originaux restent disponibles. Le village de test représente 75 648 cellules
natives et 96 modules ; il dépasse la limite d'un BTBP individuel.

**Partiel ou expérimental :** objets/prefabs et escaliers réels, gratuits avec
les options créatives appropriées, mais désactivés automatiquement par défaut
et conditionnés à l'ABI et aux drapeaux. L'exemple natif se prépare dans le jeu
depuis son registre, puis s'exporte ; aucun ID de démonstration n'est inventé.
Les pièces utilisent leurs limites réelles, qui ne prouvent pas la montée exacte.
Les recommandations d'autels échantillonnent les limites natives ; l'aperçu des
objets est un marqueur. L'annulation et le diagnostic de chantier restent partiels.

**Non disponible :** objets logiques/spéciaux/progression dans les composites,
requêtes de collision de toutes les entités, fantômes texturés des modèles,
jonctions automatiques de toitures, spirale arrondie, création automatique d'autels,
reprise automatique, remboursement supplémentaire et rollback complet. Les
composites ne sont pas pris en charge sur serveur distant/dédié.

## Risques et recette nécessaire

Le moteur peut encore refuser un placement après vérification. Un arrêt conserve
les lots acquittés ; un lot déjà payé peut être incomplet. Les limites englobantes
peuvent refuser des assemblages pourtant possibles. Le chargement/tri est synchrone,
une pièce reste indivisible, et les budgets ne garantissent pas une limite de temps
réel. Le contenu des coffres, la logique, les quêtes et la progression ne sont pas
sauvegardés par ces blueprints. Un changement du jeu peut désactiver le mod ou
seulement ses extensions natives.

Fermer le jeu, conserver l'ancienne DLL et une copie personnelle du monde de test,
choisir un seul chargeur/une seule copie du mod et suivre les douze étapes de
[EXTENDED-TESTING.md](EXTENDED-TESTING.md). Commencer par les fonctions originales
puis les trois modules ; essayer le village et les natifs ensuite. Le guide donne
le résultat attendu, les journaux à récupérer et la procédure de retour à l'original.
Ne pas utiliser le monde principal pour cette campagne.

## Historique local de la mission dev3

- `e0e3a1c` — consolidation du placement, rotations natives, Home, garde ABI,
  diagnostic, configuration, tests et exemples.
- `a348062` — dimensions des objets du catalogue et erreurs de sauvegarde INI.
- `5c46286` — manifests d'exemple identiques sous Windows/WSL, indépendants des fins de ligne du checkout.
- Le commit de distribution complète ce travail avec les guides, le vérificateur
  d'archives et le packaging ; son identifiant est disponible dans `git log`.

Aucun push, tag de release, upload ou déploiement automatique n'a été effectué.
Voir [DEV3-AUDIT.md](DEV3-AUDIT.md), [DEV3-EXAMPLES.md](DEV3-EXAMPLES.md) et
[EXTENDED-STATUS.md](EXTENDED-STATUS.md) pour le détail des corrections et replis.
