/*
Sistemas de Operacion I (CI-3825)

Proyecto I: Chat

Autores:
Guillermo Betancourt, carnet 11-10103
Gabriel Giménez, carnet 12-11006

cliente.c:
Contiene la logica principal del cliente.
Mediante un pipe de conexiones entrantes el cliente se comunica
con el servidor para indicarle su nombre con el que posteriormente 
se identificara, y servira para establecer la conexion.
*/

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
#define LINES_MIN 10
#define COLS_MIN 25
#define ALTO 4
#define MAX_MESSAGE_LEN 256
#define MAX_PIPE_LEN 100
#define MAX_NAME_LEN 100
#define MAX_FULLM_LEN (MAX_NAME_LEN + MAX_MESSAGE_LEN)
#define MAX_FULLP_LEN (MAX_NAME_LEN + MAX_PIPE_LEN)

// Declaraciones globales, una vez inicializados no son cambiados.
WINDOW *ventanaOutput, *ventanaInput;
int name_dec, server_dec;
char name[MAX_NAME_LEN], to_name[MAX_FULLP_LEN + 6] , to_server[MAX_FULLP_LEN + 6];

// Cierra el cliente correctamente.
void close_client();
// Permite la identificacion del cliente con el servidor.
void send_hello(char *);
// Limpia la interfaz de escritura del cliente.
void limpiarVentanaInput();
// Enfoca la ventana de input, una vez que se ha escrito en otra pantalla.
void enfocarInput();
// Manejador de la interrupcion CTRL+C
void term_handler();

// Codigo principal del cliente
void main(int argc, char * argv[]){
	
	// Declaraciones
	int max_desc;
	char message[MAX_MESSAGE_LEN], command[MAX_FULLM_LEN], 
		pipe[MAX_PIPE_LEN], cmd[MAX_MESSAGE_LEN];
	char buffer[MAX_MESSAGE_LEN] = "", c[2] = {0,'\0'};
	fd_set lectura;
	fd_set c_lectura;
	struct timeval tv;
	int status,i,alto,x,y,salir = 0;
	
	// Inicializar senales.
	if (signal(SIGINT, term_handler) == SIG_ERR) {
        	printf("Ocurrio un error al colocar la signal.\n");
		exit(1);
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

	// Verificar tamanio del terminal
  	if (LINES < LINES_MIN || COLS < COLS_MIN) {
        	endwin(); // Restaurar la operación del terminal a modo normal
        	printf("El terminal es muy pequeño para correr este programa.\n");
        	exit(1);
    	}

	// Obtener nombre para el cliente.
	strcpy(name,getlogin());
	if ( strcmp(name, "" ) == 0 ) { perror("Error getting name"); }	

	// Obtener pipe a utilizar.
	strcpy(pipe, DEFAULT_PIPE);
	if (argc > 1) {
		for (i = 1; i < argc; i ++){
			if ( strcmp(argv[i], "-p") == 0) {
				if ( strlen(argv[i]) <= MAX_PIPE_LEN - 1 ) {
					strcpy(pipe, argv[i+1]);
					i++;
				} else { 
					endwin();
					printf("El path del pipe escogido es muy largo,\
						debe ser menor a %d caracteres.", MAX_PIPE_LEN - 1);
					exit(1);
				}
			} else {
				if ( strlen(argv[i]) <= MAX_NAME_LEN - 1 ) {
					strcpy(name,argv[i]);
				} else {
					endwin();
					printf("El nombre escogido es muy largo,\
						debe ser menor a %d caracteres.", MAX_NAME_LEN - 1);
					exit(1);
				}
			}
		}
	}

	// Cerramos el cliente si no tiene nombre, por alguna razon.
	if ( strcmp(name,"") == 0) { 
		endwin();
		printf("Ha ocurrido un error con el nombre, y es vacio");
		exit(1);
	}

	// Identificarse con el servidor.
	send_hello(pipe);
	wprintw(ventanaOutput, "Conectando al servidor. [%s]\n", pipe);
	wrefresh(ventanaOutput);
	enfocarInput();
	
	// Inicializando select
	FD_ZERO(&lectura);
	FD_SET(name_dec, &lectura);
	max_desc = name_dec+1;
	
	// Inicializando manejador de input.
	while(!salir){
		tv.tv_sec = 0;
		tv.tv_usec = 10000;
		c_lectura = lectura;
		if (select(max_desc, &c_lectura, NULL, NULL, &tv) < 0)
			perror("Error en el select");
		if( FD_ISSET(name_dec, &c_lectura)){
			status = read(name_dec, message, sizeof(message));
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
		

		//  Manejador de input por teclado.			
		c[0] = wgetch(ventanaInput);
                if (c[0] == '\r'){
			// Es un mensaje al servidor.
			if ( buffer[0] != '\0')  {
				sprintf(command, "%s %s\n", name, buffer);
				if ( write(server_dec, command, strlen(command)+1) == -1 ) { 
					perror("write sending_message"); 
				}
				wprintw(ventanaOutput, "<%s> %s\n", name, buffer);
				wrefresh(ventanaOutput);
				sscanf(buffer, "%s %[^\t]", cmd, buffer);
				salir = strcmp(cmd,"-salir") == 0;
				memset(buffer,0,sizeof(buffer));
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
			if (strlen(buffer) < MAX_MESSAGE_LEN) {
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

// Cierra el cliente.
void close_client(){
	close(name_dec);
	close(server_dec);
	if ( unlink(to_name) == 1 ) { 
		endwin(); perror("unlink closing"); exit(1);}
	if ( unlink(to_server) == 1 ) { 
		endwin(); perror("unlink closing server"); exit(1);}
	endwin();
}

// Identifica el cliente ante el servidor.
void send_hello(char * pipe){
	int fd, messagelen;
 	char message[MAX_NAME_LEN+10];
	struct stat st = {0};

	// Chequear carpeta
	if (stat(PIPE_COM, &st) == -1) {
    		if ( mkdir(PIPE_COM, 0777) == -1 ) { 
			endwin(); perror("mkdir error"); exit(1);}
	}
	sprintf(to_name, "%s/%s", PIPE_COM, name);
	unlink(to_name);

	// Crear pipe para recibir
        if ( mknod(to_name, S_IFIFO, 0) == 1 ) { 
		endwin(); perror("error mknod receive"); exit(1);}
        if ( chmod(to_name, 0660) == 1 ) { 
		endwin(); perror("error chmod receive"); exit(1);}
	sprintf(to_server, "%s/%s_serv",PIPE_COM, name);
        unlink(to_server);

	// Crear pipe para enviar
        if ( mknod(to_server, S_IFIFO, 0) == 1 ) { 
		endwin(); perror("error mknod send"); exit(1);}
        if ( chmod(to_server, 0660) == 1 ) { 
		endwin(); perror("error chmod send"); exit(1);}
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
	if ( name_dec == -1 ) { 
		endwin(); perror("error descriptor entrada"); exit(1);}
   	if ( write(fd, message, messagelen) == -1 ) { 
		endwin(); perror("error sending hello"); exit(1);}
	do { server_dec = open(to_server, O_WRONLY | O_NONBLOCK); } while 
		(server_dec == -1);

}

// Limpiar ventana de input.
void limpiarVentanaInput(){
        wclear(ventanaInput);
        mvwhline(ventanaInput, 0, 0, 0, COLS);
        wmove(ventanaInput, 1, 0);
        wrefresh(ventanaInput);
}

// Enfoca el cursor en la ventana de input.
void enfocarInput(){
	int x,y;
	getyx(ventanaInput, y,x);
	wmove(ventanaInput, y,x);
	wrefresh(ventanaInput);
}

// Manejador de interrupcion (CTRL - C)
void term_handler(){
	char tmp[MAX_NAME_LEN + 7];
	sprintf(tmp, "%s -salir", name);
	if ( write(server_dec, tmp, strlen(tmp)+1) == 1 ) { 
		close_client(); 
		perror("write -salir handler"); 
		exit(1);
	}
	close_client();
	exit(0);
}
