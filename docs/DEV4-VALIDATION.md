# Building Tools Extended 0.34.0-dev4 — validation hors jeu

Correctif du démarrage des actions composites ; base `939cfe9` (dev3).
Commit fonctionnel : `8489141`. Branche : `feature/composite-projects`.

Le journal réel dev3 a permis d'isoler des boutons désactivés sans callback ni
log pour une forme incompatible. La forme vue dans cette trace correspond à un
enregistrement BTBP existant à pas de 1 m. La correction conserve le cube moteur
de 0,5 m obligatoire, affiche les dimensions et tous les refus et rend la remise
de la fenêtre au thread d'exécution explicite. Voir
[DEV4-ACTIONS-FIX.md](DEV4-ACTIONS-FIX.md) pour la preuve et le retest ciblé.

## Livrables

Depuis `C:\Users\User\enshrouded\Enshrouded-BuildingTools` :

- `build/dinput8.dll` : DLL Windows x64 dev4.
- `dist/EnshroudedBuildingTools-0.34.0-dev4.zip` : installation standard.
- `dist/EnshroudedBuildingTools-Shroudforge-0.34.0-dev4.zip` : disposition Shroudforge.
- `dist/SHA256SUMS-0.34.0-dev4.txt` : empreintes des deux ZIP.
- `build/dev4-validation.log`, `build/dev4-actions-sanitizers.log`,
  `build/dev4-artifacts.log` : traces locales de validation, ignorées par Git.

Les archives dev1/dev2/dev3 sont conservées. Le pack inclut les quinze sources et
manifests des cinq exemples, les quinze langues, le modèle d'INI non actif, la
licence inchangée, les guides de dev4 et le bilan dev3 clairement historique.
Il n'installe rien dans le jeu et ne modifie aucun INI actif ou monde.

## Tests et portée

| Contrôle | Résultat |
|---|---|
| Compilation Windows freestanding x64 clang/lld sous WSL | Réussie |
| Tests historiques, langues et contrôles binaires locaux | Réussis |
| `test_project` | 145 203 assertions, 0 échec |
| `test_project_actions` | 209 assertions, 0 échec : boutons réellement activés, deux commandes, dimensions, états fenêtre/projet/session/mode, hooks/bases, journal, doublons/Entrée, fermeture, horloge circulaire et commit abandonné |
| `test_extended` | 2 457 assertions, 0 échec |
| `test_extended_config` | 16 assertions, 0 échec |
| `test_project_export` | 0 échec |
| ASan + UBSan du nouveau dispatcher/lifecycle | Réussis |
| Python, fixtures | 3 tests réussis sous Windows et sous WSL |
| Archives | CRC, contenu exact, même DLL, PE32+ x64/imports/proxy, licence, langues, fixtures, liens et JSON du chargeur vérifiés |
| Recompilation | DLL identique depuis les mêmes sources, horodatage PE nul |

Le nouveau test appelle le même routeur de boutons, la même politique de refus
et le même automate de fermeture que la DLL. Ses doubles d'API comptent les appels
et vérifient qu'un refus ne démarre/arme/ferme rien. Il couvre en particulier
`windowOpen=false` avec le statut d'écran encore périmé : aucun clic n'est autorisé
avant l'acquittement de fermeture et un écran de jeu prêt. Il ne simule pas l'UI
native, SendInput, le moteur, le paiement, les entités ou les écritures du monde.

Les contrôles de sécurité de dev3 restent actifs : aucune couverture forcée,
aucune tolérance ajoutée pour construire avec un cube de 1 m, aucun remboursement,
retrait ou replay automatique. Les lots éventuellement payés peuvent rester
incomplets après une erreur. Les fonctionnalités natives expérimentales restent
désactivées par défaut ; leurs limites ne sont pas changées par ce correctif.

**Le comportement en jeu de dev4 reste à valider personnellement.** Le chargement,
l'interface et Vulkan ont été observés par l'utilisateur avec dev3 ; cela ne valide
pas les placements du nouveau correctif. Suivre
[EXTENDED-TESTING.md](EXTENDED-TESTING.md) et le retest ciblé avant un grand projet.
Aucun lancement du jeu, installation, sauvegarde ou publication de release n'a
été effectué par l'agent.
