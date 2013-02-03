#include <stdio.h>

int main(int argc, char *argv[]){
	
	printf("Please enter a size of matrix to generate N: ");
	
	int dimensions, x = 1, y = 1;
	scanf("%d", &dimensions);
	
	FILE * fp = fopen("output.txt", "w");
	
	fprintf(fp, "%d %d\n", dimensions, dimensions);
	for(x = 1; x <= dimensions; x++){
		
		for(y = 1;y <= dimensions; y++){
			
			fprintf(fp, "%d ", x+y);
			
		}
		
		fprintf(fp, "\n");
		
	}
	
	return 0;
	
}
