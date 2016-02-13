#ifndef server_chat
#define server_chat

struct User {
        char name[50];
        char to_server[50];
        char status[50];
        struct User * last_w;
	int index;
};

// Maneja las conexiones entrantes por el pipe nominal server
int accept_connection(char *, struct User *);
// Responde al comando -quienes
void send_quienes(struct User *, struct User *);
// Facilita el envio de mensajes entre usuarios
int send_message(char * msg, struct User * user, struct User from);
// Dado un nombre, devuelve el objeto usuario correspondiente
struct User * get_user(char * uname, struct User *);
// Responde a los cambios de notificaciones y desconexion de los usuarios
void notificar_estado(char * msg, struct User * user, struct User *);
#endif
