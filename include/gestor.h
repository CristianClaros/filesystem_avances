#ifndef INCLUDE_GESTOR_H_
#define INCLUDE_GESTOR_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <commons/bitarray.h>
#include <sys/mman.h>
#include <commons/config.h>
#include <commons/collections/list.h>

#define TIEMPO_UNIDAD_TRABAJO 250
#define BLOCK_SIZE 64
#define BLOCK_COUNT 1024
#define RETRASO_COMPACTACION 50000
#define PATH_BASE_DIALFS "/home/utnso/dialfs"

#define ARCHIVO_BITMAP "bitmap.dat"
#define ARCHIVO_BLOQUE "bloques.dat"

typedef enum{
	EXIT = 0,
	IO_FS_CREATE,
	IO_FS_DELETE,
	IO_FS_TRUNCATE,
	IO_FS_WRITE,
	IO_FS_READ,
	VER_ARCHIVO
}t_opcion;

typedef struct{
	char* nombre;
	int bloque_inicial;
	int tamanio_archivo;
}t_metadata;

extern char* buffer_bloques;
extern t_bitarray* buffer_bitmap;

#endif /* INCLUDE_GESTOR_H_ */
