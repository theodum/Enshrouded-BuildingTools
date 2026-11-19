# Premier château complet — Couronne des Anciens

Suite au test en jeu du joueur, la V1 ci-dessous est conservée comme référence :
son extérieur est apprécié, ses intérieurs sont trop serrés. La
[V2 plus spacieuse](ROYAL-CASTLE-V2.md) répond à ce retour avec des cages et
des pièces redessinées. Les rapports de livraison V1 restent historiques.

Projet d'architecture pour **Building Tools Extended 0.34.0-dev7**. La génération
multiétage et les escaliers de la démonstration dev7 ont été confirmés en jeu par
le joueur. Ce château doit encore passer sa propre recette dans Enshrouded.
Le moteur dev7 suffit : aucune DLL ni limite de sécurité n'a été modifiée.

## Livraison

`dist/Forteresse_Royale_Demo-dev7.zip` contient un seul projet sélectionnable,
`Forteresse_Royale_Demo.btproject`, et toutes ses dépendances dans
`BuildingTools-Projects`. Les rapports et aperçus sont dans
`Forteresse-Royale-Guide`. Il suffit d'extraire ces dossiers à côté du jeu, en
conservant la DLL dev7 déjà installée. Le script ne fait aucune installation.

Les fichiers sources prêts à importer sont dans `examples/royal-castle`.
Lire `Architecture.md`, `Validation.md`, `Placement.md` et `Validation.json`
dans ce dossier pour les dimensions, compteurs, substitutions et contrôles
exacts. `Plans.svg` est une section réelle des voxels pour les six altitudes.
Les quatre PNG montrent l'extérieur et des coupes du hall et de la salle du
trône. Les couleurs indiquent les rôles architecturaux ; les textures du jeu
ne sont pas reproduites. Les coupes masquent des murs et planchers à l'affichage.

La composition privilégie un corps royal central, deux ailes, une cour ouverte,
quatre tours octogonales intégrées et un donjon. Les niveaux principaux sont
à 0 / 4,5 / 9 m ; les chemins de ronde à 13,5 m, les tours à 18 m, le sommet du
donjon à 22,5 m. Le hall à double hauteur possède un véritable escalier impérial
à deux branches, les ailes deux L et un U, les tours et le donjon des U.

## Reproduire et tester

Depuis la racine du dépôt, sous WSL Ubuntu 24.04 :

```sh
c++ -std=c++17 -O2 tools/castle_host.cpp -o build/castle_host
python3 tools/make_royal_castle.py --output build/mon-chateau
python3 tools/make_royal_castle.py --output build/variante --palette tools/castle-palette.example.json
bash tools/test_royal_castle.sh
```

Le dossier de sortie doit être nouveau. Le manifeste n'est publié qu'après les
validations géométriques et la relecture par le véritable parseur et planificateur
du mod. Un échec d'export peut laisser des dépendances dans un dossier neuf,
sans manifeste chargeable. Les fichiers existants ne sont jamais remplacés.
Sous Windows, Python appelle le même exécutable hôte dans WSL. Les sorties
Windows doivent rester dans le dépôt pour que les deux environnements y accèdent.

Pour les illustrations, installer Pillow dans `build/castle-render-libs` et
ajouter `--render`. Pour une archive d'import neuve, ajouter
`--zip dist/ma-variante.zip`. Le générateur et les validations ordinaires n'ont
aucune dépendance Python externe. Les tests C++ existants sont recompilés en
outils hôtes ; le script ne recompile pas le mod. `ENSHROUDED_EXE` active aussi
les contrôles de signatures et d'ABI en lecture seule sur l'exécutable local.

## Garanties et limites

- Les profils d'escalier viennent de `voxel::prepare` / `voxel::emit` en production.
  Ils ont neuf contremarches de 0,5 m, un giron de 0,5 m, une largeur de branche
  de 1,5 m et 2,5 m d'air réservé. Le corps central est deux fois plus large.
- Les planchers et partitions suivent un repère global unique ; l'union des
  enveloppes de tours et d'ailes élimine les murs cachés aux jonctions.
- Les soutiens massifs sont ajourés sous les tabliers lorsqu'ils croiseraient
  le dégagement d'un niveau précédent. Aucun tablier, marche ou palier n'est omis.
- Le graphe est dérivé des voxels finaux et autorise seulement les sols prévus,
  marches, paliers, estrade et rebord bas du bassin. Corniches et appuis de
  fenêtres ne peuvent servir de détour pour masquer une pièce inaccessible.
- Les contrôles Python refusent plafond, porte bouchée, marche manquante,
  pièce inaccessible et chevauchement de matériaux incompatibles. Les tests
  vérifient aussi une palette à quatre rôles distincts, la reproductibilité des
  octets livrés et la préservation d'un dossier existant.
- La relecture C++ vérifie chaque BTBP, les doublons, toutes les réservations,
  les quatre rotations, les lots de 1/32/128/256 cellules et les interruptions
  de la machine d'état. Elle n'émule ni le jeu, ni un paiement, ni une écriture.
- Le Bloc de pierre des Anciens est le seul matériau identifié avec certitude
  dans les preuves locales disponibles. Il remplace explicitement les autres
  rôles. Pour personnaliser, exporter un petit BTBP ne contenant que le matériau
  voulu et renseigner son chemin/nom dans la configuration de palette ; aucun
  ID n'est déduit du nom. La résolution native reste obligatoire dans le jeu.
- Toutes les marches restent des cubes de 0,5 m : leurs connexions géométriques
  ne certifient pas un franchissement sans saut. La physique, la stabilité,
  le terrain, les stocks et la couverture réelle des autels restent à vérifier.
- Le journal de dev7 n'est pas une reprise automatique de chantier. Les lots
  déjà acquittés peuvent rester après interruption ; aucun rollback supplémentaire
  n'est fourni. L'archive ne modifie pas les sauvegardes et n'en contient aucune.

Les anciens exemples et la DLL dev7 restent inchangés. La suite de régression
inclut les projets de 312, 800, 8 068 et 75 648 cellules, ainsi que ce château.
La recette en jeu complète est fournie dans `examples/royal-castle/Placement.md`.
Les résultats détaillés des suites exécutées sont consignés dans
[le rapport de livraison](ROYAL-CASTLE-VALIDATION.md).
