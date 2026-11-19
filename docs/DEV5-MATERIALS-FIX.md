# Dev5 — résolution des matériaux et coûts natifs

## Diagnostic du test dev4

Le journal local `BuildingTools.log`, daté du 19 novembre 2026, contient la même
séquence que le signalement : cinq modules, 800 cellules, cube tenu de 0,5 m,
commande reçue, arrêt à `next=0 completed=0`. À 09:52:53.717, le scanner confirme
dans le registre le bloc **`0x7ad0559c` pour l'octet `0x9d`**. L'erreur survient
avant la tarification des cellules. Ce journal ne contient pas l'ID remboursé
qui permettrait de nommer entièrement le conflit ; dev5 l'enregistre.

Deux problèmes sont identifiés dans le code et les données livrées :

1. Les dix exemples utilisent **`0x25a43c2f` / `0x9d`**, copiés d'un ancien BTBP,
   sans confirmation native. Le chemin dev4 qui trouve un vrai bloc mais le
   refuse pointe vers le garde-fou de remboursement incompatible. Une provenance
   locale ne constituait pas une validation du matériau : les fixtures étaient
   incorrectes. Dev5 les référence avec le bloc effectivement confirmé, sans
   changer leur géométrie ni toucher au fichier source personnel.
2. Le précontrôle composite cherchait un bloc depuis `state` (définition de la
   forme du marteau), alors que `bp_fill` utilise `pdata` (matériau tenu). Cela
   pouvait chercher dans une autre allocation. Le cache gardait aussi les échecs
   par octet, empêchant une recherche ultérieure correcte. Dev5 utilise le même
   résolveur et la même ancre que le placement individuel ; seuls les succès sont
   conservés, et leur pointeur, type, octet et identité native sont revalidés.

La divergence d'ancre ne suffit pas à expliquer à elle seule le journal fourni :
celui-ci montre qu'un vrai bloc a bien été trouvé. La correction des fixtures est
donc nécessaire ; changer uniquement la DLL laisse les anciens exemples ambigus.

## Comparaison avec les blueprints individuels

La version 0.33.0 (`a5524c1`) cherchait l'ID sauvegardé dans l'allocation du
matériau tenu, d'abord avec l'octet sauvegardé, puis sans cet octet pour supporter
sa renumérotation. Cette recherche ne prouvait pas encore que l'item était un
bloc de construction enregistré. Le correctif amont du problème #61 (`2304d64`,
base 0.33.2) a ajouté cette preuve et le contrôle des anciens IDs de remboursement
afin d'éviter des blocs internes impossibles à supprimer.

Dev5 conserve ces protections et partage leur politique entre les deux modes :

| Cas | Décision |
|---|---|
| ID enregistré correspondant à un vrai petit bloc | Utiliser son octet natif actuel, même si l'octet BTBP est ancien |
| Ancien ID remboursé, vrai bloc du même octet, remboursement natif identique | Utiliser ce vrai bloc |
| Aucun remboursement disponible dans ce contexte, vrai bloc confirmé | Conserver la compatibilité amont des nouveaux mondes |
| Remboursement disponible avec un autre ID | Refuser avec les deux IDs ; remplacement explicite dans l'interface |
| Aucun bloc natif enregistré correspondant | Refuser comme matériau inconnu |

Les anciens BTBP personnels ne sont jamais réécrits. Si leur identité ne peut
pas être prouvée, sélectionner intentionnellement un matériau de la bibliothèque
native et appliquer le remplacement dans l'aperçu. Le seul octet BTBP ne permet
pas de déduire avec certitude le matériau voulu d'un ancien fichier incompatible.

## Coûts et diagnostics

Les champs `cells` / `items` de la table BTBP sont des statistiques enregistrées,
pas la facture. Chaque cellule est tarifiée par la procédure native existante.
Le code lit les entrées natives de 24 octets : ID d'item à `+0x10`, quantité à
`+0x14`. Le registre doit connaître cet item, qui peut être une ressource ordinaire.
Une ressource remboursée n'est pas passée à la place de l'ItemInfo d'un bloc.

Les limites restent **128 items de coût distincts**, **2 147 483 647 unités par
item** et **256 entrées par liste native temporaire**. Les additions sont vérifiées
avant modification du total. Une quantité native nulle avec un ID connu est
acceptée ; ID nul/inconnu, dépassement, trop d'items et liste native mal formée
ont des erreurs distinctes. Les listes et les coûts sont revérifiés avant chaque
lot construit, avec un compteur de lot séparé du total de prévalidation.

Creative Building évite le contrôle de stock/paiement selon les règles existantes.
Il ne transforme jamais un matériau inconnu en matériau valide et ne supprime
aucune limite de coûts. Aucun garde-fou d'autel, de session, de collision, de
dimensions, de paiement ou d'ABI n'est désactivé.

Les logs `project material CONTEXT/RESOLVE` donnent l'opération, le module
(index à partir de zéro), le fichier, la cellule, le slot, l'ID sauvegardé,
l'octet sauvegardé, le bloc natif, son octet, le remboursement connu/inconnu,
le chemin choisi et le code d'erreur. Les noms sont affichés s'ils sont déjà en
cache ; aucune fonction de localisation réservée à l'interface n'est appelée
depuis le serveur. `project cost REJECT` précise l'item de coût, le total précédent,
la quantité demandée, la valeur refusée et la limite. Une erreur de résolution
indique `cost=not_computed`, car aucun coût n'a encore été calculé.

Le motif français détaillé reste dans l'aperçu et dans le rapport d'opération.
Les logs `cost NATIVE` montrent les premières entrées et `cost TOTAL` les totaux
prévalidés. Aucun ID ni quantité n'est inventé pour masquer une erreur native.

## Retest à effectuer dans Enshrouded

1. Fermer le jeu et installer le ZIP Standalone dev5, **avec les langues et les
   dix BTBP d'exemple mis à jour dans `BuildingTools-Projects`**. Conserver ailleurs
   vos éventuelles modifications personnelles des exemples avant leur remplacement.
   Aucun remplacement automatique de vos fichiers n'a été effectué par l'agent.
2. Vérifier la bannière `0.34.0-dev5`, utiliser un monde local de test, une zone
   libre couverte par les autels et le cube du marteau de 0,5 m.
3. Charger `Dev3 - trois bâtiments reliés` : 5 modules / 800 cellules. En créatif,
   cliquer **Vérifier sans construire**. Attendre `project material RESOLVE` avec
   `saved_item=0x7ad0559c`, `route=registry`, `issue=none`, puis un résultat valide.
   L'inspection ne doit poser aucun bloc.
4. Cliquer **Construire**. Attendre la validation, `BUILD_STARTED`, puis les lots
   et leurs acquittements. Vérifier visuellement les 800 cellules, leur matériau,
   les raccords et le comportement après rechargement du monde de test.
5. Refaire avec les trois modules (312 cellules), puis les trois niveaux et
   trémies (1 444 cellules). Les ouvertures et rotations doivent rester identiques.
6. En mode normal, refaire un petit projet avec un stock suffisant puis insuffisant.
   Contrôler les quantités natives/paiement. Refaire un blueprint individuel valide
   et un ancien blueprint légitime pour vérifier la résolution commune.
7. Conserver une copie de l'ancien exemple dans un dossier de test si l'on veut
   reproduire le conflit : il doit être refusé avec ses IDs exacts, sans nouvelle
   construction. Choisir ensuite explicitement le bon remplacement dans Matériaux.

En cas d'échec, conserver `BuildingTools.log`, `BuildingTools-Projects.log` et le
rapport exportable, avec le projet exact et le mode utilisé. Les tests automatisés
ne remplacent aucune de ces observations. **Correction préparée ; validation en
jeu en attente du test utilisateur.** Voir [DEV5-VALIDATION.md](DEV5-VALIDATION.md).
