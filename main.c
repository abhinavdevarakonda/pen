# include "metadata.h"
# include <stdio.h>
# include <string.h>
# include <stdlib.h>

int main (int argc, char **argv) {
    Metadata db = {NULL, 0};

    metadataLoad(&db, "metadata.json");
    metadataFree(&db);

    return 0;
}
