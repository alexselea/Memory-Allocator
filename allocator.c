#include<stdio.h>
#include<stdlib.h>
#include<string.h>

//structura necesara functiilor SHOW
typedef struct{
	int free_bytes;
	int free_blocks;
	int used_bytes;
	int used_blocks;
	int reserved_bytes;
	int occupied_bytes_zone;
	int free_bytes_zone;
} arena;

unsigned char *initialize (int n)
{
	unsigned char *PointerArena;
	PointerArena = (unsigned char *) malloc(n *sizeof(char *));

	int i;
	//se face 0 pe fiecare octet din arena
	for (i = 0; i <= 100; i++)
	{
		PointerArena[i] = 0;
	}

	return PointerArena;
}

void finalize(unsigned char *PointerArena)
{
	//se elibereaza memoria alocata pentru arena
	free (PointerArena);
}

void dump (int n, unsigned char *PointerArena)
{
	int i;
	int ArenaLine = -1;// - 1 ca sa devina 0 in for
	for(i = 0; i < n; i++){
		ArenaLine++;    //arenaline aduna octetii si la fiecare 16, se printeaza coloana de rang
		if(i % 16 == 0){
			if(i != 0)printf("\n");

			/*printf: newline+numar+tab*/
			printf("%08X\t", ArenaLine);
		}

		if(i % 8 ==0){
			printf(" ");
		}

		printf("%02X ", PointerArena[i]);//printeaza in hexa ce e la pointerul PointerArena la poz i
	}

	ArenaLine++;
	printf("\n%08X\t", ArenaLine);
	printf("\n");
}

void fill (int index, int size, int value, unsigned char *PointerArena)
{
	int i;
	for(i = index; i < index + size; i++){
		PointerArena[i] = value;
	}
}

//declarare pt ca e folosita in my_alloc
void aloca( unsigned char *PointerArena, unsigned char *Ptr, unsigned char *Ptr_next, unsigned char *Ptr_previous, int size, int flag);

int my_alloc (int size, int n, unsigned char *PointerArena){
	int bytes_block;
	unsigned char *Ptr = PointerArena;
	unsigned char *Ptr_previous = PointerArena;
	unsigned char *Ptr_next = PointerArena;
	//Ptr este pointerul cu care ne plimbam prin arena
	//Ptr_previous si Ptr_next sunt doi pointeri ajutatori
	//pentru apelarea functiei aloca

	if( *(int *) PointerArena == 0){//cazul 1, in care arena e goala
		*(int *) PointerArena = 4;
		Ptr += 4;

		//aloca
		aloca( PointerArena, Ptr, Ptr, PointerArena, size, 1);
		return( *(int *)PointerArena + 12);
	}



	if( ( *(int *)Ptr != 4) && ( Ptr - PointerArena == 0) ){//cazul 2 in care arena e goala	imediat dupa primii 4 octeti, se verifica daca se poate aloca acolo
		if ( *(int *)Ptr - 4 >= size + 12){
			Ptr += 4;
			//aloca
			aloca(PointerArena, Ptr, PointerArena + *(int *)PointerArena, PointerArena, size, 2);
			return( Ptr + 12 - PointerArena);
		}
	}

	while( Ptr - PointerArena < n - 4){//altfel, cat timp se poate plimba pointerul Ptr prin 						arena, mai exista 2 cazuri
		if(Ptr - PointerArena > 0){
			if(( *(int *)Ptr == 0) && (PointerArena + n - Ptr >= 12)){
				bytes_block = * (int *)(Ptr + 8);

				if( (PointerArena + n) - Ptr >= size + 12 + bytes_block){
					Ptr_previous = Ptr;
					Ptr = Ptr + bytes_block;

					if ( Ptr + size + 12 - PointerArena <= n){
						//cazul 3 in care se aloca la finalul blocurilor
						aloca(PointerArena, Ptr, Ptr, Ptr_previous, size, 1);
						return(Ptr + 12 - PointerArena);
						//aloca
					}
					else{
						return 0;
					}

				}

				else{
					return 0;
					break;
				}
			}

			else{
				if( PointerArena + *(int *)Ptr - Ptr - *(int *)(Ptr + 8) >= size +12){
					//cazul 4 in care se aloca intre 2 blocuri existente
					Ptr_previous = Ptr;
					Ptr_next = PointerArena + *(int *)Ptr;
					Ptr += *(int *)(Ptr + 8);
					aloca(PointerArena, Ptr, Ptr_next, Ptr_previous, size, 4);
					return( Ptr - PointerArena + 12);
					//aloca
				}
			}
			//atribuirile necesare unei noi treceri prin while
			Ptr_previous = Ptr;
			Ptr = PointerArena + *(int *)Ptr;

		}
		else{
			Ptr = PointerArena + *(int *)PointerArena;
		}
	}
	return 0;
}

void aloca( unsigned char *PointerArena, unsigned char *Ptr, unsigned char *Ptr_next, unsigned char *Ptr_previous, int size, int flag)
{
	//flags

	*(int *) Ptr_previous = Ptr - PointerArena;

	if( flag != 1){
		//daca nu este in cazul 1 sau 3 de alocare, flag-ul primit va fi 1
		//ca sa nu "existe" un bloc viitor la care ar trebui schimbati
		//indicii
		*(int *)(Ptr_next + 4) = Ptr - PointerArena;
	}
	if( flag != 1){
		*(int *) Ptr = Ptr_next - PointerArena;
	}
	else{
		*(int *) Ptr = 0;
	}

	//modificarea zonei de gestiune a blocului nou alocat
	*(int *)(Ptr + 4) = Ptr_previous - PointerArena;
	*(int *)(Ptr + 8) = size + 12;

}

void Free( unsigned char *Ptr, unsigned char *PointerArena)
{
	//trimite Ptr ca PointerArena + size - 12;

	unsigned char *Ptr_previous = PointerArena + *(int *)(Ptr + 4);
	unsigned char *Ptr_next;

	//zona precedenta pointeaza spre next
	if(* (int *)Ptr){
		Ptr_next = PointerArena + *(int *)Ptr;
		*(int *)Ptr_previous = Ptr_next - PointerArena;
	}

	else{
		*(int *)Ptr_previous = 0;
	}

	//zona urmatoare pointeaza spre previous
	if(*(int *)Ptr){
		*(int *)(Ptr_next + 4) = Ptr_previous - PointerArena;

	}


}

void show_free(unsigned char *PointerArena, int n, arena *my_arena)
{

	unsigned char *Ptr = PointerArena;
	my_arena->free_bytes = 0;
	my_arena->free_blocks = 0;

	if( *(int *)Ptr == 0){
		//in aceasta functie checkerul considera ca primul block e ocupat si
		//bytes-ii sunt 96, spre deosebire de functia show usage unde nu se mai considera
		// :D
		my_arena->free_bytes = n - 4;
		my_arena->free_blocks = 1;
		return;
	}

	if(PointerArena + *(int *)Ptr - Ptr - 4 > 0){
		//daca intre unde pointeaza Ptr si PointerArena, e diferenta
		//mai mare de 4, exista un bloc liber intre prima zona de gestiune
		// si blocul urmator de memorie
		my_arena->free_bytes += PointerArena + *(int *)Ptr - Ptr - 4;
		my_arena->free_blocks++;
	}

	Ptr = PointerArena + *(int *)Ptr;

	while( Ptr < PointerArena + n){//Ptr se plimba din bloc in bloc in arena
		if( *(int *)Ptr){//daca la Ptr exista un indice nenul, mai exista un bloc

			if(PointerArena + *(int *)Ptr - Ptr - *(int *)(Ptr + 8) != 0){
				my_arena->free_bytes += PointerArena + *(int *)Ptr - Ptr - *(int *)(Ptr + 8);
				my_arena->free_blocks++;
			}

			Ptr = PointerArena + *(int *)Ptr;
		}

		else{
			break;
		}
	}

	if(PointerArena + n - Ptr - *(int *)(Ptr + 8)){
		my_arena->free_bytes += PointerArena + n - Ptr - *(int *)(Ptr + 8);
		my_arena->free_blocks++;
	}


}

void show_usage(unsigned char *PointerArena, int n, arena *my_arena)
{
	unsigned char *Ptr = PointerArena;

	my_arena->used_blocks = 0;
	my_arena->used_bytes = 0;

	if( *(int *)Ptr !=0){
		Ptr += *(int *)Ptr;

		while(Ptr < PointerArena + n){
			if( *(int *)Ptr){
				my_arena->used_bytes += *(int *)(Ptr + 8) - 12;
				my_arena->used_blocks++;
				Ptr = PointerArena + *(int *)Ptr;
			}

			else{
				my_arena->used_bytes += *(int *)(Ptr + 8) - 12;
				my_arena->used_blocks++;
				break;
			}
		}
	}
	else{
		my_arena->used_blocks = 0;
		my_arena->used_bytes = 0;
	}
	show_free(PointerArena, n, my_arena);
	//printf("%d free blocks\n", my_arena->free_blocks);
	my_arena->reserved_bytes = n - (my_arena->free_bytes);
}

void show_allocations(unsigned char *PointerArena, int n)
{
	int bytes_block = 0;
	unsigned char *Ptr = PointerArena;

	if(*(int *)Ptr){
		//primii 4 octeti se considera ocupati orice ar fi
		printf("OCCUPIED 4 bytes\n");
		if(*(int *)Ptr > 4){
			printf("FREE %d bytes\n",(int)( PointerArena + *(int *)Ptr  - Ptr - 4));
		}
		Ptr += *(int *)Ptr;
		while(Ptr < PointerArena +n){
			if( *(int *)Ptr){
				bytes_block = *(int *)(Ptr + 8);
				printf("OCCUPIED %d bytes\n", bytes_block);
				if( PointerArena + *(int *)Ptr - Ptr > bytes_block){
					printf("FREE %d bytes\n",(int)(PointerArena + *(int *)Ptr - Ptr - bytes_block));
				}

				Ptr = PointerArena + *(int *)Ptr;
			}

			else{
				bytes_block = *(int *)(Ptr + 8);
				printf("OCCUPIED %d bytes\n", bytes_block);
				if( PointerArena + n - Ptr > bytes_block){
					printf("FREE %d bytes\n",(int)(PointerArena + n - Ptr - bytes_block));
				}
				break;
			}
		}
	}

	else{
		printf("OCCUPIED 4 bytes\n");
		printf("FREE %d bytes\n", n - 4);
	}

}


int take_command(char *p, unsigned char *PointerArena, int n)
{
	int parametru;
	arena my_arena;
	arena * Pointer_to_my_arena = &my_arena;
	unsigned char *Ptr = PointerArena;

	if (strstr(p, "INITIALIZE")){
		p = strtok(NULL, " ");
		n = atoi(p);
		PointerArena = initialize(n);
		return n;
		//initialize(parametru);
	}

	if (strstr(p, "FINALIZE")){
		finalize(PointerArena);
		return 0;
		//finalize();
	}

	if (strstr(p, "DUMP")){
		dump(n, PointerArena);
		return 1;
		//dump();
	}

	if (strstr(p, "ALLOC")){
		p = strtok(NULL, " ");
		parametru = atoi(p);
		printf("%d\n",  my_alloc(parametru, n, PointerArena));
		return 1;
		//alloc(p);
	}

	if (strstr(p, "FREE")){
		p = strtok(NULL, " ");
		parametru = atoi(p);
		Ptr += parametru - 12;
		Free(Ptr, PointerArena);
		return 1;
		//free(parametru);
	}

	if(strstr(p, "FILL")){
		int index, size, value;
		p = strtok(NULL, " ");
		index = atoi(p);
		p = strtok(NULL, " ");
		size = atoi(p);
		p = strtok(NULL, " ");
		value = atoi(p);
		fill(index, size, value, PointerArena);
		return 1;
		//fill (index, size, value);
	}

	if(strstr(p, "SHOW")){
		p = strtok(NULL, " ");
		if(strstr(p, "FREE")){
			show_free(PointerArena, n, Pointer_to_my_arena);
			printf("%d blocks (%d bytes) free\n", Pointer_to_my_arena->free_blocks,
					Pointer_to_my_arena->free_bytes);

			return 1;
		}

		if(strstr(p, "USAGE")){
			show_usage(PointerArena, n, Pointer_to_my_arena);

			printf("%d blocks (%d bytes) used\n", Pointer_to_my_arena->used_blocks,
					Pointer_to_my_arena->used_bytes);

			printf("%d\%% efficiency\n",( (int)(( (float)(Pointer_to_my_arena->used_bytes)/
								(float)(Pointer_to_my_arena->reserved_bytes)) * 100) %100));

			int fragm;
			if(Pointer_to_my_arena->used_blocks != 0){

				fragm = ((Pointer_to_my_arena->free_blocks) - 1)*100/(Pointer_to_my_arena->used_blocks);
				printf("%d\%% fragmentation\n", fragm);
			}
			else{
				printf("0\%% fragmentation\n");
			}
			return 1;
			//show_usage
		}

		if(strstr(p, "ALLOCATIONS")){
			show_allocations(PointerArena, n);
			return 1;
			//show_allocations
		}
	}
}

int main()
{
	unsigned char *PointerArena;
	int n;
	arena my_arena;
	arena *pointer_structarena = &my_arena;


	//PointerArena = initialize (100);

	char str[100];
	char *p;
	fgets(str, 100, stdin);
	p = strtok(str, " ");
	p = strtok(NULL, " ");
	n = atoi(p);
	PointerArena = initialize(n);

	fgets(str, 100, stdin);
	p = strtok(str, " ");

	while(take_command(p, PointerArena, n)){
		fgets(str, 100, stdin);
		p = strtok(str, " ");
	}

	return 0;
}
