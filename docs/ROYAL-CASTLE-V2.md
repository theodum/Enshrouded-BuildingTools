# Forteresse Royale V2 — grands intérieurs

Le joueur a construit et parcouru la V1 : son extérieur est apprécié, mais ses
tours, escaliers et passages intérieurs sont trop serrés. La V2 conserve la
composition du château et redessine les intérieurs pièce par pièce, en cubes
de **0,5 m**. La DLL **0.34.0-dev7** reste inchangée.

Le projet à sélectionner est `Forteresse_Royale_Spacieuse_V2.btproject`.
Ses fichiers sont dans `examples/royal-castle-v2` ; l'archive d'import est
`dist/Forteresse_Royale_Spacieuse_V2-dev7.zip`. Les noms `FR2_*.btbp` permettent
de conserver la V1 et ses dépendances pour comparaison.

Les tours font 16 m de diamètre et le donjon 18 × 18 m. Les niveaux principaux
sont à 0 / 5 / 10 m. Les volées secondaires font 2,5 m utiles entre les
garde-corps, la volée impériale centrale 5 m, les paliers 3 m. Les passages des
ailes ont des allées protégées de 3 m. Les cages réservent 3,5 m de hauteur libre.

Les soutiens pleins superposés ont été remplacés par des bandes de pierre
de 1 m, des noyaux porteurs et des garde-corps extérieurs aux marches. Aucun
soutien n'est découpé pour accommoder un autre escalier. Le contrôle refuse
les chevauchements entre composants même de matériau identique et interdit
le placement du mobilier dans les cages ou les axes de circulation.

La vérification de confort ajoute un graphe des sols avec une emprise de
**1,5 × 1,5 m**, raccordé par tous les paliers des escaliers vérifiés. Ce contrôle
a notamment décelé un virage trop étroit devant le donjon ; la terrasse a été
avancée pour le corriger. Cette emprise est une vérification géométrique,
elle ne reproduit pas la capsule du personnage ni son franchissement des marches.

Les rapports `Architecture.md`, `Validation.md`, `Validation.json` et
`Pieces.csv` dans le dossier V2 décrivent chaque pièce, ses surfaces, ses
aménagements et ses accès. `Plans.svg` montre les six niveaux en coupe. Les
PNG représentent les voxels réels ; les coupes masquent certaines parois et
planchers pour rendre les circulations visibles. Les couleurs représentent
les rôles de matériaux, pas les textures réelles du jeu.

## Reproduire

Depuis la racine du dépôt, sous WSL Ubuntu 24.04 :

```sh
c++ -std=c++17 -O2 tools/castle_host.cpp -o build/castle_host
python3 tools/make_royal_castle_v2.py --output build/ma-forteresse-v2
python3 tools/make_royal_castle_v2.py --output build/ma-variante-v2 --palette tools/castle-palette.example.json
bash tools/test_royal_castle.sh
```

La sortie doit être un nouveau dossier. `--render` ajoute les images avec Pillow
installé dans `build/castle-render-libs`. `--zip dist/ma-variante.zip` ajoute une
nouvelle archive d'import. Les validations et la génération ordinaire n'utilisent
aucune dépendance Python externe. Sous Windows, l'outil appelle le même hôte C++
dans WSL ; la sortie doit rester dans le dépôt.

Les tests couvrent la V1 conservée et la V2, les refus d'empiètement, les cages
vides protégées, le mobilier, les allées de 3 m, les marches manquantes, les
plafonds et le cas de terrasse trop étroite. Les fichiers distribués sont relus
par les véritables décodeur et planificateur du mod, avec rotations, lots et
interruptions de la machine d'état.

Le placement réel se fait selon `examples/royal-castle-v2/Placement.md`, sur un
site plus vaste et couvert par les autels. La Pierre des Anciens reste la
substitution explicite disponible. Les marches de 0,5 m et le confort réel de
la V2 doivent encore être testés dans Enshrouded ; aucun test automatique ne
certifie la marche sans saut, la stabilité ou les collisions du monde. Aucun
script ne modifie de sauvegarde et aucune reprise automatique n'est inventée.

Les résultats exécutés et le reçu d'installation sont résumés dans le
[rapport de livraison V2](ROYAL-CASTLE-V2-VALIDATION.md).
