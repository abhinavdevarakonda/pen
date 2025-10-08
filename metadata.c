# include "metadata.h"
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <cjson/cJSON.h>
# include <unistd.h>
# include <errno.h>

# define EDITOR "nvim"

char *readFile(const char *filename);
void metadataLoad(Metadata *db, const char *filename); // load json into struct Metadata
void metadataAddNote(Metadata *db, const char *name, const char *link,const char **tags, int tagCount,int useMarkdown); 
void metadataSave(Metadata *db, const char *filename); // save struct memory into metadata.json
Note* metadataFindNote(Metadata *db, const char *name); // find and return note by name
void metadataRemoveNote(Metadata *db, const char *name); // remove note from metadata.json
void metadataFree(Metadata *db);
void metadataList(Metadata *db, char **tags, int tagCount);

void metadataRemoveNote(Metadata *db, const char *name) {
    for (int i=0;i<db->count;i++) {
        if (strcmp(db->notes[i].name,name) == 0) {
            const char *filePath = db->notes[i].file;
            char fullPath[512];

            if (filePath) {
                snprintf(fullPath, sizeof(fullPath), "%s", filePath);
            } else {
                fullPath[0] = '\0';
            }

            if (fullPath[0] && access(fullPath, F_OK) == 0) {
                if (remove(fullPath) == 0) {
                    printf("file %s deleted successfully\n",fullPath);
                } else {
                    perror("failed to delete file\n");
                }
            }

            free(db->notes[i].name);
            free(db->notes[i].file);
            free(db->notes[i].link);
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

void metadataAddNote(Metadata *db, const char *name, const char *link,const char **tags, int tagCount,int useMarkdown) {
    Note *check = metadataFindNote(db, name);
    if (check) {
        fprintf(stderr,"note %s already exists\n",name);
        return;
    }

    const char *home = getenv("HOME");
    if (!home) {
        fprintf(stderr,"couldn't find HOME env variable\n");
        return;
    }

    const char *ext = useMarkdown ? ".md" : ".txt";

    size_t dirpathLen = strlen(home) + 7 + 1;
    char *dirPath = malloc(dirpathLen);
    if (!dirPath) {
        fprintf(stderr,"malloc failed [dirPath]\n");
        return;
    }
    snprintf(dirPath, dirpathLen, "%s/.notes", home);

    char mkdirCmd[512];
    snprintf(mkdirCmd, sizeof(mkdirCmd), "mkdir -p %s", dirPath);
    system(mkdirCmd);


    size_t notePathLen = strlen(dirPath) + 1 + strlen(name) + strlen(ext) + 1; // "/" + <ext> + null
    char *notePath = malloc(notePathLen);
    if (!notePath) {
        fprintf(stderr,"malloc failed [notePath]\n");
        return;
    }
    snprintf(notePath, notePathLen, "%s/%s%s", dirPath,name,ext);

    FILE *fp = fopen(notePath,"a+");
    if (!fp) {
        fprintf(stderr,"failed to create note file %s", notePath);
        return;
    }
    fclose(fp);

    char openCmd[512];
    snprintf(openCmd, sizeof(openCmd), "%s %s", EDITOR, notePath);
    system(openCmd);

    int newCount = db->count + 1;
    Note *temp = realloc(db->notes, newCount * sizeof(Note));
    if (!temp) {
        fprintf(stderr,"failed to reallocate memory.\n");
        return;
    }
    db->notes = temp;

    Note newNote;
    newNote.name = strdup(name);
    if (link) {
        newNote.link = strdup(link);
    } else {
        newNote.link = NULL;
    }

    size_t fileLen = strlen(notePath) + 1;
    newNote.file = malloc(fileLen);
    snprintf(newNote.file, fileLen, "%s", notePath);

    if (link) {
        FILE *linkCheck = fopen(link,"r");
        if (!linkCheck) {
            fprintf(stderr,"Warning: link file '%s' does not exist.\n",link);
        } else {
            fclose(linkCheck);
        }
    }

    newNote.tags = malloc(sizeof(char*) * tagCount);
    for (int i=0;i<tagCount;i++) {
        newNote.tags[i] = strdup(tags[i]);
    }
    newNote.tagCount = tagCount;

    db->notes[newCount - 1] = newNote;
    db->count = newCount;

    free(notePath);
    free(dirPath);
}

void metadataSave(Metadata *db, const char *filename) {
    cJSON *json = cJSON_CreateObject();
    cJSON *notesArray = cJSON_CreateArray();
    cJSON_AddItemToObject(json,"notes",notesArray);

    for (int i=0;i<db->count;i++) {
        cJSON *noteObj = cJSON_CreateObject();
        Note n = db->notes[i];
        cJSON_AddStringToObject(noteObj , "name", n.name);
        cJSON_AddStringToObject(noteObj , "file", n.file);
        cJSON_AddStringToObject(noteObj , "link", n.link);
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
    const cJSON *file = NULL;
    const cJSON *link = NULL; 
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
        n.name = NULL; n.link = NULL; n.tags = NULL;
        n.tagCount = 0;

        name = cJSON_GetObjectItemCaseSensitive(note, "name");
        file = cJSON_GetObjectItemCaseSensitive(note, "file");
        link = cJSON_GetObjectItemCaseSensitive(note, "link");
        tags = cJSON_GetObjectItemCaseSensitive(note, "tags");

        if (cJSON_IsString(name) && name->valuestring) {
            n.name = strdup(name->valuestring);
        }

        if (cJSON_IsString(file) && file->valuestring) {
            n.file = strdup(file->valuestring);
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
        free(db->notes[i].file);
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

void metadataList(Metadata *db, char **tags, int tagCount) {
    if (tags == NULL) {
        for (int i=0;i<db->count;i++) {
            Note note = db->notes[i];
            printf("name: %s\n",note.name);
            printf("filepath: %s\n",note.file);
            printf("link: %s\n",note.link);
            printf("tags: [");
            for (int j=0;j<note.tagCount;j++) {
                printf("%s",note.tags[j]);
                if (j<note.tagCount - 1) printf(", ");
            }
            printf("]\n");
            printf("--------------------\n");
        }
    } else {
        for (int i=0;i<db->count;i++) {
            Note note = db->notes[i];
            int matchFound = 0;

            for (int j=0;j<note.tagCount;j++) {
                for (int k=0;k<tagCount;k++) {
                    if (strcmp(note.tags[j],tags[k]) == 0) {
                        matchFound = 1;
                        break;
                    }
                }
                if (matchFound) break;
            }

            if (matchFound) {
                printf("name: %s\n", note.name);
                printf("filepath: %s\n", note.file);
                printf("link: %s\n", note.link);
                printf("tags: [");
                for (int j=0;j<note.tagCount;j++) {
                    printf("%s",note.tags[j]);
                    if (j<note.tagCount - 1) printf(", ");
                }
                printf("]\n");
                printf("--------------------\n");
            }
        }
    }
}
