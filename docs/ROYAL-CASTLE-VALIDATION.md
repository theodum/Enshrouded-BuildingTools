# Validation de la livraison Couronne des Anciens

Contrôles exécutés le **19 novembre 2026**, dans le dépôt local. Le projet est
compatible avec la DLL dev7 existante ; aucune nouvelle compilation de la DLL
et aucune installation dans le jeu n'ont été effectuées.

## Résultat livré

| Mesure | Résultat |
|---|---|
| Projet sélectionnable | `Forteresse_Royale_Demo.btproject` |
| Cellules / sources BTBP | 57 817 / 75 |
| Dimensions X × Y × Z | 45 × 25 × 42,5 m |
| Volume de validation avec air | 45 × 25,5 × 42,5 m |
| Niveaux principaux | 0 / 4,5 / 9 m |
| Escaliers | 1 monumental, 2 L, 22 U |
| Espaces / colonnes réservées | 346 / 2 268 |
| Pièces et destinations reliées géométriquement | 61 / 61 |
| Graphe des surfaces prévues | 14 749 nœuds, une seule composante |
| Planchers isolés / doublons exportés / chevauchements contradictoires | 0 / 0 / 0 |
| Lots théoriques à 128 cellules | 452 |
| Pièces natives expérimentales / objets | 0 / 0 |

Le graphe exclut les corniches, créneaux, toitures et appuis de fenêtres. Il ne
peut pas valider une connexion intérieure grâce à un détour sur les façades.
Toutes les surfaces d'escalier, y compris les deux branches du monumental et
les paliers, rejoignent cette composante. Hauteur libre réservée aux escaliers :
2,5 m ; modèle de circulation : 2 m d'air et différence de niveau maximale de
0,5 m. Ce modèle ne reproduit pas les collisions du personnage ni sa physique.

## Tests exécutés

Commande principale : `bash tools/test_royal_castle.sh`, avec `ENSHROUDED_EXE`
pointant sur l'exécutable local en lecture seule. Journal intégral :
`build/royal-castle-tests.log`.

| Suite | Résultat |
|---|---|
| Core, langues et contrôles du binaire local | Tous réussis |
| Signatures uniques contrôlées aux adresses attendues | 40 / 40 |
| Réflexion et garde-fous ABI Extended | Réussis |
| Projets composites | 145 203 assertions, 0 échec |
| Actions et fermeture de l'interface native | 209 assertions, 0 échec |
| Matériaux et coûts | 209 assertions, 0 échec |
| Démonstration multistorey dev6 | 28 220 assertions, 0 échec |
| Escaliers voxels dev7 | 130 749 assertions, 0 échec |
| Extended | 2 457 assertions, 0 échec |
| Configuration | 16 assertions, 0 échec |
| Export C++ | 0 échec |
| Reproductibilité du projet dev7 | 59 fichiers identiques, 8 068 cellules |
| Exemples Python existants | 4 tests, 0 échec |
| Château Python | 8 tests, 0 échec |
| Relecture finale par `core::bp_decode`, `project::add/finish`, `spaces::validate` | Réussie |
| Rotations globales, toutes les cellules | 0°, 90°, 180°, 270° réussis |
| Machine de lots / arrêts à plusieurs préfixes / acquittements partiels | Réussis pour 1, 32, 128 et 256 cellules |
| Hôte château avec AddressSanitizer + UndefinedBehaviorSanitizer | Réussi, aucun diagnostic |
| Archive : CRC, dépendances, fichiers source, SHA-256, disposition sûre | Réussie |
| Plan SVG | XML valide |

Les tests spécifiques injectent une porte bouchée, un plafond dans l'escalier,
une marche manquante, une obstruction au point d'accès d'une pièce et un conflit
de matériaux. Ils vérifient aussi quatre rôles de matériaux distincts, la
non-écriture dans un dossier existant et la reproductibilité exacte des BTBP.
Les profils d'escalier sont produits par le véritable code dev7, pas recopiés
dans un second algorithme de marches.

L'archive finale est `dist/Forteresse_Royale_Demo-dev7.zip` (381 261 octets).
Son SHA-256 est :

```text
06ab0448380e9cf79e79c8510ed434b1c43d08b168b3d707e6b31c3354b18946
```

Le générateur ne lit pas les sauvegardes et n'y écrit pas. Le seul accès au jeu
pendant ce travail a été la lecture des journaux, des en-têtes des blueprints
locaux et de l'exécutable pour ses signatures ; aucun placement natif n'a eu lieu.
La DLL `build/dinput8.dll` conserve son SHA-256 dev7 :
`aa7109f61a59d0c814b0a06357b594a736eb083da312875ac5acf2f40380fb59`.

## Ce qui reste à tester dans Enshrouded

Le joueur a confirmé le bâtiment de démonstration multiétage dev7. L'agent
n'a pas testé ce château en jeu. Ses marches de 0,5 m peuvent nécessiter des
sauts ; aucun résultat automatique ne certifie une montée fluide. Couverture
réelle des autels, terrain, ressources, collisions d'entités, stabilité et
persistance après sauvegarde/rechargement doivent encore passer la recette.

La palette livrée utilise explicitement le Bloc de pierre des Anciens confirmé
par le registre dans les journaux locaux. Les identités du Bloc de château,
de la pierre raffinée et de l'obsidienne restent non établies ; les substitutions
sont documentées et configurables depuis de vrais BTBP exportés du jeu.

Le moteur dev7 conserve ses limites de reprise et de rollback : les tests d'arrêt
vérifient sa machine d'état, pas une interruption réelle du jeu ni une restauration
de sauvegarde. La procédure complète est dans `examples/royal-castle/Placement.md`.
