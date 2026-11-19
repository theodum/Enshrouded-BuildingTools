# Dev6 — démonstration multiétages et recette en jeu

Cette procédure prépare un projet avec **trois niveaux habitables**, leurs
planchers, des escaliers natifs, paliers, trémies, murs, embrasures, fenêtres et
toiture. Elle ne prétend pas que le modèle choisi est déjà praticable : le test
à pied fait partie de la validation. Aucun monde de joueur n'a été modifié pour
préparer la livraison.

## 1. Installer et rejouer le socle validé

1. Fermer le jeu, conserver votre version précédente et installer le ZIP
   **Standalone 0.34.0-dev6** à côté de `enshrouded.exe`, avec ses langues, guides
   et exemples. Garder une seule copie de Building Tools chargée.
2. Ouvrir un monde **local de test**. Vérifier la bannière dev6 et le journal.
   Une copie externe préalable de ce monde reste à votre charge ; la DLL ne
   crée aucune sauvegarde et les scripts de livraison n'accèdent pas aux mondes.
3. Ouvrir une fois le sélecteur natif dans une zone d'autel, prendre le cube
   **de 0,5 m**, puis ouvrir Home → Options → Projets composites.
4. Rejouer `Trois_modules_test` : vérifier puis construire, attendre 312 cellules
   acquittées, contrôler le matériau et l'alignement. Rejouer les trois bâtiments
   reliés et leurs passages. Ces scénarios étaient validés en dev5 ; ils doivent
   rester corrects avec dev6. Arrêter la recette en cas de régression.

## 2. Calibrer une volée du vrai modèle

5. Bibliothèque native → choisir un matériau de construction, puis Pièces natives
   → choisir **un escalier reconnu visuellement**, pas une fondation quelconque.
   Noter son nom et son ID. Tous les éléments de construction de cette catégorie
   ne sont pas des escaliers ; le mod ne déduit pas leur sémantique d'une boîte.
6. Composants architecturaux → Escaliers natifs. Régler l'**orientation du modèle**
   pour que sa montée locale suive +Z. La **rotation du projet** est distincte et
   tourne l'assemblage complet. Lire largeur/hauteur/longueur affichées.
7. Préparer une seule volée droite, dans une zone de test libre. Activer
   explicitement Placement natif expérimental dans Options, vérifier puis
   construire. Monter et descendre à pied ; déterminer la différence d'altitude
   réelle entre ses surfaces basse et haute. La hauteur AABB peut inclure une
   rampe ou un socle : ce n'est pas automatiquement la montée réelle.
8. Renseigner **Montée réelle d'une volée** avec cette mesure, par pas de 0,5 m.
   La nouvelle démo refuse zéro et les dimensions incompatibles avec sa grille.
   Si le modèle ne convient pas, conserver le placement manuel ou en choisir un
   autre ; ne modifier aucune protection pour le faire accepter.

## 3. Préparer le bâtiment complet

9. Conserver/exporter le brouillon précédent s'il doit être gardé. Dans la même
   page Escaliers, cliquer **Préparer le bâtiment de trois étages avec escaliers
   natifs**. Cela remplace seulement le brouillon en mémoire. Le réglage Ajouter
   de l'ancien générateur ne s'applique pas à cette fabrique de démonstration.
   Elle utilise les IDs, l'orientation du modèle, la montée et la rotation globale ;
   son tracé droit et ses dimensions sont calculés, indépendamment des autres
   réglages de génération et de décalage.
10. Le mod utilise les vrais IDs sélectionnés. Il prépare trois planchers à
    surfaces Y=0/H/2H ; H est un multiple de la montée réelle, au moins 4 m.
    Les deux montées droites sont séparées par un palier et sortent sur un palier
    supérieur. Leur volume et 2 m d'air sont réservés dans les planchers. Des
    galeries latérales permettent de contourner les trémies.
11. Le brouillon comporte 16 modules et un nombre de pièces/cellules dépendant
    du modèle. Les mêmes matériau et ID natif sont utilisés pour structure et
    escaliers. **Les embrasures ne contiennent pas de portes interactives.** Les
    poser manuellement plus tard ; leur automatisation composite reste refusée.
12. Si la préparation échoue, lire le motif : sélection, montée, ABI, grille,
    budget ou dégagement. Le brouillon précédent doit rester intact. Une ABI
    incompatible laisse le placement natif désactivé, même avec l'opt-in.

## 4. Inspecter et valider avant construction

13. Dans l'aperçu, relever dimensions X/Y/Z, modules, cellules, pièces, matériaux
    et IDs indisponibles. Utiliser les tranches d'altitude pour voir les trois
    planchers, leurs trémies et les paliers. Tourner le projet : les dimensions
    X/Z et tous les éléments doivent suivre la rotation.
14. Autels → Analyser la couverture. Lire les limites minimum/maximum de la zone
    sélectionnée, les échantillons vérifiés et les zones non couvertes. L'assistance
    n'est pas une autorisation de poser des autels ; `BaseAt` reste le contrôle
    de construction. Garder la totalité du bâtiment et son dégagement couverts.
15. Cliquer **Vérifier sans construire**. Attendre la validation complète sans
    apparition de bloc. Un calcul partiel n'est pas un devis complet ; une erreur
    indique position/module/cellule/pièce ou identifiants et coûts concernés.
    Exporter le rapport d'opération dans Diagnostics.
16. Déplacer de 0,5 m, tourner ou changer le mode créatif : l'ancien résultat et
    son devis doivent être signalés absents/périmés pour ce nouveau contexte.
    Vérifier de nouveau à la position retenue. Même une validation réussie reste
    historique : Construire revérifie le monde.

## 5. Exporter, recharger et construire

17. **Enregistrer le projet**, noter le nom affiché, puis revenir à la liste et
    charger le `.btproject` exporté. Les BTBP et les IDs natifs sont écrits dans
    `BuildingTools-Projects`, sans écrasement. L'export peut découper les modules
    en tuiles : le nombre de modules peut changer, pas les cellules ni les pièces.
18. Refaire Vérifier puis Construire à une position libre. Attendre les journaux
    `VALIDATION_QUEUED`, `project validation RESULT ... valid=1`, `BUILD_STARTED`,
    les lots blocs/pièces et les acquittements. Ne relancer aucune opération
    après un timeout ou un lot partiellement payé sans inspection du monde.
19. Parcourir **sol → premier étage → deuxième étage**, puis redescendre sans
    saut, chute ni passage par une trémie vide. Inspecter chaque raccord, marche,
    palier, plafond, mur, ouverture et toiture. Vérifier le matériau des escaliers.
20. Refaire sur des emplacements distincts aux rotations 90°, 180°, 270°.
    Fermer/recharger le monde de test et refaire le parcours : aucune validation
    de persistance ne peut être déduite du seul acquittement d'un lot.
21. Refaire un petit cas en mode normal : stock suffisant puis insuffisant,
    consommation native observée, refus avant nouvelle écriture si stock absent.

## 6. Essais négatifs et relevé

| Essai | Résultat attendu, à constater |
|---|---|
| Forme tenue de 1 m | Refus explicite ; aucune écriture |
| Petit projet hors autel / à sa limite en Y | Refus avec coordonnées ; aucune couverture forcée |
| Bloc existant dans le volume visé | Collision explicite ; aucune suppression automatique |
| Placement natif expérimental désactivé | Refus du projet avec pièces, projets de blocs conservés |
| ID natif inconnu dans une **copie de test** du manifeste exporté | Chargement refusé avec ID/pièce/matériau ; original intact |
| Génération avec montée zéro / modèle incompatible | Erreur explicite et brouillon précédent conservé |
| Échap, perte de focus, changement de mode | Arrêt des lots futurs ; ce qui est déjà construit reste à inspecter |

Noter pour chaque essai : version du jeu/mod, ID et nom d'escalier, matériau,
orientation du modèle, montée, rotation globale, dimensions, cellules/pièces,
résultat d'inspection, paiement, marche, rechargement et erreur éventuelle.
Conserver `BuildingTools.log`, `BuildingTools-Projects.log` et les rapports
exportés. Utiliser des zones libres distinctes et ne pas confondre un résultat
attendu avec un résultat observé.

**État à la livraison : tests automatisés réussis selon le bilan ; comportement
dev6 en jeu à valider par l'utilisateur.**
Voir [DEV6-VALIDATION.md](DEV6-VALIDATION.md) et [DEV6-AUDIT.md](DEV6-AUDIT.md).
