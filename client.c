#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <ncurses.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#define DEFAULT_PIPE "server"
#define ALTO 4

WINDOW *ventanaOutput, *ventanaInput;

int readLine(int fd, char *str) {
        int n;
        do {
                n = read(fd, str, 1);
        } while(n > 0 && *(str++) != '\0');
return(n > 0);
}

void send_hello(char * name, char * pipe){
	int fd, messagelen,i;
	char name_serv[55];
 	char message[150];
	unlink(name);
        mknod(name, S_IFIFO, 0);
        chmod(name, 0660);
        strcpy(name_serv, name);
        strcat(name_serv, "_serv");
        unlink(name_serv);
        mknod(name_serv, S_IFIFO, 0);
        chmod(name_serv, 0660);
	sprintf(message, "New user: %s", name);
 	fd = open(pipe, O_WRONLY);
	messagelen = strlen(message) + 1; 
   	write(fd, message, messagelen);
 	close(fd);
}

void limpiarVentanaInput(){
        wclear(ventanaInput);
        mvwhline(ventanaInput, 0, 0, 0, COLS);
        wmove(ventanaInput, 1, 0);
        wrefresh(ventanaInput);
}

void enfocarInput(){
	int x,y;
	getyx(ventanaInput, y,x);
	wmove(ventanaInput, y,x);
	wrefresh(ventanaInput);
}

void term_handler(){
	//printf("It worked");
	endwin();
	exit(0);
}

void main(int argc, char * argv[]){
	int file, max_desc, desc[2];
	char name[50], to_server[50], message[306], command[256],pipe[100];
	char probando[200], c[2];
	fd_set lectura;
	fd_set c_lectura;
	struct timeval tv;
	int retval,status,i,alto,x,y,salir = 0;
	if (signal(SIGINT, term_handler) == SIG_ERR) {
        	printf("An error occurred while setting a signal handler.\n");
    	}

	// Interfaz
	initscr();
	alto = LINES - ALTO;
	ventanaOutput = newwin(alto, 0, 0, 0);
	ventanaInput = newwin(ALTO, 0, alto, 0);
	nonl();
	cbreak();
	noecho();
	notimeout(ventanaInput, TRUE);
	nodelay(ventanaInput, TRUE);
	scrollok(ventanaOutput, TRUE);
	scrollok(ventanaInput, TRUE);
	limpiarVentanaInput();
	// Fin Interfaz

	strcpy(name,getlogin());
	strcpy(pipe, DEFAULT_PIPE);
	if (argc > 1) {
		for (i = 1; i < argc; i ++){
			if ( strcmp(argv[i], "-p") == 0) {
				strcpy(pipe, argv[i+1]);
				i++;
			} else {
				strcpy(name,argv[i]);
			}
		}
	}
	send_hello(name, pipe);
	strcpy(to_server, name);
	strcat(to_server, "_serv");
	do {
		desc[0] = open(name, O_RDONLY | O_NONBLOCK);
	} while (desc[0] == -1);
	//desc[1] = 0;	
	FD_ZERO(&lectura);
	FD_SET(desc[1], &lectura);
	FD_SET(desc[0], &lectura);
	max_desc = desc[0]+1;
	//printf("Connecting to the server...\n");
	c[1] = '\0';
	memset(probando,0,200);
	while(!salir){
		//wprintw(ventanaOutput, "mensaje");
	//	wrefresh(ventanaOutput);
		tv.tv_sec = 0;
		tv.tv_usec = 10000;
		c_lectura = lectura;
		if (select(max_desc, &c_lectura, NULL, NULL, &tv) < 0)
			perror("SELECT FAILED");
		for (i = 0; i < 1; i++){
			if( FD_ISSET(desc[i], &c_lectura)){
				memset(message, 0, sizeof(message));
				status = read(desc[i], message, sizeof(message));
				//status = readLine(desc[i],message);
				if (status){	
					if ( i == 1) {
						//Envio mensaje al servidor
						//printf("Mensaje enviado al servidor: %s", message);
					//	if ( message[0] == '\n' ) {
					//		sprintf(command, "%s %s", name, message);
					//		file = open(to_server, O_WRONLY | O_NDELAY);
					//		if ( file == -1 )
					//			perror("error");
					//		write(file, command, strlen(command)+1);
					//		wprintw(ventanaOutput, "%s", command);
					//		wrefresh(ventanaOutput);
					//		enfocarInput();
							salir = strcmp(message,"-exit\n") == 0;
					//		limpiarVentanaInput();
					//	} else {
					//		enfocarInput();
					//	}
					} else {
						// Es un mensaje del servidor
						//printf("%s", message);
						//fflush(stdout);
						wprintw(ventanaOutput, "%s", message);
						wrefresh(ventanaOutput);
						enfocarInput();
					}
				} else {
					//printf("NO STATUS\n");
				}
				}	
			}

					
		c[0] = wgetch(ventanaInput);
                if (c[0] == '\r'){
			sprintf(command, "%s %s\n", name, probando);
			file = open(to_server, O_WRONLY | O_NDELAY);
			if ( file == -1 )
				perror("error");
			write(file, command, strlen(command)+1);
                        wprintw(ventanaOutput, "<%s> %s\n", name, probando);
                        wrefresh(ventanaOutput);
                        memset(probando,0,200);
                        limpiarVentanaInput();
			enfocarInput();
                } else if (c[0] == 127) {
			getyx(ventanaInput,y,x);
			wmove(ventanaInput,y,--x);
			probando[(y-1)*COLS+x] = '\0';
			wdelch(ventanaInput);
		} else if (c[0] == '\033'){
			wgetch(ventanaInput);
			wgetch(ventanaInput);
		} else {			
			if (c[0] != ERR){
			getyx(ventanaInput,y,x);
			wprintw(ventanaInput,"%c", c[0]);
			wmove(ventanaInput,y,++x);
			wrefresh(ventanaInput);
                        strcat(probando,c);
			} 
                }
	}
	endwin();
	unlink(name);
	unlink(to_server);
	exit(0);

}
