# include <stdio.h>

typedef struct {
    char *name;
    char *link;
    char *content;
    char **tags;
    int tagCount;
} Note;

typedef struct {
    Note *notes;
    int count;
} Metadata;

char *readFile(const char *filename);
void metadataLoad(Metadata *db, const char *filename);
void metadataSave(Metadata *db, const char *filename);
void metadataAddNote(Metadata *db, const char *name, const char *link, const char *content, char **tags, int tagCount);
Note *metadataFindNote(Metadata *db, const char *name);
void metadataRemoveNote();
void metadataFree(Metadata *db);


