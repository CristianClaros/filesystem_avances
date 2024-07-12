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

	switch(opcion){
		case IO_FS_CREATE:
				printf("Creando archivos\n");
			break;

		case VER_ARCHIVO:
				printf("Archivos\n");
				abrir_archivo(ruta_absoluta("bloques.dat"), (void*)ver_bloques);
			break;
		default:
			printf("Opcion invalida\n");
	}

	return EXIT_SUCCESS;
}

void obtener_datos_filesystem(){
	obtener_archivo(ruta_absoluta("bloques.dat"), (void*) bloques);
	obtener_archivo(ruta_absoluta("bitmap.dat"), (void*) bitmap);
}

int obtener_archivo(char* ruta, void (*tipo_archivo)(int)){
	if(access(ruta, F_OK) == -1){
		crear_archivo(ruta, (*tipo_archivo));
	}else{
		printf("Archivo %s abierto correctamente!!!\n", ruta);
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

	if((fd = open(ruta, O_CREAT|O_RDWR|O_TRUNC, S_IRWXU)) == -1){
		printf("No se pudo crear el archivo!!!\n");
		return EXIT_FAILURE;
	}else{
		(*tipo_Archivo)(fd);

		close(fd);
	}
	printf("Archivo %s creado correctamente!!!\n", ruta);

	return EXIT_SUCCESS;
}

int abrir_archivo(char* ruta, void (*tipo_Archivo)(int)){
	int fd;

	if((fd = open(ruta, O_RDWR|O_TRUNC)) == -1){
		printf("No se pudo abrir el archivo!!!\n");
		return EXIT_FAILURE;
	}else{
		(*tipo_Archivo)(fd);
		close(fd);
	}
	printf("Archivo %s abierto correctamente!!!\n", ruta);

	return EXIT_SUCCESS;
}

void ver_bloques(int file_descriptor){
	struct stat buf;

	fstat(file_descriptor, &buf);
	char* buffer = mmap(NULL, buf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, file_descriptor, 0);
	for(int i=0;i<buf.st_size;i++){
		if(i%8 == 0){
			printf("\n");
		}
		printf("%c", buffer[i]);
	}
}

void bloques(int file_descriptor){
	int tamanio_archivo = BLOCK_COUNT * ceil(BLOCK_SIZE/8);

	ftruncate(file_descriptor, tamanio_archivo);
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

	for(int i=0;i<buf.st_size;i++){
		bitarray_clean_bit(bitmap, i);
	}
}

int crear_metadata(char* ruta, char* bloque_inicial, char* tamanio_archivo){
	int fd;

	if((fd = open(ruta, O_CREAT, S_IRWXU)) == -1){
		perror("No se pudo crear el archivo!!!\n");
		return EXIT_FAILURE;
	}else{
		close(fd);

		modificar_metadata(ruta, bloque_inicial, tamanio_archivo);
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

int modificar_metadata(char* ruta, char* bloque_inicial, char* tamanio_archivo){
	t_config* config = config_create(ruta);
	config_set_value(config, "BLOQUE_INICIAL", bloque_inicial);
	config_set_value(config, "TAMANIO_ARCHIVO", tamanio_archivo);
	config_save(config);

	config_destroy(config);

	return EXIT_SUCCESS;
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
