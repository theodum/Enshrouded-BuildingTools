"""Sync keys owned by the Extended fork, preserving upstream translation bytes."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
FRENCH = {
    'extended.title': 'Building Tools Extended',
    'grid.on': 'Plan de construction virtuel', 'grid.step': 'Pas de la grille',
    'grid.height': 'Altitude du plan Y (m)', 'grid.x': 'Origine de la grille X (m)',
    'grid.z': 'Origine de la grille Z (m)', 'grid.axes': 'Afficher les axes',
    'grid.level': 'Niveau de référence', 'grid.store': 'Enregistrer ce niveau',
    'grid.recall': 'Utiliser ce niveau', 'grid.aim': 'Origine au point visé',
    'grid.help': 'Viser le plan vers le haut ou le bas. Origine et huit altitudes conservées entre placements et sessions. La grille ne crée aucun bloc.',
    'extended.creative': 'Objets natifs gratuits (avec construction créative)',
    'extended.objects': 'Inclure les objets compatibles dans les projets',
    'extended.zones': 'Afficher les zones des autels', 'extended.modules': 'Afficher les volumes des modules',
    'extended.object_creative': 'Activer les objets natifs créatifs ou quitter le mode créatif pour payer les objets',
    'extended.object_unsafe': 'Objet inconnu, spécial ou relié à la logique refusé ; omettre les objets pour construire les blocs seuls',
    'extended.object_help': 'Les objets natifs conservent modèle et comportement. Portes et fenêtres liées à la logique, objets câblés et autels refusés. Le paiement natif vérifie les stocks par lot ; un refus peut survenir après construction des blocs.',
    'extended.catalog': 'Bibliothèque native : matériaux et objets',
    'extended.catalog_help': 'Identifiants et noms lus dans le jeu. Choisir un matériau pour les composants ou remplacements, un élément natif pour les escaliers, ou un objet compatible à prévisualiser et placer sans le posséder en mode créatif étendu. Objets liés à la logique en consultation seule.',
    'extended.materials': 'Matériaux de construction', 'extended.props': 'Objets natifs compatibles',
    'extended.pieces': 'Éléments de construction natifs', 'extended.linked': 'Objets liés à la logique (consultation)',
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
