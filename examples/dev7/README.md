# Trois niveaux avec escaliers entièrement en voxels

Charger `Dev7_Batiment_trois_niveaux_voxels.btproject` depuis Projets composites.
L'exemple contient 8 068 cellules, 58 tuiles BTBP et 22 dégagements, sans aucune
pièce native ni objet. L'option native expérimentale peut rester désactivée.

Surfaces des planchers : 0/4/8 m ; deux escaliers droits de huit contremarches,
montée 0,5 m, largeur 2 m, profondeur 1 m, paliers 2 m et soutiens pleins. Les
trémies et 2 m d'air au-dessus des surfaces sont conservés dans le manifeste.
Une contremarche de 0,5 m n'est pas supposée franchissable sans saut : tester dans
le jeu. Les embrasures ne contiennent pas de portes interactives.

Matériau : `0x7ad0559c` / `0x9d`, issu de l'exemple confirmé en dev5. Les coûts
restent inconnus dans les fichiers ; le moteur les recalcule. Aucun fichier de
joueur n'est recopié ou modifié. Dev7 est nécessaire pour lire les dégagements.

Reproduction avec la même fabrique et le même exporteur que la DLL, adaptés aux
fichiers hôtes (WSL/Linux, C++17) :

```sh
c++ -std=c++17 -O1 tools/make_voxel_example.cpp -o build/make_voxel_example
./build/make_voxel_example --material-from examples/three-modules/hall.btbp --output NEW-DIRECTORY
./build/make_voxel_example --material-from examples/three-modules/hall.btbp --verify examples/dev7
```

La création refuse un dossier existant. La vérification lit l'exemple sans le
modifier et génère sa comparaison dans un nouveau dossier de build.
Le protocole détaillé est dans `docs/DEV7-VOXEL-STAIRS.md`.
