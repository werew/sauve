

-------------------- 1 Conditions d'arret ----------------------------

1.1 Scanners 

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

1.2 Analyzer 

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


------------------ 2 Syncronization des threads ---------------------

2.1 Acces aux buffers

Le programme utilise trois FIFO:

    - folders_queue: une liste chainee contenant les repertoires 
      a traiter
    
    - files_queue: un buffer circulaire contenant les fichiers a 
      traiter

    - term_queue: une liste chainee contenant les ids des threads 
      termines

A chaqune des ces FIFO est associe un mutex et une ou plusieurs
conditions pour en garantire l'access syncronise.

2.1.1  Listes: folders_queue et term_queue

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
    


2.1.2 Buffer circulaire: files_queue

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


2.2 Arret des threads

Dans la premiere partie de ce document j'ai presente les conditions
d'arret de chaque thread, maintenant c'est le moment de voir
plus en detail le mechanisme de syncronisation utilise dans le
programme pour mettre en place les principes enonces dans la
premiere partie.

2.2.1 Scanner 

Un thread de type "scanner" verifie ses condition d'arret lorque il a
termine de traiter un repertoire et est en train de chercher un
nouveau repertoire a traiter. Pour etre en mesure de savoir le nombre
de scanners actives on introduit un compteur qui est initialize avec
le nombre totale des scanners et:

    - decremente lorque un scanner est en train de chercher un
      repertoire

    - incremente lorque un scanner a obtenu un nouveau repertoire a
      traiter

Pour garantir la terminaison des toutes les scanners, le premier
scanner a se terminer doit "liberer" tous les scanners inactives (en
attende de recevoir un repertoire) par le biais d'un signal.
L'algorithme a la lecture d'un repertoire peut etre ainsi resume:

    Obtenir repertoire ( queue ):
        Lock queue.mutex
        queue.scanners_actives -= 1

        tant que queue est vide:
            si queue.scanners_actives = 0:
                Broadcast signal queue.condition
                Unlock queue.mutex
                terminer
            sinon attendre queue.condition 

        element := queue.pop() 
        queue.scanners_actives += 1
        Unlock queue.mutex
        renvoyer element


2.2.2 Analyzer

Pour tester ses conditions d'arret un analyzer doit pouvoir verifier
si les scanners ont termine le traiter toutes les repertoires.  Pour
cette raison on introduit une variable qui indique si le traitement
des repertoires a ete complete et donc si le buffer des fichiers peut
recevoir ou non des nouveaux fichiers dans le futur. Cette variable
est initialise at 1 et mise at 0 par les scanners lorque le traitement
des repertoires a ete termine. Pour garantir la terminaison des
toutes les analyzers, le premier scanner a se terminer doit "liberer"
tous les analyzers en attende de recevoir un fichier par le biais d'un
signal.  Apres cette consideration le nouveau algorithme a la lecture 
d'un repertoire sera le suivant:

    Notifier analyzers (queue_files):
        Lock queue_files
        queue_files.va_recevoir = 0
        Broadcast signal queue_files.condition_lecture
        Unlock queue_files

    Obtenir repertoire ( queue_folders, queue_files ):
        Lock queue_folders.mutex
        queue_folders.scanners_actives -= 1

        tant que queue_folders est vide:
            si queue_folders.scanners_actives = 0:
                Broadcast signal queue_folders.condition
                Unlock queue_folders.mutex
                Notifier analyzers
                terminer
            sinon attendre queue_folders.condition 

        element := queue_folders.pop() 
        queue_folders.scanners_actives += 1
        Unlock queue_folders.mutex
        renvoyer element

En ce qui concerne les analyzers, il suffit de tester si le buffer des
fichiers est vede et prendre en compte le valeur de la variable
"va_recevoir" pour determiner la condition d'arret.

    Obtenir fichier ( queue_files ):
        Lock queue_files.mutex

        tant que queue_files est vide:
            si queue_files.va_recevoir = 0:
                terminer
            attendre queue_files.condition_lecture 

        element := queue_files.pop() 
        Signal queue_files.condition_ecriture
        Unlock queue_files.mutex
        renvoyer element
