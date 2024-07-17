#include "../include/main.h"

int main(){

	obtener_datos_filesystem();

	int opcion;

	do{
		menu();

		scanf("%i", &opcion);

		ejecutar_opcion(opcion);

	}while(opcion != EXIT);

	return EXIT_SUCCESS;
}

int menu(){
	printf("Menu\n");
	printf("0 - EXIT\n");
	printf("1 - IO_FS_CREATE\n");
	printf("2 - IO_FS_DELETE\n");
	printf("3 - IO_FS_TRUNCATE\n");
	printf("4 - IO_FS_WRITE\n");
	printf("5 - IO_FS_READ\n");
	printf("6 - VER ARCHIVOS\n");
	printf("Opcion: ");

	return EXIT_SUCCESS;
}

int ejecutar_opcion(int opcion){
	char* nombre_archivo = malloc(50*sizeof(char));
	switch(opcion){
		case IO_FS_CREATE:
				printf("Creando archivos\n");
				printf("Ingrese un nombre de archivo a crear: ");
				scanf("%s", nombre_archivo);
//				nombre_archivo = recibir_string(buffer);
				fs_create(nombre_archivo);
			break;
		case IO_FS_DELETE:
				printf("Borrando archivos\n");
				printf("Ingrese un nombre de archivo a borrar: ");
				scanf("%s", nombre_archivo);
				//nombre_archivo = recibir_string(buffer);
				fs_delete(nombre_archivo);
			break;
		case IO_FS_TRUNCATE:
				printf("Truncando archivos\n");
			break;
		case IO_FS_WRITE:
				printf("Escribiendo archivos\n");
				printf("Ingrese un nombre de archivo a Escribir: ");
				scanf("%s", nombre_archivo);
				//nombre_archivo = recibir_string(buffer);
				//registro_direccion = recibir_int(buffer);
				//registro_tamanio = recibir_int(buffer);
				//registro_puntero = recibir_int(buffer);
				int registro_direccion = 8;
				int registro_tamanio = 1024;
				int registro_puntero = 0;
				fs_write(nombre_archivo, registro_direccion, registro_tamanio, registro_puntero);
			break;
		case IO_FS_READ:
				printf("Leyendo archivos\n");
			break;
		case VER_ARCHIVO:
				printf("Archivos\n");
				abrir_archivos(ARCHIVO_BLOQUE, ARCHIVO_BITMAP);
			break;
		default:
			printf("Opcion invalida\n");
	}

	return EXIT_SUCCESS;
}

int fs_create(char* nombre){
	int bloque_libre;

	if((bloque_libre = buscar_bloque_bitmap(ARCHIVO_BITMAP, BLOQUE_UNITARIO)) == -1){
		printf("No se puede encontrar un bloque libre, debe compactar\n");
	}

	asignar_bloque_bitmap(ARCHIVO_BITMAP, bloque_libre, BLOQUE_UNITARIO);

	crear_metadata(nombre, bloque_libre, TAMANIO_VACIO);

	return EXIT_SUCCESS;
}

int fs_delete(char* archivo_metadata){
	int cantidad_bloques;
	t_metadata* metadata = datos_metadata(archivo_metadata);

	if(metadata->tamanio_archivo == 0){
		cantidad_bloques = 1;
	}else{
		cantidad_bloques = ceil(metadata->tamanio_archivo/8);
	}

	remover_bloque_bitmap(ARCHIVO_BITMAP, metadata->bloque_inicial, cantidad_bloques);
	//Aqui debo borrar datos del bloques.dat

	remove(ruta_absoluta(archivo_metadata));
	free(metadata);

	return EXIT_SUCCESS;
}

int fs_write(char* ruta, int registro_direccion, int registro_tamanio, int registro_puntero){
	//Pedir a memoria que me devuelva la cantidad de registro_tamanio bytes en registro_direccion
	int tamanio_auxiliar;
	int bloque_cantidad;
	int nuevo_bloque_cantidad;

	t_metadata* metadata = datos_metadata(ruta);
	tamanio_auxiliar = metadata->tamanio_archivo;
	metadata->tamanio_archivo += registro_tamanio;
	nuevo_bloque_cantidad = ceil(metadata->tamanio_archivo/8);

	if(tamanio_auxiliar == 0){
		bloque_cantidad = 1;
	}else{
		bloque_cantidad = ceil(tamanio_auxiliar/8);
	}

	if(nuevo_bloque_cantidad > bloque_cantidad){
		//si estan ocpuados busco nuevo espacio
		//supongo que el bit continuo esta ocupado

		//Verifico si hay espacio contiguo
		//por hacer

		int nuevo_bloque = buscar_bloque_bitmap(ARCHIVO_BITMAP, nuevo_bloque_cantidad);

		asignar_bloque_bitmap(ARCHIVO_BITMAP, nuevo_bloque, nuevo_bloque_cantidad);
		remover_bloque_bitmap(ARCHIVO_BITMAP, metadata->bloque_inicial, bloque_cantidad);

		modificar_metadata(ruta, nuevo_bloque, metadata->tamanio_archivo);
	}else{

		modificar_metadata(ruta, metadata->bloque_inicial, metadata->tamanio_archivo);
	}
	free(metadata);

	return EXIT_SUCCESS;
}

int asignar_bloque_bitmap(char* ruta, int bloque_libre, int longitud){
	int fd;
	struct stat buf;
	char* buffer;
	t_bitarray* bitmap;

	fd = open(ruta_absoluta(ruta), O_RDWR);
	fstat(fd, &buf);
	buffer = mmap(NULL, buf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	bitmap = bitarray_create_with_mode(buffer, buf.st_size, LSB_FIRST);

	for(int i = bloque_libre;i<(bloque_libre+longitud);i++){
		bitarray_set_bit(bitmap, i);
	}

	msync(bitmap, bitmap->size, MS_SYNC);
	munmap(bitmap, bitmap->size);

	return EXIT_SUCCESS;
}

int remover_bloque_bitmap(char* ruta,int bloque_inicial, int longitud){
	int fd;
	struct stat buf;
	char* buffer;
	t_bitarray* bitmap;

	fd = open(ruta_absoluta(ruta), O_RDWR);
	fstat(fd, &buf);
	buffer = mmap(NULL, buf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	bitmap = bitarray_create_with_mode(buffer, buf.st_size, LSB_FIRST);

	for(int i = bloque_inicial;i<(bloque_inicial+longitud);i++){
		bitarray_clean_bit(bitmap, i);
	}

	msync(bitmap, bitmap->size, MS_SYNC);
	munmap(bitmap, bitmap->size);

	return EXIT_SUCCESS;
}

int buscar_bloque_bitmap(char* ruta, int longitud){
	int fd;
	struct stat buf;
	char* buffer;
	t_bitarray* bitmap;

	fd = open(ruta_absoluta(ruta), O_RDWR);
	fstat(fd, &buf);
	buffer = mmap(NULL, buf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	bitmap = bitarray_create_with_mode(buffer, buf.st_size, LSB_FIRST);

	int n_bloque = -1; //Si devuelve este valor, no hay lugar libre para ese tamaño
	int flag = 0; // 0  es false, 1 es true
	int contador_espacios = 0;
	for(int i=0;i<bitmap->size;i++){
		if(!bitarray_test_bit(bitmap, i)){
			if(flag == 0){
				n_bloque = i;
				flag = 1;
			}
			contador_espacios++;
			if(contador_espacios == longitud){
				return n_bloque;
			}
		}else{
			n_bloque = -1;
			contador_espacios = 0;
			flag = 0;
		}
	}

	msync(bitmap, bitmap->size, MS_SYNC);
	munmap(bitmap, bitmap->size);

	return n_bloque;
}

void obtener_datos_filesystem(){
	obtener_archivo(ARCHIVO_BLOQUE, (void*) bloques);
	obtener_archivo(ARCHIVO_BITMAP, (void*) bitmap);
}

int obtener_archivo(char* ruta, void (*tipo_archivo)(int)){
	if(access(ruta, F_OK) == -1){
		crear_archivo(ruta, (*tipo_archivo));
	}

	return EXIT_SUCCESS;
}

char* ruta_absoluta(char* ruta_relativa){
	int tamanio_ruta = strlen(PATH_BASE_DIALFS) + strlen(ruta_relativa);
	char* ruta = malloc(tamanio_ruta*sizeof(char));

	strcpy(ruta, PATH_BASE_DIALFS);
	strcat(ruta, "/");
	strcat(ruta, ruta_relativa);

	return ruta;
}

int crear_archivo(char* ruta, void (*tipo_Archivo)(int)){
	int fd;

	if((fd = open(ruta_absoluta(ruta), O_CREAT|O_RDWR|O_TRUNC, S_IRWXU)) == -1){
		printf("No se pudo crear el archivo!!!\n");
		return EXIT_FAILURE;
	}else{
		(*tipo_Archivo)(fd);
		close(fd);
	}

	return EXIT_SUCCESS;
}

int abrir_archivos(char* ruta_bloque, char* ruta_bitmap){
	int fd_bloque;
	int fd_bitmap;

	if(((fd_bloque = open(ruta_absoluta(ruta_bloque), O_RDWR)) == -1) || ((fd_bitmap = open(ruta_absoluta(ruta_bitmap), O_RDWR)) == -1)){
		printf("No se pueden abrir los archivos!!!\n");
		return EXIT_FAILURE;
	}else{
		struct stat buf_bloque;
		struct stat buf_bitmap;

		char* buffer_bloque;
		char* buffer_bitmap;
		t_bitarray* bitmap;

		fstat(fd_bloque, &buf_bloque);
		buffer_bloque = mmap(NULL, buf_bloque.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd_bloque, 0);

		fstat(fd_bitmap, &buf_bitmap);
		buffer_bitmap = mmap(NULL, buf_bitmap.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd_bitmap, 0);
		bitmap = bitarray_create_with_mode(buffer_bitmap, buf_bitmap.st_size, LSB_FIRST);

		int aux=0;

		for(int i=0;i<bitmap->size;i++){
			printf("%i - ", i);
			if(bitarray_test_bit(bitmap,i)){
				printf("1\t");
			}else{
				printf("0\t");
			}
			for(int j=0;j<ceil(BLOCK_SIZE/8);j++){
				printf("%c", buffer_bloque[j+aux]);
			}
			printf("\n");
			aux+=ceil(BLOCK_SIZE/8);
		}

		close(fd_bloque);
		close(fd_bitmap);
	}

	return EXIT_SUCCESS;
}

void bloques(int file_descriptor){
	int tamanio_archivo = BLOCK_COUNT * ceil(BLOCK_SIZE/8);

	ftruncate(file_descriptor, tamanio_archivo);

	char* buffer_bloques;
	struct stat buf;

	fstat(file_descriptor, &buf);
	buffer_bloques = mmap(NULL, buf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, file_descriptor, 0);

	for(int i=0;i<tamanio_archivo;i++){
		strcat(buffer_bloques, "0");
	}

	msync(buffer_bloques, buf.st_size, MS_SYNC);
	munmap(buffer_bloques, buf.st_size);
}

void bitmap(int file_descriptor){
	int tamanio_archivo = BLOCK_COUNT;

	ftruncate(file_descriptor, tamanio_archivo);

	t_bitarray* bitmap;
	char* buffer_bitmap;
	struct stat buf;

	fstat(file_descriptor, &buf);
	buffer_bitmap = mmap(NULL, buf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, file_descriptor, 0);
	bitmap = bitarray_create_with_mode(buffer_bitmap, buf.st_size, LSB_FIRST);

	for(int i=0;i<bitmap->size;i++){
		bitarray_clean_bit(bitmap, i);
	}
	msync(bitmap, bitmap->size, MS_SYNC);
	munmap(bitmap, bitmap->size);
}

int crear_metadata(char* ruta, int bloque_inicial, int tamanio_archivo){
	int fd;

	if((fd = open(ruta_absoluta(ruta), O_CREAT, S_IRWXU)) == -1){
		perror("No se pudo crear el archivo!!!\n");
		return EXIT_FAILURE;
	}else{
		close(fd);

		char* bloque_string = malloc(50*sizeof(char));
		char* tamanio_string = malloc(50*sizeof(char));

		sprintf(bloque_string, "%i", bloque_inicial);
		sprintf(tamanio_string, "%i", tamanio_archivo);

		modificar_metadata(ruta, bloque_inicial, tamanio_archivo);

		free(bloque_string);
		free(tamanio_string);
	}

	return EXIT_SUCCESS;
}

int cargar_metadata(char* ruta, t_metadata* metadata){
	t_config* config = config_create(ruta);

	if(!config){
		perror("No se pudo crear el config correctamente!!!\n");
		return EXIT_FAILURE;
	}

	metadata->bloque_inicial = config_get_int_value(config, "BLOQUE_INICIAL");
	metadata->tamanio_archivo = config_get_int_value(config, "TAMANIO_ARCHIVO");

	config_destroy(config);

	return EXIT_SUCCESS;
}

int modificar_metadata(char* ruta, int bloque_inicial, int tamanio_archivo){
	t_config* config = config_create(ruta_absoluta(ruta));

	char* bloque_string = malloc(50*sizeof(char));
	char* tamanio_string = malloc(50*sizeof(char));

	sprintf(bloque_string, "%i", bloque_inicial);
	sprintf(tamanio_string, "%i", tamanio_archivo);

	config_set_value(config, "BLOQUE_INICIAL", bloque_string);
	config_set_value(config, "TAMANIO_ARCHIVO", tamanio_string);
	config_save(config);

	free(bloque_string);
	free(tamanio_string);
	config_destroy(config);

	return EXIT_SUCCESS;
}

t_metadata* datos_metadata(char* ruta){
	t_metadata* metadata = malloc(sizeof(t_metadata));
	t_config* config = config_create(ruta_absoluta(ruta));

	metadata->bloque_inicial = config_get_int_value(config, "BLOQUE_INICIAL");
	metadata->tamanio_archivo = config_get_int_value(config, "TAMANIO_ARCHIVO");

	config_destroy(config);
	return metadata;
}

//Funciones que no son necesarios solo para simular Kernel
//Solo es una funcion que me dara la lista de archivos en FS, que en realidad me lo debera dar el Kenel apenas me conecte
t_list* cargar_archivos(char* ruta){
	FILE* fd;
	char linea[50];
	struct stat buf;

	t_list* lista_archivos = list_create();

	if(!(fd = fopen(ruta,"r" ))){
		printf("No se pudo abrir el archivo!!!\n");
		return NULL;
	}

	fstat(fd->_fileno, &buf);
	if(buf.st_size == 0){
		printf("No hay archivos en Filesystem!!!\n");
		return NULL;
	}

	while(feof(fd) == 0){
		t_metadata* metadata = malloc(sizeof(t_metadata));
		metadata->nombre = malloc(sizeof(char));
		fgets(linea, 50, fd);
		strcpy(metadata->nombre, strtok(linea,"\n"));

		list_add(lista_archivos, metadata);
	}
	fclose(fd);

	return lista_archivos;
}

void mostrar_archivos(t_metadata* metadata){
	printf("Nombre: %s\n", metadata->nombre);
}

int agregar_archivo(char* archivo, char* archivos_abiertos){
	char* nombre_archivo = malloc(50*sizeof(char));
	strcpy(nombre_archivo, archivo);

	strcat(nombre_archivo,"\n");

	int fd_archivo;

	if((fd_archivo = open(archivos_abiertos, O_RDWR | O_APPEND)) == -1){
		perror("No se pudo abrir o crear el archivo correctamente!!!\n");
		return EXIT_FAILURE;
	}
	write(fd_archivo, nombre_archivo, strlen(nombre_archivo));
	close(fd_archivo);

	return EXIT_SUCCESS;
}
