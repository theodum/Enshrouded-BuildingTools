# Placement en jeu

1. Conserver Building Tools Extended **0.34.0-dev7** déjà testé. Aucune mise à jour
   de DLL n'est nécessaire. Jouer dans un monde local de test dont vous possédez
   une copie. Le générateur n'a touché ni installation ni sauvegarde.
2. Copier tous les `FR_*.btbp` et `Forteresse_Royale_Demo.btproject` directement dans
   `BuildingTools-Projects`, à côté de `enshrouded.exe`. Les rapports et images
   peuvent rester dans leur dossier. Le ZIP fourni possède cette arborescence.
3. Préparer au moins **47 × 45 m**
   horizontaux, et **27 m** verticaux libres et
   couverts. Sols à Y=-0,5 m et surface d'entrée à Y=0 relativement à l'ancrage.
   Placer la base du projet hors terrain et hors constructions existantes ;
   le moteur ne terrasse ni ne retire de blocs automatiquement.
4. Prendre le **cube de 0,5 m**, ouvrir une fois le sélecteur de matériaux dans
   la base, puis passer en Blueprint → Placer → Projets composites. Sélectionner
   uniquement `Forteresse_Royale_Demo`. Conserver le placement natif expérimental
   désactivé. Le créatif original peut servir au premier essai.
5. Vérifier **57,817 cellules**, 75 modules, 346
   dégagements, zéro pièce native et zéro objet. Le nom affiché est « Forteresse Royale — Couronne des Anciens ».
   Le pivot est au sol du corps central ; le portail regarde -Z à rotation 0°.
   Utiliser Placer ici et les réglages de hauteur/rotation pour tout le château.
6. Analyser les autels. Les bornes locales occupées sont
   `[-22.5, -0.5, -20.0]` à `[22.5, 24.5, 22.5]` m ; les réservations d'air
   portent le volume complet à **45 × 25.5 × 42.5 m** (X × Y × Z), jusqu'à
   Y=25 m au-dessus de l'ancrage. Une couverture réelle W×D×H permet au mieux
   ceil(X/W)×ceil(Z/D)×ceil(Y/H) boîtes théoriques, avant les contraintes natives,
   les recouvrements, quotas et terrain. Par exemple avec 40×40 m horizontaux
   et assez de hauteur : au moins quatre boîtes géométriques pour cette emprise.
   Ce n'est pas une promesse que quatre autels de votre niveau suffisent.
7. Cliquer **Vérifier sans construire**, attendre la fin et résoudre tout refus
   précis (couverture, collision, matériau, stock). Un aperçu visible ne suffit
   pas. Ne pas supprimer `spaces` ni désactiver les contrôles pour forcer le projet.
8. Après validation, cliquer **Construire une seule fois**. Garder la fenêtre
   active et le même outil. Le moteur refait ses contrôles, puis place tous les
   modules par lots. Avec un plafond 128 : 452
   lots ; le plafond adaptatif peut les rendre plus nombreux.
9. En cas d'Échap, perte de focus ou erreur, conserver les journaux et le rapport
   Diagnostics. Les lots écrits restent en place. Il n'existe pas de reprise
   automatique garantie ; ne pas rejouer aveuglément sur un chantier partiel.

## Recette à consigner après la construction

Parcourir parvis → hall → deux branches impériales → galerie → trône ; puis les
deux ailes, bibliothèque, trophées et appartements à 9 m. Vérifier les L/U dans
les deux sens, chaque palier et les trémies ; noter tout saut nécessaire. Visiter
chaque niveau des quatre tours, les chemins de ronde à 13,5 m, les terrasses
arrière, le donjon et son sommet à 22,5 m. Vérifier les parapets, les garde-corps,
les accès latéraux et l'absence de chute imposée. Inspecter aussi façades et toits.
Sauvegarder/recharger le monde de test puis refaire les parcours. Consigner
version jeu/mod, ancrage, rotation, créatif, durée, obstacles et journaux.

Une réussite des tests hors jeu prouve la géométrie et la compatibilité du fichier.
**Elle ne valide pas la circulation réelle du personnage.**
