Progetto di Reti di Calcolatori

*Chatroom TCP client / server multi-thread*

Introduzione
============

Il progetto consiste nella creazione di un modello client server per
scambi di messaggi testuali tra più utenti in diverse stanze.

Client e server utilizzano socket TCP per comunicare tra loro.

Il server gestisce più client utilizzando un modello multi-thread
chiamando la subroutine *handleClient* in un thread separato.

Dopodiché si mette in ascolto per connessioni in entrata all'indirizzo e
porta specificati nelle opzioni, o su quelle di default.

Usage: server \[-?\] \[-h IP\] \[-p PORT\] \[\--host=IP\]
\[\--port=PORT\]

![](./media/image1.png){width="5.905511811023622in"
height="0.7668908573928259in"}

Il client in multithread gestisce l'invio e la ricezione dei messaggi.

All'avvio del client l'utente tenta la connessione al server specificato
nelle opzioni.\
Se la connessione va a buon fine il server crea un utente con uid
univoco e username a scelta e lo inserisce nella stanza principale
"General".

Usage: client \[-?\] \[-d DOMAIN\] \[-h IP\] \[-p PORT\]

\[\--domain=DOMAIN\] \[\--host=IP\] \[\--port=PORT\]

![](./media/image2.png){width="5.905511811023622in"
height="0.7422025371828521in"}

Configurazione lato Server
==========================

Creazione socket
----------------

Il server crea una socket TCP e ci assegna l'interfaccia e la porta su
cui ascolterà.\
Di default indirizzo e porta vengono assegnati tramite delle define
*SERVER\_HOST* e *SERVER\_PORT* presenti in *"server.h"* altrimenti
vengono assegnati ai valori specificati come opzioni *---host* e
*---port* gestite dal parser tramite la funzione *parserArgv* definita
in *"parser.h".*

![Immagine che contiene testo Descrizione generata
automaticamente](./media/image3.png){width="6.775in"
height="2.463888888888889in"}

Opzioni socket
--------------

Tramite *setsockopt* vengono settate le opzioni della socket per la
dimensione del buffer di invio *SO\_SNDBUF* e di ricezione *SO\_RCVBUF.*

![Immagine che contiene testo Descrizione generata
automaticamente](./media/image4.png){width="6.775in"
height="1.1388888888888888in"}

Bind
----

Eseguo la *bind* della socket all'indirizzo e porta specificati e la
preparo ad accettare connessioni tramite *listen.*

![Immagine che contiene testo Descrizione generata
automaticamente](./media/image5.png){width="6.775in"
height="2.3229166666666665in"}

Accept
------

*accept* aspetta una connessione su *server\_sd* e quando la riceve apre
una nuova socket, *client\_sd* e assegna a *client\_addr* l'indirizzo
del client che si sta connettendo.

![Immagine che contiene testo Descrizione generata
automaticamente](./media/image6.png){width="6.775in"
height="1.1104166666666666in"}

Handle Client
-------------

Il server crea un nuovo utente assegnandogli l'indirizzo della
connessione ricevuta e la socket corrispondente, dopodiché crea un nuovo
thread chiamando la funzione *handleClient* che si occupa della gestione
dei messaggi, interpreta i comandi e inoltra i messaggi agli utenti
interessati.

![Immagine che contiene testo Descrizione generata
automaticamente](./media/image7.png){width="6.775in"
height="1.3381944444444445in"}

Configurazione lato Client
==========================

Creazione socket
----------------

Come nel server il client crea una socket TCP e ci assegna l'indirizzo e
la porta a cui dovrà connettersi. Di default indirizzo e porta vengono
assegnati tramite delle define *CONNECTION\_HOST* e *CONNECTION\_PORT*
presenti in *"client.h"* altrimenti vengono assegnati ai valori
specificati come opzioni *---host* ( o *---domain* che verrà tradotto in
un host tramite *resolveHostname* ) e *---port* gestite dal parser
tramite la funzione *parserArgv* definita in *"parser.h".*

![Immagine che contiene testo Descrizione generata automaticamente](./media/image8.png){width="6.775in" height="2.3006944444444444in"} Opzioni socket
-----------------------------------------------------------------------------------------------------------------------------------------------------

Tramite *setsockopt* vengono settate le opzioni della socket per la
dimensione del buffer di invio *SO*\_*SNDBUF* e di ricezione
*SO*\_*RCVBUF*

![Immagine che contiene testo Descrizione generata
automaticamente](./media/image9.png){width="6.775in"
height="1.1180555555555556in"}

Connect
-------

Il client apre una connessione verso il server.

![Immagine che contiene testo Descrizione generata
automaticamente](./media/image10.png){width="6.775in"
height="1.6826388888888888in"}

Send
----

Il server si aspetta di ricevere un username per creare l'utente.
Tramite la funzione *getCurrentStdin* e con gli opportuni controlli di
lunghezza dell'input preleviamo l'username da *stdin* e con *send* lo
mandiamo al server

![](./media/image11.png){width="6.775in" height="0.7152777777777778in"}

Message Handler
---------------

il client lancia due thread diversi per l'invio e la ricezione dei
messaggi in modo da poter gestire messaggi in arrivo durante l'invio di
un messaggio. Il thread di ricezione termina solo se il client riceve 0
byte da una lettura, evento che succede solo se il server crasha o
chiude la socket del client.

Tramite una *pthread*\_*join* si attende la terminazione del thread di
ricezione, dopodiché si interrompe l'esecuzione del programma, l'utente
chiude la socket ed esce dal server.

![Immagine che contiene testo Descrizione generata
automaticamente](./media/image12.png){width="6.775in"
height="1.895138888888889in"}

Implementazione chat
====================

Room structure
--------------

All'avvio il server inizializza la lista delle stanze inserendo una
stanza "General" e un utente "Admin" come amministratore della stanza di
default.

![](./media/image13.png){width="6.775in" height="0.5083333333333333in"}

*roomList* è un nodo sentinella che contiene il numero di stanze
presenti nel server, due puntatori alla testa e alla coda della lista e
un *mutex* per consentire l'accesso alla lista delle stanze in mutua
esclusione tra i vari thread, per evitare race conditions.

![Immagine che contiene testo Descrizione generata
automaticamente](./media/image14.png){width="6.775in"
height="1.9215277777777777in"}

Dopo l'inizializzazione la testa della lista punterà alla stanza
"General" che verrà usata come stanza di default, inserita nella lista
con la funzione *addRoom*.

Ogni stanza ha un uid univoco, un nome, un proprietario (l'utente che
crea la stanza), una lista di utenti, il nome del file di log da cui
recuperare la cronologia dei messaggi (verrà inizializzato più avanti
come "(room-\>name).log" e puntatori alla stanza precedente e successiva
nella lista.

![Immagine che contiene testo Descrizione generata
automaticamente](./media/image15.png){width="6.775in"
height="2.5243055555555554in"}

Alla creazione di una stanza la sua lista utenti viene inizializzata,
allocando memoria, tramite la funzione *initializeUserList*

![](./media/image16.png){width="6.775in" height="0.5465277777777777in"}

User Structure
--------------

*user\_list* è un nodo sentinella che contiene il numero di utenti
presenti nella stanza, due puntatori alla testa e alla coda della lista
e un *mutex* per consentire l'accesso alla lista degli utenti della
stanza in mutua esclusione tra i vari thread, per evitare race
conditions.

![Immagine che contiene testo Descrizione generata
automaticamente](./media/image17.png){width="6.775in"
height="1.9270833333333333in"}

Ogni utente un uid univoco, un username, un colore con cui verrà
visualizzato dagli altri utenti (inizializzato random tra una lista di
colori durante la creazione dell'utente), un socket file descriptor, una
struttura *sockaddr\_in* contenente l'indirizzo del client
corrispondente, un puntatore alla stanza a cui appartiene e puntatori
all'utente precedente e successivo nella lista.

![Immagine che contiene testo Descrizione generata
automaticamente](./media/image18.png){width="6.775in"
height="2.691666666666667in"}

Il server permette un certo numero di utenti, definito in
*MAX\_CLIENT\_COUNT*.

Alla connessione di un nuovo client, se il numero di utenti collegati o
che stanno tentando l'accesso supera *MAX\_CLIENT\_COUNT*, nega
l'accesso e chiude la socket.\
\
Il client, non ricevendo la conferma di connessione, saprà che il server
è pieno e terminerà l'esecuzione stampando un messaggio di errore.

![](./media/image19.png){width="5.905511811023622in"
height="0.4612051618547682in"}

![Immagine che contiene testo Descrizione generata
automaticamente](./media/image20.png){width="6.775in"
height="3.1034722222222224in"}All'accesso l'utente viene inserito nella
stanza "*General*".

SendMessage
-----------

La funzione *sendMessage* prende in input una stringa e un utente e
tramite la funzione *send* presente in *"socket.h"* prova ad inviarlo al
socket file descriptor dell'utente.

![Immagine che contiene testo Descrizione generata
automaticamente](./media/image21.png){width="6.775in"
height="2.841666666666667in"}

SendBroadcastMessage
--------------------

La funzione *sendBroadcastMessage* chiama *sendMessage* su tutti gli
utenti della stanza dell'utente che invia il messaggio, tranne che a sé
stesso, e tramite *keepLog* salva il messaggio nel file di log della
stanza.

![Immagine che contiene testo Descrizione generata
automaticamente](./media/image22.png){width="6.775in"
height="2.722916666666667in"}

Restore Log
-----------

Con il comando */restore* l'utente può ricevere dal server l'elenco di
tutti i messaggi mandati in quella stanza prima che vi entrasse.

![Immagine che contiene testo Descrizione generata
automaticamente](./media/image23.png){width="6.775in"
height="3.2979166666666666in"}

Print Room List
---------------

Con il comando /rooms l'utente può stampare la lista di stanza
disponibili con i vari utenti connessi. La propria stanza e il proprio
username verranno evidenziati nel messaggio.

![](./media/image24.png){width="5.905511811023622in"
height="1.5265944881889764in"}

New Room
--------

Con il comando /*newroom* l'utente può creare una nuova stanza di cui
sarà proprietario, e che quindi solo lui potrà eliminare.

Il server accetta un massimo numero di stanze definito *in
MAX\_ROOM\_COUNT.*

Alla creazione di una nuova stanza da parte di un utente il
*cleanEmptyRooms* controlla il numero di stanze già presenti e se è
superiore cerca una stanza vuota da eliminare.\
Se il server ha già raggiunto il numero di stanze massimo tutte
contengono almeno un utente non sarà possibile creare una nuova stanza.

![Immagine che contiene testo Descrizione generata
automaticamente](./media/image25.png){width="6.775in"
height="4.509027777777778in"}

Change Room
-----------

L'utente inoltre può scegliere una delle stanze esistenti create da
altri utenti con il comando /changeroom tramite l'uid univoco della
stanza. Al client verrà chiesto di inserire il numero uid della stanza
in cui vuole entrare, dopo avergli mostrato la lista completa delle
stanze disponibili. Il server interpreta il messaggio ricevuto
controllando che sia un numero valido e che corrisponda ad una stanza
esistente. L'utente, quindi, verrà rimosso dalla stanza a cui
appartiene, notificando gli altri utenti della cosa, e verrà inserito
nella stanza da lui scelta.

![Immagine che contiene testo Descrizione generata
automaticamente](./media/image26.png){width="6.775in"
height="3.3159722222222223in"}

Delete Room
-----------

Il proprietario di una stanza può decidere di eliminarla con il comando
*/deleteroom* e di spostare automaticamente tutti gli utenti connessi
nella stanza generale.

L'admin può eliminare ogni stanza. Nessuno, compreso l'admin, può
eliminare la stanza di default.

![Immagine che contiene testo, monitor, nero, screenshot Descrizione
generata automaticamente](./media/image27.png){width="6.775in"
height="2.6979166666666665in"}

Exit
----

Tramite il comando */exit* (o se il client viene interrotto mediante
segnali di *SIGINT, SIGTSTP* o *SIGQUIT)*, l'utente viene disconnesso
dal server. Durante la fase di logOut viene rimosso dalla stanza in cui
è attualmente e la proprietà delle stanze che possiede verrà trasferita
all'utente Admin

![Immagine che contiene testo Descrizione generata
automaticamente](./media/image28.png){width="6.775in"
height="3.698611111111111in"}

Sommario {#sommario .Titolosommario}
========

[[Introduzione]{.underline} 1](#introduzione)

[[Configurazione lato Server]{.underline}
2](#configurazione-lato-server)

[[Creazione socket]{.underline} 2](#creazione-socket)

[[Opzioni socket]{.underline} 2](#opzioni-socket)

[[Bind]{.underline} 2](#bind)

[[Accept]{.underline} 3](#accept)

[[Handle Client]{.underline} 3](#handle-client)

[[Configurazione lato Client]{.underline}
4](#configurazione-lato-client)

[[Creazione socket]{.underline} 4](#creazione-socket-1)

[[Opzioni socket]{.underline}
4](#immagine-che-contiene-testo-descrizione-generata-automaticamente-opzioni-socket)

[[Connect]{.underline} 4](#connect)

[[Send]{.underline} 5](#send)

[[Message Handler]{.underline} 5](#message-handler)

[[Implementazione chat]{.underline} 6](#implementazione-chat)

[[Room structure]{.underline} 6](#room-structure)

[[User Structure]{.underline} 7](#user-structure)

[[SendMessage]{.underline} 8](#sendmessage)

[[SendBroadcastMessage]{.underline} 9](#sendbroadcastmessage)

[[Restore Log]{.underline} 9](#restore-log)

[[Print Room List]{.underline} 10](#print-room-list)

[[New Room]{.underline} 10](#new-room)

[[Change Room]{.underline} 11](#change-room)

[[Delete Room]{.underline} 11](#delete-room)

[[Exit]{.underline} 12](#exit)

[\
\
\
]{.underline}
