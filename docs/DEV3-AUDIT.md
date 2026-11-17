# Consolidation dev3 — audit de dev1/dev2

Point de départ : `c64874c`, branche `feature/composite-projects`, arbre propre.
Audit effectué avant les modifications dev3 ; aucun lancement du jeu ni accès aux
sauvegardes. Les contrôles binaires restent distincts d'une validation en jeu.

| Chemin examiné | Constat et action dev3 |
|---|---|
| Home → Options → extensions | Options présentes mais accès aux projets indirect ; ajouter des entrées et une page de diagnostics |
| BTBP / manifeste / transformations | Format v1 conservé ; renforcer les tests 0,5/1/2/4 m et les rotations communes blocs/objets/pièces |
| Générateur d'escaliers | La rotation de départ tournait la pièce mais pas le tracé ni les paliers ; corriger tout l'assemblage et tester les quatre rotations |
| Plan et matériaux | Recalcul répété des compteurs non idempotent ; propriété des doublons et catégorie à conserver ensemble |
| Objets seuls | Coûts cachés quand aucun matériau voxel ; enveloppe globale incomplète pour les objets hors du module |
| Catalogue natif | Ajouter contrôles de table/indices et explications pour les catégories refusées |
| Pièces/objets automatiques | ABI contrôlée statiquement, comportement non testé ; activation expérimentale explicite avant toute écriture |
| Assistance aux autels | Dev2 analyse uniquement les voxels ; inclure les repères des pièces/objets et signaler la nature estimative |
| Interruptions | Arrêt des lots et refus de commit tardif existants ; conserver une identité d'opération et fournir un rapport sans suppression/reprise aveugle |
| Performances | Réduire et rendre réglable le lot voxel ; borner séparément les sondes de pièces et les entités |
| Configuration | Plusieurs affichages non persistés ; tester chargement, valeurs invalides, aller-retour et préservation des clés étrangères |
| Vulkan | Tampon de lignes borné et segment par image protégés par fence ; contrôler shaders et cas de plans très étendus |
| Export | CREATE_NEW et publication du manifeste en dernier ; compléter les cas limites et vérifier les archives finales |

## Corrections réalisées

Les actions du tableau sont implémentées dans dev3. Le premier commit de
consolidation est `e0e3a1c`. Un complément contrôle les dimensions des objets du
catalogue avant modification du brouillon et signale les échecs d'écriture de
l'INI, sans prétendre que les réglages sont persistés lorsque l'API a refusé.
Le contrôle croisé Windows/WSL a également révélé une différence CRLF/LF dans les
fixtures : générateurs et attributs Git imposent maintenant LF pour les manifests,
et les trois tests Python passent dans les deux environnements.

Les sondes d'une pièce sont limitées à 4 096 voxels, dégagement inclus ; celles
d'un objet à 512. Le lot voxel démarre à 128 par défaut, baisse après un contrôle
de plus de 50 ms et ne change jamais l'ordre ou les coordonnées. Une pièce native
reste une opération indivisible ; ce seuil n'est pas une garantie de temps réel.
Le chargement/tri du plan reste synchrone et borné à un million de cellules.

L'ABI est contrôlée par le code utilisé à la fois dans la DLL et dans `test_core` :
descripteurs réfléchis ItemInfo (`0x1918790`, taille `0x828`), EquipmentSetup
(`0x19cfc30`, taille `0x440`) et ItemFlag (`0x1989f80`, 22 valeurs). Les pointeurs
doivent rester dans l'image lisible, les membres/tailles et toutes les valeurs
doivent correspondre. Les tests modifient une copie du descripteur et vérifient
le refus. Ces RVA sont propres au binaire local ; déplacement ou changement
désactive l'automatisation native au lieu de deviner une nouvelle structure.

Les contrôles de prix/commit de dev2 restent nécessaires. Un identifiant réel ne
suffit pas : les nouveaux placements demandent des dimensions valides, la bonne
catégorie, les drapeaux autorisés, la couverture et le paiement natif. Les objets
PIDE/connaissances/logiques, spéciaux, PNJ/animaux/appâts, usines, récompenses et
commerce sont séparés ; les bits inconnus sont refusés. Les outils originaux et
leur ancienne voie BTBP ne sont pas réécrits.

Les zones des autels restent une lecture de BaseAt et des volumes observés. Les
API nécessaires au quota/niveau/création/progression ne sont pas établies par cet
audit. L'assistance manuelle est conservée, conformément au repli autorisé.

Le journal donne ID, empreinte diagnostique et résultats de lots ; le rapport
donne le préfixe acquitté, pas une propriété exclusive des voxels. Les mutations
ultérieures, l'absence de version/identité par voxel et les lots éventuellement
payés mais incomplets empêchent un retrait/rejeu automatique sûr. L'étude conduit
à livrer le diagnostic et l'annulation native partielle, pas un faux rollback.

Les verrous de prévisualisation, la publication GPU par image et le budget de
4 096 lignes sont conservés ; les deux modules SPIR-V de lignes sont validés par
`spirv-val`. Aucun FPS, dessin Vulkan, clic natif, escalier ou objet n'a été testé
dans Enshrouded. Le guide personnel couvre les interactions et le rechargement.

Résultats détaillés dans [DEV3-VALIDATION.md](DEV3-VALIDATION.md) et protocole dans
[EXTENDED-TESTING.md](EXTENDED-TESTING.md).
