#include "metadata.h"
#include <stdio.h>

int main() {
    Metadata db = {NULL, 0};

    metadataLoad(&db, "metadata.json");

    char *tags1[] = {"algo", "rotation"};
    metadataAddNote(&db, "rotate", "rotate.c", "rotate function", tags1, 2);

    char *tags2[] = {"algo", "sorting", "divide_and_conquer"};
    metadataAddNote(&db, "merge_sort", "merge_sort.c", "merge sort implementation", tags2, 3);

    // testing adding duplicate
    metadataAddNote(&db, "rotate", "rotate.c", "rotate function", tags1, 2);

    // print all notes
    for (int i = 0; i < db.count; i++) {
        Note *n = &db.notes[i];
        printf("NOTE: %s\nLINK: %s\nCONTENT: %s\nTAGS: ", n->name, n->link, n->content);
        for (int j = 0; j < n->tagCount; j++) {
            printf("%s, ", n->tags[j]);
        }
        printf("\n---\n");
    }

    // find and print note
    Note *found = metadataFindNote(&db, "merge_sort");
    if (found) printf("Found note: %s\n", found->name);

    // save and clean
    metadataSave(&db, "metadata.json");
    metadataFree(&db);

    return 0;
}

