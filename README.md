# Mini-synthé pédagogique

Un petit synthétiseur en ligne, libre et gratuit, pour **comprendre comment fonctionne un synthé** : on suit le parcours du son de la source jusqu'à la sortie, et on voit en direct ce que chaque réglage change.

Il a été conçu pour les cours de musiques actuelles et jazz (FM, MAO, cours d'instrument), mais il sert à tout élève ou enseignant curieux de la synthèse sonore.

👉 **Utiliser le mini-synthé :** https://mlevan8395.github.io/mini-synthe/

## Installer l'appli sur un téléphone ou un ordinateur

Le mini-synthé s'installe comme une application, gratuitement et sans passer par une boutique :

- **Android** (Chrome) : ouvre le site, puis touche le bouton **Installer l'appli** en haut de la page, ou menu ⋮ → **Installer l'application**.
- **iPhone / iPad** (Safari) : bouton Partager → **Sur l'écran d'accueil**. Attention : sur iPhone, désactive le mode silencieux, sinon on n'entend rien.
- **Ordinateur** (Chrome, Edge) : icône d'installation dans la barre d'adresse.

Il fonctionne ensuite **hors ligne** et se met à jour tout seul à chaque ouverture avec une connexion.

## Le parcours du son

**Oscillateur + bruit → filtre → ampli → saturation → chorus → delay → sortie**

| Module | Réglages | Ce qu'on observe |
|---|---|---|
| **Oscillateur** | Forme d'onde (sinus, triangle, dent de scie, carré), niveau, accord fin | Les harmoniques de chaque forme d'onde sur le spectre |
| **Générateur de bruit** | Bruit blanc ou rose, niveau | Un son sans hauteur, qui contient toutes les fréquences |
| **Filtre** | Passe-bas, passe-haut, passe-bande ; fréquence de coupure, résonance | La courbe du filtre, qui bouge en direct |
| **Enveloppe du filtre** | Quantité (en octaves, positive ou négative) et ADSR | La coupure qui s'ouvre et se referme à chaque note |
| **Ampli et enveloppe** | ADSR du volume | Un point qui parcourt la courbe pendant la note |
| **Effets** | Saturation (gain, dosage), chorus stéréo (vitesse, profondeur, dosage), delay (temps, réinjection, dosage) | La forme d'onde qui s'écrase avec la saturation, l'écho qui se répète |
| **Écran** | Forme d'onde (zoom automatique) ou spectre | La note jouée en solfège et sa fréquence (ex. La3 · 440 Hz) |

Chaque module a une explication **Comprendre** rédigée simplement.

**9 sons de départ**, chacun avec une consigne d'écoute : son pur, orgue, basse synthé, son pincé, cuivre, nappe (pad), lead saturé, charleston (bruit seul), vent.

**Jouer** : clavier à l'écran (au doigt ou à la souris, on peut glisser d'une touche à l'autre), ou clavier d'ordinateur AZERTY (Q S D F G H J K pour les touches blanches, Z E T Y U pour les noires, W / X pour changer d'octave). Le bouton **Tenir les notes** garde le son pendant qu'on manipule les réglages.

## Principes

- **Aucune donnée collectée.** Pas de compte, pas de publicité, pas de cookie.
- **Rien à installer.** Une seule page web, sans échantillon sonore : tous les sons sont fabriqués en direct par le navigateur (Web Audio).
- **Conventions de notation françaises** : notes en Do, Ré, Mi… ; octaves numérotées à la française (La3 = 440 Hz).

## Licences

- **Code** : licence [MIT](LICENSE).
- **Textes pédagogiques** : [Creative Commons BY-SA 4.0](LICENCE-CONTENU.md).

## Crédits

- Conception et textes : Mickael Le Van, professeur de musiques actuelles et jazz.
- Développement : réalisé avec l'aide de Claude (Anthropic).
- Police : Space Grotesk et JetBrains Mono (Google Fonts, licence SIL OFL).
