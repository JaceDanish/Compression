#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
//maximum storage of unsigned short int: 65535, max of unsigned char: 255
#define MAX_COUNTER 255
#define COUNTER_TYPE unsigned char
#define COMP "comp"
#define DECOMP "decomp"
#define BUFFER_TYPE long int
#define BUFFER_SIZE 2

bool compress_file(FILE *fr, FILE *fc, FILE *fw, char *newName);
bool decompress_file(FILE *fr, FILE *fw);
void end_file(FILE *fr, FILE *fw);
FILE *get_file_ptr(const char *file, const char *mode);
char *new_comp_file_name(char *oldName);
char *new_decomp_file_name(char *oldName);
bool compare_arrays(const BUFFER_TYPE *arr1, const BUFFER_TYPE *arr2);

int main(int argc, char **argv)
{
    if (argc != 3 || (strcmp(argv[1], COMP) != 0 && strcmp(argv[1], DECOMP) != 0)){
        fprintf(stdout, "Usage: compression.c comp/decomp file\n");
        exit(EXIT_FAILURE);
    }
    char *newName;
    int compDecomp;

    if (strcmp(argv[1], COMP) == 0){
        newName = new_comp_file_name(argv[2]);
        compDecomp = 0;
    }else if (strcmp(argv[1], DECOMP) == 0){
        newName = new_decomp_file_name(argv[2]);
        compDecomp = 1;
    }else{
        fprintf(stderr, "Failure to read arguments");
        exit(EXIT_FAILURE);
    }

    FILE *fileReader = get_file_ptr(argv[2], "rb"); //for reading to buffer
    FILE *fileChecker = get_file_ptr(argv[2], "rb");//for checking for EOF
    FILE *fileWriter = get_file_ptr(newName, "wb");


    switch (compDecomp){
        case 0 : compress_file(fileReader, fileChecker, fileWriter, newName);
                 printf("comp\n"); break;
        case 1 : decompress_file(fileReader, fileWriter);
                 printf("decomp\n"); remove(argv[2]); break;
    }

    free(newName);
    fclose(fileWriter);

    return 0;
}

bool compress_file(FILE *fr, FILE *fc, FILE *fw, char *newName)
{
    BUFFER_TYPE buffer1[BUFFER_SIZE], buffer2[BUFFER_SIZE];
    BUFFER_TYPE checkBuf[BUFFER_SIZE];
    //counts number of repeated occurances. 0 if EOF
    COUNTER_TYPE counter = 1;

    fseek(fr, 0, SEEK_SET);
    fseek(fc, 0, SEEK_SET);
    fseek(fw, 0, SEEK_SET);

    //buffer1 filled first time
    if ((fread(&buffer1, sizeof(BUFFER_TYPE), BUFFER_SIZE, fr)) != BUFFER_SIZE){
        fprintf(stderr, "Unable to read file\n");
        remove(newName);
        exit(EXIT_FAILURE);
    }

    fseek(fc, sizeof(BUFFER_TYPE) * BUFFER_SIZE, SEEK_CUR);
    while (fseek(fr, 0, SEEK_CUR) == 0 && counter != 0){
        counter = 1;
        if ((fread(&checkBuf, sizeof(BUFFER_TYPE), BUFFER_SIZE, fc)) == BUFFER_SIZE){
            //if file checker says OK for next block
            fread(&buffer2, sizeof(BUFFER_TYPE), BUFFER_SIZE, fr);
            while (compare_arrays(buffer1, buffer2) && counter != 0 && counter != MAX_COUNTER){
                counter++;
                if ((fread(&checkBuf, sizeof(BUFFER_TYPE), BUFFER_SIZE, fc)) == BUFFER_SIZE){
                fread(&buffer2, sizeof(BUFFER_TYPE), BUFFER_SIZE, fr);
                }else{
                    fwrite(&counter, sizeof(COUNTER_TYPE), 1, fw);
                    fwrite(&buffer1, sizeof(BUFFER_TYPE), BUFFER_SIZE, fw);
                    counter = 0;
                    break;
                }
            }
        }else{
            //if file checker couldn't read the block
            fwrite(&counter, sizeof(COUNTER_TYPE), 1, fw);
            fwrite(&buffer1, sizeof(BUFFER_TYPE), BUFFER_SIZE, fw);
            counter = 0;
            break;
        }
        if (counter != 0){
            fwrite(&counter, sizeof(COUNTER_TYPE), 1, fw);
            fwrite(&buffer1, sizeof(BUFFER_TYPE), BUFFER_SIZE, fw);
            memcpy(buffer1, buffer2, BUFFER_SIZE * sizeof(BUFFER_TYPE));
        }else{
            break;
        }
    }
    fwrite(&counter, sizeof(COUNTER_TYPE), 1, fw);
    end_file(fr, fw);
}

void end_file(FILE *fr, FILE *fw)
{
    int ch;
    while ((ch = getc(fr)) != EOF){
        putc(ch, fw);
    }
}

bool decompress_file(FILE *fr, FILE *fw)
{
    BUFFER_TYPE buffer[BUFFER_SIZE];
    COUNTER_TYPE counter = 0;
    int i;

    fseek(fr, 0, SEEK_SET);
    fseek(fw, 0, SEEK_SET);

    do{
        fread(&counter, sizeof(COUNTER_TYPE), 1, fr);
        if (counter != 0){
            fread(&buffer, sizeof(BUFFER_TYPE), BUFFER_SIZE, fr);
            for (i = 0; i < counter; i++){
                fwrite(&buffer, sizeof(BUFFER_TYPE), BUFFER_SIZE, fw);
            }
        }
    }
    while (counter != 0);
    end_file(fr, fw);
}

FILE *get_file_ptr(const char *file, const char *mode)
{
    FILE *fp = fopen(file, mode);
    if (fp == NULL){
        fprintf(stderr, "Unable to open %s\n", file);
        exit(EXIT_FAILURE);
    }
    return fp;
}

char *new_comp_file_name(char *oldName)
{
    int nameLen = strlen(oldName);
    char *newName = malloc(sizeof(char) * (nameLen + 5));
    strcpy(newName, oldName);
    strcat(newName, ".comp");
    return newName;
}

char *new_decomp_file_name(char *oldName)
{
    int nameLen = strlen(oldName);
    char nameArr[nameLen];
    strcpy(nameArr, oldName);
    nameArr[nameLen - 5] = '\0';
    char *newName = malloc(sizeof(nameArr));
    strcpy(newName, nameArr);
    return newName;
}

bool compare_arrays(const BUFFER_TYPE *arr1, const BUFFER_TYPE *arr2)
{
    for (int i = 0; i < BUFFER_SIZE; i++)
        if (arr1[i] != arr2[i])
            return false;
    return true;
}