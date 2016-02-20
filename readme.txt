Proyecto de Chat

Guillermo Betancourt, carnet 11-10103
Gabriel Giménez, carnet 12-11006

- Detalles de implementación:

Nuestro proyecto consta de dos códigos principales, client.c y server.c.

El cliente al momento de ejecutarse envía al servidor, a través del pipe de conexiones
entrantes, su nombre para que el servidor le asigne posteriormente su puesto correspondiente.

Como convención, se toma que el servidor se comunicará con el cliente a través de un pipe
con el mismo nombre que el nombre del cliente, y el mismo se comunicará con el servidor con un pipe
con el mismo nombre que el nombre del cliente seguido del postfijo "_serv".

Por convención, así mismo, el cliente se identifica con el servidor por un pipe llamado 
"server_c", que puede ser cambiado posteriormente con las respectivas llamadas.

Se tomó como carpeta para las comunicaciones "/tmp/12-11006", donde se crean los pipes 
tanto del cliente, como del servidor.

Se establecieron algunos límites para el cliente:

1) Cada cliente puede tener un nombre máximo de 99 caracteres.
2) El path máximo para un pipe son 99 caracteres.
3) El mensaje más largo a enviar es de 256 caracteres.

Los mismos límites aplican para el servidor.

Del lado del servidor, una vez iniciado, crea el pipe para recibir conexiones entrantes,
y crea un usuario "Servidor" que facilita el envío de mensajes por parte del servidor y 
facilita la utilizacion del select.

Por otro lado, para la notificar de la desconexión de los clientes se tomó la misma convención
que con el cambio de estado: se le notifica a los usuarios que por última vez le escribieron al
usuario que se acaba de desconectar.
