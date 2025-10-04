# include "metadata.h"
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <cjson/cJSON.h>

char *readFile(const char *filename);
void metadataLoad(Metadata *db, const char *filename); // load json into struct Metadata
void metadataAddNote(Metadata *db, const char *name, const char *link, const char *content, char **tags, int tagCount); // add to struct
void metadataSave(Metadata *db, const char *filename); // save struct memory into metadata.json
Note* metadataFindNote(Metadata *db, const char *name); // find and return note by name
void metadataRemoveNote(Metadata *db, const char *name); // remove note from metadata.json

void metadataRemoveNote(Metadata *db, const char *name) {
    for (int i=0;i<db->count;i++) {
        if (strcmp(db->notes[i].name,name) == 0) {
            free(db->notes[i].name);
            free(db->notes[i].link);
            free(db->notes[i].content);
            for (int j=0;j<db->notes[i].tagCount;j++) {
                free(db->notes[i].tags[j]);
            }
            free(db->notes[i].tags);
            for (int j=i;j<db->count-1;j++) {
                db->notes[j] = db->notes[j+1];
            }
            db->count--;
            if (db->count > 0) {
                db->notes = realloc(db->notes, db->count * sizeof(Note));
            } else {
                free(db->notes);
                db->notes = NULL;
            }
            break;
        }
    }
}

void metadataAddNote(Metadata *db, const char *name, const char *link, const char *content, char **tags, int tagCount) {
    Note *check = metadataFindNote(db, name);
    if (check) {
        fprintf(stderr,"note %s already exists",name);
        return;
    }

    int newCount = db->count + 1;
    db->notes = realloc(db->notes, newCount * sizeof(Note));

    Note newNote;
    newNote.name = strdup(name);
    newNote.link = strdup(link);
    newNote.content = strdup(content);

    newNote.tags = malloc(sizeof(char*) * tagCount);
    for (int i=0;i<tagCount;i++) {
        newNote.tags[i] = strdup(tags[i]);
    }
    newNote.tagCount = tagCount;

    db->notes[newCount - 1] = newNote;
    db->count = newCount;
}

void metadataSave(Metadata *db, const char *filename) {
    cJSON *json = cJSON_CreateObject();
    cJSON *notesArray = cJSON_CreateArray();
    cJSON_AddItemToObject(json,"notes",notesArray);

    for (int i=0;i<db->count;i++) {
        cJSON *noteObj = cJSON_CreateObject();
        Note n = db->notes[i];
        cJSON_AddStringToObject(noteObj , "name", n.name);
        cJSON_AddStringToObject(noteObj , "link", n.link);
        cJSON_AddStringToObject(noteObj , "content", n.content);
        cJSON *tags = cJSON_CreateArray();
        for (int j=0;j<n.tagCount;j++) {
            cJSON_AddItemToArray(tags, cJSON_CreateString(n.tags[j]));
        }
        cJSON_AddItemToObject(noteObj,"tags",tags);

        cJSON_AddItemToArray(notesArray,noteObj);
    }
    char *string = cJSON_Print(json);
    if (!string) {
        fprintf(stderr,"failed to save metadata to json.\n");
        return;
    }
    FILE *fp;
    fp = fopen(filename,"w+");
    if (!fp) {
        fprintf(stderr,"failed to write to metadata.json\n");
        return;
    }
    fputs(string,fp);
    fputc('\n',fp);
    fclose(fp);
    printf("updated db!");

    free(string);
    cJSON_Delete(json);
}

void metadataLoad(Metadata *db, const char *filename) {
    const cJSON *notes = NULL; 
    const cJSON *note = NULL; 
    const cJSON *name = NULL; 
    const cJSON *link = NULL; 
    const cJSON *content = NULL;
    const cJSON *tags = NULL;

    if (db->notes) {
        metadataFree(db);
    }
    db->notes = NULL;
    db->count = 0;

    char *raw = readFile(filename);
    if (!raw) {
        fprintf(stderr, "failed to read file %s\n",filename);
        return;
    }
    cJSON *metadata = cJSON_Parse(raw);

    if (!metadata) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "error before: %s\n", error_ptr);
        }
        return;
    }

    notes = cJSON_GetObjectItemCaseSensitive(metadata,"notes");
    if (!cJSON_IsArray(notes)) {
        fprintf(stderr,"no notes array found in metadata.json\n");
        cJSON_Delete(metadata);
        return;
    }

    int num_notes = cJSON_GetArraySize(notes);
    db->notes = malloc(sizeof(Note) * num_notes);
    db->count = num_notes;

    note = NULL; // to iterate over array of objects (notes)
    int i = 0;
    cJSON_ArrayForEach(note, notes) {
        Note n = {0};
        n.name = NULL; n.link = NULL; n.content = NULL; n.tags = NULL;
        n.tagCount = 0;

        name = cJSON_GetObjectItemCaseSensitive(note, "name");
        link = cJSON_GetObjectItemCaseSensitive(note, "link");
        content = cJSON_GetObjectItemCaseSensitive(note, "content");
        tags = cJSON_GetObjectItemCaseSensitive(note, "tags");

        if (cJSON_IsString(name) && name->valuestring) {
            n.name = strdup(name->valuestring);
        }
        if (cJSON_IsString(link) && link->valuestring) {
            n.link= strdup(link->valuestring);
        }
        if (cJSON_IsString(content) && content->valuestring) {
            n.content = strdup(content->valuestring);
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

    free(raw);
    return;
}

Note* metadataFindNote(Metadata *db, const char *name) {
    for (int i = 0;i<db->count;i++) {
        if (strcmp(db->notes[i].name,name) == 0) {
            // return pointer to the correct note
            return &db->notes[i]; 
        }
    }
    return NULL;
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
        free(db->notes[i].content);
        for (int j=0;j<db->notes[i].tagCount;j++) {
            free(db->notes[i].tags[j]);
        }
        free(db->notes[i].tags);
    }
    free(db->notes);
    db->notes = NULL;
    db->count = 0;
}
