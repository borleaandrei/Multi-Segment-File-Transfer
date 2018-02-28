# Transfer multisegment de fisiere

## Server

Structura unei comenzi: <b>/nume_comanda param1 param2 ...</b><br>

Comanda : {raspuns1, ..., raspunsN}
- <b>exista nume_fisier</b> : {0, N}
	- 0 in caz ca fisierul nu exista pe server
	- N - marimea in bytes a fisierului
- <b>numar_canale_disponibile</b> : N
	- N - numarul de canale de transfer ramase a putea fi deschise (N >= 0), (?? fiecare server putand avea la un momentdat cel mult <100?> conexiuni de transfer deschise ??).
- <b>descarca nume_fis nr_seg marime_seg1 adr_inceput_seg1 ... marime_segN adr_inceput_segN</b> : {-1, nr_porturi nr_port1 sha256_seg1 ... nr_portN sha256_segN}
	- -1 - eroare: exista constrangerea numar_segmente <= numar_canale_disponibile. Se va returna o eroare daca nu este respectata aceasta constrangere - fie ca clientul nu a tinut cont de constrangerea anterioara, fie au fost alocate canale altui client intre timp.
	- nr_porturi nr_port1 sha256_seg1 ... nr_portN sha256_segN- numarul porturilor, numarul fiecarui port deschis pentru fiecare segment si checksum pentru fiecare segment, in ordinea ceruta(port1 - segment1 sha256_seg1 ... portN - segmentN sha256_segN) 
		- se deschid numar_segment canale de comunicatie cu serverul, urmand ca pe fiecare astfel de canal sa se transfere un segment specificat.
		- clientul va salva fiecare segment intr-un fisier diferit, iar cand vor fi descarcate toate segmentele (de pe oricate servere), va uni toate aceste fisiere intr-unul. 
		- serverul va deschide nume_fisier de numar_segment ori in read-only mode, transmitand fiecare astfel de segment pe port ul specificat anterior
		- va fi verificata integritatea fiecarui segment preluat.
- <b>sha256 nume_fisier</b> : sh256sum_nume_fisier
	- sh256sum_nume_fisier - checksum sha256 pentru nume_fisier
	- #serverul va returna sh256sum al nume_fisier, pentru a se verifica integritatea finala a transferului. (Se poate cere numai de la un server sha256, sau de la toate pentru o extra comparare.)
- <b>lista_fisiere</b> : numar_fisiere_disponibile lista_fisiere
	- numar_fisier_disponibil - ...
	- lista_fisiere - numar_fisiere_disponibile stringuri separate printr-un spatiu

## Client

Comenzi posibile:
- <b>descarca nume_fisier numar_segment</b> : {NUME_FISIER_NU_EXISTA, EROARE_CHECKSUM_FISIER, SUCCES}
	- NUME_FISIER_NU_EXISTA - fisierul cerut nu este disponibil pe nici un server din fisierul de configuratie
	- EROARE_CHECKSUM_FISIER - ..., se intreaba daca se reincearca transferul. Daca transferul a esuat doar pe un segment, se reincearca de (~3 ori? de la alt server?) descarcarea, altfel se intoarce eroare
	- SUCCES - ...
- <b>fisiere_disponibile</b> : N fisier1 ... fisierN
	- N fisier1 ... fisierN - numarul de fisiere si numele fiecaruia separat printr-un spatiu