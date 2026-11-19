# Dev5 — état de validation

Version : `0.34.0-dev5`, Standalone Windows x64.

Le test utilisateur de dev4 confirme l'arrivée des commandes au moteur, puis un
refus de résolution du premier matériau. Aucun placement dev5 n'a été exécuté par
l'agent. Le diagnostic et la recette figurent dans
[DEV5-MATERIALS-FIX.md](DEV5-MATERIALS-FIX.md).

## Livrables locaux

- `build/dinput8.dll` : DLL PE32+ Windows x64, version dev5.
- `dist/EnshroudedBuildingTools-0.34.0-dev5.zip` : installation Standalone.
- `dist/EnshroudedBuildingTools-Shroudforge-0.34.0-dev5.zip` : même DLL, autre chargeur.
- `dist/SHA256SUMS-0.34.0-dev5.txt` : empreintes des deux archives.

SHA-256 de la DLL :
`1ab4b0186f4f891655fe690d70f968b2ebdefc122ff2f0464d61503f72a72484`.
Les anciens ZIP sont conservés. Rien n'est installé automatiquement dans le jeu.

## Résultats automatisés du 19 novembre 2026

| Contrôle | Résultat |
|---|---|
| Compilation Windows freestanding x64 avec clang/lld sous WSL | Réussie |
| Tests historiques, langues et contrôles du binaire local | Réussis, dont 40 signatures uniques vérifiées et 3 contrôles de réflexion ABI |
| Projets | 145 203 assertions, 0 échec |
| Boutons et fermeture de fenêtre | 209 assertions, 0 échec |
| Matériaux et coûts | 209 assertions, 0 échec |
| Fonctions Extended | 2 457 assertions, 0 échec |
| Configuration | 16 assertions, 0 échec |
| Export | 0 échec |
| Matériaux/coûts avec ASan + UBSan | 209 assertions, 0 échec, aucun diagnostic mémoire |
| Fixtures Python sous Windows et WSL | 4 tests réussis dans chaque environnement |
| Comparaison binaire des dix BTBP à dev4 | Seuls les 4 octets d'ID changent ; dimensions, compteurs, géométrie et objets identiques |
| Archives Standalone / Shroudforge | 45 / 48 entrées, CRC, sources, liens, licence, langues, exemples et chargeur vérifiés |
| Recompilation depuis les mêmes sources | DLL strictement identique, horodatage PE nul |

Le nouveau test compile le scanner réel, l'adaptateur natif du résolveur et
l'adaptateur de lecture/accumulation des coûts de la DLL avec des allocations et
appels de registre/remboursement contrôlés. Il vérifie notamment : ID direct,
octet renuméroté, ancien ID de remboursement, conflit refusé, nouveau monde sans
remboursement, mauvaise allocation, absence de cache négatif, pointeur déplacé,
données non enregistrées, terrain et forme non admissible. Pour les coûts :
offsets natifs, quantité nulle, item inconnu, limite exacte, dépassement sans
mutation du compteur, 129e item, mélange objet/bloc, liste mal formée, prévalidation
et revérification de construction, créatif activé et désactivé.

Ces doubles d'API ne reproduisent ni l'algorithme de tarification d'Enshrouded,
ni son moteur de placement, le paiement, les collisions ou la persistance du
monde. L'ID de remboursement réel au moment du refus dev4 n'était pas journalisé ;
sa valeur dans le test est une hypothèse contrôlée, pas une observation native.
La preuve native disponible concerne le bloc `0x7ad0559c` / `0x9d`.

Binaire du jeu utilisé en lecture seule pour les contrôles statiques : SHA-256
`5d9ef2f140b8ed49e30dbee655841b89d08baec20f799a1817e171a18a850c77`.
Traces locales ignorées par Git : `build/dev5-package.log`,
`build/dev5-sanitizers.log`, `build/dev5-artifacts.log`, `build/dev5-rebuild.log`.
Le code et les fixtures sont versionnés ; les DLL/ZIP
restent dans les dossiers de build/dist ignorés par Git.

**Validation en jeu en attente.** Le chargement dev4 et l'arrivée des commandes
ne prouvent pas que dev5 construit ni qu'il facture correctement. Suivre la
[recette ciblée](DEV5-MATERIALS-FIX.md), installer aussi les exemples corrigés,
puis vérifier le matériau, les coûts, les acquittements et le monde rechargé.
