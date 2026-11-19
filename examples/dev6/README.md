# Projet multiétages paramétrique dev6

Le projet est préparé dans l'interface avec un escalier et un matériau sélectionnés
dans le registre du jeu. Il n'y a pas de fichier BTBP universel avec un ID
d'escalier supposé. Le code de la fabrique est `src/storey_demo.h`, son adaptateur
`src/architecture_storeys.h`.

Choisir Bibliothèque → matériau et escalier ; calibrer une volée et sa montée ;
Composants architecturaux → Escaliers → **Préparer le bâtiment de trois étages
avec escaliers natifs**. Le brouillon obtenu peut être inspecté, exporté en vrais
BTBP/`.btproject`, rechargé puis construit depuis les actions habituelles.

Contenu : 16 modules (trois planchers, murs par niveau, toit), deux montées avec
paliers, trémies et 2 m de dégagement, embrasures et fenêtres. Le nombre de volées
et les dimensions dépendent du modèle. Les portes interactives restent à poser
manuellement ; la circulation réelle doit être testée dans Enshrouded.

Le protocole complet est dans `docs/DEV6-MULTISTOREY.md`. Les cinq projets statiques
existants sont conservés sans changement. Aucun fichier de joueur n'est modifié.
