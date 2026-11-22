# Dev8 — objets natifs dans les composites

Livraison `0.34.0-dev8`, préparée le 19 novembre 2026. **Aucune validation en jeu de ces ajouts n'est revendiquée.** Le château et ses fichiers restent inchangés. Aucun lancement du jeu, placement ou accès en écriture aux sauvegardes n'a été effectué.

## Ce qui est livré

- Un catalogue exportable depuis le registre réel : identifiant numérique, nom localisé (ou `null`), éventuelle troncature du nom, drapeaux natifs, bornes locales mesurées en mètres, disponibilité selon la politique du mod et contraintes connues. Le type exact de fixation du modèle n'est pas exposé : `requiredSupport: "not_exposed"`. La présence d'un identifiant dans le registre ne prouve pas une interaction correcte.
- Un générateur de pièce de **12 × 10 m**, avec voxels de 0,5 m, entrée de 2 m et passage central de 2 m. Il prépare **11 objets** : lit, table, deux chaises, deux flambeaux muraux, tableau, coffre, deux décorations sur la table et une décoration au sol. Tous les identifiants et volumes viennent des huit choix réels effectués dans la bibliothèque. Un modèle trop grand ou une sélection manquante provoque un refus, sans substitution.
- L'export `.btproject`/BTBP d'origine pour enregistrer ce brouillon. Les modules de mobilier sont des BTBP ne contenant aucun voxel ; ils utilisent le format existant. Le chargement conserve identifiant, position et quaternion.
- La même fonction `bp_place_object` que les blueprints individuels, appelée après les voxels et les pièces natives. Aucun second moteur de placement n'est ajouté.
- Des contrôles des volumes entre objets prévus, contre les voxels et pièces prévus, contre les objets existants détectés et contre les voxels du monde. Des contacts de support et la couverture 3D des autels sont vérifiés avant les lots. Les objets sont triés par altitude basse pour placer la table avant ses décorations.
- Les mécanismes créatifs et le paiement natif déjà présents. Des compteurs distinguent appels tentés, retours positifs et échecs ; le journal ajoute `OBJECT_RESULT` avec l'identifiant de chaque objet.

Les 194 BTBP trouvés dans les dossiers actuels ne contiennent aucun objet. Vous avez confirmé ne pas disposer de blueprint meublé. **Aucun catalogue nominatif ni projet complet avec des identifiants de mobilier supposés n'est donc fourni hors jeu.** La recette fournie indique des identifiants `null` ; le bouton dans la DLL produit le projet complet à partir du registre réel de votre session. Les suggestions par noms français/anglais restent une aide à la sélection : vérifier visuellement chaque modèle.

## Premier test dans Enshrouded

1. Jeu fermé, installer les fichiers de l'archive dev8 correspondant à votre chargeur habituel. La livraison ne les installe pas automatiquement. Conserver les INI et les projets existants. Utiliser une zone de test distincte du château.
2. Ouvrir le sélecteur natif des matériaux, puis sélectionner le véritable **cube de marteau de 0,5 m**. Cela fournit le contexte natif, le registre et les informations des autels. Revenir dans les projets composites et ouvrir **Bibliothèque native**. Actualiser.
3. Dans **Matériaux**, sélectionner un bloc réel pour la pièce. Dans **Objets natifs compatibles**, utiliser **Suggérer des objets disponibles**, puis contrôler les huit lignes de rôle. Pour corriger un rôle, choisir sa destination dans **Utiliser le choix de la bibliothèque pour**, puis cliquer sur le véritable objet du catalogue. Les huit rôles doivent être renseignés. Les flambeaux doivent être des modèles muraux ; les décorations de table doivent être petites.
4. Certains modèles, notamment portes ou lumières, portent le drapeau logique `0x80000`. S'ils sont refusés, l'option **Autoriser la copie originale des objets à logique (expérimental)** réutilise le contournement déjà présent dans le blueprint d'origine. Cette option est désactivée au démarrage. Elle ne restaure ni liens ni câblage. Les objets spéciaux/progression restent refusés.
5. Cliquer sur **Exporter le catalogue des objets natifs identifiés**. Le nouveau `NativeCatalog_*.json` apparaît dans `BuildingTools-Projects`. Un catalogue incomplet ne peut pas être exporté comme complet. Le matériau et les huit choix sont inclus. Un fichier `.partial` éventuellement laissé par un échec n'est pas un catalogue valide.
6. Cliquer sur **Préparer la pièce témoin meublée** : cela remplace seulement le brouillon. Si le modèle ne rentre pas, choisir explicitement un autre modèle plus petit. Le brouillon précédent est conservé en cas d'échec. Aucun objet du monde n'est placé par ce bouton.
7. **Enregistrer le projet dans de nouveaux fichiers**, noter le nom `Extended_*.btproject`, puis **recharger ce projet** depuis la liste. Vérifier que le diagnostic indique **11 objets**. Les nouveaux fichiers apparaissent directement dans `BuildingTools-Projects`, à côté des autres projets.
8. Activer **Inclure les objets compatibles dans les projets** et l'option expérimentale de placement natif déjà existante. Pour le test sans consommation, activer à la fois la construction créative et **Objets natifs gratuits**. En mode normal, fournir les objets : le stock est vérifié par le paiement natif par lot, pas garanti pour l'ensemble du mobilier par le devis.
9. Placer l'aperçu sur une zone libre et couverte par un autel. Exécuter **Vérifier sans construire**. Cette opération doit lire les objets existants puis vérifier le projet sans ajouter d'objet au monde. Un refus indique l'index et l'identifiant ; consulter aussi `BuildingTools.log` et `BuildingTools-Projects.log`.
10. Déclencher **Construire** une seule fois. Les voxels sont construits avant les objets. Résultat attendu : **11/11**, aucun échec. Vérifier réellement le lit, l'ouverture du coffre, l'assise/orientation si applicable, les flambeaux et leur lumière, le sens du tableau et les deux décorations sur la table. Vérifier les collisions du personnage et le passage. Recharger la partie puis vérifier la persistance. Un retour positif de `PlaceProp` ne certifie pas ces comportements.

Ne pas reconstruire au même endroit pour tester la relecture : les objets déjà présents doivent être signalés comme collisions. Tester une seconde copie dans une autre zone libre. En cas de lot partiel, ne pas relancer automatiquement : le mod arrête les lots suivants et conserve le compte des appels déjà acquittés.

## Test de la sauvegarde blueprint d'origine

Après construction réussie de la pièce témoin, ouvrir le **système de sauvegarde blueprint individuel d'origine**, sélectionner toute la pièce avec ses objets et enregistrer sous un nouveau nom. Inclure les pivots des objets dans le volume de capture. Le message de sauvegarde et `blueprint saved: ... 11 objects` doivent confirmer les objets capturés. Une capture dépassant 512 objets est désormais refusée, sans sauvegarde tronquée.

Recharger ce BTBP individuel, afficher ses objets et construire une copie à un autre emplacement libre avec le mécanisme original. Vérifier à nouveau les 11 objets et leurs interactions, puis la persistance après rechargement de la partie. Ce test de capture réelle **reste à faire en jeu** ; les tests automatisés couvrent son aide de capture et le codec, pas la lecture réelle du monde par le callback.

À conserver pour le retour : `NativeCatalog_*.json`, les nouveaux `.btproject`/BTBP, le BTBP de capture originale, le rapport de diagnostic après construction et les deux journaux. Ne pas transmettre votre sauvegarde pour ce test.

## Génération hors jeu après export du catalogue

Le bouton du mod est la voie directe sous Windows. Une fabrique de fichiers utilise aussi exactement la disposition C++, le codec et l'exporteur de production, une fois un catalogue réel exporté. Dans WSL, depuis le dépôt :

```sh
c++ -std=c++17 -O1 tools/native_furniture_host.cpp -o build/native_furniture_host
python3 tools/make_native_furniture_project.py --catalog /chemin/NativeCatalog_REEL.json --outdir /chemin/nouveau-dossier
```

Le dossier doit être nouveau. Copier ensuite tous ses `.btproject` et `.btbp` dans `BuildingTools-Projects`. La fabrique refuse les rôles manquants, identifiants inconnus/refusés, volumes invalides ou modèles qui ne rentrent pas. Elle ajoute une provenance avec l'empreinte du catalogue et `inGameValidated: false`. Un catalogue JSON est un export descriptif, pas une preuve cryptographique de l'origine de ses données.

## Décoration du château : étape suivante, après votre validation

Le projet de décoration du château n'est **pas généré maintenant** : votre demande conditionne cette étape à un démonstrateur validé en jeu. Le support technique des projets composés uniquement d'objets est présent et testé ; il n'ajoute aucun voxel ni pièce native.

Pour cette étape, il faudra reprendre **le manifeste exact du château construit**, son `anchor`, ainsi que l'origine mondiale et la rotation consignées dans son journal de construction/diagnostic. Reprendre le même `anchor` JSON seul ne suffit pas à retrouver l'emplacement dans le monde. Ces coordonnées ne doivent pas être devinées. Les modules de mobilier auront des décalages locaux mesurés et des BTBP ne contenant que les objets ; le projet final devra annoncer **0 voxel, 0 pièce native** et le compte de mobilier prévu. Les empreintes des fichiers du château serviront de référence et devront rester identiques.

Les éléments de mobilier voxel déjà présents dans le château devront être pris en compte comme obstacles : aucune suppression automatique de ces éléments, des murs ou des escaliers. Les ensembles chambre, bibliothèque, trône, banquet et atelier pourront être réutilisés comme modules d'objets, avec des supports et passages réservés propres à chaque salle. La pièce témoin est le premier ensemble de ce mécanisme, pas encore une bibliothèque complète de mobilier de château.

## Limites des contrôles

- Les collisions utilisent les **boîtes englobantes transformées des modèles**, avec tolérance de contact de 1 cm. Elles peuvent refuser des objets dont les boîtes se croisent mais dont les maillages ne se touchent pas. Elles ne prouvent pas la collision physique réelle ni la bonne orientation visible.
- Le support est un contact géométrique : empreinte intégralement au-dessus d'un meuble, ou cinq sondes sur une même face vers des voxels. Il ne connaît pas les règles propres à chaque attache murale ni la forme détaillée du dessus d'une table. Tester la fixation et la stabilité en jeu.
- Maximum : 2048 objets prévus, 512 objets existants pertinents par lecture et 512 cubes de contrôle par objet. Dépassement, registre/volumes indisponibles, table d'entités modifiée pendant la lecture ou lecture trop longue : refus explicite.
- La lecture des objets existants réutilise le lecteur d'entités original, réparti sur les callbacks. Ce n'est pas une transaction atomique du monde : éviter les modifications concurrentes pendant le test.
- Le créatif réutilise l'interception du paiement original. L'absence réelle de consommation reste à vérifier dans le jeu, de même que le paiement normal et le comportement des objets interactifs.
- Seuls ID/position/quaternion sont sérialisés par le format original. Contenu des coffres, propriétaire, personnalisations non sérialisées, réseaux logiques et câblages ne sont pas copiés. Le gestionnaire original supprime l'événement de liaison logique lorsque le contexte blueprint n'a pas de writer natif ; son résultat doit rester expérimental.

L'audit du code et le bilan des tests sont détaillés dans [DEV8-VALIDATION.md](DEV8-VALIDATION.md).
