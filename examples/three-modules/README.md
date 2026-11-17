# Exemple composite à trois modules

`Trois_modules_test.btproject` référence trois véritables fichiers BTBP v1,
pour **312 cellules natives de 0,5 m**, sans objets. Le hall utilise une grille
source de 1 m, les deux autres une grille de 0,5 m. Avec la limite interne de
256 cellules par lot, le test traverse plusieurs lots après une seule validation.

| Module | Coin inférieur relatif (X,Y,Z), mètres | Rotation locale | Dimensions après rotation, mètres |
|---|---|---|---|
| hall.btbp | (0,0,0) | 0° | 6 × 3 × 4 |
| tower.btbp | (8,1,0) | 90° | 1 × 1,5 × 1 |
| bridge.btbp | (2,2,5) | 270° | 1 × 0,5 × 3 |

Le hall a un sol et un mur arrière ; la tour est un petit pilier de repérage,
et le pont une plateforme en hauteur. Ce sont des témoins d'alignement simples.

Matériau : item `0x25a43c2f`, octet sauvegardé `0x9d`, **lus sans modification**
dans le blueprint existant `Tour_BTBP_Source_Officiel_V4.btbp` du dossier local
BuildingTools-Blueprints. Aucun identifiant inventé et aucun fichier personnel
recopié. Le mod doit encore confirmer cet item dans le registre du jeu lors du
test ; aucun nom de matériau n'est supposé.

Les fixtures sont reproductibles avec :

```powershell
python tools/make_project_example.py --material-from CHEMIN_VERS_UN_BLUEPRINT.btbp --output build/nouvel-exemple
```

Le répertoire de sortie doit être nouveau. Le générateur lit seulement le
blueprint source et refuse d'écraser un répertoire existant.

Installation et recette : [../../docs/EXTENDED-TESTING.md](../../docs/EXTENDED-TESTING.md).
