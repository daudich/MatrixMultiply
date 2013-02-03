/*
 * ASSIGNMENT NAME	: Cannon's Algorith
 * 
 * AUTHOR			: Dhiren Audich
 * 
 * STUDENT ID		: 0694717
 * 
 * PROGRAM NAME		: cannon.c
 * 
 * DATE CREATED		: 29 - Nov - 2012
 * 
 * DATE MODIFIED	: 29 - Nov - 2012
 * 
 * DESCRIPTION		: This programs multiplies two matrices using Cannon's Algorithm and utilises Pilot libraries for inter-processor messaging.
 * 
 */
 
//x = row
//y = column

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pilot.h>

typedef struct sample{
	
	int row, column;
	int ** matrix;
	struct sample * next;
	
} SAMPLE;

typedef struct work{
	
	/*
	 * Where from : x, y
	 * Where to: dx, dy
	 * Coordinates from the reference matrix(@ x, y): ax, ay, bx, by
	 */
	int x, y, dx, dy;
	struct work * next;
	
} WORK;

typedef struct reference{
	
	int Ax, Ay, Bx, By;
	
} REFERENCE;

PI_PROCESS **Worker;
PI_CHANNEL **toWorker;
PI_CHANNEL **result;

SAMPLE * matrices = NULL;
REFERENCE ** refMatrix = NULL;
int dimensions = -1, GAx = 0, GAy = 0, GBx = 0, GBy = 0;
int ** GResult;

int load(SAMPLE * node, char * dir, char * fileName){
	
	char command[100];
	command[0] = '\0';
	sprintf(command, "%s/%s", dir, fileName);
	
	int i, row, column;
	
	FILE * fp = fopen(command, "r");
	
	if(fp == NULL)
		return 1;

	fscanf(fp, "%d %d", &node->row, &node->column);
	
	if(dimensions == -1)
		dimensions = node->row;
	else if(node->row != dimensions)
		return 1;
	
	node->matrix = malloc(node->row * sizeof(int *));
	for(i = 0; i < node->row; i++){
		
		node->matrix[i] = malloc(node->column * sizeof(int));
		
	}

	for(row = 0; row < node->row; row++){//row
	
		for(column = 0; column < node->column; column++){//column
			
			fscanf(fp, "%d", &node->matrix[row][column]);

		}

	}
	
	fclose(fp);
	
	return 0;
	
}

int loadMatrices(SAMPLE *list, char * dir){
	
	char command[100];
	command[0] = '\0';
	sprintf(command, "ls %s", dir);

	int check = 1;
	
	FILE * fp = popen(command, "r");
	
	while (-1 != fscanf(fp, "%s", command)) {
		
		SAMPLE * newNode = malloc(sizeof(SAMPLE));
		
		check = load(newNode, dir, command);
		
		if(check != 0){
			
			fprintf(stderr, "ERROR: There was an error creating a node from file.\n");
			break;
			
		}
		
		newNode->next = list->next;
		list->next = newNode;
			
	}
	
	pclose(fp);
	
	return check;
	
}

int computeRef(){
	
	if(dimensions == -1)
		return 1;
		
	//do the As
	int CAx = GAx, CAy = GAy, row, col;
	
	for(row = 0; row < dimensions; row++){
		
		for(col = 0; col < dimensions; col++){
			
			refMatrix[row][col].Ax = CAx;
			refMatrix[row][col].Ay = (CAy + col) % dimensions;
			
		}
		
		CAx++;
		CAy = (CAy + 1) % dimensions;
		
	}
	
	GAy = (GAy + 1) % dimensions;
	
	//do the Bs
	int CBx = GBx, CBy = GBy;
	
	for(col = 0; col < dimensions; col++){
		
		for(row = 0; row < dimensions; row++){
			
			refMatrix[row][col].Bx = (CBx + row) %dimensions;
			refMatrix[row][col].By = CBy;
			
		}
		
		CBy++;
		CBx = (CBx + 1) % dimensions;
		
	}
	
	GBx = (GBx + 1) % dimensions;
	
	return 0;
	
}

int slave(int index, void * args){

	int x, y, dx, dy, row, col;
	
	PI_Read(toWorker[index], "%d %d %d %d", &x, &y, &dx, &dy);
	
	int ** A = matrices->next->next->matrix, ** B = matrices->next->matrix, check = 0;
	do{
		
		for(row = x; row <= dx; row++){
			
			for(col = y; col <= dy; col++){

				GResult[row][col] += A[refMatrix[row][col].Ax][refMatrix[row][col].Ay] * B[refMatrix[row][col].Bx][refMatrix[row][col].By];
				
			}
			
		}
		
		computeRef();
		
		if(check)
			check += 1;
		if(GAx == 0 && GAy == 0)
			check += 1;
		
	}while(check != 2);
	
	PI_Write(result[index], "%d %d %d %d", x, y, dx, dy);

	for(row = x; row <= dx; row++){
			
		for(col = y; col <= dy; col++){

			PI_Write(result[index], "%d", GResult[row][col]);
				
		}
			
	}
					
	return 0;
	
}

int slaveFunc(int x, int y, int dx, int dy){
	
	int ** A = matrices->next->next->matrix, ** B = matrices->next->matrix, check = 0;
	do{
		int row, col;
		for(row = x; row <= dx; row++){
			
			for(col = y; col <= dy; col++){

				GResult[row][col] += A[refMatrix[row][col].Ax][refMatrix[row][col].Ay] * B[refMatrix[row][col].Bx][refMatrix[row][col].By];
				
			}
			
		}
		
		computeRef();
		
		if(check)
			check += 1;
		if(GAx == 0 && GAy == 0)
			check += 1;
		
	}while(check != 2);
	
	return 0;
	
}

int calcWorkSize(int numProcs, int * row, int * column){
	
	if(numProcs == 0)
		numProcs++;
		
	if(dimensions == -1)
		return 1;
	else {
		
		if((dimensions*dimensions % numProcs) != 0){
			
			fprintf(stderr, "ERROR: Dimension^2 %% Processors != 0, please try a different number.\n\n");
			return 1;
		}

		int dim = dimensions, x, y;		
		int finalDim = (dim * dim) / numProcs;

		for(x = dim; x >= 1; x--){
			
			for(y = dim; y >= 1; y--){

				if((x * y) == finalDim){
					
					*row = x;
					*column = y;

					return 0;
					
				}
				
			}
			
		}
		
	}
	
	return 1;
	
}

int buildQueue(int row, int column, WORK * head){
	
	int rowCursor = row, colCursor = column;
	
	for(; rowCursor <= dimensions; rowCursor += row){
		
		for(; colCursor <= dimensions; colCursor += column){
			
			WORK * newNode = malloc(sizeof(WORK));
			
			newNode->x = rowCursor - row;
			newNode->y = colCursor - column;
			newNode->dx = rowCursor - 1;
			newNode->dy = colCursor - 1;
			newNode->next = head->next;
			head->next = newNode;
			
		}
		
		colCursor = column;
		
	}
	
	return 0;
	
}

void initResult(){
	
	GResult = malloc(dimensions * sizeof(int *));
	int i;
	for(i = 0; i < dimensions; i++){
		
		GResult[i] = malloc(dimensions * sizeof(int));
		
	}
	
	int row, col;
	for(row = 0; row < dimensions; row++)
		for(col = 0; col < dimensions; col++)
			GResult[row][col] = 0;
	
}

void report(){
	
	printf("The result: \n\n");
	int row, column;
	
	for(row = 0; row < dimensions; row++){
		
		for(column = 0; column < dimensions; column++){
			
			printf("%d ", GResult[row][column]);
			
		}
		
		printf("\n");
		
	}
	
}

int process(int N){

	int row, column, check;
	WORK * head = malloc(sizeof(WORK)), * temp = NULL;
	head->next = NULL;

	check = calcWorkSize(N, &row, &column);

	if(check != 0)
		return 1;
	
	check = buildQueue(row, column, head);
	
	if(check != 0)
		return 1;
	
	initResult();
	
	temp = head->next;
	
	if(N >= 1){

		//create communication channels for round robbin
		int i;
		Worker = calloc(N, sizeof(PI_PROCESS *));
		toWorker = calloc(N, sizeof(PI_CHANNEL *));
		result = calloc(N, sizeof(PI_CHANNEL *));

		for ( i = 0; i < N; i++ ) {
		
			Worker[i] = PI_CreateProcess( slave, i, NULL );
			toWorker[i] = PI_CreateChannel( PI_MAIN, Worker[i] );
			result[i] = PI_CreateChannel( Worker[i], PI_MAIN );
		
		}
		
		PI_StartAll();
	
		int slaveIndex = 0;
	
		for(;temp != NULL; temp = temp->next){

			PI_Write(toWorker[slaveIndex], "%d %d %d %d", temp->x, temp->y, temp->dx, temp->dy);

			slaveIndex++;
			
		}

		for(slaveIndex = 0; slaveIndex < N; slaveIndex++) {

			int x, y, dx, dy, row, col;
			
			PI_Read(result[slaveIndex], "%d %d %d %d", &x, &y, &dx, &dy);

			for(row = x; row <= dx; row++){
			
				for(col = y; col <= dy; col++){

					PI_Read(result[slaveIndex], "%d", &GResult[row][col]);
				
				}
			
			}
			
		}
		
	}
	else {

		PI_StartAll();
	
		for(;temp != NULL; temp = temp->next){

			check = slaveFunc(temp->x, temp->y, temp->dx, temp->dy);
		
			if(check != 0){//there was an error
		
				return 1;
				
			}
			
		}
		
	}

	//report
	report();

	//free result matrix
	
	return 0;
	
}

int main( int argc, char *argv[] ){

	char matrixdir[50];
	
	int N = PI_Configure(&argc, &argv) - 1;
	
	if(argc != 2){
		
		fprintf(stderr, "USAGE: cannon matricesdir\n\n");
		
		return EXIT_FAILURE;
		
	}
	else {
		
		strcpy(matrixdir, argv[1]);
		
	}
	
	matrices = malloc(sizeof(SAMPLE));
	matrices->next = NULL;
	matrices->matrix = NULL;
	
	//load parallaldos in arrays
	int check = -1;
	check = loadMatrices(matrices, matrixdir);
	
	if(check != 0){
		
		fprintf(stderr, "ERROR: There was problem loading the matrices!\n");
		return EXIT_FAILURE;
		
	}

	refMatrix = malloc(dimensions * sizeof(REFERENCE *));
	int i;
	for(i = 0; i < dimensions; i++){
		
		refMatrix[i] = malloc(dimensions * sizeof(REFERENCE));
		
	}
	
	check = computeRef();
	if(check != 0){
		
		fprintf(stderr, "ERROR: There was problem creating the reference table!\n");
		return EXIT_FAILURE;
		
	}
	
	//PI_StartTime();

	check = process(N);

	//double time = PI_EndTime();

	if(check != 0){
		
		fprintf(stderr, "ERROR: There was problem creating the shift table!\n");
		return EXIT_FAILURE;
		
	}
	//printf("Total time: %lf\n", time);
	
	//free everything
	
	PI_StopMain(0);
	
	return EXIT_SUCCESS;
	
}
