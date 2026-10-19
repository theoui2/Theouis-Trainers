# Raccourci du trainer du jeu en cours

1. Installe les trois versions à jour, ou le lanceur et uniquement le jeu souhaité.
2. Ouvre une fois **Theoui's Trainers** dans Ultrahand.
3. Ouvre **Shortcut settings**, choisis un preset, puis **Return to Ultrahand**.
4. Dans le jeu, utilise ce raccourci pour ouvrir le bon trainer.

**L3 + R3** est proposé au premier lancement si la combinaison est libre. L3/R3 désignent les clics des sticks.

Presets : **L3 + R3**, **L + R + L3**, **L + R + R3**, **ZL + ZR + L3**, **ZL + ZR + R3**, **L + R + Haut**, ou **Off**.

- Dans BL2, le raccourci ouvre le trainer BL2 ; dans TOTK, il ouvre le trainer TOTK.
- Si le trainer est déjà affiché, le même raccourci le masque.
- Relâche les touches entre deux utilisations. Une réouverture du même trainer conserve sa page et son état de session.
- Un jeu sans trainer correspondant affiche la liste habituelle. Les associations ambiguës demandent une sélection manuelle.
- Une combinaison déjà utilisée est refusée avec une explication ; le réglage précédent est conservé.
- Le raccourci global d'Ultrahand et ses autres réglages sont conservés. Si **Launch Combos** est désactivé dans Ultrahand, active-le avant de choisir un preset.

Les préférences sont enregistrées dans `/config/theouis-trainers/config.ini`. Les mises à jour ne fournissent aucun fichier destiné à les écraser.

Pour un futur jeu, ajoute `title_id=IDENTIFIANT_DU_JEU` dans le `.ini` placé à côté de son `.ovl`. Le nom et la version du fichier compilé gardent leur rôle habituel. Voir **SHORTCUTS.md** pour le format complet et les contrôles locaux.

Le raccourci fonctionne depuis Ultrahand et nos trainers à jour. Avec un autre overlay ancien qui ne gère pas les raccourcis Ultrahand, reviens d'abord à Ultrahand. Le fonctionnement sur console reste à valider.
