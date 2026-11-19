# Dev7 — validation de la livraison voxel

Version : `0.34.0-dev7`. Référence précédente : `df7bcba` (dev6).
La construction des petits composites reste validée **sur dev5** par le retour
utilisateur. Aucun test en jeu de dev7, de ses escaliers ou de sa franchissabilité
n'a été effectué par l'agent.

## Livrables locaux

- `build/dinput8.dll` : DLL Windows PE32+ x64.
- `dist/EnshroudedBuildingTools-0.34.0-dev7.zip` : installation Standalone.
- `dist/EnshroudedBuildingTools-Shroudforge-0.34.0-dev7.zip` : même DLL, autre chargeur.
- `dist/SHA256SUMS-0.34.0-dev7.txt` : empreintes des deux ZIP.
- `examples/dev7/Dev7_Batiment_trois_niveaux_voxels.btproject` et ses 58 BTBP :
  exemple installable, 8 068 cellules, 22 dégagements et aucune pièce native.

SHA-256 de la DLL :
`aa7109f61a59d0c814b0a06357b594a736eb083da312875ac5acf2f40380fb59`.

## Contrôles exécutés le 19 novembre 2026

| Contrôle | Résultat |
|---|---|
| Compilation Windows x64 freestanding clang/lld sous WSL | Réussie |
| Suite core, langues, binaire local | Réussie ; 40 signatures uniques et 3 contrôles de réflexion ABI |
| Projets composites | 145 203 assertions, 0 échec |
| Boutons / fermeture native | 209 assertions, 0 échec |
| Matériaux et coûts | 209 assertions, 0 échec |
| Démonstration native dev6 | 28 220 assertions, 0 échec |
| Escaliers voxels / espaces / export | 130 749 assertions, 0 échec |
| Fonctions Extended | 2 457 assertions, 0 échec |
| Configuration | 16 assertions, 0 échec |
| Export historique | 0 échec |
| ASan + UBSan, nouveaux escaliers | 130 749 assertions, aucun diagnostic mémoire |
| Fixtures Python existantes, Windows et WSL | 4 tests réussis dans chaque environnement |
| Nouvel exemple | 59 fichiers reproduits octet pour octet avec la fabrique et l'exporteur réels |
| Exemples dev5 | 15 fichiers BTBP/manifeste inchangés |
| ZIP Standalone / Shroudforge | 110 / 113 entrées ; CRC, contenu, PE/proxy/imports, même DLL, licence, langues, exemples, liens et chargeur vérifiés |
| Recompilation des mêmes sources | DLL strictement identique, horodatage PE nul |

La suite nouvelle couvre les quatre tracés, les deux soutiens, les rotations,
576 combinaisons de dimensions/nombres de marches, les largeurs d'un nombre
impair de voxels, les paliers et le graphe de raccords géométriques. Elle refuse
les montées non représentables, budgets dépassés, plafonds/blocs internes dans
l'air, surfaces manquantes et JSON mal formés/hors budget. Elle vérifie l'arrêt
au premier refus du visiteur d'air et la restauration du brouillon après échec.

La vraie transaction `ArchitectureEdit`, le vrai adaptateur de génération et
l'exporteur sont compilés avec des fonctions hôtes contrôlées. L'export complet
est relu par le vrai décodeur, puis comparé en coordonnées, catégories et espaces.
La démo est admise par la politique d'actions avec l'opt-in natif désactivé. Les
tests des versions précédentes continuent à passer ; les adaptateurs hôtes ont
été regroupés sans suppression de leurs assertions.

Ces doubles ne reproduisent pas le déplacement du personnage, les collisions
complètes du moteur, le paiement ou les writes du monde. Les nouvelles sondes
d'air réutilisent le dry-run natif déjà employé pour les voxels ; leurs effets
réels sont encore à tester en jeu. Une visite géométrique sans obstacle ne prouve
pas qu'une contremarche de 0,5 m est franchissable sans saut.

## Compatibilité et sécurité

BTBP v1 reste inchangé. Dev7 accepte les anciens manifestes ; le champ optionnel
`spaces` est initialisé à vide pour ceux-ci. Les nouveaux exports conservent les
dégagements et nécessitent dev7 lorsqu'ils utilisent ce champ. Les anciennes DLL
les refusent explicitement, sans omission silencieuse du contrôle.

Les voxels d'air ne sont ni construits ni tarifés. Les restrictions de session,
cube, autels, collision, matériaux, coûts et limites des lots restent actives.
Les fonctionnalités natives expérimentales gardent leur opt-in et leurs gardes
ABI ; elles ne sont pas nécessaires au nouvel exemple. Aucun fichier du jeu,
INI actif ou sauvegarde n'a été modifié par les outils de préparation.

Traces locales ignorées par Git : `build/dev7-package.log`,
`build/dev7-sanitizers.log`, `build/dev7-fixtures-wsl.log`,
`build/dev7-artifacts.log` et `build/dev7-rebuild.log`.
Le binaire du jeu utilisé en lecture seule pour les tests statiques a le SHA-256
`5d9ef2f140b8ed49e30dbee655841b89d08baec20f799a1817e171a18a850c77`.

**À valider dans le jeu :** rejeu des cas dev5 sur cette DLL, marches avec/sans
saut, quatre tracés et rotations, coûts réels, détection d'un plafond/bloc dans
l'air, trémies et paliers, export/rechargement et persistance du monde.
Suivre [DEV7-VOXEL-STAIRS.md](DEV7-VOXEL-STAIRS.md) avant de déclarer ces nouveaux
comportements fonctionnels.
