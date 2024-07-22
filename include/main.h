#ifndef INCLUDE_MAIN_H_
#define INCLUDE_MAIN_H_

#include "gestor.h"

int crear_metadata(char* ruta, int bloque_inicial, int tamanio_archivo);
int modificar_metadata(char* ruta, int bloque_inicial, int tamanio_archivo);
int cargar_metadata(char* ruta, t_metadata* metadata);
t_metadata* datos_metadata(char* ruta);

int obtener_archivo(char* ruta, void (*tipo_creacion)(int), void (*tipo_apertura)(int));

char* ruta_absoluta(char* ruta_relativa);

int crear_archivo(char* ruta, void (*tipo_Archivo)(int));
int abrir_archivo(char* ruta, void (*tipo_Archivo)(int));
int abrir_archivos();

t_list* cargar_archivos(char* ruta);
bool comparar_bloque_inicial(t_metadata* metadata_lista, t_metadata* metadata);
int mostrar_archivos(char* ruta_directorio);
void mostrar_lista_archivos(t_metadata* metadata);

int compactar();

void crear_bitmap(int file_descriptor);
void crear_bloques(int file_descriptor);

void abrir_bitmap(int file_descriptor);
void abrir_bloques(int file_descriptor);
char* datos_memoria(int registro_direccion, int registro_tamanio);

int buscar_bloque_bitmap(int longitud);
int asignar_bloque_bitmap(int bloque_libre, int longitud);
int remover_bloque_bitmap(int bloque_inicial, int longitud);
int remover_datos_bloque(int posicion_inicial, int cantidad);
int escribir_datos_bloque(char* buffer, int posicion_inicial, int cantidad);

int fs_create(char* nombre);
int fs_delete(char* archivo_metadata);
int fs_truncate(char* ruta, int registro_tamanio);
int fs_write(char* ruta, int registro_direccion, int registro_tamanio, int registro_puntero);
int fs_read(char* ruta, int registro_direccion, int registro_tamanio, int registro_puntero);

void obtener_datos_filesystem();
int menu();
int ejecutar_opcion(int opcion);
int cantidad_bloques(int tamanio);
int inicio_bloque(int n_bloque);

void crear_memoria(int file_descriptor);
void abrir_memoria(int file_descriptor);

#endif /* INCLUDE_MAIN_H_ */
