/*
Sistemas de Operacion I (CI-3825)

Proyecto I: Chat

Autores:
Guillermo Betancourt, carnet 11-10103
Gabriel Giménez, carnet 12-11006

server.h:
Contiene la declaracion de las funciones utilizadas en el servidor
asi como la declaracion de la estructura usuario.
Se decidio separar del server.c pues se considera extenso para mantenerlo
en el mismo.
*/

#ifndef server_chat
#define server_chat

struct User {
        char name[50];
	int to_name, to_server;
        char status[50];
        struct User * last_w;
	int index;
};

// Lee una linea completa de un descriptor.
int readLine(int fd, char *str);

// Envia mensaje en casos especiales, seguido de una desconexion 
// {NAME_USED, NO_MORE_ROOM}
void send_message_close(int t1, int t2, char * msg);

// Maneja las conexiones entrantes por el pipe nominal server
int accept_connection(char *, struct User *);

// Responde a los cambios de notificaciones y desconexion de los usuarios
void notificar_estado(char * msg, struct User * user, struct User *);

// Responde al comando -quienes
void send_quienes(struct User *, struct User *);

// Envia un mensaje global, por ejemplo cuando un usuario se conecta
void send_message_global(char * msg, struct User * user, struct User *);

// Responde a los cambios de notificaciones y desconexion de los usuarios
void notificar_estado(char * msg, struct User * user, struct User *);

// Facilita el envio de mensajes entre usuarios
int send_message(char * msg, struct User * user, struct User from);

// Dado un nombre, devuelve el objeto usuario correspondiente
struct User * get_user(char * uname, struct User *);

// Manejador de senial SIGPIPE
void term_handler();
#endif
