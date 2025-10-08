# include <stdio.h>

typedef struct {
    char *name;
    char *file;
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
void metadataAddNote(Metadata *db, const char *name, const char *link,const char **tags, int tagCount, int useMarkdown);
Note *metadataFindNote(Metadata *db, const char *name);
void metadataRemoveNote(Metadata *db, const char *name);
void metadataFree(Metadata *db);
void metadataList(Metadata *db, char **tags, int tagCount);

