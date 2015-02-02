#include <stdlib.h>
#include <string.h>
#include <stdio.h> 
#include <sys/stat.h>
#include <sys/types.h>
#include <direct.h>

#define HEAD_ENTRY_SIZE 0x120
#define FILE_COUNT_OFFSET 0xC
#define INITIAL_HEAD_LENGTH 0x14
#define MAGIC_LENGTH 8
#define PAC_MAGIC "DW_PACK"

typedef struct {
    unsigned int unk1;
    unsigned short file_id; // file number
    unsigned short unk3;
    char file_name[0x108];
    unsigned int unk4; // compressed size
    unsigned int unk5; // uncompressed size
    unsigned int unk6; 
    unsigned int file_offset; // which is relative to header offset
} file_entry; // total size 0x120

typedef struct {
    file_entry *flist;
    int file_count;
    int header_offset;
} pac_header;

static void mkdirp(const char *dir) {
    char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp),"%s",dir);

    len = strlen(tmp);
    if(tmp[len - 1] == '/')
        tmp[len - 1] = 0;

    for(p = tmp + 1; *p; p++) {
        if(*p == '\\') {
            *p = 0;
            _mkdir(tmp);
            *p = '\\';
        }
    }
}

static void extract(pac_header *pac, char *input_file_name, FILE *input) {
    FILE *output;
    char *buff;
    char *dot = strrchr(input_file_name, '.');
    char output_file_name[256];
    file_entry *fet;
    int i;
    
    if(dot != NULL)
        *dot = '\0';
    
    for(i=0; i<pac->file_count; i++) {
        fet = &pac->flist[i];
        
        //printf("%x\t%d\t%x\t%8x\t%8x\t%x\t%8x\t%s\n", 
        //    fet->unk1, fet->file_id, fet->unk3, fet->unk4, fet->unk5, fet->unk6, 
        //    pac->header_offset + fet->file_offset, fet->file_name);
        
        printf("%d/%d %s\n", fet->file_id+1, pac->file_count, fet->file_name);

        snprintf(output_file_name, sizeof(output_file_name), "%s\\%s", input_file_name, fet->file_name);
        
        mkdirp(output_file_name);
        
        output = fopen(output_file_name, "wb");
        if (output == NULL) {
            fprintf(stderr, "Error: %s could not be opened for writing.\n", output_file_name);
            continue;
        }

        buff = malloc(fet->unk4 * sizeof(char));
        
        fseek(input, pac->header_offset + fet->file_offset, SEEK_SET);
        fread(buff, 1, fet->unk4, input);
        fwrite(buff, 1, fet->unk4, output);
        
        free(buff);
        fclose(output);
    }
}

int main(int argc, char *argv[]) {
    FILE *fp;
    char *magic;
    pac_header pac;
    int i;
    
    if (argc != 2 ) {
        fprintf(stderr, "Usage: %s <pac_file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        fprintf(stderr, "Error: %s could not be opened for reading.\n", argv[1]);
        return EXIT_FAILURE;
    }
    
    magic = malloc(MAGIC_LENGTH * sizeof(char)); // DW_PACK\0
    fread(magic, MAGIC_LENGTH, sizeof(char), fp);
    
    if (strcmp(magic, PAC_MAGIC) != 0) {
        fprintf(stderr, "Error: Not a valid .pac file. <%s>\n", argv[1]);
        return EXIT_FAILURE; 
    }
    
    fseek(fp, FILE_COUNT_OFFSET, SEEK_SET);
    fread(&pac.file_count, 1, sizeof(int), fp);
    
    pac.header_offset = INITIAL_HEAD_LENGTH + (pac.file_count * HEAD_ENTRY_SIZE);
    pac.flist = (file_entry*) malloc(HEAD_ENTRY_SIZE * pac.file_count * sizeof(char));
    
    fseek(fp, INITIAL_HEAD_LENGTH, SEEK_SET);
    for(i=0; i<pac.file_count; i++) {
        fread(&pac.flist[i], 1, HEAD_ENTRY_SIZE, fp);
    }
    
    printf("Extracting %s\n", argv[1]);
    //printf("file_count: %d\n", pac.file_count);

    extract(&pac, argv[1], fp);
    
    fclose(fp);
    free(magic);
    free(pac.flist);

    return EXIT_SUCCESS;
}
