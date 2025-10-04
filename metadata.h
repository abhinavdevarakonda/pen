# include <stdio.h>

typedef struct {
    char *name;
    char *link;
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
void metadataAddNote(Metadata *db, const char *name, const char *link, char **tags, int tagCount);
void metadataFree(Metadata *db);

