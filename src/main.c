#include "../include/main.h"

int main(){

	obtener_datos_filesystem();

	int opcion;

	do{
		menu();

		scanf("%i", &opcion);

		ejecutar_opcion(opcion);

	}while(opcion != EXIT);

	//Limpia datos
	munmap(buffer_bloques, strlen(buffer_bloques)+1);
	munmap(buffer_bitmap, buffer_bitmap->size);
	munmap(memoria, strlen(memoria)+1);



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
	printf("6 - VER ARCHIVOS(BITMAP, BLOQUES, MEMORIA)\n");
	printf("7 - MOSTRAR ARCHIVOS\n");
	printf("8 - COMPACTAR\n");
	printf("9 - MEMORIA\n");
	printf("Opcion: ");

	return EXIT_SUCCESS;
}

int ejecutar_opcion(int opcion){
	char* nombre_archivo = malloc(50*sizeof(char));
	int registro_direccion; //Seria direccion logica
	int registro_tamanio;//Cantidad de datos
	int registro_puntero; //Desde donde comienza apartir de la direccion logica;

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

				fs_delete(nombre_archivo);
			break;
		case IO_FS_TRUNCATE:
				printf("Truncando archivos\n");
				printf("Ingrese un nombre de archivo a truncar: ");
				scanf("%s", nombre_archivo);
				printf("Ingrese registro tamanio: ");
				scanf("%i", &registro_tamanio);
				fs_truncate(nombre_archivo, registro_tamanio);
			break;
		case IO_FS_WRITE:
				printf("Escribiendo archivos\n");
				printf("Ingrese un nombre de archivo a Escribir: ");
				scanf("%s", nombre_archivo);
				printf("Ingrese registro direccion: ");
				scanf("%i", &registro_direccion);
				printf("Ingrese registro tamanio: ");
				scanf("%i", &registro_tamanio);
				printf("Ingrese registro puntero: ");
				scanf("%i", &registro_puntero);
				fs_write(nombre_archivo, registro_direccion, registro_tamanio, registro_puntero);
			break;
		case IO_FS_READ:
				printf("Leyendo archivos\n");
				printf("Ingrese un nombre de archivo a leer: ");
				scanf("%s", nombre_archivo);
				printf("Ingrese registro direccion: ");
				scanf("%i", &registro_direccion);
				printf("Ingrese registro tamanio: ");
				scanf("%i", &registro_tamanio);
				printf("Ingrese registro puntero: ");
				scanf("%i", &registro_puntero);
				fs_read(nombre_archivo, registro_direccion, registro_tamanio, registro_puntero);
			break;
		case VER_ARCHIVO:
				printf("Archivos\n");
				abrir_archivos("bloques.dat", "bitmap.dat");
			break;
		case MOSTRAR_ARCHIVOS_DIRECTORIO:
				printf("Archivos\n");
				mostrar_archivos(PATH_BASE_DIALFS);
			break;
		case COMPACTAR:
				printf("Compactar\n");
				compactar(0);
			break;
		case	DATOS_MEMORIA:
				printf("Datos memoria\n");
				printf("Ingrese registro direccion: ");
				scanf("%i", &registro_direccion);
				printf("Ingrese registro tamanio: ");
				scanf("%i", &registro_tamanio);
				printf("Ingrese registro puntero: ");
				scanf("%i", &registro_puntero);
				char* datos = datos_memoria(registro_direccion, registro_tamanio);
				printf("Datos: (%s)", datos);

				free(datos);
			break;
		case 15:
			return EXIT_SUCCESS;
		default:
			printf("Opcion invalida\n");
	}

	free(nombre_archivo);

	return EXIT_SUCCESS;
}

//Funciones del dialFs
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
	remover_datos_bloque(inicio_bloque(metadata->bloque_inicial), metadata->tamanio_archivo);

	char* ruta_archivo = ruta_absoluta(archivo_metadata);
	remove(ruta_archivo);

	free(metadata);
	free(ruta_archivo);

	return EXIT_SUCCESS;
}

int fs_truncate(char* ruta, int registro_tamanio){
	t_metadata* metadata = datos_metadata(ruta);

	int tamanio_anterior = metadata->tamanio_archivo;
	int cantidad_bloques_actual = cantidad_bloques(metadata->tamanio_archivo);
	int cantidad_nueva_bloques = cantidad_bloques(registro_tamanio);
	int nuevo_tamanio = registro_tamanio;

	int diferencia_bloques = cantidad_nueva_bloques - cantidad_bloques_actual;

	int ultimo_bloque = cantidad_bloques_actual + metadata->bloque_inicial - 1;
	if(diferencia_bloques > 0){ //Quiere decir que el archivo se va a expandir
		int nuevo_bloque;
		if((nuevo_bloque = buscar_bloque_bitmap(diferencia_bloques)) == -1){
			printf("No hay espacio libre para expandir el archivo, debe compactar el archivo!!!\n");

			compactar(1);

			free(metadata);

			t_metadata* metadata_compactacion_1 = datos_metadata(ruta);
			ultimo_bloque = cantidad_bloques_actual + metadata_compactacion_1->bloque_inicial - 1;

			//Como compacto busco un nuevo lugar
			if((nuevo_bloque = buscar_bloque_bitmap(cantidad_bloques(cantidad_nueva_bloques))) == -1){
				printf("No hay espacio en memoria!!! debe esperar que se libere la memoria\n!!!");
				free(metadata_compactacion_1);
				return EXIT_FAILURE;
			}else{
				printf("Copiando todos los datos al nuevo lugar\n");
				asignar_bloque_bitmap(nuevo_bloque, cantidad_bloques(nuevo_tamanio));
				//Primero copio lo que estaba en los bloques anteriores al inicio del nuevo bloque
				escribir_datos_bloque(buffer_bloques + inicio_bloque(metadata_compactacion_1->bloque_inicial), inicio_bloque(nuevo_bloque), metadata_compactacion_1->tamanio_archivo);
				//Ahora limpio el espacio donde estaba el bloque antes
				remover_datos_bloque(inicio_bloque(metadata_compactacion_1->bloque_inicial), tamanio_anterior);
				remover_bloque_bitmap(metadata_compactacion_1->bloque_inicial, cantidad_bloques_actual);

				modificar_metadata(ruta, nuevo_bloque, nuevo_tamanio);
				free(metadata_compactacion_1);
			}
		}else{
			if((nuevo_bloque - ultimo_bloque) == 1){//Los bloques son contiguos
				printf("Los bloques son contiguos \n");
				asignar_bloque_bitmap(nuevo_bloque, diferencia_bloques);
				modificar_metadata(ruta, metadata->bloque_inicial, nuevo_tamanio);
			}
			else{
				printf("No hay espacio contiguo suficiente, debe compactar!!!");

				compactar(1);

				t_metadata* metadata_compactacion_2 = datos_metadata(ruta);
				ultimo_bloque = cantidad_bloques_actual + metadata_compactacion_2->bloque_inicial - 1;

					//Como compacto debo buscar todo el nuevo tamano completo
				if((nuevo_bloque = buscar_bloque_bitmap(cantidad_bloques(nuevo_tamanio))) == -1){
					printf("No hay espacio en memoria!!! debe esperar que se libere la memoria!!!");
					free(metadata_compactacion_2);
					return EXIT_FAILURE;
				}else{
					printf("Copiando datos al nuevo lugar\n");
					asignar_bloque_bitmap(nuevo_bloque, cantidad_bloques(nuevo_tamanio));
					//Primero copio lo que estaba en los bloques anteriores al inicio del nuevo bloque
					escribir_datos_bloque(buffer_bloques + inicio_bloque(metadata_compactacion_2->bloque_inicial), inicio_bloque(nuevo_bloque), metadata_compactacion_2->tamanio_archivo);
					//Ahora limpio el espacio donde estaba el bloque
					remover_datos_bloque(inicio_bloque(metadata_compactacion_2->bloque_inicial), tamanio_anterior);
					remover_bloque_bitmap(metadata_compactacion_2->bloque_inicial, cantidad_bloques_actual);
					modificar_metadata(ruta, nuevo_bloque, nuevo_tamanio);

					free(metadata_compactacion_2);
				}
			}
		}
	}else{
		if(diferencia_bloques == 0){
			//Aca entra en el mismo bloque
			modificar_metadata(ruta, metadata->bloque_inicial, nuevo_tamanio);
		}else{
			//Aca remueve del bitmap la cantidad que sobra
			remover_bloque_bitmap(metadata->bloque_inicial + cantidad_nueva_bloques, abs(diferencia_bloques));
			modificar_metadata(ruta, metadata->bloque_inicial, nuevo_tamanio);
		}
	}
	free(metadata);

	return EXIT_SUCCESS;
}

int fs_write(char* ruta, int registro_direccion, int registro_tamanio, int registro_puntero){
	//Pedir a memoria que me devuelva la cantidad de registro_tamanio bytes en registro_direccion
	char* datos = "Hola Lorena, como estas, espero que bien, quiero que sepas que te sigo amando con locura :(";
	//Arriba es memoria

	t_metadata* metadata = datos_metadata(ruta);

	int tamanio_bloques = cantidad_bloques(metadata->tamanio_archivo)*BLOCK_SIZE/8; //Es el peso en bytes de los bloques que ocupa

	if(registro_puntero > tamanio_bloques){ //Si el registro_puntero es mas grande que el bloque
		printf("No se puede escribir en ese sector, pertenece a otro archivo!!!\n");

		free(metadata);
		return EXIT_FAILURE;
	}else{
		if(registro_puntero + registro_tamanio > tamanio_bloques){ //SI registro_puntero + tamanio se pasan del tamanio de bloques
			escribir_datos_bloque(datos, inicio_bloque(metadata->bloque_inicial) + registro_puntero, tamanio_bloques - registro_puntero);
			modificar_metadata(ruta, metadata->bloque_inicial, tamanio_bloques);
		}else{
			if((registro_puntero + registro_tamanio) > metadata->tamanio_archivo){ //Este caso es cuando puntero + tamanio es mas grande que tamanio_archivo
				escribir_datos_bloque(datos, inicio_bloque(metadata->bloque_inicial) + registro_puntero, registro_tamanio);
				modificar_metadata(ruta, metadata->bloque_inicial, registro_puntero + registro_tamanio);
			}else{
				escribir_datos_bloque(datos, inicio_bloque(metadata->bloque_inicial) + registro_puntero, registro_tamanio);
			}
		}
	}

	free(metadata);

	return EXIT_SUCCESS;
}

int fs_read(char* ruta, int registro_direccion, int registro_tamanio, int registro_puntero){
	t_metadata* metadata = datos_metadata(ruta);

	if(registro_puntero + registro_tamanio > metadata->tamanio_archivo){
		//Esto es a memoria
		memcpy(memoria + registro_direccion, buffer_bloques + inicio_bloque(metadata->bloque_inicial) + registro_puntero, abs(metadata->tamanio_archivo - registro_puntero));
		//
	}
	else{
		//Esto es memoria
		memcpy(memoria + registro_direccion, buffer_bloques + inicio_bloque(metadata->bloque_inicial) + registro_puntero, registro_tamanio);
	}

	free(metadata);

	return EXIT_SUCCESS;
}

//Funciones generales para bitmap y bloques
char* ruta_absoluta(char* ruta_relativa){
	int tamanio_ruta = strlen(PATH_BASE_DIALFS) + strlen(ruta_relativa) + 2;
	char* ruta = malloc(tamanio_ruta*sizeof(char));

	strcpy(ruta, PATH_BASE_DIALFS);
	strcat(ruta, "/");
	strcat(ruta, ruta_relativa);

	return ruta;
}

void obtener_datos_filesystem(){
	obtener_archivo("bloques.dat", (void*) crear_bloques, (void*) abrir_bloques);
	obtener_archivo("bitmap.dat", (void*) crear_bitmap, (void*) abrir_bitmap);
	//Memoria
	obtener_archivo("memoria.dat", (void*) crear_memoria, (void*)abrir_memoria);
}

int obtener_archivo(char* ruta, void (*tipo_creacion)(int), void (*tipo_apertura)(int)){
	char* ruta_archivo = ruta_absoluta(ruta);
	if(access(ruta_archivo, F_OK) == -1){
		crear_archivo(ruta, (*tipo_creacion));
	}else{
		abrir_archivo(ruta, (*tipo_apertura));
	}

	free(ruta_archivo);

	return EXIT_SUCCESS;
}

int crear_archivo(char* ruta, void (*tipo_Archivo)(int)){
	int fd;

	char* ruta_archivo = ruta_absoluta(ruta);

	if((fd = open(ruta_archivo, O_CREAT|O_RDWR|O_TRUNC, S_IRWXU)) == -1){
		printf("No se pudo crear el archivo!!!\n");
		return EXIT_FAILURE;
	}else{
		(*tipo_Archivo)(fd);
		close(fd);
	}

	free(ruta_archivo);

	return EXIT_SUCCESS;
}


int abrir_archivo(char* ruta, void (*tipo_Archivo)(int)){
	int fd;

	char* ruta_archivo = ruta_absoluta(ruta);

	if((fd = open(ruta_archivo, O_RDWR)) == -1){
		printf("No se pudo abrir el archivo!!!\n");
		return EXIT_FAILURE;
	}else{
		(*tipo_Archivo)(fd);
		close(fd);
	}

	free(ruta_archivo);

	return EXIT_SUCCESS;
}

//Funciones de bloques.dat
void crear_bloques(int file_descriptor){
	int tamanio_archivo = BLOCK_COUNT * ceil(BLOCK_SIZE/8);

	ftruncate(file_descriptor, tamanio_archivo);

	struct stat buf;

	fstat(file_descriptor, &buf);
	buffer_bloques = mmap(NULL, buf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, file_descriptor, 0);

	int posicion = 0;
	for(int i=0;i<tamanio_archivo;i++){
		memset(buffer_bloques + posicion, 0x00, sizeof(char));
		posicion++;
	}
	msync(buffer_bloques, buf.st_size, MS_SYNC);

}

void abrir_bloques(int file_descriptor){
	struct stat buf;

	fstat(file_descriptor, &buf);
	buffer_bloques = mmap(NULL, buf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, file_descriptor, 0);

}

int inicio_bloque(int n_bloque){
	int bytes;

	bytes = n_bloque * (BLOCK_SIZE/8);

	return bytes;
}

int cantidad_bloques(int tamanio){
	int cantidad = ((tamanio+7)/ 8);
	if(cantidad == 0){
		return 1;
	}
	return cantidad;
}

int escribir_datos_bloque(char* buffer, int posicion_inicial, int cantidad){
	memcpy(buffer_bloques + posicion_inicial, buffer, cantidad);

	msync(buffer_bloques, strlen(buffer_bloques),MS_SYNC);

	return EXIT_SUCCESS;
}

int remover_datos_bloque(int posicion_inicial, int cantidad){
	for(int i = posicion_inicial;i< (posicion_inicial + cantidad);i++){
		memset(buffer_bloques + i, 0x00, sizeof(char));
	}
	msync(buffer_bloques, strlen(buffer_bloques),MS_SYNC);

	return EXIT_SUCCESS;
}

//Funciones de bitmap.dat
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

void abrir_bitmap(int file_descriptor){
	char* buffer;
	struct stat buf;

	fstat(file_descriptor, &buf);
	buffer = mmap(NULL, buf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, file_descriptor, 0);
	buffer_bitmap = bitarray_create_with_mode(buffer, buf.st_size, LSB_FIRST);

	msync(buffer_bitmap, buffer_bitmap->size, MS_SYNC);
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

//Archivos de metadata

int crear_metadata(char* ruta, int bloque_inicial, int tamanio_archivo){
	int fd;
	char* ruta_archivo = ruta_absoluta(ruta);

	if((fd = open(ruta_archivo, O_CREAT, S_IRWXU)) == -1){
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
		free(ruta_archivo);
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
	char* ruta_archivo = ruta_absoluta(ruta);
	t_config* config = config_create(ruta_archivo);

	char* bloque_string = malloc(50*sizeof(char));
	char* tamanio_string = malloc(50*sizeof(char));

	sprintf(bloque_string, "%i", bloque_inicial);
	sprintf(tamanio_string, "%i", tamanio_archivo);

	config_set_value(config, "BLOQUE_INICIAL", bloque_string);
	config_set_value(config, "TAMANIO_ARCHIVO", tamanio_string);
	config_save(config);

	free(ruta_archivo);
	free(bloque_string);
	free(tamanio_string);
	config_destroy(config);

	return EXIT_SUCCESS;
}

t_metadata* datos_metadata(char* ruta){
	t_metadata* metadata = malloc(sizeof(t_metadata));
	char* ruta_archivo = ruta_absoluta(ruta);
	t_config* config = config_create(ruta_archivo);

	metadata->bloque_inicial = config_get_int_value(config, "BLOQUE_INICIAL");
	metadata->tamanio_archivo = config_get_int_value(config, "TAMANIO_ARCHIVO");

	free(ruta_archivo);
	config_destroy(config);

	return metadata;
}

//Proceso de compactacion

int compactar(int pid){
	t_list* lista_archivos = cargar_archivos(PATH_BASE_DIALFS);

	//Aca debe limpiar todo el buffer del bitmap
	remover_bloque_bitmap(0, buffer_bitmap->size);

	int contador_posicion = 0;
	for(int i=0;i<list_size(lista_archivos);i++){
		t_metadata* metadata = (t_metadata*) list_get(lista_archivos, i); //Agarra el primer elemento de la lista
		int nuevo_bloque_inicial = contador_posicion;

		//Muevo todos los datos del bloque al nuevo bloque
		if(metadata->bloque_inicial != contador_posicion){
			escribir_datos_bloque(buffer_bloques + inicio_bloque(metadata->bloque_inicial), nuevo_bloque_inicial, metadata->tamanio_archivo);
		}
		asignar_bloque_bitmap(contador_posicion, cantidad_bloques(metadata->tamanio_archivo)); //Aca voy seteando el bitmap con los datos del archivo

		contador_posicion += cantidad_bloques(metadata->tamanio_archivo);

		modificar_metadata(metadata->nombre, nuevo_bloque_inicial, metadata->tamanio_archivo);

		free(metadata->nombre);
		free(metadata);
	}

	list_destroy(lista_archivos);

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
			metadata->nombre = malloc(50*sizeof(char));
			strcpy(metadata->nombre, ent->d_name);

			//Mete todo a una lista pero ordenada segun su bloque inicial
			list_add_sorted(lista_archivos, metadata, (void*) comparar_bloque_inicial);
		}
	}
	closedir(dir);

	return lista_archivos;
}

bool comparar_bloque_inicial(t_metadata* metadata_lista, t_metadata* metadata){
	return metadata_lista->bloque_inicial < metadata->bloque_inicial;
}

//De aca para abajo solo son funciones para ver resultados esperados y testear

//Solo son funciones para ver archivos
int mostrar_archivos(char* ruta_directorio){
	t_list* lista_archivos = cargar_archivos(ruta_directorio);

	list_iterate(lista_archivos, (void*) mostrar_lista_archivos);

	return EXIT_SUCCESS;
}

void mostrar_lista_archivos(t_metadata* metadata){
	printf("Nombre_archivo: %s - Bloque_inicial: %i\n", metadata->nombre, metadata->bloque_inicial);
}

//Funciones para onservar datos solamente
int abrir_archivos(){
		for(int i=0;i<buffer_bitmap->size;i++){
			printf("%i - ", i);
			if(bitarray_test_bit(buffer_bitmap,i)){
				printf("1\t");
			}else{
				printf("0\t");
			}
			char* bloque = malloc((BLOCK_SIZE/8)*sizeof(char));
			strncpy(bloque, buffer_bloques + inicio_bloque(i), (BLOCK_SIZE/8)+1);
			printf("%s\n", bloque);
			free(bloque);
		}

	return EXIT_SUCCESS;
}

//solo para implementar memoria
void crear_memoria(int file_descriptor){
	char* nota = "Tengo un amor Un amor que se alquila Estaba triste, solo y sin amor Y en un local yo di con ella La cosa más bonita del lugar Me dio su amor por dos monedas Nunca creí poderme enamorar Pero ya ven lo que ha pasado Y, aunque ella vende amor en la ciudad Ella es mi amor, la que yo amo Y en ese cuarto con luz roja Cayó en el suelo tu ropa y mi ropa Así nos amamos, y fuimos sudando Gotitas de amor y placer En tu cuerpo alquilado Amor de alquiler Que no me reprochas, que tarde he llegado Amor de alquiler Tu nombre en mi piel lo llevo tatuado Amor de alquiler No importa saber ¿con quién has estado? Amor de alquiler Quisiera poder morirme a tu lado ¡Quiero morirme a tu lado! ¡Sí! Estaba triste, solo y sin amor Y en un local yo di con ella La cosa más bonita del lugar Me dio su amor por dos monedas Nunca creí poderme enamorar Pero ya ven lo que ha pasado Y, aunque ella vende amor en la ciudad Ella es mi amor, la que yo amo Y en ese cuarto con luz roja Cayó en el suelo tu ropa y mi ropa Así nos amamos, y fuimos sudando Gotitas de amor y placer En tu cuerpo alquilado Amor de alquiler Que no me reprochas, que tarde he llegado Amor de alquiler Tu nombre en mi piel lo llevo tatuado Amor de alquiler No importa saber, ¿con quién te has acostado? Amor de alquiler Quisiera poder morirme a tu lado Amor de alquiler Que no me reprochas, que tarde he llegado Amor de alquiler Tu nombre en mi piel lo llevo tatuado Amor de alquiler No importa saber ¿con quién has estado? Amor de alquiler Quisiera poder morirme a tu lado ¡Quiero morirme a tu lado!";
	ftruncate(file_descriptor, TAMANIO_MEMORIA);

	struct stat buf;

	fstat(file_descriptor, &buf);
	memoria = mmap(NULL, buf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, file_descriptor, 0);
	if(strlen(nota) > buf.st_size){
		memcpy(memoria, nota, buf.st_size);
	}else{
		memcpy(memoria, nota, strlen(nota));
	}


	msync(memoria, buf.st_size, MS_SYNC);

}

void abrir_memoria(int file_descriptor){
	struct stat buf;

	fstat(file_descriptor, &buf);
	memoria = mmap(NULL, buf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, file_descriptor, 0);

}

char* datos_memoria(int registro_direccion, int registro_tamanio){
	char* datos = malloc(registro_tamanio*sizeof(char));

	memcpy(datos, memoria + registro_direccion, registro_tamanio);

	free(datos);

	return datos;
}
