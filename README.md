# Operativos
Chat para linux en C / Utilizando PIPES para la comunicacion entre procesos, y ncurses. Modelo Cliente - Servidor

Make:
gcc client.c -o client -lncurses
gcc serverP.c -o serverP

Dependencias:
sudo apt-get install ncurses

Comandos del cliente:
-quien
-estoy [estado]
-escribir [target] mensaje
[mensaje]
-salir
