

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


## Syncronization des threads


### Acces aux buffers

Le programme utilise trois FIFO:

    - folders_queue: une liste chainee contenant les repertoires 
      a traiter
    
    - files_queue: un buffer circulaire contenant les fichiers a 
      traiter

    - term_queue: une liste chainee contenant les ids des threads 
      termines

A chaqune des ces FIFO est associe un mutex et une ou plusieurs
conditions pour en garantire l'access syncronise.

####  Listes: folders_queue et term_queue

L'acces (en lecture ou ecriture) a ces listes est soumis a l'utilise
d'un mutex. Dans le cas d'un access en lecture, si la liste est vide
le lecteur doit attendre une condition afin d'etre notifie lors d'une
adjonction dans la liste. Apres chaque adjonction un ecrivain doit de
son cote notifier un des lecteurs en attende par le biais d'un signal
sur la condition. L'algorithme peut etre ainsi resume:

    Lire element ( queue ):
        Lock queue.mutex
        tant que queue est vide:
            attendre queue.condition /* unlock/lock implicite */
        element := queue.pop() 
        Unlock queue.mutex
        renvoyer element
         
    Ecrire element ( queue, element ):
        Lock queue.mutex
        Signal queue.condition
        queue.push(element ) 
        Unlock queue.mutex
    


#### Buffer circulaire: files_queue

Etant donne que la taille d'un buffer circulaire est limite, l'acces 
syncronisee a cette structure de donnees comporte une legere 
difference par rapport aux FIFO qu'on a vu avant. 
Cette fois-ci un ecrivain peut devoir attendre si le buffer est plein.
L'algorithme est donc le meme que celui utilisee pour les listes sauf
pour une conditione supplementaire qui permet a l'ecrivain d'attendre
quand le buffer est plein et etre notifie dans le cas une slot du
buffer ait ete libere. L'algorithme peut etre ainsi resume:


    Lire element ( queue ):
        Lock queue.mutex
        tant que queue est vide:
            attendre queue.condition_lecture 
        element := queue.pop() 
        Signal queue.condition_ecriture
        Unlock queue.mutex
        renvoyer element
         
    Ecrire element ( queue, element ):
        Lock queue.mutex
        tant que queue est plein:
            attendre queue.condition_ecriture
        queue.push( element ) 
        Signal queue.condition_lecture
        Unlock queue.mutex



