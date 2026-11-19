# Validation technique V2

**PASS hors jeu ; recette V2 en jeu non effectuée.** Le retour du joueur porte
sur la V1 et a motivé le redessin des intérieurs.

| Contrôle exécuté | Résultat |
|---|---|
| Cellules / modules BTBP | 156888 / 30 |
| Espaces réservés / colonnes | 431 / 7986 |
| Pièces et destinations reliées | 62 / 62 |
| Éléments intérieurs / allées protégées | 142 / 32 |
| Graphe des surfaces / composante depuis l'entrée | 46918 / 46918 |
| Empreinte du second contrôle de circulation | 1,5 × 1,5 m, 3 m de hauteur |
| Chevauchements physiques entre composants | 0, même matériau inclus |
| Corps d'escaliers intersectés / soutiens découpés | 0 / 0 |
| Pièces natives expérimentales / objets | 0 / 0 |
| Lots théoriques à 128 cellules | 1226 |

Le premier contrôle vérifie les volumes occupés, les réservations d'air, chaque
colonne de soutien, les allées de 3 m et les cages protégées. Le placement d'un
meuble dans une cage est refusé même s'il n'occupe pas encore la cellule d'une
marche. Tout chevauchement avec un autre composant est refusé, sans exception
fondée sur l'identité de matériau. Les jonctions de maçonnerie et de dalles sont
attribuées une seule fois lors du calcul de la structure.

Le graphe des surfaces vérifie toutes les marches et tous les paliers. Le second
contrôle utilise des empreintes **1,5 × 1,5 m** sur les sols plats, puis raccorde
les composantes par les escaliers et leurs paliers vérifiés. Il contrôle les
accès des pièces, les arrivées et les changements de direction. Les corniches,
toitures et appuis de fenêtres ne servent pas de chemins. Les intérieurs des
deux bassins décoratifs sont exclus du réseau large, et sont explicitement
signalés dans `Validation.json`. Aucune pièce ni aucun palier n'est exclu.

La relecture C++ emploie `core::bp_decode`, `project::add/finish` et
`spaces::validate`, puis vérifie les quatre rotations et les lots 1/32/128/256
avec interruptions de la machine d'état. Chaque BTBP conserve une grille de
0,5 m et au plus 64 000 cellules denses. Le plafond 128 modules reste inchangé.
Les modules d'export sont des secteurs spatiaux de catégorie `general` ; les
rôles de matériaux restent configurables au moment de la génération.

Ces contrôles ne reproduisent pas la capsule, le pas maximal ou la physique du
personnage. La pratique des marches, la stabilité, les collisions du monde,
les ressources et la couverture des autels restent à tester en jeu. La reprise
automatique et le rollback complet restent indisponibles dans dev7.
