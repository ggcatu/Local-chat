# Operativos
Chat para linux en C

Utilizando PIPES para la comunicacion entre procesos, y ncurses. Modelo Cliente - Servidor

## Make:
```
make all
make client
make server
make clean
```

## Dependencias:
```
sudo apt-get install ncurses
```

## Ejecutar:
Si no se especifica un SERVER_PIPE se utiliza uno por defecto.
```
./client [-p SERVER_PIPE] [name]
./server [SERVER_PIPE]
```

## Comandos del cliente:
| Comandos | Informacion |
| --- | --- |
| -quien | Listar los usuarios conectados. |
| -estoy [estado] | Cambiar de estado en el chat. |
| -escribir [target] mensaje | Escribir un mensaje al usuario [target] |
| [mensaje] | Si anteriormente ha escrito un mensaje a un usuiario, puede omitir el -escribir |
| -salir | Cierra el cliente |
