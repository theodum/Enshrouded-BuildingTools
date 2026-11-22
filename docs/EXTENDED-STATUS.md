# État de Building Tools Extended 0.34.0-dev12

Le retour réel confirme la capture et la reconstruction classique d'une table
**avec quatre décorations**. Dev12 réutilise ce BTBP intact et la file native
`bp_fill` pour les composites. Les modules `content=objects` / `objectPlacement=blueprint`
ne construisent aucun voxel source ; le lecteur historique et les contacts
capturés servent aux contrôles supplémentaires. Les autres projets conservent
leur politique par défaut. Voir [DEV12-CLASSIC-COMPOSITIONS.md](DEV12-CLASSIC-COMPOSITIONS.md).
La priorité est ce démonstrateur réel de cinq objets. **Sa reconstruction
composite et sa persistance restent à valider en jeu**, sans décorer le château.
Les bilans suivants sont historiques.

Le test réel dev10 confirme la résolution de la table mais échoue sur
`E_TABLE_CHANGED`, avant toute pose. Dev11 remplace la lecture sur plusieurs
callbacks par une collecte complète des entrées occupées dans un seul callback,
contrôlée avant/après et bornée. Une lecture instable ou partielle reste refusée.
Continuer **uniquement avec Dev10_A_Table_Unique** et le protocole de
[DEV11-TABLE-CHANGED.md](DEV11-TABLE-CHANGED.md). Pose et persistance restent à
valider en jeu ; aucun ameublement du château n'est développé dans cette étape.
Les bilans ci-dessous décrivent leurs livraisons historiques.

Dev10 corrige le contrôle des proxies natifs dégénérés et ajoute des diagnostics
distincts pour la lecture du monde, la résolution et le placement. La capture
classique peut enregistrer une calibration d'un seul meuble. Les projets A
(une table) et D (cinq meubles, après A/B/C) contiennent zéro structure.
La compilation et les régressions automatiques réussissent ; **le placement natif
réel et la persistance attendent le test utilisateur**. Le château reste inchangé.
Suivre en priorité [DEV10-NATIVE-PLACEMENT-FIX.md](DEV10-NATIVE-PLACEMENT-FIX.md).
Le semis du monde et la bougie refusée sont deux causes distinctes ; les bornes
brutes absentes des logs dev9 ne sont pas inventées. Les bilans ci-dessous
conservent leur contexte historique.

Dev9 ajoute les 3 526 références du CSV attribué, la recherche/catégorisation,
la résolution au chargement et un véritable export uniquement mobilier. Les
états distinguent référence importée, politique native et appels natifs acceptés ;
aucun état ne certifie une interaction réelle. Le démonstrateur contient 11 IDs
réels du CSV mais reste à valider en jeu. Voir [DEV9-CATALOG.md](DEV9-CATALOG.md).

Dev8 ajoute le catalogue natif exportable et la pièce témoin de 11 objets réels
sélectionnés dans le registre, en réutilisant le placement blueprint original.
Les tests de fichiers et les contrôles sur monde simulé sont automatisés ; la
capture réelle, les interactions et la consommation restent à valider en jeu.
Le château n'est pas modifié ; sa décoration séparée attend cette validation.
Voir [DEV8-NATIVE-OBJECTS.md](DEV8-NATIVE-OBJECTS.md) et
[DEV8-VALIDATION.md](DEV8-VALIDATION.md). Les mentions de refus systématique des
objets à logique dans les bilans dev3–dev7 sont historiques : dev8 propose une
activation expérimentale explicite du mécanisme original, sans copie des liens.

Les fonctionnalités ci-dessous sont du code dans la DLL Windows. « Implémenté »
ne signifie pas « validé dans Enshrouded » : aucun monde n'a été chargé par l'agent.
Le retour utilisateur de dev5 confirme les trois modules (312 cellules), les
bâtiments reliés/passages, l'alignement, les matériaux et les limites d'autels
dans les scénarios joués. La recette dev6 reste nécessaire pour les nouveaux cas.

Le premier test utilisateur de dev3 confirme le chargement de la DLL, l'interface,
Vulkan et les manifests ; les actions ne démarraient pas. Dev4 corrige les refus
silencieux et sécurise la fermeture de la fenêtre avant l'exécution. Voir
[DEV4-ACTIONS-FIX.md](DEV4-ACTIONS-FIX.md) et [DEV4-VALIDATION.md](DEV4-VALIDATION.md).
Le test utilisateur de dev4 confirme que les commandes arrivent au moteur, puis
échouent dès la résolution du matériau, avant tout placement. Dev5 corrige les
métadonnées des exemples et unifie la résolution avec les blueprints individuels,
en conservant la protection contre les matériaux ambigus et les limites natives.
Voir [DEV5-MATERIALS-FIX.md](DEV5-MATERIALS-FIX.md) et
[DEV5-VALIDATION.md](DEV5-VALIDATION.md).
Le retour dev5 décrit ci-dessus complète ce bilan historique. Les escaliers,
objets et le paiement normal ne sont pas validés par ce retour.
Dev6 ajoute une fabrique de projet multiétages native et des diagnostics plus
complets ; **leur validation dans Enshrouded reste à faire**. Voir
[DEV6-AUDIT.md](DEV6-AUDIT.md), [ARCHITECTURE-ROADMAP.md](ARCHITECTURE-ROADMAP.md),
[DEV6-MULTISTOREY.md](DEV6-MULTISTOREY.md) et [DEV6-VALIDATION.md](DEV6-VALIDATION.md).

Dev7 ajoute une alternative **entièrement en voxels**, ses soutiens et ses
dégagements persistants, avec un exemple de trois niveaux sans pièces natives.
Seuls les cubes de 0,5 m sont utilisés ; une montée plus fine est refusée et
la franchissabilité reste à tester. Voir [DEV7-VOXEL-STAIRS.md](DEV7-VOXEL-STAIRS.md)
et [DEV7-VALIDATION.md](DEV7-VALIDATION.md). Les portes interactives et les variantes
inclinées/demi-blocs ne sont pas ajoutées par cette livraison.

Dev3 est préparée pour cette campagne, sans installation automatique. Les objets
et pièces natifs automatiques sont **expérimentaux et désactivés par défaut**.
Home → Options contient maintenant huit sections et des accès à la fenêtre native.
L'audit et les résultats sont dans [DEV3-AUDIT.md](DEV3-AUDIT.md) et
[DEV3-VALIDATION.md](DEV3-VALIDATION.md) ; la recette de douze étapes est dans
[EXTENDED-TESTING.md](EXTENDED-TESTING.md).

| Demande | État du code |
|---|---|
| Audit, branche et commits, compilation Windows | Réalisés ; architecture clang/lld freestanding et licence MIT conservées |
| Composites, ancrage et rotation globaux, grilles compatibles | Implémentés ; calculs entiers et tests de coordonnées |
| Une confirmation et lots progressifs | Implémentés ; deux passes et arrêt sur erreur, compte rendu par lot |
| Chargement des anciens BTBP | Conservé ; protections supplémentaires contre fichiers invalides |
| Grille virtuelle et construction dans le vide | Implémentées ; altitude, pas 0,5/1 m, origine, axes, huit niveaux persistants |
| Aperçu natif | Implémenté : cellules/volumes, module par module, tranche d'altitude, limites des pièces, repères d'objets |
| Collisions et couverture | Contrôles de voxels, volumes de pièces, dégagement et zones natives ; premier défaut marqué après Vérifier |
| Matériaux | Bibliothèque du registre, remplacement global/module/catégorie, prix natifs et affichage du stock connu |
| Objets composites | Placement natif des objets compatibles, position/quaternion conservés ; filtrage des objets spéciaux et logiques |
| Créatif étendu | Option de gratuité pour objets compatibles dans la voie native ; bibliothèque pour placer les compatibles sans possession |
| Autels phase A | Lecture des zones/sous-zones, analyse 3D progressive, estimation et 64 propositions modifiables |
| Autels phase B | Repli prévu par la mission : assistance uniquement, aucun placement automatique |
| Murs | Dimensions, épaisseur, matériau, ouverture, encadrement secondaire |
| Sols | Surface, épaisseur, bordures, trémie |
| Toitures | Réutilisation des profils gable/hip/gambrel/mansard/shed/conique/dôme ; fonctions Area originales conservées |
| Tours | Carrées/rondes, niveaux, fenêtres, réservations de planchers, créneaux et plateforme supérieure |
| Escaliers | Pièces natives choisies dans le registre, volées droites/tournantes/impériales et spirale carrée avec paliers |
| Dégagement des escaliers | Réservations dans les planchers du brouillon, contrôle du volume natif et de 2 m d'air au-dessus |
| Mobilier associé au projet | Ajout de compatibles depuis la bibliothèque avec position, rotation et module d'appartenance |
| Sauvegarde du projet conçu | Export en nouveaux BTBP et manifeste publié en dernier ; pas d'écrasement |
| Erreurs de génération | Restauration du brouillon en mémoire, y compris les paliers |
| Sauvegardes personnelles | Aucun accès ; pas de lecteur/éditeur `.ksd` ni d'installation automatique |
| Consolidation dev3 | Rotations complètes des escaliers/paliers, comptages idempotents, enveloppes d'objets, paramètres persistés et coûts d'objets seuls |
| Garde ABI native | ItemInfo, EquipmentSetup et les 22 drapeaux réfléchis contrôlés ; changement = placement natif désactivé |
| Lots/diagnostic | 128 voxels par défaut, réduction après contrôle lent ; 1 pièce ou 4 objets par lot ; ID/empreinte/rapport de l'opération |
| Campagne d'exemples | Trois bâtiments reliés, trois niveaux, grille dans le vide et village de 75 648 cellules ; exemple natif généré depuis le vrai registre |
| Démonstration dev6 | Fabrique paramétrique de trois niveaux, 16 modules, vraies volées natives, paliers, trémies, embrasures et toiture ; exportable, praticabilité à tester |
| Diagnostics dev6 | Dimensions après rotation, validation/devis par contexte, IDs indisponibles, erreurs spatiales et instantané détaillé d'opération ; tests automatisés, recette UI en attente |

## Limites qui empêchent de déclarer toute la mission terminée

### Événements des portes, fenêtres et objets spéciaux

`bpobjects.h` explique le problème déjà présent dans l'amont : des objets portant
le drapeau natif `IsLogicEntity` demandent un `LogicEntityPlacedEvent`. Le contexte
du système Blueprint ne possède pas son writer. L'ancien placement classique
utilise un writer privé et abandonne l'événement ; il ne faut pas en déduire un
comportement interactif complet. Le nouveau système composite refuse ces objets.

La réflexion de l'exécutable local confirme `ItemInfo.flags` à `0x5d8`,
`IsLogicEntity` au bit 19, `IsFactoryEntity` au bit 20, ainsi que les drapeaux
HasKnowledgeUnlocks, HasPIDE, IsNpc et IsAnimal. Ceux-ci sont traités séparément,
avec le champ natif requiresAltarSlot déjà exclu par le mod.
Dev3 exclut aussi les plans/outils spéciaux, appâts, récompenses Twitch, objets
de commerce et bits inconnus. La procédure native reste à tester pour chaque
compatible : l'existence de son ID n'est pas considérée comme validation d'usage.

Écrire dans le writer d'un autre système serait une hypothèse dangereuse sur
l'ordonnancement du moteur. Il reste à identifier une voie de livraison native
dont la propriété et le cycle de vie sont vérifiés, puis à tester ouvertures,
fermetures, câblage et rechargement d'un monde de test. Le placement manuel normal
conserve son propre contexte ; les objets logiques ordinaires peuvent y rester
gratuits si les autres drapeaux spéciaux ne les excluent pas.

Pour faciliter cette recherche, `NativeObjectTrace=1` dans la section `[Extended]`
de `BuildingTools-Extended.ini` active une trace bornée des vrais placements
manuels : item, résultat, index d'événement et variation du buffer. Elle lit le
writer de son propriétaire ; elle ne réutilise ce writer ni n'injecte d'événement.
L'option est désactivée par défaut et n'a pas été exécutée dans un jeu ici.
Elle est désormais réglable dans Diagnostics, sans édition manuelle de l'INI.

### Autels et annulation totale

Le code sait lire les zones et appeler BaseAt. Il ne sait pas encore garantir
le quota de création, le niveau disponible, les contraintes de terrain et les
événements de progression lors de la création d'un autel. La mission autorise
explicitement ce repli : conserver l'assistance plutôt que placer des autels
sur la base d'hypothèses. Aucun autel existant n'est supprimé.

Le jeu limite les enregistrements d'annulation ; une grande action conserve donc
une annulation partielle. Le journal ne constitue pas une transaction de sauvegarde.
Sans API native de rollback vérifiée, un remboursement ou une suppression massive
automatique risquerait d'effacer des modifications ultérieures ou de dupliquer
des ressources. Un lot payé mais incomplet reste possible et est signalé.
Le rapport dev3 conserve l'identité, l'empreinte du plan et le préfixe acquitté.
Il facilite le diagnostic, mais ne prouve pas l'état actuel du monde. Un retrait
automatique ne peut pas distinguer un bloc ajouté puis modifié d'une construction
ultérieure identique ; aucune reprise/suppression aveugle n'est activée.

### Aperçu, collisions d'entités et géométrie

Les meubles sont représentés par des repères, les pièces par leurs dimensions
et les grands projets par des volumes. Les matériaux portent leurs vrais noms
et coûts, sans rendu de leurs textures. Il n'existe pas encore de voie vérifiée
de création de fantômes du modèle natif sans entité persistante. Aucun objet
temporaire n'est donc créé dans le monde pour simuler un aperçu.

La détection d'intersection avec tous les meubles/entités existants n'est pas
implémentée. Le contrôle actuel couvre les voxels et les volumes de construction,
pas toute la physique du moteur. Une toiture peut être composée avec les outils
existants, mais les jonctions automatiques de plusieurs ailes et la génération
d'un vrai escalier hélicoïdal arrondi ne sont pas livrées.

Les escaliers utilisent réellement la pièce choisie. Les dimensions de son
volume ne prouvent pas son sens de montée ou la hauteur exacte de son palier.
Rotation et montée sont réglables, et la circulation doit être vérifiée en jeu.
Une fondation sélectionnée à la place d'un escalier resterait une fondation.

## Contrôles complémentaires

Pour dev3, suivre en priorité [EXTENDED-TESTING.md](EXTENDED-TESTING.md) et
[DEV3-EXAMPLES.md](DEV3-EXAMPLES.md). Les points ci-dessous restent pertinents,
avec l'activation expérimentale explicite pour les objets/pièces automatiques.

1. **Grille** : activer Free Placement (F10 par défaut), puis dans Home activer
   le plan virtuel et prendre l'altitude de la caméra comme référence. Ajuster Y.
   Vérifier les axes, les pas, le placement
   sans support, la conservation de l'origine et les huit niveaux après redémarrage.
2. **Contrôle sans construction** : charger les trois modules, fixer l'ancrage,
   lancer Vérifier, attendre le retour à l'aperçu. Le journal doit contenir INSPECTED
   et aucune entrée BATCH_BEGIN. Déplacer sur un bloc puis contrôler le repère rouge.
3. **Matériaux** : choisir un vrai matériau dans la bibliothèque, reprendre le
   projet en mémoire et remplacer dans un seul module, puis globalement. Vérifier
   le prix et l'absence de mutation du monde avant Construire.
4. **Autels** : analyser un projet dépassant une zone, en X/Z puis en Y. Vérifier
   zones or, propositions vertes et déplacement d'une proposition. Contrôler le
   refus de construire hors couverture ; aucun autel ne doit apparaître.
5. **Composants** : créer un mur avec ouverture et encadrement, un sol avec
   trémie, une tour ronde à trois niveaux et une toiture. Changer les filtres
   d'aperçu et vérifier qu'ils n'excluent pas des modules de la construction.
6. **Escaliers** : choisir le véritable élément natif dans la bibliothèque,
   contrôler son orientation +Z et les dimensions affichées, puis générer une
   petite volée avec ses réservations. Après placement, monter sans sauter.
   Tester quart tournant, palier impérial et spirale carrée séparément.
7. **Objets** : avec l'option créative étendue activée, choisir un compatible
   que l'inventaire ne contient pas, fixer sa position/rotation, vérifier et
   placer. Contrôler son modèle, usage, éclairage éventuel et rechargement du monde.
   Vérifier le refus des objets spéciaux/logiques dans un composite.
8. **Export** : enregistrer dans de nouveaux fichiers, fermer la fenêtre,
   recharger le manifeste exporté et comparer positions, matériaux, objets et
   pièces natives. Tester les fichiers invalides dans une copie de l'exemple.
9. **Non-régression** : modes Area/Replace, Free Placement original, blueprints
   classiques, objets manuels et annulation native, sur le monde de test.

Les valeurs des contrôles architecturaux sont en mètres ; les limites de taille
sont détaillées dans `BTPROJECT.md`. Enregistrer le projet avant de quitter le jeu :
le brouillon peut être repris entre fenêtres, mais seule la grille est persistée
automatiquement. Aucun des essais ci-dessus n'a été effectué dans Enshrouded ici.
