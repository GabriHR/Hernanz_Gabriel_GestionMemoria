#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#define SIZE 4096

int _tmain(int argc, _TCHAR* argv[]) {
    // Crear un objeto de mapeo de archivos compartidos
    HANDLE hMapFile;
    LPCTSTR pBuf;

    hMapFile = CreateFileMapping(
            INVALID_HANDLE_VALUE,    // Usar archivo de paginación
            NULL,                    // Seguridad predeterminada
            PAGE_READWRITE,          // Acceso de lectura/escritura
            0,                       // Tamaño máximo del objeto (DWORD de orden superior)
            SIZE,                    // Tamaño máximo del objeto (DWORD de orden inferior)
            _T("Local\\MyFileMappingObject")); // Nombre del objeto de mapeo

    if (hMapFile == NULL) {
        _tprintf(_T("Could not create file mapping object (%d).\n"),
                 GetLastError());
        return 1;
    }

    // Mapear la vista del archivo en la memoria
    pBuf = (LPTSTR)MapViewOfFile(hMapFile,   // Manejar al objeto de mapeo
                                 FILE_MAP_ALL_ACCESS, // Permiso de lectura/escritura
                                 0,
                                 0,
                                 SIZE);

    if (pBuf == NULL) {
        _tprintf(_T("Could not map view of file (%d).\n"),
                 GetLastError());

        CloseHandle(hMapFile);

        return 1;
    }

    // Escribir en la memoria compartida
    _stprintf((TCHAR*)pBuf, _T("Hello, child process!"));

    // Configuración para el proceso hijo
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Iniciar el proceso hijo
    if (!CreateProcess(NULL,   // Sin nombre de módulo (usar línea de comandos)
                       _T("ChildProcess.exe"), // Línea de comandos
                       NULL,          // Manejar del proceso no heredable
                       NULL,          // Manejar del hilo no heredable
                       FALSE,         // Establecer la herencia del manejar en FALSE
                       0,             // Sin banderas de creación
                       NULL,          // Usar el bloque de entorno del padre
                       NULL,          // Usar el directorio de inicio del padre
                       &si,           // Puntero a la estructura STARTUPINFO
                       &pi))          // Puntero a la estructura PROCESS_INFORMATION
    {
        _tprintf(_T("CreateProcess failed (%d).\n"), GetLastError());
        return 1;
    }

    // Esperar hasta que el proceso hijo salga
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Imprimir el contenido compartido
    _tprintf(_T("Child reads: %s\n"), pBuf);

    // Desmapear la vista del archivo de la memoria
    UnmapViewOfFile(pBuf);

    // Cerrar el objeto de mapeo de archivos compartidos
    CloseHandle(hMapFile);

    return 0;
}
