#ifndef INCLUDE_MAIN_H_
#define INCLUDE_MAIN_H_

#include "gestor.h"

int crear_metadata(char* ruta, char* bloque_inicial, char* tamanio_archivo);
int modificar_metadata(char* ruta, char* bloque_inicial, char* tamanio_archivo);
int cargar_metadata(char* ruta, t_metadata* metadata);

int obtener_archivo(char* ruta, void (*tipo_archivo)(int));

char* ruta_absoluta(char* ruta_relativa);

int crear_archivo(char* ruta, void (*tipo_Archivo)(int));
int abrir_archivo(char* ruta, void (*tipo_Archivo)(int));

void ver_bloques(int file_descriptor);

void bitmap(int file_descriptor);
void bloques(int file_descriptor);

int agregar_archivo(char* archivo, char* archivos_abiertos);
void mostrar_archivos(t_metadata* metadata);
t_list* cargar_archivos(char* ruta);

void obtener_datos_filesystem();
int menu();
int ejecutar_opcion(int opcion);

#endif /* INCLUDE_MAIN_H_ */
