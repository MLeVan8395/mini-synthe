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

**Oscillateur (ou tirettes) + bruit → filtre → ampli → saturation → phaser → flanger → chorus → trémolo → delay → sortie**

| Module | Réglages | Ce qu'on observe |
|---|---|---|
| **Oscillateur** | Forme d'onde (sinus, triangle, dent de scie, carré), niveau, accord fin | Les harmoniques de chaque forme d'onde sur le spectre |
| **Tirettes harmoniques** | 9 tirettes d'orgue (16' à 1'), de 0 à 8, et des registrations toutes prêtes | La synthèse additive : chaque tirette fait monter une harmonique sur le spectre |
| **Générateur de bruit** | Bruit blanc ou rose, niveau | Un son sans hauteur, qui contient toutes les fréquences |
| **Filtre** | Passe-bas, passe-haut, passe-bande ; fréquence de coupure, résonance | La courbe du filtre, qui bouge en direct |
| **Enveloppe du filtre** | Quantité (en octaves, positive ou négative) et ADSR | La coupure qui s'ouvre et se referme à chaque note |
| **Ampli et enveloppe** | ADSR du volume | Un point qui parcourt la courbe pendant la note |
| **Effets** | Vibrato (sur l'oscillateur), saturation (gain, dosage), phaser et flanger (vitesse, profondeur, réinjection), trémolo (vitesse, profondeur), chorus stéréo (vitesse, profondeur, dosage), delay (temps, réinjection, dosage) | La forme d'onde qui s'écrase avec la saturation, l'écho qui se répète |
| **Écran** | Forme d'onde (zoom automatique) ou spectre | La note jouée en solfège et sa fréquence (ex. La3 · 440 Hz) |

Chaque module a une explication **Comprendre** rédigée simplement. Chaque section, et chaque effet, est un **volet** qu'on ouvre ou ferme en touchant son titre.

**13 sons de départ**, chacun avec une consigne d'écoute : son pur, orgue, orgue à tirettes, basse synthé, son pincé, cuivre, nappe (pad), nappe phaser, lead flanger, trémolo, lead saturé, charleston (bruit seul), vent.

**Jouer** : clavier à l'écran (au doigt ou à la souris, on peut glisser d'une touche à l'autre), ou clavier d'ordinateur AZERTY (Q S D F G H J K pour les touches blanches, Z E T Y U pour les noires, W / X pour changer d'octave). Le bouton **Tenir les notes** garde le son pendant qu'on manipule les réglages.

## Principes

- **Aucune donnée collectée.** Pas de compte, pas de publicité, pas de cookie.
- **Rien à installer.** Une seule page web, sans échantillon sonore : tous les sons sont fabriqués en direct par le navigateur (Web Audio).
- **Conventions de notation françaises** : notes en Do, Ré, Mi… ; octaves numérotées à la française (La3 = 440 Hz).

## Clavier MIDI

Avec Chrome ou Edge (ordinateur) ou Chrome (Android), branche un clavier en USB et touche **🎹 Clavier MIDI** : les touches jouent le synthé, la molette de pitch fait glisser la hauteur (± 2 demi-tons), la molette de modulation dose le vibrato, la pédale de sustain tient les notes. **Apprendre un bouton** relie un potard ou un fader à n'importe quel curseur (coupure, résonance, ADSR…). Pas disponible sur iPad / iPhone (Apple ne le permet pas).

## Licences

- **Tout le projet** (code, textes, exercices, schémas, partitions) : © 2026 Mickael Le Van, **tous droits réservés** (voir [`LICENSE`](LICENSE)). Reproduction, diffusion et usage commercial interdits sans autorisation écrite de l’auteur. Les éléments de tiers cités restent sous leur propre licence.

## Crédits

- Conception et textes : Mickael Le Van, professeur de musiques actuelles et jazz.
- Développement : réalisé avec l'aide de Claude (Anthropic).
- Police : Space Grotesk et JetBrains Mono (Google Fonts, licence SIL OFL).
