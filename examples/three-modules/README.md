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

Matériau des exemples à partir de dev5 : bloc natif `0x7ad0559c`, octet `0x9d`.
Ce couple a été confirmé par le registre du jeu dans `BuildingTools.log` du
19 novembre 2026, à 09:52:53.717. Aucun nom de matériau n'est supposé.

Les anciens exemples utilisaient `0x25a43c2f` / `0x9d`, copiés d'un BTBP local.
Cette provenance ne prouvait pas que l'item était un bloc natif valide : c'était
une erreur dans les métadonnées livrées. Dev5 change seulement les quatre octets
de l'ID de matériau dans les dix BTBP d'exemple ; géométrie et coûts enregistrés
restent identiques. Aucun blueprint personnel n'est modifié. Le registre et les
coûts natifs seront encore vérifiés au placement ; le test en jeu reste nécessaire.

Les fixtures sont reproductibles avec :

```powershell
python tools/make_project_example.py --material-from CHEMIN_VERS_UN_BLUEPRINT.btbp --output build/nouvel-exemple
```

Le répertoire de sortie doit être nouveau. Le générateur lit seulement le
blueprint source et refuse d'écraser un répertoire existant.

Installation et recette : [../../docs/EXTENDED-TESTING.md](../../docs/EXTENDED-TESTING.md).
