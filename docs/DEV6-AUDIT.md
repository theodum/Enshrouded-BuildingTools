# Audit technique — socle dev5 et préparation dev6

Date : 19 novembre 2026. Référence auditée : `591090f` (0.34.0-dev5), branche
`feature/composite-projects`. Les ajouts décrits comme dev6 sont séparés du bilan
de cette référence. Aucun lancement du jeu ni accès aux sauvegardes par l'agent.

## 1. Ce qui constitue une preuve

| Niveau | Source | Portée |
|---|---|---|
| J5 | Retour utilisateur accompagnant cette mission, sur dev5 | Observation réelle des scénarios cités ci-dessous ; aucun nouveau journal complet n'est fourni dans cette tâche |
| A5 | Suites C++/Python et bilan historique dev5 | Algorithmes, adaptateurs contrôlés, fichiers et compilation ; ne reproduit pas le moteur |
| B | Contrôles statiques du binaire local et garde ABI au chargement | Signatures/structures compatibles avec ce binaire ; ne prouve ni interaction ni persistance |
| A6 | Tests dev6, résultats dans DEV6-VALIDATION | Nouveaux diagnostics, géométrie et adaptateurs ; aucun résultat en jeu dev6 |

Le retour **J5** confirme : chargement des composites ; construction des trois
modules avec **312 cellules** ; construction de trois bâtiments reliés ;
alignement et passages de cet ensemble ; résolution des matériaux de ces essais ;
contrôle des limites des autels dans les essais réalisés. Il ne valide pas par
extension les escaliers, toutes les familles de matériaux, le paiement en mode
normal, la persistance après rechargement ou les objets interactifs.

Les anciennes mentions « en attente de test dev5 » dans les rapports de livraison
décrivent leur état historique. Ce retour utilisateur les complète pour les seuls
scénarios ci-dessus. La nouvelle DLL dev6 demande sa propre non-régression.

## 2. Architecture et chemins d'exécution

La DLL est un proxy `dinput8.dll`, Windows x64, C++ freestanding, compilé par
clang/lld. `main.cpp` assemble les adaptateurs ; les modules `core.h`, `project.h`,
`extended_core.h`, `materials.h` et `project_actions.h` portent les politiques
portables partagées avec les tests. Licence MIT et attribution amont conservées.

```text
BTBP / manifeste ou générateur d'aperçu
  -> plan sparse Y/Z/X, palette, modules, pièces et objets
  -> position et rotation globales
  -> bouton Vérifier ou Construire, contrôles de session/outil/état
  -> fermeture acquittée de la fenêtre native
  -> validation complète : matériaux, couverture, collisions, prix/stock
  -> inspection terminée OU construction par lots revérifiés
  -> prix et paiement natifs -> commit natif -> acquittement -> lot suivant
```

Les générateurs modifient le **brouillon**, jamais le monde. La construction garde
la voie native de prix/paiement/commit et les restrictions des mondes locaux.
Les rapports et exports écrivent de nouveaux fichiers dans le dossier des projets,
sans lecteur/éditeur de sauvegarde `.ksd` et sans installation automatique.

## 3. Inventaire précis

Les états se cumulent : une fonction peut avoir des tests automatisés, rester
expérimentale et demander une validation dans le jeu.

| Fonction | Code / tests | État et limite de preuve |
|---|---|---|
| BTBP v1, RLE, palette, extension d'objets | `core.h`, `blueprint.h` ; `test_core` | Implémenté et testé. Compatibilité de format conservée ; tous les anciens fichiers possibles ne sont pas validés en jeu |
| JSON strict, fichiers voisins, grilles, ancrage et rotations | `project.h` ; `test_project` | Implémenté et testé. J5 confirme le chargement et l'alignement sur les exemples joués |
| Raffinement 1/2/4 m en voxels de 0,5 m | `project.h` ; `test_project` | Implémenté et testé. Le cube tenu de 0,5 m reste obligatoire ; J5 n'est pas une validation de toutes les combinaisons |
| Déduplication / matériaux contradictoires | `project::finish` ; `test_project` | Implémenté et testé : doublons identiques fusionnés, conflit de matériau refusé |
| Construire / Vérifier / Entrée / fermeture native | `project_actions.h`, `blueprint_ui.h`, `project_runtime.h` ; `test_project_actions` | Implémenté et testé ; commandes et construction observées dans J5. Focus, timeout, journal et transitions restent principalement couverts par doubles d'API |
| Prévalidation complète et lots | `project::Job`, runtime ; tests Job/actions | Implémenté ; J5 confirme les petits composites. Les cas d'échec après paiement et gros volumes restent à éprouver en jeu |
| Résolution de matériau | `materials.h`, adaptateurs blueprint ; `test_materials` | Implémenté et testé ; résolution fonctionnelle dans J5. Preuve native exigée, anciens remboursements ambigus refusés |
| Coûts / stock / mode créatif | `project_material_cost.h`, hooks natifs ; `test_materials` | Calcul et limites testés avec entrées contrôlées. La facture et les débits réels en mode normal ne sont pas validés par le retour fourni |
| Couverture d'autel à la construction | `project_covered`, `BaseAt` ; vérifications B | Implémenté ; limites observées dans J5. Pas de preuve des quotas/progression ni de placement automatique d'autel |
| Collisions avec blocs / incohérences de volume | `project_check`, `project_check_pieces`, `plan_intersects` ; `test_extended` | Implémenté, conservateur, premier défaut bloquant. Pas de moteur général de collisions d'entités/terrain ni de validation de navigation |
| Aperçu Vulkan / modules / tranches / objets | `vkdraw.h`, `extended_scene.h`, UI ; géométrie portable testée | Implémenté ; affichage UI/Vulkan déjà observé lors des versions précédentes. Toutes les tranches et grandes scènes ne sont pas validées |
| Grille virtuelle, altitude, huit niveaux | `extended_core.h`, configuration et UI ; `test_extended`, `test_extended_config` | Implémenté et testé hors jeu ; utilisation complète à valider en jeu. Dépend de Free Placement |
| Catalogue de vrais IDs | `extended_catalog.h`, registre/itérateur natifs | Implémenté et borné. Liste des pièces générique : ne reconnaît pas automatiquement la direction ou la montée d'un escalier |
| Remplacement de matériaux | `replace_material`, UI ; `test_extended` | Partiel : portée globale/module/catégorie sur les **voxels**. Ne réaffecte pas les IDs explicites des pièces natives ni les objets |
| Murs, ouvertures, encadrements | `Component`, `architecture_component` ; `test_extended` | Implémenté et testé géométriquement ; constructions générées spécifiques à valider en jeu |
| Sols, bordures, trémies | mêmes modules/tests | Implémenté et testé ; pas de graphe d'accès automatique entre étages dans le socle dev5 |
| Toitures et tours | profils `core`, `Component` ; `test_core`, `test_extended` | Implémenté en voxels : profils, tours carrées/rondes, fenêtres, planchers, créneaux. Pas de château intégré ni de preuve de circulation |
| Escaliers droits, tournants, impériaux, spirale carrée | `stair_layout`, `architecture_stairs` ; `test_extended` | Prototype natif testé pour les tracés/rotations. Modèle, montée et praticabilité non validés en jeu ; spirale carrée, pas hélicoïdale |
| Réservation de dégagement | suppressions de cellules Floor, volumes natifs + 2 m ; `test_extended` | Partiel : réserve des planchers et refuse des volumes occupés. Ne déduit pas les marches, rampes et chemins praticables d'une collision native |
| Exemple natif dev5 | `architecture_native_example` | Deux planchers, pas un bâtiment complet à trois étages. Montée estimée par AABB, pas de preuve en jeu ; insuffisant pour cette mission |
| Objets / mobilier composites | `bpobjects.h`, catalogue/runtime ; politiques de drapeaux et transformations testées | Expérimental, activation explicite requise. Pas de validation en jeu par famille d'objet ; états d'inventaire/quête/câblage non copiés |
| Portes / fenêtres / objets logiques interactifs | filtre `extended_prop_item`, writer d'événements étudié | Non disponible en automatique composite : garde conservée, événement logique complet non établi. Embrasures en blocs possibles ; portes à poser manuellement |
| Autels : zones affichées / assistance | `extended_altars.h`, `extended_scene.h` ; calculs purs testés | Assistance partielle : zones observées et propositions, pas de validation de quota, terrain ou progression ; pièces/objets échantillonnés |
| Export nouveau projet | `project_export.h`, `plan_tiles` ; `test_project_export` | Implémenté et testé sur vrais fichiers temporaires : publication du manifeste en dernier, refus d'écrasement. Rechargement dans le jeu à valider pour les nouveaux cas |
| Annulation / arrêt / rapport | Job/runtime, hooks undo ; tests actions | Arrêt des futurs lots et journal ; undo natif limité, **pas de transaction/rollback intégral**, ni reprise sûre après erreur |
| Multijoueur composite | admission locale | Désactivé/refusé ; pas de protocole de transfert de projets au serveur dédié |
| Villages et grands ensembles | manifestes statiques, exemple 75 648 cellules | Composition de modules implémentée/testée hors jeu ; générateur de quartiers, routes ou métiers absent |

## 4. Garde-fous et budgets à conserver

| Ressource | Limite actuelle |
|---|---|
| Modules / matériaux | 128 / 64 |
| Cellules du plan composite | 1 048 576 occupées |
| BTBP chargé individuellement | tampon de 65 536 cellules |
| Pièces natives / objets par BTBP | 2 048 / 512 |
| Lot de blocs | 128 par défaut, maximum 256, réduction après contrôle lent |
| Lot natif | 1 pièce ou 4 objets |
| Sondes d'une pièce / d'un objet | 4 096 / 512 voxels d'enveloppe |
| Coûts | 128 items distincts ; 2 147 483 647 unités par item ; 256 entrées de liste temporaire |
| Temps d'acquittement | arrêt à 10 s, aucun replay automatique |
| Assistance d'autels / traits d'aperçu | 64 propositions / 4 096 traits |

Le registre doit confirmer les blocs ; les anciens IDs de remboursement ne sont
acceptés qu'avec les preuves prévues par le correctif amont #61. Le créatif ne
supprime pas ces contrôles ni les limites de coûts. La réflexion ItemInfo,
EquipmentSetup et les drapeaux, ainsi que les instructions natives concernées,
doivent correspondre. Les objets spéciaux, de progression et logiques restent
refusés dans les composites. `ExperimentalNativePlacement=0` reste la valeur par
défaut. Aucun changement dev6 ne déverrouille ces protections.

## 5. Ce qui manquait aux diagnostics dev5 et ajouts dev6

| Constat dans dev5 | Ajout ciblé dev6 | Validation nécessaire |
|---|---|---|
| Résumé des cellules/modules, mais pas de dimensions totales explicites après rotation | Dimensions X/Y/Z des cellules et enveloppes natives dans aperçu, logs et rapport | Comparer au bâtiment en jeu |
| Coûts affichables même après prévalidation partielle ou déplacement | État de validation lié au brouillon, origine, rotation, créatif et option objets ; devis partiel/périmé distinct | Déplacer, tourner et changer de mode après Vérifier |
| Erreur native de chargement peu localisée | Index de pièce, ID, matériau et motif ; erreur persistante dans la liste | Essai contrôlé avec ID inconnu |
| Couverture/collision : raison générique et repère pour certains cas | Position, module/cellule/pièce dans UI/logs, échec explicite de prévalidation | Collision et sortie d'autel connues |
| Rapport limité au nom/progrès/empreinte | Instantané des dimensions, palette, coûts complets/partiels et zones observées de l'opération | Exporter avant/après chargement d'un autre projet |
| Pas de bâtiment natif de trois étages | Fabrique paramétrique détaillée ci-dessous | Marche, orientation, facture et rechargement du monde |

L'empreinte FNV64 reste un identifiant diagnostique, pas une preuve cryptographique
du monde. Une validation passée reste historique : le joueur peut modifier le
monde ensuite ; Construire répète les contrôles. Les compteurs de disponibilité
signalent des références refusées/inconnues, sans garantir la création d'entités.
Les snapshots d'autels ne remplacent pas `BaseAt` ; une observation périmée peut
être absente du rapport sans que cela prouve l'absence de tout autel.

## 6. Démonstration multiétages dev6

`storey_demo.h` décrit la géométrie paramétrique ; `architecture_storeys.h` la lie
à un escalier et un matériau **sélectionnés dans le registre du jeu**. Le choix
de cette sélection native a été confirmé par l'utilisateur pendant la mission.
Ni ID fictif ni nouveau format de blueprint ne sont livrés.

Le brouillon comporte 16 modules : trois planchers, quatre murs par niveau et
une toiture ; deux montées reliées par un palier, avec un palier supérieur, des
trémies réservant les volumes et 2 m d'air, des embrasures et des fenêtres. Les
marches utilisent les vraies pièces natives, les paliers des surfaces de blocs.
Le matériau choisi est commun aux voxels et pièces.

La hauteur d'étage est un multiple de la montée calibrée d'une volée, au moins
4 m. Le parcours droit privilégie la lisibilité du test. Les dimensions et comptes
dépendent du modèle choisi. Exemple **de test contrôlé, pas d'ID réel** : pièce de
2 m de large, 2 m de haut et 3 m de long, montée 2 m : 4 pièces, 4 660 voxels,
enveloppe occupée de 9 × 14,5 × 22 m. Ces chiffres ne sont pas universels.

Le test compile la vraie transaction `ArchitectureEdit` et l'adaptateur de démo,
vérifie la restauration après échec tardif/allocation, les quatre orientations,
les réservations, paliers et l'export/relecture du projet complet. Une matrice
de quaternion indépendante fournit les volumes contrôlés ; elle ne reproduit pas
la collision d'un escalier du jeu. Un test sans arrondi a révélé des faux contacts
de bord après rotation : la conversion des limites natives canonise seulement
les écarts à une grille de 1/16 m inférieurs à **0,0001 unité**, soit 6,25 µm. Un
débordement réel supérieur reste arrondi de façon conservatrice et bloquant.

Le générateur demande une montée explicitement réglée ; zéro (estimation de
boîte) est refusé pour cette démo. Il ne connaît pas la surface marchable ni le
sens sémantique d'un modèle : la calibration d'une volée et le test à pied sont
indispensables. Une pièce incompatible ou hors budget laisse le brouillon intact.

La livraison est une **fabrique de projet dans l'interface**, puis un export
BTBP/`.btproject` avec les vrais IDs de cette installation. Il n'existe pas de
BTBP universel d'escalier prévalidé. Après export, le projet se recharge dans la
liste habituelle, puis utilise les boutons Vérifier/Construire existants.
Recette : [DEV6-MULTISTOREY.md](DEV6-MULTISTOREY.md).

## 7. Manques pour un moteur architectural

- Description de recettes persistantes : paramètres, rôles de matériaux,
  variantes, seed et version, distincts du résultat BTBP exporté.
- Connecteurs nommés : sol, porte, marche basse/haute, galerie, tour/rempart,
  hauteur libre ; solveur des raccords et graphe d'accès entre pièces et étages.
- Profils natifs calibrés par version du jeu : montée réelle, axe, connecteurs,
  volumes de collision et dégagement, au lieu de déductions depuis la seule AABB.
- Règles d'urbanisme : routes, parcelles, quartiers, budgets, emprise et couverture
  avant génération ; les « métiers » seraient d'abord des thèmes architecturaux.
- Décoration guidée par des points d'attache sûrs ; writer et cycle de vie des
  portes/logiques à établir avant toute automatisation interactive.
- Contrôles d'entités/terrain/support, parcours joueur et performances à mesurer
  en jeu. Le code actuel ne peut pas les certifier.
- Publication et récupération après erreur : snapshot complet du contexte,
  historique vérifiable et protocole sûr. Ne pas présenter l'undo partiel actuel
  comme une transaction de château entier.

La faisabilité et l'ordre de travail figurent dans
[ARCHITECTURE-ROADMAP.md](ARCHITECTURE-ROADMAP.md). Cette mission n'implémente pas
un générateur de château/village ni un déverrouillage des objets logiques.
