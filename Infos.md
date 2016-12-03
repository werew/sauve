

## Conditions d'arret 

### Scanner 

Un thread de type "scanner" peut/doit se terminer lorsque il le
traitement de toutes les repertoires de l'arborescence a ete termine,
ce qui se traduit par l'etat suivant: le buffer des repertoires est
vide et n'est pas susceptible d'etre rempli. 
Cet etat se verifie lorsque ces deux conditions sont satisfactes:

    - le buffer des repertoires ne contiens aucune entree a traiter

    - tous les autres threads "scanner" se sont termine ou sont 
      inactives (ne sont pas en train de traiter des repertoires)

La deuxieme condition assure que aucun nouveau repertoire puisse
etre mis dans le buffer. 

### Analyzer 

La condition d'arret des threads de type "analyzer" est tres proche
de cella des "scanners". Un "analyzer" peut/doit se terminer lorque 
le traitement de tous les fichiers et repertoires de l'arborescence
a ete termine, ce qui se traduit par l'etat suivant: le buffer des
fichier est vide et n'est pas susceptible d'etre rempli.
Cet etat se verifie lorsque ces deux conditions sont satisfactes:

    - le buffer des fichiers ne contiens aucun entree a traiter

    - les threads "scanner" ont termine le traitement des tous les 
      repertoires de l'arborescence

La deuxieme condition est satistaite lorsque les conditions d'arret
des "scanners" decrite ci-dessus sont verifiee et assure qu'aucun
nouveau fichier puisse etre mis dans le buffer.


