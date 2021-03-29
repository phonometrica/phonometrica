Roadmap for Phonometrica
========================

Version 0.8.0
-------------

Done:
- new user interface based on wxWidgets
- new scripting engine
- ability to search files in the project manager
- ability to save and load queries and get_concordances
- redesigned query editor
- query joints (union, intersection, complement)
- press space to play a match or enter to edit it in a concordance view
- properties are now directly edited from the information panel
- improved shortcuts to show/hide the panels of the main window (see the Window menu)
- updating a plugin now cleanly removes the previous version
- the description file for plugins is now ``description.phon`` (instead of ``description.json``)
- plugins can now display documentation (stored in a directory named "Documentation")
- "Search annotations" is now "Find in annotations"
- Edit menu (find/replace, undo/redo)
- syntax completion and call tips in script views
- improved LPC analysis based on Burg's algorithm
- improved bookmarks
- ability to use modules in plugins
- signal/slot mechanism for plugins (``create_signal()``, ``connect()``, ``disconnect()``, ``emit()``)
- option to restore open views from previous session
- resources directory is now determined automatically based on the executable's path
- License is now GPL2 or later (instead of GPL3 or later)
- usability improvements across the board
- bug fixes

TODO:

- coding protocols (coding protocol: champ libre texte (ex: mot précédent un codage) ????)
- annotation views
- formant queries: mesures acoustiques sur une sélection et pas seulement sur un point.
- npoint en plus de npoint average (diphtongues, pitch)
- priorité : refactoriser les vues spreadsheet (finir CSV)
- bug décalage: décalage proportionnel à la taille du zoom. Si on fait glisser la fenêtre d'un côté ou de l'autre, le décalage sera tantôt à gauche ou à droite. Si on met un curseur fixe (tracking on), décalage avec la borne dans les layers. 
- std::bad_alloc: ouvre un fichier son -> catch and rethrow
- raccourcis play/pause stop, ajout/suppression d'ancres
- mac : bug aléatoire quand on clique play
- recherches formantiques, mettre les unités dans la view plutôt que dans le query editor
- bug selecting_tier
- mettre à jour la concordance en cours quand on édite un item. 
- finalize settings dialog (add search operator selector)
- update documentation

Version 0.9.0+:

- contexte autour de la target de la contrainte de référence 
- complex queries
- project metadata dans le panel info
- measure pitch, intensity and duration
- settings: possibilité de sauvagarde périodique des annotations (peut-être fichier buffer ?)
- regarder suppression et curseur sur mac
- plugin section on the website
- finalize documentation (arborescence complète du modèle objet pour les scripts)
- project metadata 


Version 1.0:

- Extraction d’une partie sélectionnée du fichier son. 
- extraire extrait son + annotation
- couper/coller/modifier son
- drag and drop des fichiers sur le projet
- touchpad en plus de la molette dans les annotations ? Touchpad supporté par wxWidgets ?


Post 1.0

- Tool/wizzard to create plugins
- POS tagging?
- Statistics (intégrer le code pour la régression)
- get_concordances avec n mots à gauche et à droite plutôt que des chaînes de n caractères
- lexical measures (functional load, neighborhood density)
- recording

============================================================================

    3) Dans le plugin IPFC dolmen, search grammar, voyelle ou consonne :
"TierSelectingField": 1 ?? --> layer_field
"TierNamePattern": --> "layer_name" "cod-[iyue2oajHw]" ??
Si j’ai bien compris, ces lignes permettent de sélectionner la bonne tier dans le premier champ de recherche. 
    a) Je n’ai pas l’impression (dans dolmen) que ceci fonctionne bien. Si un code commençant par 01 (codage du i) se retrouve sur une tire « cod-u », il est pourtant bien trouvé dans la query où le premier champ est spécifié comme « cod-i ».  
    b) Si ce n’est pas grave, ok 
    c) S’il faut absolument que la bonne tire soit sélectionnée, quelle syntaxe dans le json du coding protocol ?

    4) Dans les coding protocols : choices/display ne fonctionnent pas !

============================================================================

Non car trop compliqué :
- ancre fantôme sous le curseur (puis disparaît si on insère une ancre)


- ovh adresse mail / liste de diffusion


Bugs sound view
- when the info panel is moved, the wavebar selection is not updated
- wrong selection in waveform