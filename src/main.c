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
	printf("6 - VER ARCHIVOS(BITMAP, BLOQUES\n");
	printf("7 - MOSTRAR ARCHIVOS\n");
	printf("8 - COMPACTAR\n");
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
				printf("Ingrese un nombre de archivo a truncar: ");
				scanf("%s", nombre_archivo);
				//nombre_archivo = recibir_string(buffer);
				fs_truncate(nombre_archivo, 1);
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
		case MOSTRAR_ARCHIVOS_DIRECTORIO:
				printf("Archivos\n");
				mostrar_archivos(PATH_BASE_DIALFS);
			break;
		case COMPACTAR:
				printf("Compactar\n");
				compactar();
			break;
		default:
			printf("Opcion invalida\n");
	}

	return EXIT_SUCCESS;
}

int fs_create(char* nombre){
	int bloque_libre;
	int tamanio = 0;

	if((bloque_libre = buscar_bloque_bitmap(cantidad_bloques(tamanio))) == -1){
		printf("No se puede encontrar un bloque libre\n");
	}

	asignar_bloque_bitmap(bloque_libre, cantidad_bloques(tamanio));

	crear_metadata(nombre, bloque_libre, tamanio);

	return EXIT_SUCCESS;
}

int fs_delete(char* archivo_metadata){
	t_metadata* metadata = datos_metadata(archivo_metadata);

	remover_bloque_bitmap(metadata->bloque_inicial, cantidad_bloques(metadata->tamanio_archivo));
	//Aqui debo borrar datos del bloques.dat

	remove(ruta_absoluta(archivo_metadata));
	free(metadata);

	return EXIT_SUCCESS;
}

int fs_truncate(char* ruta, int registro_tamanio){
	t_metadata* metadata = datos_metadata(ruta);

	int cantidad_bloques_actual = cantidad_bloques(metadata->tamanio_archivo);
	int cantidad_nueva_bloques = cantidad_bloques(registro_tamanio);

	metadata->tamanio_archivo = registro_tamanio;

	int diferencia_bloques = cantidad_nueva_bloques - cantidad_bloques_actual;

	int ultimo_bloque = cantidad_bloques_actual + metadata->bloque_inicial - 1;

	if(diferencia_bloques > 0){
		int nuevo_bloque;
		if((nuevo_bloque = buscar_bloque_bitmap(diferencia_bloques)) == -1){
			printf("No hay espacio libre para expandir el archivo, debe compactar el archivo!!!\n");
			//compactar(ARHCIVO_BITMAP);
			//Como compacto busco un nuevo lugar
			if((nuevo_bloque = buscar_bloque_bitmap(cantidad_bloques(cantidad_nueva_bloques))) == -1){
				printf("No hay espacio en memoria!!! debe esperar que se libere la memoria!!!");
				return EXIT_FAILURE;
			}else{
				asignar_bloque_bitmap(nuevo_bloque, cantidad_bloques(metadata->tamanio_archivo));
				remover_bloque_bitmap(metadata->bloque_inicial, cantidad_bloques_actual);
				modificar_metadata(ruta, nuevo_bloque, metadata->tamanio_archivo);
			}
		}else{
			if((nuevo_bloque - ultimo_bloque) == 1){
				asignar_bloque_bitmap(nuevo_bloque, diferencia_bloques);
				modificar_metadata(ruta, metadata->bloque_inicial, metadata->tamanio_archivo);
			}
			else{
				printf("No hay espacio suficiente, debe compactar(PERO HAY)!!!");
					//compactar(ARCHIVO_BITMAP);
					//Como compacto debo buscar todo el nuevo tamano completo
				if((nuevo_bloque = buscar_bloque_bitmap(cantidad_bloques(metadata->tamanio_archivo))) == -1){
					printf("No hay espacio en memoria!!! debe esperar que se libere la memoria!!!");
					return EXIT_FAILURE;
				}else{
					asignar_bloque_bitmap(nuevo_bloque, cantidad_bloques(metadata->tamanio_archivo));
					remover_bloque_bitmap(metadata->bloque_inicial, cantidad_bloques_actual);
					modificar_metadata(ruta, nuevo_bloque, metadata->tamanio_archivo);
				}
			}
		}
	}else{
		if(diferencia_bloques == 0){
			modificar_metadata(ruta, metadata->bloque_inicial, metadata->tamanio_archivo);
		}else{
			//Aca remueve del bitmap la cantidad que sobra
			remover_bloque_bitmap(metadata->bloque_inicial + cantidad_nueva_bloques, abs(diferencia_bloques));
			modificar_metadata(ruta, metadata->bloque_inicial, metadata->tamanio_archivo);
		}
	}
	free(metadata);

	return EXIT_SUCCESS;
}

int fs_write(char* ruta, int registro_direccion, int registro_tamanio, int registro_puntero){
	//Pedir a memoria que me devuelva la cantidad de registro_tamanio bytes en registro_direccion

	t_metadata* metadata = datos_metadata(ruta);

	int anterior_tamanio = metadata->tamanio_archivo;
	int cantidad_anterior_bloques = cantidad_bloques(anterior_tamanio);
	metadata->tamanio_archivo += registro_tamanio;

	int ultimo_bloque = cantidad_anterior_bloques + metadata->bloque_inicial - 1;

	int cantidad_extra_bloques = cantidad_bloques(metadata->tamanio_archivo) - cantidad_anterior_bloques;

	if(cantidad_extra_bloques == 0){
		//quiere decir que los datos entran en los bloques que ya tengo
		modificar_metadata(ruta, metadata->bloque_inicial, metadata->tamanio_archivo);
	}else{
		int nuevo_bloque;
		if((nuevo_bloque = buscar_bloque_bitmap(cantidad_anterior_bloques)) == -1){
			printf("No hay espacio suficiente, debe compactar!!!");

			//compactar(ARCHIVO_BITMAP);

			//Como compacto debo buscar todo el nuevo tamanio completo
			if((nuevo_bloque = buscar_bloque_bitmap(cantidad_bloques(metadata->tamanio_archivo))) == -1){
				printf("No hay espacio en memoria!!! debe esperar que se libere la memoria!!!");
				return EXIT_FAILURE;
			}else{
				asignar_bloque_bitmap(nuevo_bloque, cantidad_bloques(metadata->tamanio_archivo));
				remover_bloque_bitmap(metadata->bloque_inicial, cantidad_anterior_bloques);
				modificar_metadata(ruta, nuevo_bloque, metadata->tamanio_archivo);
			}
		}else{
			if((nuevo_bloque - ultimo_bloque) == 1){
				asignar_bloque_bitmap(nuevo_bloque, cantidad_extra_bloques);
				modificar_metadata(ruta, metadata->bloque_inicial, metadata->tamanio_archivo);
			}
			else{
				printf("No hay espacio suficiente, debe compactar!!!(PERO HAY)");

				//compactar(ARCHIVO_BITMAP);

				//Como compacto debo buscar todo el nuevo tamano completo
				if((nuevo_bloque = buscar_bloque_bitmap(cantidad_bloques(metadata->tamanio_archivo))) == -1){
					printf("No hay espacio en memoria!!! debe esperar que se libere la memoria!!!");
					return EXIT_FAILURE;
				}else{
					asignar_bloque_bitmap(nuevo_bloque, cantidad_bloques(metadata->tamanio_archivo));
					remover_bloque_bitmap(metadata->bloque_inicial, cantidad_anterior_bloques);
					modificar_metadata(ruta, nuevo_bloque, metadata->tamanio_archivo);
				}
			}
		}
	}
	free(metadata);

	return EXIT_SUCCESS;
}

int cantidad_bloques(int tamanio){
	int cantidad = ceil(tamanio/8);
	if(cantidad == 0){
		return 1;
	}
	return cantidad;
}

int asignar_bloque_bitmap(int bloque_libre, int longitud){

	for(int i = bloque_libre;i<(bloque_libre+longitud);i++){
		bitarray_set_bit(buffer_bitmap, i);
	}

	msync(buffer_bitmap, buffer_bitmap->size, MS_SYNC);

	return EXIT_SUCCESS;
}

int remover_bloque_bitmap(int bloque_inicial, int longitud){
	for(int i = bloque_inicial;i<(bloque_inicial+longitud);i++){
		bitarray_clean_bit(buffer_bitmap, i);
	}

	msync(buffer_bitmap, buffer_bitmap->size, MS_SYNC);

	return EXIT_SUCCESS;
}

int buscar_bloque_bitmap(int longitud){
	int n_bloque = -1; //Si devuelve este valor, no hay lugar libre para ese tamaño
	int flag = 0; // 0  es false, 1 es true
	int contador_espacios = 0;
	for(int i=0;i<buffer_bitmap->size;i++){
		if(!bitarray_test_bit(buffer_bitmap, i)){
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

	msync(buffer_bitmap, buffer_bitmap->size, MS_SYNC);

	return n_bloque;
}

char* ruta_absoluta(char* ruta_relativa){
	int tamanio_ruta = strlen(PATH_BASE_DIALFS) + strlen(ruta_relativa);
	char* ruta = malloc(tamanio_ruta*sizeof(char));

	strcpy(ruta, PATH_BASE_DIALFS);
	strcat(ruta, "/");
	strcat(ruta, ruta_relativa);

	return ruta;
}

void obtener_datos_filesystem(){
	obtener_archivo(ARCHIVO_BITMAP, (void*) crear_bloques, (void*) abrir_bloques);
	obtener_archivo(ARCHIVO_BLOQUE, (void*) crear_bitmap, (void*) abrir_bitmap);
}

int obtener_archivo(char* ruta, void (*tipo_creacion)(int), void (*tipo_apertura)(int)){
	if(access(ruta_absoluta(ruta), F_OK) == -1){
		crear_archivo(ruta, (*tipo_creacion));
	}else{
		abrir_archivo(ruta, (*tipo_apertura));
	}

	return EXIT_SUCCESS;
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

void crear_bloques(int file_descriptor){
	int tamanio_archivo = BLOCK_COUNT * ceil(BLOCK_SIZE/8);

	ftruncate(file_descriptor, tamanio_archivo);

	struct stat buf;

	fstat(file_descriptor, &buf);
	buffer_bloques = mmap(NULL, buf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, file_descriptor, 0);

}

void crear_bitmap(int file_descriptor){
	int tamanio_archivo = BLOCK_COUNT;

	ftruncate(file_descriptor, tamanio_archivo);

	char* buffer;
	struct stat buf;

	fstat(file_descriptor, &buf);
	buffer = mmap(NULL, buf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, file_descriptor, 0);
	buffer_bitmap = bitarray_create_with_mode(buffer, buf.st_size, LSB_FIRST);

	for(int i=0;i<buffer_bitmap->size;i++){
		bitarray_clean_bit(buffer_bitmap, i);
	}
	msync(buffer_bitmap, buffer_bitmap->size, MS_SYNC);
}

int abrir_archivo(char* ruta, void (*tipo_Archivo)(int)){
	int fd;

	if((fd = open(ruta_absoluta(ruta), O_RDWR)) == -1){
		printf("No se pudo abrir el archivo!!!\n");
		return EXIT_FAILURE;
	}else{
		(*tipo_Archivo)(fd);
		close(fd);
	}

	return EXIT_SUCCESS;
}

void abrir_bloques(int file_descriptor){
	struct stat buf;

	fstat(file_descriptor, &buf);
	buffer_bloques = mmap(NULL, buf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, file_descriptor, 0);

}

void abrir_bitmap(int file_descriptor){
	char* buffer;
	struct stat buf;

	fstat(file_descriptor, &buf);
	buffer = mmap(NULL, buf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, file_descriptor, 0);
	buffer_bitmap = bitarray_create_with_mode(buffer, buf.st_size, LSB_FIRST);

	msync(buffer_bitmap, buffer_bitmap->size, MS_SYNC);
}

int abrir_archivos(){
		int aux=0;

		for(int i=0;i<buffer_bitmap->size;i++){
			printf("%i - ", i);
			if(bitarray_test_bit(buffer_bitmap,i)){
				printf("1\t");
			}else{
				printf("0\t");
			}
			for(int j=0;j<cantidad_bloques(BLOCK_SIZE);j++){
				printf("%c", buffer_bloques[j+aux]);
			}
			printf("\n");
			aux+=cantidad_bloques(BLOCK_SIZE);
		}

	return EXIT_SUCCESS;
}

t_list* cargar_archivos(char* ruta_directorio){
	t_list* lista_archivos = list_create();
	DIR *dir = opendir(ruta_directorio);
	if(!dir){
		perror("opendir");
		return NULL;
	}

	struct dirent *ent;
	while((ent = readdir(dir))){
		//Muestra todos los archivos menos bloques.dat y bitmap.dat
		if(strlen(ent->d_name) > 4 && strcmp(ent->d_name + strlen(ent->d_name)-4,".txt") == 0){
			t_metadata* metadata = datos_metadata(ent->d_name);
			metadata->nombre = malloc(strlen(ent->d_name)*sizeof(char));
			strcpy(metadata->nombre, ent->d_name);

			list_add_sorted(lista_archivos, metadata, (void*) comparar_bloque_inicial);
		}
	}
	closedir(dir);

	return lista_archivos;
}

bool comparar_bloque_inicial(t_metadata* metadata_lista, t_metadata* metadata){
	return metadata_lista->bloque_inicial < metadata->bloque_inicial;
}

int mostrar_archivos(char* ruta_directorio){
	t_list* lista_archivos = cargar_archivos(ruta_directorio);

	list_iterate(lista_archivos, (void*) mostrar_lista_archivos);

	return EXIT_SUCCESS;
}

int compactar(){
	t_list* lista_archivos = cargar_archivos(PATH_BASE_DIALFS);

	//Aca debe limpiar todo el bufffer, es mas facil?
	for(int i=0;i<buffer_bitmap->size;i++){
		bitarray_clean_bit(buffer_bitmap, i);
		msync(buffer_bitmap, buffer_bitmap->size, MS_SYNC);
	}

	int contador_posicion = 0;
	for(int i=0;i<list_size(lista_archivos);i++){
		t_metadata* metadata = (t_metadata*) list_get(lista_archivos, i); //Agarra el primer elemento de la lista
		int nuevo_bloque_inicial = contador_posicion;

		for(int j=contador_posicion; j<(contador_posicion + cantidad_bloques(metadata->tamanio_archivo));j++){
			bitarray_set_bit(buffer_bitmap, j);
			msync(buffer_bitmap, buffer_bitmap->size, MS_SYNC);
		}
		contador_posicion += cantidad_bloques(metadata->tamanio_archivo);

		modificar_metadata(metadata->nombre, nuevo_bloque_inicial, metadata->tamanio_archivo);
	}

	return EXIT_SUCCESS;
}

void mostrar_lista_archivos(t_metadata* metadata){
	printf("Nombre_archivo: %s - Bloque_inicial: %i\n", metadata->nombre, metadata->bloque_inicial);
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
