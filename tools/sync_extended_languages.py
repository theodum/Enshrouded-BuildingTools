"""Sync keys owned by the Extended fork, preserving upstream translation bytes."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
FRENCH = {
    'extended.title': 'Building Tools Extended',
    'grid.on': 'Plan de construction virtuel', 'grid.step': 'Pas de la grille',
    'grid.height': 'Altitude du plan Y (m)', 'grid.x': 'Origine de la grille X (m)',
    'grid.camera_y': 'Prendre l’altitude de la caméra comme référence',
    'grid.z': 'Origine de la grille Z (m)', 'grid.axes': 'Afficher les axes',
    'grid.level': 'Niveau de référence', 'grid.store': 'Enregistrer ce niveau',
    'grid.recall': 'Utiliser ce niveau', 'grid.aim': 'Origine au point visé',
    'grid.help': 'Viser le plan vers le haut ou le bas. Origine et huit altitudes conservées entre placements et sessions. La grille ne crée aucun bloc.',
    'extended.creative': 'Objets natifs gratuits (avec construction créative)',
    'extended.objects': 'Inclure les objets compatibles dans les projets',
    'extended.zones': 'Afficher les zones des autels', 'extended.modules': 'Afficher les volumes des modules',
    'extended.object_creative': 'Activer les objets natifs créatifs ou quitter le mode créatif pour payer les objets',
    'extended.object_unsafe': 'Objet inconnu, spécial ou relié à la logique refusé ; omettre les objets pour construire les blocs seuls',
    'extended.object_help': 'Les objets compatibles utilisent leur procédure de création originale ; leur comportement reste à tester en jeu. Portes et fenêtres liées à la logique, objets câblés et autels refusés. Le paiement natif vérifie le stock par lot et peut échouer après construction des blocs.',
    'extended.catalog': 'Bibliothèque native : matériaux et objets',
    'extended.catalog_help': 'Identifiants et noms lus dans le jeu. Choisir un matériau pour les composants ou remplacements, un élément natif pour les escaliers, ou un objet compatible à prévisualiser et placer sans le posséder en mode créatif étendu. Objets liés à la logique en consultation seule.',
    'extended.materials': 'Matériaux de construction', 'extended.props': 'Objets natifs compatibles',
    'extended.pieces': 'Éléments de construction natifs', 'extended.linked': 'Objets spéciaux ou liés à la logique (consultation)',
    'extended.category': 'Catégorie', 'extended.refresh': 'Actualiser la bibliothèque',
    'extended.page': 'Page', 'extended.selected': 'Élément natif sélectionné',
    'extended.replace_from': 'Matériau source', 'extended.replace_to': 'Matériau de remplacement',
    'extended.use_catalog': 'Utiliser le matériau choisi dans la bibliothèque',
    'extended.all': 'Projet entier', 'extended.module': 'Module', 'extended.scope': 'Portée du remplacement',
    'extended.general': 'Général', 'extended.wall': 'Mur', 'extended.floor': 'Sol',
    'extended.roof': 'Toiture', 'extended.tower': 'Tour', 'extended.stairs': 'Escaliers natifs',
    'extended.replace': 'Appliquer le remplacement dans l’aperçu',
    'extended.replaced': 'Matériau remplacé dans {0} cellules ; monde inchangé',
    'extended.check': 'Vérifier sans construire',
    'extended.check_ok': 'Projet vérifié sans modifier le monde. Construire effectuera une nouvelle vérification.',
    'extended.altars': 'Assistance aux zones des autels', 'extended.zone_template': 'Zone existante servant de modèle',
    'extended.propose': 'Analyser la couverture et proposer des autels',
    'extended.coverage': 'Échantillons non couverts : {0}. Vérifiés : {1}/{2}. Zones proposées : {3}.',
    'extended.altar_help': 'Or : zones existantes. Vert : propositions théoriques aux dimensions observées, positions modifiables. Nombre autorisé, niveau, terrain et permissions restent à vérifier dans le jeu ; aucun autel placé automatiquement.',
    'extended.proposal': 'Autel proposé', 'extended.unassigned': 'Cellules hors des propositions : {0} (64 propositions au maximum)',
    'extended.architecture': 'Composants architecturaux',
    'extended.arch_help': 'Choisir les matériaux réels dans la bibliothèque. Les composants ne modifient que l’aperçu ; vérifier puis construire. Dimensions et décalages en mètres. Pour les sols, Y de l’ouverture représente la profondeur Z.',
    'extended.append': 'Ajouter au projet courant', 'extended.structure_mat': 'Sélection de bibliothèque pour la structure',
    'extended.frame_mat': 'Sélection de bibliothèque pour encadrements et bordures',
    'extended.width': 'Largeur X (m)', 'extended.height': 'Hauteur Y (m)', 'extended.depth': 'Profondeur Z (m)',
    'extended.thickness': 'Épaisseur (m)', 'extended.opening': 'Ouverture / trémie',
    'extended.opening_x': 'Décalage X de l’ouverture (m)', 'extended.opening_y': 'Décalage Y / Z pour les sols (m)',
    'extended.opening_w': 'Largeur de l’ouverture (m)', 'extended.opening_h': 'Hauteur / profondeur pour les sols (m)',
    'extended.round': 'Tour ronde', 'extended.floors': 'Nombre de niveaux', 'extended.floor_height': 'Hauteur par niveau (m)',
    'extended.crenels': 'Créneaux', 'extended.floor_hole': 'Réservations dans les planchers de la tour',
    'extended.straight': 'Droit', 'extended.turning': 'Quart tournant avec palier', 'extended.imperial': 'Impérial à deux branches',
    'extended.spiral': 'Spirale carrée avec paliers', 'extended.layout': 'Disposition des escaliers',
    'extended.flights': 'Éléments natifs par volée', 'extended.rise': 'Montée par élément natif (m ; 0 = volume mesuré)',
    'extended.reserve': 'Réserver les trémies et 2 m de dégagement dans l’aperçu',
    'extended.native_stair_help': 'Choisir un vrai escalier dans la bibliothèque. Orienter le modèle pour que sa montée suive +Z ; la rotation du projet tourne ensuite tout le tracé et ses paliers. Dimensions natives mesurées. Spirale carrée. Régler la montée réelle et tester la circulation en jeu. Aucune marche cubique générée.',
    'extended.generate': 'Générer dans l’aperçu du projet',
    'extended.generate_fail': 'Génération refusée : vérifier dimensions, ouvertures, matériau, élément natif, chevauchements et limites',
    'extended.piece_overlap': 'Un élément natif chevauche des blocs prévus ; réserver la trémie ou ajuster son décalage',
    'extended.export': 'Enregistrer le projet dans de nouveaux fichiers', 'extended.exported': 'Projet exporté : {0}',
    'extended.export_fail': 'Export refusé : vérifier les droits d’écriture et la limite de 128 modules de sortie. De nouveaux fichiers incomplets peuvent rester ; aucun fichier existant écrasé.',
    'extended.slice': 'Afficher une tranche d’altitude', 'extended.slice_y': 'Altitude basse locale de l’aperçu (m)',
    'extended.slice_height': 'Hauteur du niveau affiché (m)', 'extended.visible': 'Module visible dans l’aperçu',
    'extended.visual_only': 'Visibilité et tranche d’altitude concernent seulement l’aperçu. Tous les modules seront construits. Les grands plans utilisent les volumes des modules et les dimensions des éléments natifs.',
    'extended.object_preview': 'Afficher les repères de position des objets',
    'extended.quote': 'Coûts calculés lors de la dernière vérification (partiels si elle a été interrompue) :',
    'extended.cost_row': '{0} : {1} requis ; stock connu {2}',
    'extended.resume': 'Reprendre le projet conservé en mémoire',
    'extended.native_count': 'Éléments de construction natifs : {0}',
    'extended.job_check': 'Vérification des placements : {0}/{1}. Échap pour annuler.',
    'extended.job_build': 'Construction des placements : {0}/{1}. Échap arrête les prochains lots.',
    'extended.job_done': 'Projet terminé : {0} placements effectués',
    'extended.job_stop': 'Projet arrêté après {0} placements terminés : {1}. Voir BuildingTools-Projects.log.',
    'extended.native_dims': 'Dimensions natives mesurées X/Y/Z : {0} / {1} / {2} m',
    'extended.section': 'Section Extended',
    'extended.preview': 'Prévisualisation',
    'extended.diagnostics': 'Diagnostics',
    'extended.overview': 'Vue d’ensemble',
    'extended.open_projects': 'Ouvrir les projets composites',
    'extended.open_preview': 'Ouvrir l’aperçu du brouillon',
    'extended.launch_help': 'Tenir le marteau de construction. Cette action ouvre la fenêtre native sans construire. Choisir le cube de 0,5 m pour construire un composite.',
    'extended.experimental': 'Placement natif expérimental (monde de test)',
    'extended.native_disabled': 'Placement automatique des objets et pièces natives désactivé. Activer Placement natif expérimental dans Home > Options > Créatif / Objets natifs après lecture des limites. Les projets de blocs restent disponibles.',
    'extended.experiment_help': 'Objets et escaliers natifs non validés en jeu. Objets spéciaux, de progression ou liés à la logique exclus des composites. Collisions entre entités et rollback complet indisponibles. Utiliser un monde de test.',
    'extended.linked_help': 'Catégorie en lecture seule : ces objets spéciaux, de progression ou liés à la logique nécessitent des événements absents du contexte composite. Utiliser leur outil de placement original.',
    'extended.batch': 'Maximum de voxels par lot',
    'extended.piece_budget': 'Volume natif trop grand : plus de 4 096 voxels de contrôle, dégagement inclus. Choisir une pièce plus petite ou la placer manuellement.',
    'extended.trace': 'Tracer les événements des objets manuels (lecture seule)',
    'extended.abi': 'ABI des pièces natives',
    'extended.registry': 'Registre des objets',
    'extended.bases': 'Zones d’autels observées',
    'extended.available': 'Disponible',
    'extended.unavailable': 'Indisponible ou non observé',
    'extended.diag_row': '{0} : {1}',
    'extended.diag_help': 'Les contrôles binaires ne valident pas le comportement en jeu. Ouvrir une fois le sélecteur de matériaux du marteau pour observer le registre et les autels. Journaux : BuildingTools.log et BuildingTools-Projects.log.',
    'extended.operation': 'Opération {0} ; placements acquittés {1}/{2}',
    'extended.report': 'Enregistrer le rapport de l’opération',
    'extended.reported': 'Rapport enregistré dans BuildingTools-Projects : {0}',
    'extended.report_fail': 'Aucune opération arrêtée ou terminée à signaler, ou création du fichier impossible. Voir BuildingTools.log.',
    'extended.recovery_help': 'Arrêter conserve les lots acceptés. Le rapport identifie l’opération et son préfixe acquitté ; il ne permet pas une suppression ou une reprise automatique sûre. Annulation native partielle.',
    'extended.altar_samples': 'Les blocs sont contrôlés individuellement. Pièces et objets utilisent les coins et le centre de leur volume comme estimation ; la construction refait les contrôles natifs de couverture.',
    'extended.stair_model': 'Orientation du modèle natif (montée vers +Z)',
    'extended.native_example': 'Préparer un exemple d’escalier natif entre deux niveaux',
    'extended.object_budget': 'Objet trop grand : plus de 512 voxels de contrôle. Choisir un objet compatible plus petit ou utiliser le placement manuel original.',
    'extended.check_ms': 'Contrôle le plus lent (ms) ; limite actuelle par lot : {0} ; {1}',
    'extended.object_bounds_fail': 'Objet natif non préparé : incompatibilité, registre ou dimensions indisponibles, ou limite du projet atteinte. Aucun objet placé. Voir Diagnostics et BuildingTools.log.',
    'extended.config_fail': 'Réglages Extended non enregistrés. Les valeurs restent actives pour cette session ; vérifier l’accès en écriture à BuildingTools-Extended.ini. Voir BuildingTools.log.',
    'extended.action_busy': 'Une opération composite est déjà en cours. L’arrêter avant d’en lancer une autre.',
    'extended.action_window': 'Ouvrir la fenêtre de placement et sélectionner un projet composite avant cette action.',
    'extended.action_armed': 'Un placement est déjà armé. L’annuler avant de demander une autre action.',
    'extended.action_no_project': 'Aucun projet composite valide chargé. Sélectionner ou recharger un projet.',
    'extended.action_disabled': 'Building Tools est désactivé. Activer le mod avant de vérifier ou construire.',
    'extended.action_mode': 'L’exécution composite exige le mode Blueprint et l’outil Blueprint activé. L’opération n’a pas été poursuivie.',
    'extended.action_shape': 'Action refusée : forme capturée de {0} × {1} × {2} m. Cube de 0,5 × 0,5 × 0,5 m requis. Fermer cette fenêtre, choisir ce cube dans le marteau puis rouvrir le projet.',
    'extended.action_systems': 'Fonctions natives de validation ou placement indisponibles. Voir Diagnostics et BuildingTools.log ; aucune construction démarrée.',
    'extended.action_no_bases': 'Zones d’autels non observées. Ouvrir Actualiser autels et stocks, puis revenir au projet.',
    'extended.action_stale_bases': 'Données d’autels trop anciennes pour démarrer. Ouvrir Actualiser autels et stocks, puis réessayer. Les contrôles de couverture restent obligatoires.',
    'extended.action_current_shape': 'Forme capturée : {0} × {1} × {2} m. Cube requis : 0,5 m.',
    'extended.action_refresh': 'Actualiser autels et stocks (sélecteur natif)',
    'extended.action_close_timeout': 'La fenêtre native n’est pas revenue au jeu en 5 secondes. Opération arrêtée avant tout nouveau clic. Voir BuildingTools.log.',
    'extended.action_window_changed': 'L’écran actif a changé de façon inattendue. Exécution composite arrêtée ; aucun nouveau lot envoyé.',
    'extended.action_focus': 'Le jeu a perdu le focus. Exécution composite arrêtée ; aucun nouveau lot envoyé.',
    'extended.action_creative': 'Le mode créatif a changé pendant l’opération. Exécution arrêtée pour conserver des coûts cohérents.',
}


def main():
    entries = re.findall(r'X\(\w+, "([^"]+)", "([^"\n]*)"\)', (ROOT / 'src/strings.h').read_text(encoding='utf-8'))
    for path in (ROOT / 'lang').glob('*.txt'):
        data = path.read_bytes()
        for key, value in entries:
            if not key.startswith(('extended.', 'grid.')):
                continue
            if path.stem == 'fr':
                value = FRENCH.get(key, value)
            row = (key + ' = ' + value).encode('utf-8')
            pattern = re.compile(rb'^' + re.escape(key.encode()) + rb'\s*=.*$', re.MULTILINE)
            if pattern.search(data):
                data = pattern.sub(lambda _: row + b'\r', data)
            else:
                data += row + b'\r\n'
        path.write_bytes(data)


if __name__ == '__main__':
    main()
