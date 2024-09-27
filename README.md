# WLANXfer

Una aplicación simple de transferencia de archivos utilizando un modelo cliente-servidor, con detección automática de cambios y revisiones periódicas de archivos. Solo disponible para Windows.

## Funcionalidades
- **Servidor**:
  - Servidor HTTP en un puerto configurable.
  - Aloja un directorio y sirve archivos a los clientes.
  - También tiene versión web
    
- **Cliente**:
  -  Se conecta al servidor y verifica periódicamente si hay modificaciones en los archivos.
  -  Descarga automáticamente los archivos que han sido modificados en el servidor.
  -  Notifica al cliente cuando se descarcarga un archivo
    
- **Minimizar al Tray**: La aplicación puede minimizarse en el área de notificación (tray) mediante una combinación de teclas (<kbd>Ctrl</kbd> + <kbd>Alt</kbd> + <kbd>Shift</kbd> + <kbd>W</kbd>).

## Descarga
Puedes descargar las últimas versiones desde la sección [Releases](https://github.com/otema666/WLANXfer/releases).

## Licencia
Este proyecto está bajo la licencia [MIT](https://github.com/otema666/WLANXfer/blob/master/LICENSE).

##

![alt](images/app.png)

##
