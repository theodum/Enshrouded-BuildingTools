# Compte rendu de validation — 17 novembre 2026

Version : **0.34.0-dev1**. Branche : `feature/composite-projects`.
Base du fork : `6af1321`, Building Tools 0.33.2.

## Contrôles réalisés

| Contrôle | Résultat |
|---|---|
| Compilation initiale 0.33.2 et tests historiques | Réussis avant changements fonctionnels |
| DLL Extended Windows x86-64, clang/lld 18.1.3 sous WSL Ubuntu 24.04 | Réussie |
| Suite `test_core`, langues incluses | Réussie |
| Contrôles binaires contre l'exécutable local | 40 signatures contrôlées uniques aux RVA attendues ; contrôles des prologues et sites de patch réussis |
| Suite composite `test_project` | 144 704 assertions, 0 échec |
| AddressSanitizer + UndefinedBehaviorSanitizer sur la suite composite | Réussis |
| Générateur d'exemple Python | 2 tests réussis : fixtures reproductibles et refus d'écraser une sortie existante |
| Plan de 70 000 cellules, trois modules | Coordonnées triées déterministes et lots acquittés, sans fusion dense |
| Exemple livré à trois fichiers `.btbp` | 312 cellules après subdivision ; positions vérifiées pour les quatre rotations globales |
| Archives standard et Shroudforge | ZIP intègres, même DLL que la compilation, 15 langues, licence identique, trois modules et manifeste identiques aux fixtures, JSON Shroudforge lisibles |

Les assertions nombreuses comprennent les comparaisons de chaque cellule du
grand plan ; ce nombre ne désigne pas autant de scénarios de test distincts.
Les appels et l'interface du moteur ne sont pas simulés par ces tests.

Exécutable local lu uniquement pour les vérifications statiques :
SHA-256 `5d9ef2f140b8ed49e30dbee655841b89d08baec20f799a1817e171a18a850c77`.
Aucun exécutable propriétaire n'est ajouté au dépôt ni aux archives.

## Livrables locaux

- `build/dinput8.dll` : DLL du mod.
- `dist/EnshroudedBuildingTools-0.34.0-dev1.zip` : installation standard.
- `dist/EnshroudedBuildingTools-Shroudforge-0.34.0-dev1.zip` : installation Shroudforge.
- `dist/SHA256SUMS.txt` : empreintes des archives finales.
- `build/validation.log` : sortie locale de compilation et tests, ignorée par Git.
- `docs/EXTENDED-AUDIT.md`, `docs/BTPROJECT.md`, `docs/EXTENDED-TESTING.md` :
  audit, format, installation et recette en jeu.

Les artefacts binaires sont ignorés par Git comme dans le dépôt original.
Ils sont disponibles dans le répertoire de travail ; les sources permettent de
les régénérer. Aucun push ni release publié, aucune installation dans le jeu.

## Validation encore nécessaire

**Aucun test de placement en jeu n'a été réalisé.** Les étapes de
`EXTENDED-TESTING.md` doivent être exécutées par l'utilisateur sur un monde de
test avant de déclarer le critère « trois bâtiments alignés, une validation »
accepté. L'affichage natif, la réception des clics, le prix réel, les limites des
autels, les performances et la stabilité après sauvegarde/rechargement restent
à observer en jeu.

Cette première livraison couvre les blocs composites. Les objets natifs des
composites, la grille virtuelle persistante en altitude, les aides visuelles
avancées aux autels et collisions, le créatif étendu aux objets et les composants
architecturaux ne sont pas livrés ici. L'annulation reste celle du jeu, partielle
pour les grosses opérations. Un arrêt peut laisser des lots construits et un
lot payé mais incomplet ; pas de reprise ou de rollback automatique.

Aucun fichier de sauvegarde n'a été lu, supprimé, écrasé ou modifié.
