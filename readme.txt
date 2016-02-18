Proyecto de Chat

Guillermo Betancourt, carnet 11-10103
Gabriel Giménez, carnet 12-11006

- Detalles de implementación:

Nuestro proyecto consta de dos códigos principales, client.c, server.c.

El cliente, al momento de ejecutarse, envia al servidor, a travez del pipe de conexiones
entrantes, su nombre, para que el servidor le asigne posteriormente su puesto correspondiente.
Como convencion, se toma que el servidor se comunicara con el cliente a travez de un pipe
con el mismo nombre que el cliente, y el mismo, se comunicara con el servidor con un pipe
con el mismo nombre que el cliente seguido del postfijo _serv.
Por convencion, asi mismo, el cliente se identifica con el servidor, por un pipe, llamado 
server_c, que puede ser cambiado posteriormente con las respectivas llamadas.
Se tomo como carpeta para las comunicaciones "/tmp/12-11006", donde se crean los pipes 
tanto del cliente, como del servidor.

Se establecieron varias cotas en el cliente.
Cada cliente puede tener un nombre maximo de 99 caracteres.
El path maximo para un pipe, son 99 caracteres.
El mensaje mas largo a enviar es de 256 caracteres.

Las mismas cotas aplican para el servidor.

Por su lado el servidor, una vez iniciado, crea el pipe para recibir conexiones entrantes,
y crea un usuario "Servidor" que facilita el envio de mensajes por parte del servidor, y 
facilita la utilizacion del select.

