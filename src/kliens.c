/***************************************************************************
 *   Copyright (C) 2006 by H�mor Tam�s   *
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


static int kilepes = 0, szammegadva = 0, bejelentkezve=0;
static const char segitseg[]="\nnyomjon 'q'-t a kil�p�shez!\n";
static char nev[51];

void konzolkomm(){
	switch(getchar()){
	case 'q':
	case 'Q':
		kilepes=1;
		break;
	case 'h':
	case 'H':
		printf(segitseg);
		break;
	}
}

void halokomm(int sock){
	char buffer[MSGLEN], gyoztes[PARMLEN];
	unsigned long long joszam=0;

	bzero(buffer, MSGLEN);
	if(!read(sock, buffer, MSGLEN)){
		printf("\033[7mA kapcsolat megszakadt, kil�p�s.\033[0m\n");
		kilepes=1;
	} else {
#ifdef DEBUG
		puts(buffer);
#endif	
		if(strcmp(buffer, "LEGKISEBB-ROSSZNEV\13\10")==0){
			printf("\nHiba! A kiszolg�l� elutas�totta a bejelentkez�si k�relm�t!\n\n");
			bejelentkezve = szammegadva = 0;
		}else if(sscanf(buffer, "LEGKISEBB-GYOZTES %50[!-?A-~] %llu\13\10", gyoztes, &joszam)){
			printf("\nV�ge a k�rnek.\n");
			if(strcmp(gyoztes, nev)==0)
				printf("\n\n\033[7mGratul�lok! �n nyert a %llu sz�mmal!\033[0m\n\n", joszam);
			else
				printf(	"\n\nSajnos �n most nem nyert.\n"
					"A '%s' nev� j�t�kos nyert a %llu sz�mmal.\n\n",
					gyoztes, joszam);
			szammegadva=0;
		}
	}
}

int main(int argc, char *argv[])
{
	unsigned short port=12345;
	unsigned long long szam;
	int sock, i;
	struct timeval tv;
	struct hostent* p_host;
	struct sockaddr_in addr;
	struct in_addr* p_ipaddr;
	fd_set fds;
	char *sor, hostnev[256]="localhost";

	srand((unsigned)time(NULL));

	printf("\033[7mLegkisebb Sz�m kliens v.0.01\033[0m\n"
		"\nK�rem adja meg a kiszolg�l� c�m�t (alapesetben 'localhost'):\n");

	sor=(char*)malloc(256);
	fgets(sor, 256, stdin);
	sscanf(sor, "%255[0-9A-Za-z.-]", hostnev);

	printf("\n\nAdja meg a kiszolg�l� portsz�m�t (1024 �s 65535 k�z�tti eg�sz, alapesetben 12345):\n");
	fgets(sor, 6, stdin);
	sscanf(sor, "%hu", &port);

	sor=(char*)realloc((void*)sor, MSGLEN);
	printf("\nn�v: %s, kiszolg�l� host: %s, kiszolg�l� port: %d\n", nev, hostnev, port);

	p_host = gethostbyname(hostnev);
	if(p_host==NULL){
		perror("\033[7mSikertelen n�vfelold�s.\033[0m\n");
		return -1;
	}
	p_ipaddr=(struct in_addr *)(p_host->h_addr);
	printf("Kiszolg�l� IP cim: %s\n",inet_ntoa(*p_ipaddr));
	

	if((sock=socket(AF_INET, SOCK_STREAM, 0))<0) {
		perror("\033[7mNem siker�lt socket-et l�trehozni!\033[0m\n");
		exit(-1);
	}


	/* Az addr lenullazasa. A sockaddr_in struktura (az addr tipusa)
	a sockaddr struktura helyere van definialva, es ott a nem
	hasznalt byte-okat 0 ertekkel kell feltolteni.  */
	memset(&addr, 0, sizeof(addr));   

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr = *p_ipaddr;
  
	if(connect(sock, (struct sockaddr*)&addr, sizeof(addr))!=0){
		perror("\033[7mNem siker�lt kapcsol�dni!\033[0m\n");
		return -1;
	}

	printf(segitseg);

	fcntl(sock, F_SETFL, fcntl(sock,F_GETFL) | O_NONBLOCK);

	while(!kilepes){

		if(!bejelentkezve){
			sprintf(nev, "anonim%u", (int) (999.0 * (rand()/(RAND_MAX+1.0))));
			printf("\n\nMost be�rhatja felhaszn�l�i nev�t (k�l�nben 'anonim' kezdet� lesz a neve):\n");
			fgets(sor, PARMLEN, stdin);
			sscanf(sor, "%50[!-?A-~]", nev);
			printf("\nA '%s' n�v haszn�lata.\n", nev);
			sprintf(sor, "LEGKISEBB-BEJELENTKEZES %s\13\10", nev);
			write(sock, sor, MSGLEN);
			bejelentkezve=1;
		}

		if(!szammegadva){
			i = 1 + (int) (400000000.0 * (rand()/(RAND_MAX+1.0)));
			printf("\nK�rem adja meg a tippj�t (pozit�v eg�sz sz�m, alap�rtelmez�sben %u):\n", i);
			fgets(sor, 21, stdin); /* 64 bites architekturan */
			if(0>=sscanf(sor, "%llu", &szam) || szam==0)
				szam=i;
			printf("\nA %d sz�m haszn�lata.\n", szam);
			sprintf(sor, "LEGKISEBB-MEGAD %llu\13\10", szam);
			write(sock, sor, MSGLEN);
			szammegadva=1;
		}

		tv.tv_sec = 1;
		tv.tv_usec = 0;

		FD_ZERO(&fds);
		FD_SET(sock, &fds);
		FD_SET(STDIN, &fds);


		/* A select meghivasa. Ez akkor ter vissza, ha a masodik 
		argumentumban megadott socketekbol lehet olvasni (vagy ha 
		bontodott a kapcsolat, akkor egy EOF olvashato), vagy ha a 
		timeout lejart. */
		i = select(sock+1, &fds, (fd_set *) 0, (fd_set *) 0, &tv);
		
		/* select() visszaterte utan modositja a halmazt ugy, hogy 
		csak azok a socketek vannak az fds parameterben, 
		amelyekbol lehet olvasni anelkul, hogy blokkolna a 
		programunkat. Figyeljunk arra, hogy a Linux megvaltoztatja 
		a timeout strukturat, amlyre a hivas utan definialatlankent 
		kell tekintenunk! */
		if(i) {
			if(FD_ISSET(sock, &fds))
				halokomm(sock);
			if(FD_ISSET(STDIN, &fds))
				konzolkomm();
		}
	}

	free(sor);
	close(sock);
	return EXIT_SUCCESS;
}
