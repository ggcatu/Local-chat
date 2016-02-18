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

#define PIPE_COM "/tmp/12-11006"
#define DEFAULT_PIPE "server_c"
#define ALTO 4

WINDOW *ventanaOutput, *ventanaInput;
int name_dec, server_dec;
char name[100], to_name[140] , to_server[140];

// Cierra el cliente.
void close_client(){
	close(3);
	close(4);
	if ( unlink(to_name) == 1 ) { perror("unlink closing");}
	if ( unlink(to_server) == 1 ) { perror("unlink closing server");}
	endwin();
}

void send_hello(char * pipe){
	int fd, messagelen;
 	char message[150];
	struct stat st = {0};

	// Chequear carpeta
	if (stat(PIPE_COM, &st) == -1) {
    		if ( mkdir(PIPE_COM, 0777) == -1 ) { perror("mkdir error"); };
	}
	sprintf(to_name, "%s/%s", PIPE_COM, name);
	unlink(to_name);

	// Crear pipe para recibir
        if ( mknod(to_name, S_IFIFO, 0) == 1 ) { perror("error mknod receive");}
        if ( chmod(to_name, 0660) == 1 ) { perror("error chmod receive");}
	sprintf(to_server, "%s/%s_serv",PIPE_COM, name);
        unlink(to_server);

	// Crear pipe para enviar
        if ( mknod(to_server, S_IFIFO, 0) == 1 ) { perror("error mknod send");}
        if ( chmod(to_server, 0660) == 1 ) { perror("error mknod send");}
	sprintf(message, "New user: %s", name);
	
	// Conexion con pipe de entrada al servidor
	fd = open(pipe, O_WRONLY | O_NONBLOCK);
 	if ( fd == -1) {
		close_client();
		printf("No se ha podido conectar al servidor [%s]\n", pipe);
		exit(1);
	};
	messagelen = strlen(message) + 1; 
	name_dec = open(to_name, O_RDONLY | O_NONBLOCK);
	if ( name_dec == -1 ) { perror("name_dec failed"); }
   	if ( write(fd, message, messagelen) == -1 ) { perror("error sending hello"); }
	do { server_dec = open(to_server, O_WRONLY | O_NONBLOCK); } while (server_dec == -1);
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
	char tmp[50];
	sprintf(tmp, "%s -salir", name);
	if ( write(server_dec, tmp, strlen(tmp)+1) == 1 ) { perror("write -salir handler");}
	close_client();
	exit(0);
}


void main(int argc, char * argv[]){
	// Declaraciones
	int max_desc, desc[2];
	char message[306], command[256],pipe[100], cmd[200];
	char buffer[200] = "", c[2] = {0,'\0'};
	fd_set lectura;
	fd_set c_lectura;
	struct timeval tv;
	int status,i,alto,x,y,salir = 0;
	
	// Inicializar senales.
	if (signal(SIGINT, term_handler) == SIG_ERR) {
        	printf("Ocurrio un error al colocar la signal.\n");
    	}

	// Interfaz.
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

	// Obtener usuario.
	strcpy(name,getlogin());
	if ( name == "" ) { perror("Error getting name"); }	

	// Obtener pipe.
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

	// Identificarse con el servidor.
	send_hello(pipe);
	wprintw(ventanaOutput, "Conectando al servidor. [%s]\n", pipe);
	wrefresh(ventanaOutput);
	enfocarInput();
	
	// Inicializando select
	desc[0] = name_dec;
	FD_ZERO(&lectura);
	FD_SET(desc[0], &lectura);
	max_desc = desc[0]+1;

	// Inicializando manejador de input.
	while(!salir){
		tv.tv_sec = 0;
		tv.tv_usec = 10000;
		c_lectura = lectura;
		if (select(max_desc, &c_lectura, NULL, NULL, &tv) < 0)
			perror("Error en el select");
		for (i = 0; i < 1; i++){
			if( FD_ISSET(desc[i], &c_lectura)){
				status = read(desc[i], message, sizeof(message));
				if (status){
					// Servidor esta lleno
					if ( strcmp(message,"NO_MORE_ROOM") == 0 ) {
						close_client();
						printf("El servidor esta lleno, intente de nuevo.\n");
						exit(1);
					}
					// Nombre ya utilizado
					if ( strcmp(message,"NAME_USED") == 0 ) {
                                                close_client();
                                                printf("Ya existe alguien en el chat con ese nombre.\n");
                                                exit(1);
                                        }
					wprintw(ventanaOutput, "%s", message);
					wrefresh(ventanaOutput);
					enfocarInput();
				} else {
					close_client();
					printf("El sevidor se ha cerrado.\n");
					exit(1);
				}
				memset(message, 0, sizeof(message));
			}	
		}

		//  Manejador de input por teclado.			
		c[0] = wgetch(ventanaInput);
                if (c[0] == '\r'){
			// Es un mensaje al servidor.
			if ( buffer[0] != '\0')  {
				sprintf(command, "%s %s\n", name, buffer);
				if ( write(server_dec, command, strlen(command)+1) == -1 ) { perror("write sending_message"); }
				wprintw(ventanaOutput, "<%s> %s\n", name, buffer);
				wrefresh(ventanaOutput);
				sscanf(buffer, "%s %s", cmd, buffer);
				salir = strcmp(cmd,"-salir") == 0;
				memset(buffer,0,200);
				limpiarVentanaInput();
				enfocarInput();
			}
                } else if (c[0] == 127) {
			// Se quiere borrar un caracter.
			getyx(ventanaInput,y,x);
			if (x == 0) { if (y > 1){ x = COLS;  --y; } } 
			wmove(ventanaInput,y,--x);
			buffer[(y-1)*COLS+x] = '\0';
			wdelch(ventanaInput);
		} else if (c[0] == '\033'){
			// Deshabilita las flechas del teclado.
			wgetch(ventanaInput);
			wgetch(ventanaInput);
		} else {
			if (strlen(buffer) < 256) {
			if (c[0] != ERR){
			// Formar mensaje.
				getyx(ventanaInput,y,x);
				wprintw(ventanaInput,"%c", c[0]);
				wmove(ventanaInput,y,++x);
				wrefresh(ventanaInput);
				strcat(buffer,c);
			}
		 	}
                }
	}

	close_client();	
	printf("Esperamos que vuelva a chatear pronto!\n");
	exit(0);
}
