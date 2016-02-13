#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "serverP.h"
#include <libgen.h>

#define SERV_NAME "Server"
#define PIPE_COM "/tmp/12-11006"
#define DEFAULT_PIPE "server"
#define SERV_USR lista[0]
#define MAX_CLIENT 21
#define MAX(x,y) (x > y ? x : y)

char upipe[100];

// Sacado de las laminas de la profesora.
int readLine(int fd, char *str) {
       int n;
       do {
               n = read(fd, str, 1);
       } while(n > 0 && *(str++) != '\0');
 
return(n > 0);
} 

// Maneja las conexiones entrantes por el pipe nominal server.
int accept_connection(char * msg, struct User * lista ){
	char uname[50], tmp[100];
	int i;
	sscanf(msg, "New user: %s", uname);
        printf("Nuevo usuario: %s\n", uname);
	for( i = 1; i < MAX_CLIENT; i++ ) {
		if ( strcmp(lista[i].name,"") == 0 ) {
			strcpy((&lista[i])->name, uname);
			sprintf(tmp, "%s/%s", PIPE_COM, uname);
			(&lista[i])->to_name =  open(tmp, O_WRONLY | O_NDELAY) ;
			sprintf(tmp, "%s_serv", tmp);
			(&lista[i])->to_server = open(tmp, O_RDONLY | O_NDELAY) ;
			strcpy((&lista[i])->status, "Disponible");
			(&lista[i])->last_w = NULL;
			(&lista[i])->index = i;
			return i;
		}
	}
	printf("There are no more slots\n");
	return -1;
}

// Responde a los cambios de notificaciones y desconexion de los usuarios
void notificar_estado(char * msg, struct User * user, struct User lista[MAX_CLIENT]){
	int i,file, disconnect;
	char notificacion[256];
	disconnect = strcmp(msg,"se ha desconectado.") == 0;
	for ( i = 0 ; i < MAX_CLIENT ; i ++ ){
		if ( lista[i].last_w == user ) {
			if (!disconnect) {
				sprintf(notificacion, "%s %s [%s]\n", user->name, "ha cambiado su estado a", msg);
			} else {
				sprintf(notificacion, "%s %s\n", user->name, msg);
			}
			send_message(notificacion, &lista[i], SERV_USR);
		}
	}
}

// Responde al comando -quienes
void send_quienes(struct User * user, struct User lista[MAX_CLIENT]){
	char msg[256], tmp[150], bigger[2000] = "  -- Usuarios conectados --\n\n";
	int i;
	printf("Sending connected users to %s\n", user->name);
	for (i = 0; i < MAX_CLIENT; i++){
		if (strcmp(lista[i].name,"") != 0 && strcmp(lista[i].name,SERV_NAME) != 0){
			sprintf(msg,"%s - %s\n",lista[i].name,lista[i].status);
			strcat(bigger, msg);
		}
	}
	send_message(bigger, user, lista[0]);
}

void send_message_global(char * msg, struct User * user, struct User lista[MAX_CLIENT]){
	int i, file;
	char mensaje[256], tmp[150];
	for(i=0; i < MAX_CLIENT; i++){
		if (strcmp(lista[i].name,"") != 0 && strcmp(lista[i].name,SERV_NAME) != 0){
                        send_message(msg, &lista[i], lista[0]);
                }
	}
}

// Facilita el envio de mensajes entre usuarios
int send_message(char * msg, struct User * user, struct User from){
	int file;
	char mensaje[256];
	char error[50];
	if ( user != NULL) {
		sprintf(mensaje, "[%s]: %s", from.name, msg);
		write(user->to_name, mensaje, strlen(mensaje)+1);
		printf("Enviando mensaje a %s\n", user->name);
		return 1;
	} else {
		if ( from.name != SERV_NAME ) {
			sprintf(mensaje, "%s", "El usuario no esta conectado.\n");
			write(from.to_name, mensaje, strlen(mensaje)+1);
		}
	}
	return 0;
}

// Dado un nombre, devuelve el objeto usuario correspondiente
struct User * get_user(char * uname, struct User lista[MAX_CLIENT]){
	int i;
	for ( i = 1; i < MAX_CLIENT; i++ ){
		if (strcmp(lista[i].name, uname) == 0){
			return &lista[i];
		} 
	}
	return NULL;
} 

void term_handler(){
	unlink(upipe);
	exit(0);
}

void main(int argc, char * argv []){
	int file,status,i,j,max_desc;
	char msg[256],cmd[256],who[50],target[50],* tmp;
	struct User user_list[MAX_CLIENT];
	struct User * who_usr, * target_usr;
	fd_set lectura;
	fd_set c_lectura;
	struct timeval tv;
	struct stat st = {0};
	
	// Inicializar senales.
        if (signal(SIGINT, term_handler) == SIG_ERR) {
                printf("Ocurrio un error al colocar la signal.\n");
        }
	
	// Revisando pipe
	if (argc > 1){
		strcpy(upipe, argv[1]);
	} else {
		strcpy(upipe, DEFAULT_PIPE);
	}
	
	// Inicializacion de clientes nulos
	for(i = 0; i < MAX_CLIENT; i++){
		strcpy(user_list[i].name,"\0");
	}
	
	printf("Starting server..\n");
	
	// Inicializando pipes
	tmp = strdup(upipe);
	sprintf(tmp, "%s/", dirname(tmp));
	if (stat( tmp , &st) == -1) {
                if ( mkdir( tmp , 0777) == -1 ){
			perror("MKDIR: ");
		}
        }
	// No se verifica pues esta hecho para que falle la mayoria de las veces.
	unlink(upipe);
	if ( mknod(upipe, S_IFIFO, 0) == -1) perror("MKNOD: ");
	if ( chmod(upipe, 0660)  == -1 ) perror("CHMOD: ");
	
	// Inicializando cliente servidor
	strcpy(user_list[0].name, SERV_NAME);
	printf("I am waiting connections.\n");
	
	// Inicializando select
	user_list[0].to_server = open(upipe, O_RDONLY | O_NONBLOCK);
	FD_ZERO(&lectura);
	FD_SET(user_list[0].to_server, &lectura);
	max_desc = user_list[0].to_server+1;
	while(1){
		tv.tv_sec = 1;
		c_lectura = lectura;
		if (select(max_desc, &c_lectura, NULL, NULL, &tv) < 0)
			perror("SELECT: ");
		for (i = 0; i < MAX_CLIENT; i++){
			if (strcmp(user_list[i].name, "") != 0) { 
			if (FD_ISSET(user_list[i].to_server, &c_lectura)){
				memset(msg, 0, sizeof(msg));
				status = readLine(user_list[i].to_server,msg);
				if (status){
				if ( i == 0 ) {
					// Conexion entrante
					printf("New connection \n");
					j = accept_connection(msg,user_list);
					if( j >= 0 ) {
						FD_SET(user_list[j].to_server, &lectura);
						max_desc = MAX(max_desc, user_list[j].to_server)+1;
						sprintf(msg, "%s se ha conectado.\n",user_list[j].name);
						send_message_global(msg,&user_list[0], user_list);
					} else {
						printf("Connection refused.");
					}
				} else {
					sscanf(msg, "%s %s",who,cmd);
					who_usr = get_user(who, user_list);
					if (strcmp(cmd,"-escribir") == 0) {
						// Mensaje a un usuario
						printf("%s", msg);
						i = sscanf(msg, "%s %s %s %[^\t]", who,cmd,target,msg);
						printf("Buscando target\n");
						target_usr = get_user(target, user_list);
						if (i >= 4){
							//printf("%s sent a message to %s.\n", who, target_usr->name);
							send_message(msg, target_usr, *who_usr);
							memset(msg,0,100);
						};
                                                who_usr->last_w = target_usr;

					} else if (strcmp(cmd,"-salir") == 0 || strcmp(cmd,"-exit_serv")==0){
						// Desconexion de usuario
						strcpy(msg,"se ha desconectado.");
						notificar_estado(msg,who_usr,user_list);
						printf("El usuario (%d) %s se ha desconectado.\n",who_usr->index, who);
						strcpy(who_usr->name, "");
						FD_CLR(who_usr->to_server, &lectura);
						close(who_usr->to_server);
					} else if (strcmp(cmd,"-quien") == 0 ) {
						// Quienes estan conectados
						send_quienes(who_usr, user_list);
					} else if (strcmp(cmd,"-estoy") == 0) {
						// Cambiar de estado
						sscanf(msg, "%s %s %[^\t\n]", who, cmd, msg);
						strcpy(who_usr->status,msg);
						notificar_estado(msg, who_usr, user_list);
					} else {
						// Mensaje al ultimo usuario
						printf("(%d) %s: %s\n", who_usr->index,who,cmd);
						sscanf(msg, "%s %[^\t]",who, cmd);
						if ( who_usr->last_w != NULL){
							send_message(cmd, who_usr->last_w, *who_usr);
						}
					}
				}
				}
				}		
			}
		}
	}
	unlink(upipe);
}
