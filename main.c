# include "metadata.h"
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>

# define SOURCE "metadata.json"
int useMarkdown = 0;

/*
pen add rotate -l rotate.c -t algo rotation
*/

int main(int argc, char **argv) {
    Metadata db = {NULL,0};
    metadataLoad(&db, SOURCE);

    if (argc < 2) {
        printf("Usage: pen [add|remove|list|find] ...\n");
        return 1;
    }

    if (strcmp(argv[1],"add") == 0) {
        if (argc < 3) {
            fprintf(stderr,"Usage: pen add [-md | -pt] <name> [-l link] [-t tags...]\n");
            return 1;
        }

        int useMarkdown = 0;
        const char *name = argv[2];
        const char *link = NULL;
        char **tags = NULL;
        int tagCount = 0;
        int i = 2; // start parsing only after "add"
        
        if (strcmp(argv[i],"-md") == 0) {
            useMarkdown = 1;
            i++;
        } else if (strcmp(argv[i],"-pt") == 0) {
            useMarkdown = 0;
            i++;
        }

        if (i>=argc) {
            fprintf(stderr,"Error: missing note name\n");
            return 1;
        }
        name = argv[i++];

        while (i<argc) {
            if (strcmp(argv[i],"-l") == 0) {
                link = argv[++i];
            } else if (strcmp(argv[i],"-t") == 0) {
                int start = ++i;
                tagCount = argc - start;
                tags = &argv[start];
                break;
            }
            i++;
        }

        metadataAddNote(&db,name,link,(const char **)tags,tagCount,useMarkdown);
        metadataSave(&db, SOURCE);

    }

    if (strcmp(argv[1],"remove") == 0) {
        if (argc < 3) {
            fprintf(stderr,"Usage: pen add [-md | -pt] <name> [-l link] [-t tags...]\n");
            return 1;
        }
        const char *name = argv[2];

        if (!metadataFindNote(&db, name)) {
            fprintf(stderr,"Note %s does not exist.\n", name);
        }

        metadataRemoveNote(&db, name);
        metadataSave(&db, SOURCE);

    }

    if (strcmp(argv[1], "list") == 0) {
        if (argc < 3) {
            metadataList(&db, NULL, 0);
            return 1;
        }

        int tagCount = 0;
        char **tags = NULL;

        // parsing after list
        for (int i=2; i<argc; i++) {
            if (strcmp(argv[i],"-t") == 0) {
                int start = i+1;
                if (start >= argc) {
                    fprintf(stderr, "Error: -t requires at least one tag\n");
                    return 1;
                }
                tagCount = argc - start;
                tags = &argv[start];
                break;
            }
        }
        metadataList(&db, tags, tagCount);
    }
    if (strcmp(argv[1], "here") == 0) {
        char cwd[4096];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd() error");
            return 1;
        }
        const char *target = (argc > 2) ? argv[2] : NULL;
        metadataListHere(&db, cwd, target);
        return 0;
    }

    if (strcmp(argv[1], "edit") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: pen edit <name>\n");
            return 1;
        }
        metadataEditNote(&db, argv[2]);
        return 0;
    }
}

