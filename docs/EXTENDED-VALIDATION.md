# Compte rendu de validation — 17 novembre 2026

Version : **0.34.0-dev2**. Branche : `feature/composite-projects`.
Base du fork : `6af1321`, Building Tools 0.33.2.

## Contrôles réalisés

| Contrôle | Résultat |
|---|---|
| Compilation initiale 0.33.2 et tests historiques | Réussis avant changements fonctionnels |
| DLL Extended Windows x86-64, clang/lld 18.1.3 sous WSL Ubuntu 24.04 | Réussie |
| Recompilation depuis les mêmes sources | DLL strictement identique ; horodatages PE nuls, imports KERNEL32/USER32 |
| Suite `test_core`, langues incluses | Réussie |
| Contrôles binaires contre l'exécutable local | 40 signatures uniques aux RVA attendues ; prologues, sites de patch et instructions matériau des pièces natives vérifiés |
| Suite composite `test_project` | 144 712 assertions, 0 échec |
| Suite `test_extended` | 73 assertions, 0 échec : grille, horloge, transformations, drapeaux, matériaux, composants, escaliers et dégagement |
| Suite `test_project_export` | 0 échec : export réel compilé avec adaptateurs de fichiers, relecture et coordonnées, refus d'écrasement et de publication après échec d'écriture ou de vidage disque |
| AddressSanitizer + UndefinedBehaviorSanitizer | Réussis sur les trois suites portable/composants/export |
| Générateur d'exemple Python | 2 tests réussis : fixtures reproductibles et refus d'écraser une sortie existante |
| Plan de 70 000 cellules, trois modules | Coordonnées triées déterministes et lots acquittés, sans fusion dense |
| Exemple livré à trois fichiers `.btbp` | 312 cellules après subdivision ; positions vérifiées pour les quatre rotations globales |
| Archives standard et Shroudforge | ZIP intègres, même DLL que la compilation, 15 langues, licence identique, trois modules et manifeste identiques aux fixtures, JSON Shroudforge lisibles |

Les assertions nombreuses comprennent les comparaisons de chaque cellule du
grand plan ; ce nombre ne désigne pas autant de scénarios de test distincts.
Les tests d'export utilisent la vraie fonction de sérialisation, mais remplacent
uniquement les API de fichiers Windows. Les appels et l'interface du moteur,
les événements d'entités, les entrées utilisateur et Vulkan en jeu ne sont pas
simulés par ces tests.

Exécutable local lu uniquement pour les vérifications statiques :
SHA-256 `5d9ef2f140b8ed49e30dbee655841b89d08baec20f799a1817e171a18a850c77`.
Aucun exécutable propriétaire n'est ajouté au dépôt ni aux archives.

## Livrables locaux

- `build/dinput8.dll` : DLL du mod.
- `dist/EnshroudedBuildingTools-0.34.0-dev2.zip` : installation standard.
- `dist/EnshroudedBuildingTools-Shroudforge-0.34.0-dev2.zip` : installation Shroudforge.
- `dist/SHA256SUMS-0.34.0-dev2.txt` : empreintes des deux archives.
- `build/dev2-validation.log` et `build/dev2-sanitizers.log` : sorties locales ignorées par Git.
- `docs/EXTENDED-AUDIT.md`, `docs/BTPROJECT.md`, `docs/EXTENDED-TESTING.md`,
  `docs/EXTENDED-STATUS.md` : audit, format, installation, état de la mission et recette personnelle.

Les artefacts binaires sont ignorés par Git comme dans le dépôt original.
Ils sont disponibles dans le répertoire de travail ; les sources permettent de
les régénérer. Les archives dev1 sont conservées. Aucun push ni release publié,
aucune installation dans le jeu.

## Validation encore nécessaire

**Aucun test de placement en jeu n'a été réalisé.** Les étapes de
`EXTENDED-TESTING.md` doivent être exécutées par l'utilisateur sur un monde de
test avant de déclarer le critère « trois bâtiments alignés, une validation »
accepté. L'affichage natif, la réception des clics, le prix réel, les limites des
autels, les performances et la stabilité après sauvegarde/rechargement restent
à observer en jeu.

Dev2 ajoute la grille virtuelle persistante, les aperçus bornés, les objets
compatibles et pièces natives, le catalogue, les composants architecturaux,
les matériaux, l'assistance aux autels et l'export sans écrasement. Les modèles
fantômes texturés, les objets logiques dans les composites, les collisions avec
toutes les entités, les jonctions automatiques de toitures, la spirale arrondie,
le placement automatique d'autels et le rollback complet restent indisponibles.
Le document d'état explique les contraintes natives et les replis retenus.

L'annulation reste celle du jeu, partielle pour les grosses opérations. Un arrêt
peut laisser des lots construits et un lot payé mais incomplet ; pas de reprise
ou de rollback automatique.

Aucun fichier de sauvegarde n'a été lu, supprimé, écrasé ou modifié.
