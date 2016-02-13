#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "serverP.h"

#define SERV_NAME "Server"
#define DEFAULT_PIPE "server"
#define SERV_USR lista[0]
#define MAX_CLIENT 21
#define MAX(x,y) (x > y ? x : y)

int readLine(int fd, char *str) {
	int n;
	do {
		n = read(fd, str, 1);
	} while(n > 0 && *(str++) != '\0');

return(n > 0);
} 


// Maneja las conexiones entrantes por el pipe nominal server
int accept_connection(char * msg, struct User * lista ){
	char uname[50];
	char uname_serv[55];
	int i;
	sscanf(msg, "New user: %s", uname);
        printf("Nuevo usuario: %s\n", uname);
	for( i = 1; i < MAX_CLIENT; i++ ) {
		if ( strcmp(lista[i].name,"") == 0 ) {
			strcpy(uname_serv, uname);
			strcat(uname_serv, "_serv");
			strcpy((&lista[i])->name, uname);
			strcpy((&lista[i])->to_server, uname_serv);
			strcpy((&lista[i])->status, "Disponible");
			(&lista[i])->index = i;
			return i;
		}
	}
	printf("There are no more slots");
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
	char msg[256];
	int i,file;
	printf("Sending connected users to %s\n", user->name);
	for (i = 0; i < MAX_CLIENT; i++){
		if (strcmp(lista[i].name,"") != 0 && strcmp(lista[i].name,SERV_NAME) != 0){
			sprintf(msg,"%s - %s\n",lista[i].name,lista[i].status);
			printf("%s",msg);
			file = open(user->name, O_WRONLY | O_NDELAY);
			write(file, msg, strlen(msg)+1);
		}
	}
}

void send_message_global(char * msg, struct User * user, struct User lista[MAX_CLIENT]){
	int i, file;
	char mensaje[256];
	for(i=0; i < MAX_CLIENT; i++){
		if (strcmp(lista[i].name,"") != 0 && strcmp(lista[i].name,SERV_NAME) != 0){
                        sprintf(mensaje,"[%s] %s\n", user->name, msg);
                        file = open(lista[i].name, O_WRONLY | O_NDELAY);
                        write(file, mensaje, strlen(mensaje)+1);
                }
	}
}

// Facilita el envio de mensajes entre usuarios
int send_message(char * msg, struct User * user, struct User from){
	int file;
	char mensaje[256];
	char error[50];
	file = open(user->name, O_WRONLY | O_NDELAY);
	if (file == -1){
		strcpy(error, "El mensaje no se pudo enviar.\n");
		if (strcmp(from.name,SERV_NAME) != 0){
			file = open(from.name, O_WRONLY | O_NDELAY);
			write(file, error, strlen(error)+1);
		} else {
			printf("%s", error);
		}
		return 0;
	}
	sprintf(mensaje, "[%s]: %s", from.name, msg);
	write(file, mensaje, strlen(mensaje)+1);
	return 1;
}

// Dado un nombre, devuelve el objeto usuario correspondiente
struct User * get_user(char * uname, struct User lista[MAX_CLIENT]){
	int i;
	for ( i = 0; i < MAX_CLIENT; i++ ){
		if (&lista[i] != NULL) {
			if (strcmp(lista[i].name, uname) == 0){
				return &lista[i];
			} 
		}
	}
	return NULL;
} 

void main(int argc, char * argv []){
	int file,desc[MAX_CLIENT],status,i,j,max_desc;
	char msg[256],cmd[256],who[50],target[50],pipe[100];
	struct User user_list[MAX_CLIENT];
	struct User * who_usr, * target_usr;
	fd_set lectura;
	fd_set c_lectura;
	struct timeval tv;
	if (argc > 1){
		strcpy(pipe, argv[1]);
	} else {
		strcpy(pipe, DEFAULT_PIPE);
	}
	// Inicializacion de clientes nulos
	for(i = 0; i < MAX_CLIENT; i++){
		strcpy(user_list[i].name,"\0");
	}
	printf("Starting server..\n");	
	unlink(pipe);
	mknod(pipe, S_IFIFO, 0);
	chmod(pipe, 0660);
	// Inicializando cliente servidor
	strcpy(user_list[0].name, SERV_NAME);
	printf("I am waiting connections.\n");
	desc[0] = open("server", O_RDONLY | O_NONBLOCK);
	FD_ZERO(&lectura);
	FD_SET(desc[0], &lectura);
	max_desc = desc[0]+1;
	while(1){
		tv.tv_sec = 1;
		c_lectura = lectura;
		if (select(max_desc, &c_lectura, NULL, NULL, &tv) < 0)
			perror("select");
		for (i = 0; i < MAX_CLIENT; i++){
			if (FD_ISSET(desc[i], &c_lectura)){
				memset(msg, 0, sizeof(msg));
				//status = read(desc[i], msg, sizeof(msg));
				status = readLine(desc[i],msg);
				if (status){
				if ( i == 0 ) {
					// Conexion entrante
					printf("New connection \n");
					j = accept_connection(msg,user_list);
					if( j >= 0 ) {
						desc[j] = open(user_list[j].to_server, O_RDONLY | O_NONBLOCK);
						FD_SET(desc[j], &lectura);
						max_desc = MAX(max_desc, desc[j])+1;
						sprintf(msg, "%s se ha conectado.",user_list[j].name);
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
						target_usr = get_user(target, user_list);
						if (i >= 4){
							printf("%s sent a message to %s.\n", who, target_usr->name);
							send_message(msg, target_usr, *who_usr);
							memset(msg,0,100);

						};
                                                who_usr->last_w = target_usr;

					} else if (strcmp(cmd,"-exit") == 0 || strcmp(cmd,"-exit_serv")==0){
						// Desconexion de usuario
						strcpy(msg,"se ha desconectado.");
						notificar_estado(msg,who_usr,user_list);
						printf("El usuario (%d) %s se ha desconectado.\n",who_usr->index, who);
						strcpy(who_usr->name, "");
						FD_CLR(desc[who_usr->index], &lectura);
						close(desc[who_usr->index]);
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
	unlink("server");
}
