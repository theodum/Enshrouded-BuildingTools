# Dev4 — diagnostic des actions composites et retest ciblé

## Ce que montrent les données réelles

Le `BuildingTools.log` local de dev3 a été lu sans le modifier. Il contient le
chargement des projets de 312 et 1 444 cellules et le rendu Vulkan, mais aucun
`BEGIN`, contrôle natif ou lot composite. Les lignes `exact: cancelled (window
closed)` suivent Échap dans cette trace : elles ne prouvent pas qu'une commande
acceptée aurait été annulée.

Le code de dev3 passait `shape && !g_projectRunning` comme état activé des deux
boutons. `shape` imposait exactement 0,5 m sur les trois axes. Avec une autre forme,
le bouton natif était désactivé et son wrapper ne pouvait pas appeler le callback.
Le refus n'était donc ni journalisé ni affiché à côté du bouton. La voie Entrée
pouvait également refuser sans log et son message temporaire pouvait être manqué.

La trace utilise la forme `0x10b5c02c`. En lecture seule, un ancien BTBP local porte
ce même identifiant et des pas `(1,1,1)` ; les centres de la trace sont cohérents
avec cette dimension. Cela corrobore le blocage par la forme de 1 m. Dev3 ne
journalisait pas ses dimensions capturées au moment de l'action ; le nouveau log
les affiche explicitement pour confirmer chaque tentative.

**C'est la forme tenue dans le marteau qui doit mesurer 0,5 m.** Les blueprints
sources à 1/2/4 m continuent d'être subdivisés par le planificateur. Aucune nouvelle
autorisation de construire avec un cube moteur de 1 m n'a été ajoutée.

## Correction du chemin d'exécution

- Les boutons restent cliquables et utilisent le même dispatcher qu'Entrée.
  Une admission refusée n'arme rien, ne ferme pas la fenêtre et ne démarre aucun lot.
  Entrée garde une seule voie de construction : l'événement natif d'un bouton
  focalisé ne peut pas lancer une inspection en concurrence avec ce raccourci.
- Chaque réception écrit `project action CLICK`, avec source, dimensions, état de
  fenêtre/page/mode/session, hooks et ancienneté des données de bases. Un refus
  écrit `REFUSED code=... reason=...` et conserve le message en bas de fenêtre.
- Un démarrage admis initialise uniquement la vérification et son journal. Un
  échec du journal laisse la fenêtre ouverte. L'armement et la demande de fermeture
  sont faits sous le même verrou ; le timer d'Exact classique n'envoie pas le clic.
- La fenêtre confirme sa fermeture, puis le thread d'entrée attend un écran de
  jeu à jour. Le décalage d'une image entre ces deux observations n'annule plus le
  démarrage. Sans confirmation/retour au jeu sous cinq secondes, l'opération s'arrête.
  Cette course possible est démontrée par test d'états, pas observée dans l'ancien log.
- Les étapes `SCHEDULED`, `SEND`, `RECEIVED`, `validation START/RESULT` et
  `BUILD_STARTED` rendent le parcours traçable. Les refus natifs gardent leurs contrôles.
- Avant toute publication de lot, une erreur revient à l'aperçu avec sa raison.
  Après publication potentiellement payée, pas de relance automatique. Le joueur
  peut toujours interrompre ; perte de focus, outil/mode/session changé ou écran
  inattendu arrêtent les nouveaux lots.
  Un commit marqué composite avec une file abandonnée vide reste supprimé même
  si une nouvelle opération a démarré ; il ne retombe pas sur un placement vanilla.

Les contrôles de zones, prix, ressources, matériaux, collisions, ABI et limites
ne sont pas désactivés. Actualiser autels et stocks ouvre le vrai sélecteur natif ;
cette action ne crée pas d'autel et n'accorde aucun droit de construire.

## Test personnel à faire avec dev4

Fermer le jeu pour changer de DLL. Utiliser un monde local de test et conserver
la copie de retour décrite dans [EXTENDED-TESTING.md](EXTENDED-TESTING.md).

1. **Reproduire le refus.** Avec le cube de 1 m, ouvrir Placer et charger les trois
   modules. Cliquer Vérifier, puis Construire. Attendu : fenêtre conservée, dimensions
   affichées et explication du cube de 0,5 m ; deux réceptions `CLICK`, chacune suivie
   de `REFUSED ... code=SHAPE`. Aucun `BEGIN` ou lot pour ces tentatives.
2. **Préparer le bon curseur.** Fermer la fenêtre, choisir le cube natif de 0,5 m
   dans le marteau, puis rouvrir Placer et le projet. Vérifier les trois dimensions
   `0.5000`. Si les données d'autels sont absentes/périmées, utiliser Actualiser autels
   et stocks, fermer le sélecteur et revenir. Un refus BASES reste un contrôle normal.
3. **Inspection seule.** Fixer une zone libre couverte par l'autel et cliquer Vérifier.
   Attendu : `CLICK action=inspect`, `command START`, `CLOSE_REQUEST`, `window CLOSED`,
   puis `SCHEDULED`, `SEND down=1`, `RECEIVED`, `validation START`, `CHECK/CHECKED`,
   `validation RESULT ... valid=1`, `INSPECTED` et retour à l'aperçu. Aucun
   `BUILD_STARTED`, `BATCH_BEGIN` ou placement dans le monde.
4. **Construction.** Au même ancrage libre, cliquer Construire une seule fois.
   Attendu : réception `action=build`, même vérification complète, puis
   `BUILD_STARTED`, les `BATCH_BEGIN/BATCH_RESULT` et `COMPLETE` avec 312 placements.
   Ne pas refaire de clic manuel pour aider une opération en attente.
5. **Refus sûr.** Refaire une inspection sur un bloc occupé ou hors couverture.
   Attendu : `ERROR`/`STOP`, raison visible dans l'aperçu et aucun lot construit.
   Tester aussi Entrée, un double clic et une interruption pendant le contrôle.

Si une étape ne correspond pas, fournir `BuildingTools.log` et
`BuildingTools-Projects.log` de cette session, la bannière dev4, les dimensions
affichées, le bouton/source utilisé et le message exact. Si aucun `CLICK` n'est
présent, le problème précède encore le dispatcher natif. S'il existe un `REFUSED`,
son code donne le contrôle qui a bloqué. Si `SEND` n'est pas suivi de `RECEIVED`,
le log isole la livraison d'entrée. Ne pas désactiver un contrôle pour poursuivre.

**Ces résultats sont attendus, pas constatés en jeu avec dev4.** Les tests hôtes
utilisent le code de routage/admission/fermeture partagé par la DLL, avec un adapter
qui compte les appels ; ils ne reproduisent pas le moteur ou ses événements UI.
