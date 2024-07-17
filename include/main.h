#ifndef INCLUDE_MAIN_H_
#define INCLUDE_MAIN_H_

#include "gestor.h"

int crear_metadata(char* ruta, char* bloque_inicial, char* tamanio_archivo);
int modificar_metadata(char* ruta, char* bloque_inicial, char* tamanio_archivo);
int cargar_metadata(char* ruta, t_metadata* metadata);
t_metadata* datos_metadata(char* ruta);

int obtener_archivo(char* ruta, void (*tipo_archivo)(int));

char* ruta_absoluta(char* ruta_relativa);

int crear_archivo(char* ruta, void (*tipo_Archivo)(int));
int abrir_archivos(char* ruta_bloque, char* ruta_bitmap);

void bitmap(int file_descriptor);
void bloques(int file_descriptor);

int agregar_archivo(char* archivo, char* archivos_abiertos);
void mostrar_archivos(t_metadata* metadata);
t_list* cargar_archivos(char* ruta);

int buscar_bloque_libre(char* ruta, int longitud);
int asignar_bloque_libre(char* ruta,int bloque_libre, int longitud);
int remover_bloque_bitmap(char* ruta,int bloque_inicial, int longitud);

int fs_create(char* nombre);
int fs_delete(char* archivo_metadata);
int fs_write(char* ruta, int registro_direccion, int registro_tamanio, int registro_puntero);

void obtener_datos_filesystem();
int menu();
int ejecutar_opcion(int opcion);

#endif /* INCLUDE_MAIN_H_ */
