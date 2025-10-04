# include "metadata.h"
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <cjson/cJSON.h>

# define NAME "name";

char *readFile(const char *filename);
void metadataLoad(Metadata *db, const char *filename); // load json into struct Metadata
void metadataAddNote(Metadata *db, const char *name, const char *link, char **tags, int tagCount); // add to struct
void metadataSave(Metadata *db, const char *filename); // save struct memory into metadata.json

void metadataLoad(Metadata *db, const char *filename) {
    const cJSON *notes = NULL; 
    const cJSON *note = NULL; 
    const cJSON *name = NULL; 
    const cJSON *link = NULL; 
    const cJSON *tags = NULL;

    char *raw = readFile(filename);
    if (!raw) {
        fprintf(stderr, "failed to read file %s\n",filename);
        goto end;
    }
    cJSON *metadata = cJSON_Parse(raw);

    if (!metadata) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "error before: %s\n", error_ptr);
        }
        goto end;
    }

    notes = cJSON_GetObjectItemCaseSensitive(metadata,"notes");
    if (!cJSON_IsArray(notes)) {
        fprintf(stderr,"no notes array found in metadata.json\n");
        cJSON_Delete(metadata);
        goto end;
    }

    int num_notes = cJSON_GetArraySize(notes);
    db->notes = malloc(sizeof(Note) * num_notes);
    db->count = num_notes;

    note = NULL; // to iterate over array of objects (notes)
    int i = 0;
    cJSON_ArrayForEach(note, notes) {
        Note n = {0};
        n.name = NULL; n.link = NULL; n.tags = NULL;
        n.tagCount = 0;

        name = cJSON_GetObjectItemCaseSensitive(note, "name");
        link = cJSON_GetObjectItemCaseSensitive(note, "link");
        tags = cJSON_GetObjectItemCaseSensitive(note, "tags");

        if (cJSON_IsString(name) && name->valuestring) {
            n.name = strdup(name->valuestring);
        }
        if (cJSON_IsString(link) && link->valuestring) {
            n.link= strdup(link->valuestring);
        }
        if (cJSON_IsArray(tags)) {
            int num_tags = cJSON_GetArraySize(tags);
            n.tags = malloc(sizeof(char*) * num_tags);
            n.tagCount = num_tags;

            const cJSON *tag = NULL; //to iterate over array of objects (tags)
            int idx = 0;
            cJSON_ArrayForEach(tag,tags) {
                if (cJSON_IsString(tag) && tag->valuestring) {
                    n.tags[idx++] = strdup(tag->valuestring);
                }
            }
        }
        db->notes[i++] = n;
    }

    for(int i=0;i<num_notes;i++) {
        printf("NOTE: %s\n",db->notes[i].name);
        printf("link to %s: %s\n",db->notes[i].name, db->notes[i].link);
        printf("tags to %s: ",db->notes[i].name);
        for(int j=0;j<db->notes[i].tagCount - 1;j++) {
            printf("%s, ",db->notes[i].tags[j]);
        }
        printf("%s\n",db->notes[i].tags[db->notes[i].tagCount - 1]);
        printf("=============\n");
    }

end:
    if (metadata) cJSON_Delete(metadata);
    if (raw) free(raw);
}

char *readFile(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return NULL;

    // get full size of content in json, store in size.
    fseek(fp,0,SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    char *buffer = malloc(size + 1);
    if (!buffer) return NULL;

    fread(buffer, 1, size, fp);
    buffer[size] = '\0';

    fclose(fp);
    return buffer;
}

void metadataFree(Metadata *db) {
    if (!db || !db->notes) return; 

    for (int i = 0;i < db->count;i++) {
        free(db->notes[i].name);
        free(db->notes[i].link);
        for (int j=0;j<db->notes[i].tagCount;j++) {
            free(db->notes[i].tags[j]);
        }
        free(db->notes[i].tags);
    }
    free(db->notes);
    db->notes = NULL;
    db->count = 0;
}
