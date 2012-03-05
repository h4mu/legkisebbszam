/***************************************************************************
 *   Copyright (C) 2006 by H??mor Tam??s   *
 *   hamuguy@gmail.com   *
 *                                                                         *
 *   Permission is hereby granted, free of charge, to any person obtaining *
 *   a copy of this software and associated documentation files (the       *
 *   "Software"), to deal in the Software without restriction, including   *
 *   without limitation the rights to use, copy, modify, merge, publish,   *
 *   distribute, sublicense, and/or sell copies of the Software, and to    *
 *   permit persons to whom the Software is furnished to do so, subject to *
 *   the following conditions:                                             *
 *                                                                         *
 *   The above copyright notice and this permission notice shall be        *
 *   included in all copies or substantial portions of the Software.       *
 *                                                                         *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       *
 *   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    *
 *   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. *
 *   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR     *
 *   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, *
 *   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR *
 *   OTHER DEALINGS IN THE SOFTWARE.                                       *
 ***************************************************************************/

#include "lksz.h"

static int kilepes = 0;
/*static int jatekosok = 0;*/
static struct jatekos {
	int kapcs;
	unsigned long long szam;
	char nev[PARMLEN];
} jatekosok[MAXCONN];


static const char segitseg[] = "\n\033[7mAdminisztrátor menü parancsai:\033[0m\n"
			"\th - jelen segélyképerny?\n"
			"\tl - a bejelentkezettek kilistázása\n"
			"\tk - aktív kapcsolatok száma\n"
			"\tv - a kör vége\n"
			"\tq - a szerverfolyamat leállítása\n";

static int cmp(const void *a, const void *b){
	if(((struct jatekos*)a)->szam < ((struct jatekos*)b)->szam)
		return -1;
	else if(((struct jatekos*)a)->szam == ((struct jatekos*)b)->szam)
		return 0;
	else
		return 1;
}

void konzolkomm(){
	int i, j;
	char buffer[MSGLEN];
	switch(getchar()){
	case 'q':
	case 'Q':
		kilepes=1;
		break;
	case 'h':
	case 'H':
		printf(segitseg);
		break;
	case 'v':
	case 'V':
		printf("\nA körnek vége.\n");
		qsort(jatekosok, MAXCONN, sizeof(struct jatekos), cmp);
		for(i=j=0; i<MAXCONN; i++)
			if(	jatekosok[i].szam>0
				&& jatekosok[i].kapcs!=INVALID 
				&& jatekosok[i].nev[0]!='\0' 
				&& jatekosok[j].szam!=jatekosok[i].szam) { /* a homogen sorozatokat atugorjuk */
				if(i-j!=1)
					j=i; /* uj sorozat kezdete */
				else
					i=MAXCONN+1; /* j = a legkisebb egyeduli szam indexe */
			}
		if(j==0)
			printf("Nincs Nyertes!\n");
		else {
			printf("\nA gy?ztes a '%s' nev? versenyz? a %llu számmal!\n\n",
				jatekosok[j].nev, jatekosok[j].szam);
			sprintf(buffer, "LEGKISEBB-GYOZTES %s %llu\13\10",
				jatekosok[j].nev, jatekosok[j].szam);
#ifdef DEBUG
			puts(buffer);
#endif
			for(i=0; i<MAXCONN; i++)
				if(	jatekosok[i].szam>0
					&& jatekosok[i].kapcs!=INVALID 
					&& jatekosok[i].nev[0]!='\0')
					write(jatekosok[i].kapcs, buffer, MSGLEN);

		}
		printf("Új kör kezdése.\n");
		break;
	case 'l':
	case 'L':
		printf("\n\n");
		for(i=0; i<MAXCONN; i++)
			if(jatekosok[i].kapcs!=INVALID && jatekosok[i].nev[0]!='\0')
				printf("Sorszám: %d, név: %s, szám: %llu\n", i, jatekosok[i].nev, jatekosok[i].szam);
		break;
	case 'k':
	case 'K':
		for(i=j=0; i<MAXCONN; i++)
			if(jatekosok[i].kapcs!=INVALID)
				j++;
		printf("\n%d kapcsolat akív\n", j);
		break;
	}
}

void ujkapcsolat(const int sock){
	int i;
	
	for(i=0; i<MAXCONN; i++)
		if(jatekosok[i].kapcs==INVALID) {
			if(!(jatekosok[i].kapcs = accept(sock, NULL, NULL)))
				perror("\033[7mHibás accept()\033[0m\n");
			i=MAXCONN+1;
		}
	if (i<MAXCONN+1)
		perror("\033[7mNem lehet több kapcsolatot létesíteni!\033[0m\n");
}

void halokomm(const int i){
	char buffer[MSGLEN];
	int j=0, status=0;

	bzero(buffer, MSGLEN);
	if(!read(jatekosok[i].kapcs, buffer, MSGLEN)){
		printf("\033[7mMegszakadt a kapcsolat!\033[0m\n");
		close(jatekosok[i].kapcs);
		jatekosok[i].kapcs=INVALID;
		jatekosok[i].nev[0]='\0';
		jatekosok[i].szam=0;
	} else {
#ifdef DEBUG
		puts(buffer);
#endif
		if(0<(status=sscanf(buffer, "LEGKISEBB-BEJELENTKEZES %s\13\10", jatekosok[i].nev))){
			for(j=0; j<MAXCONN; j++)
				if(j!=i && strcmp(jatekosok[i].nev, jatekosok[j].nev)==0){
					sprintf(buffer, "LEGKISEBB-ROSSZNEV\13\10");
#ifdef DEBUG
					puts(buffer);
#endif
					write(jatekosok[i].kapcs, buffer, MSGLEN);
					jatekosok[i].nev[0]='\0';
					jatekosok[i].szam=0;
					j=MAXCONN;
				}
		} else {
			status=sscanf(buffer, "LEGKISEBB-MEGAD %llu\13\10", &(jatekosok[i].szam));
			if(jatekosok[i].szam==0)
				jatekosok[i].szam=1;	/* a kliens elvileg ilyet nem adhat */
		}
		printf("\n%d. nev: %s, szam: %llu, kapcs: %d\n", i, jatekosok[i].nev, jatekosok[i].szam, jatekosok[i].kapcs);
	}
}

int main(int argc, char *argv[])
{
	unsigned short port=12345;
	int sock, i;
	struct sockaddr_in cim;
	struct timeval tv;
	fd_set fds;
	char sor[5];
	
	printf("\033[7mLegkisebb Szám server v.0.01\033[0m\n"
	"\nKérem adja meg, "
	"hogy a program mely TCP porton várja a kapcsolatokat ");
	do{
		printf("\n(1024 és 65535 közötti egész, alapesetben 12345):\n");
		fgets(sor, 5, stdin);
		sscanf(sor, "%hu", &port);
	}while(port<1024);
	printf("\nA használt port: %d\n", port);

	if((sock=socket(AF_INET, SOCK_STREAM, 0))<0) {
		perror("\033[7mNem sikerült socket-et létrehozni!\033[0m\n");
		exit(-1);
	}

	fcntl(sock, F_SETFL, fcntl(sock,F_GETFL) | O_NONBLOCK);

	memset((char*) &cim, 0, sizeof(cim));
	cim.sin_addr.s_addr = htonl(INADDR_ANY);
	cim.sin_family = AF_INET;
	cim.sin_port = htons(port);

	if(bind(sock, (struct sockaddr *) &cim, sizeof(cim))<0) {
		perror("\033[7mHiba a bind()-el!\033[0m\n");
		exit(1);
	}
	printf("bind() kész.\n");

	if(listen(sock,2)<0){
		perror("\033[7mHiba a listen()-el!\033[0m\n");
		exit(1);
	}
	printf("listen() kész.\n");

	for(i=0; i<MAXCONN; i++)
		jatekosok[i].nev[0] = jatekosok[i].kapcs = jatekosok[i].szam = INVALID;

	printf(segitseg);

	while(!kilepes){	
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		FD_ZERO(&fds);
		FD_SET(sock, &fds);
		FD_SET(STDIN, &fds);
		for(i=0; i<MAXCONN; i++)
			if(jatekosok[i].kapcs != INVALID)
				FD_SET(jatekosok[i].kapcs, &fds);

		/* A select meghivasa. Ez akkor ter vissza, ha a masodik 
		argumentumban megadott socketekbol lehet olvasni (vagy ha 
		bontodott a kapcsolat, akkor egy EOF olvashato), vagy ha a 
		timeout lejart. */
		i = select(FD_SETSIZE, &fds, (fd_set *) 0, (fd_set *) 0, &tv);
		
		/* select() visszaterte utan modositja a halmazt ugy, hogy 
		csak azok a socketek vannak az fds parameterben, 
		amelyekbol lehet olvasni anelkul, hogy blokkolna a 
		programunkat. Figyeljunk arra, hogy a Linux megvaltoztatja 
		a timeout strukturat, amlyre a hivas utan definialatlankent 
		kell tekintenunk! */
		if(i) {
			if(FD_ISSET(sock, &fds))
				ujkapcsolat(sock);
			if(FD_ISSET(STDIN, &fds)) {
				konzolkomm();
				FD_CLR(STDIN, &fds);
			}
			for(i=0; i<MAXCONN; i++)
				if(FD_ISSET(jatekosok[i].kapcs, &fds))
					halokomm(i);
		}
	}
	
	close(sock);
	return EXIT_SUCCESS;
}
