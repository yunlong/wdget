#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int ac, char * * av)
{
  	const int FILE_CHUNK = 8024;
   	FILE *fp;
   	FILE *dl_file;
   	char buffer[FILE_CHUNK];
	char prefix[64];
	char FileName[128];
  	int i, j;
	FILE *fp1, *fp2, *fp3, *fp4;
//	FILE * fps[4];	
	char tmp[5] = { 'a', 'b', 'c', 'd', '\0' };
	char c;
	
/*
	if(ac < 2)
	{
		fprintf(stdout, "param num < 2\n");
		return -1;
	}
	strcpy(prefix, av[1]);
*/	
  	if ((fp1 = fopen("Mmstest1.wmv", "wb")) == NULL)
		return -1;
	fseek(fp1, 0L, SEEK_SET);
	
  	if ((fp2 = fopen("Mmstest1.wmv", "wb")) == NULL)
		return -1;
	fseek(fp2, 20L, SEEK_SET);
  	
	if ((fp3 = fopen("Mmstest1.wmv", "wb")) == NULL)
		return -1;
	fseek(fp3, 40L, SEEK_SET);
  	
	if ((fp4 = fopen("Mmstest1.wmv", "wb")) == NULL)
		return -1;
	fseek(fp4, 60L, SEEK_SET);

	FILE * fps[4] = { fp1, fp2, fp3, fp4 };
//	fwrite("I Hate", 8, 1, fp);
	for(i = 0; i < 4; i++)
	{
		c = tmp[i];	
		fp = fps[i];
		for( j = 0; j < 10; j++)
		{
			fwrite(&c, sizeof(char), 1, fp);
		}
		fclose(fp);
	}
	
/*							
	for (i = 0; i < 4; i++)
	{
		memset(FileName, 0, strlen(FileName));
		sprintf(FileName, "%s%d", prefix, i);
		fprintf(stdout, "%s\n", FileName);

		if (!(dl_file = fopen(FileName, "rb")))
				return -1;
		while ((j = fread(buffer, sizeof(char), FILE_CHUNK, dl_file)))
		{	
			if (fwrite(buffer, sizeof(char), j, fp) != j)
				return -1;
		}
		fclose(dl_file);
	}
*/
//  	fclose(fp);
}
