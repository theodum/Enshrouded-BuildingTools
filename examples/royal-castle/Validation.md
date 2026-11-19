# Validation technique — Couronne des Anciens

**Résultat hors jeu : PASS. Test de ce château en jeu : non effectué.**
Le joueur a confirmé la génération multiétage avec escaliers de la démo dev7.
Ce retour ne valide pas encore les profils ni l'architecture de ce château.

| Contrôle exécuté sur les fichiers livrés | Résultat |
|---|---|
| Cellules occupées / placements prévus | 57,817 |
| BTBP v1, grille 0,5 m, modules / maximum | 75 / 128 |
| Réservations d'air / maximum | 346 / 512 |
| Colonnes réservées / maximum | 2268 / 65 536 |
| Doublons dans l'export / chevauchements contradictoires | 0 / 0 |
| Escaliers / types | 25 / monumental, L, U |
| Pièces et destinations géométriquement reliées | 61 / 61 |
| Surfaces de plancher candidates du graphe, isolées | 0 |
| Graphe de surfaces, nœuds / composante depuis l'entrée | 14749 / 14749 |
| Pièces natives expérimentales / objets | 0 / 0 |
| Lots théoriques de 128 cellules | 452 |
| Surface au sol construite | 1384 m² |
| Dimensions X × Y × Z | 45 × 25 × 42.5 m |

Chaque BTBP est relu par `core::bp_decode`, chaque module ajouté par
`project::add`, puis le projet finalisé par `project::finish`. Le véritable
`spaces::validate` vérifie toutes les réservations et leurs soutiens.
Les quatre rotations globales sont contrôlées cellule par cellule. La machine
`project::Job` effectue des passes avec lots 1, 32, 128 et 256 : validation sans
écriture simulée, acquittements complets, arrêt à plusieurs préfixes et refus
d'un acquittement partiel. Aucun placement natif n'est simulé ou appelé.

La vérification de circulation est dérivée **des voxels finaux**, avec surface
de soutien, 2 m d'air et adjacency horizontale limitée à une différence de
0,5 m. Toutes les pièces enregistrées, toutes les marches/paliers et les
planchers exposés doivent rejoindre l'entrée. Les noms de pièces n'ajoutent
aucune arête artificielle au graphe. Les corniches, toits, appuis de fenêtres et
créneaux sont exclus des chemins autorisés : un détour par une façade ne peut
pas remplacer une porte. Les escaliers réservent 2,5 m d'air. `Circulation.json` fournit les positions,
altitudes et distances dans ce graphe. Ce modèle ne calcule ni la capsule du
joueur, ni la physique, ni la hauteur de pas native : il ne certifie pas un
escalier utilisable sans saut, ni la stabilité structurelle dans le jeu.

## Limites et vérifications encore nécessaires

- Couverture des autels : inconnue hors jeu, contrôlée en 3D par le moteur natif.
  Il faut couvrir la boîte complète, les corniches et tous les dégagements.
  `Placement.md` donne une estimation paramétrique ; aucun quota n'est contourné.
- Coût et stock : calcul natif avant exécution ; nombre de cellules ≠ devis réel.
- Collision terrain, entités, soutien et stabilité : inspection et test en jeu.
- Reprise automatique et rollback complet : indisponibles dans dev7. Une
  interruption conserve les lots déjà acceptés. Le journal reste diagnostique.
- Palette noble : substitutions explicites, aucune identité déduite d'un nom.
- Aperçu Vulkan dev7 : boîte volumineuse donc affichage par volumes de modules ;
  les plans et vues joints représentent la géométrie réelle hors jeu.

La DLL et le format restent ceux de dev7. Ce projet utilise uniquement les
blocs composites, leurs catégories, les réservations d'air et les lots existants.
Les sommes SHA-256 permettent de vérifier le contenu distribué.
