#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_VERTICES 512
#define LEN_FILENAME 128

struct Position{
    float x;
    float y;
    float z;
};

struct Index
{
    int v;
    int t;
    int n;
};

struct Texture{
    float u;
    float v;
};

struct Vertex{
    float x;
    float y;
    float z;
    float u;
    float v;
};

struct Triangle{
    struct Vertex a;
    struct Vertex b;
    struct Vertex c;
};

struct Header{
    uint32_t nVertices;
    uint32_t nIndices;
};


int main(int argc, char *argv[])
{
    /* Map for counting unique vertices */
    // int **map = NULL;
    FILE* pFileInput = NULL; // handle for input file
    FILE* pFileOutput = NULL; // handle for output file
    char buffer[128];   // buffer for reading line from input file

    struct Position positions[NUM_VERTICES];
    uint32_t nPositions = 0; // number of unique position co-ordinates

    struct Texture texCoords[NUM_VERTICES];
    uint32_t nTexCoords = 0; // number of unique texture co-ordinates

    struct Index indices[NUM_VERTICES];
    uint32_t nIndexes = 0; // number if vert

    struct Header header;
    int *pOutputIndexs = NULL;

    struct Vertex *pOutputVertices = NULL;
    int nOutputVertices = 0;

    if(3 != argc)
    {
        char input[128] = "cube.obj";
        char output[128] = "cube.model";

        // printf("Enter obj filenme: ");
        // scanf("%s", input);

        pFileInput = fopen(input, "r");
        if(NULL == pFileInput)
        {
            printf("Failed to open input obj file: %s\n", input);
            return EXIT_FAILURE;
        }

        // printf("Enter model filename: ");
        // scanf("%s", output);
        
        pFileOutput = fopen(output, "wb");
        if(NULL == pFileInput)
        {
            printf("Failed to open output hex file: %s\n", output);
            fclose(pFileInput);
            pFileInput = NULL;
            return EXIT_FAILURE;
        }
    }
    else
    {
        pFileInput = fopen(argv[1], "r");
        if(NULL == pFileOutput)
        {
            printf("Failed to open input obj file: %s\n", argv[1]);
            return EXIT_FAILURE;
        }

        pFileOutput = fopen(argv[2], "wb");
        if(NULL == pFileOutput)
        {
            printf("Failed to open output hex file: %s\n", argv[2]);
            fclose(pFileInput);
            pFileInput = NULL;
            return EXIT_FAILURE;
        }
    }
    printf("Opened all files\n");
    while (fgets(buffer, sizeof(buffer), pFileInput))
    {
        if('v' == buffer[0])
        {
            /* process vertex information */
            if(' ' == buffer[1])
            {
                /* process vertex position */
                sscanf(buffer, "v %f %f %f", &positions[nPositions].x, &positions[nPositions].y, &positions[nPositions].z);
                nPositions++;
            }
            else if('t' == buffer[1])
            {
                /* process texture position */
                sscanf(buffer, "vt %f %f", &texCoords[nTexCoords].u, &texCoords[nTexCoords].v);
                nTexCoords++;
            }
            else if('n' == buffer[1])
            {
                /* process normal direction */
            }
        }
        else if('f' == buffer[0])
        {
            /* process index information */
            sscanf(buffer, "f %d/%d/%d %d/%d/%d %d/%d/%d", 
                   &indices[nIndexes].v, &indices[nIndexes].t, &indices[nIndexes].n,
                   &indices[nIndexes + 1].v, &indices[nIndexes + 1].t, &indices[nIndexes+1].n,
                   &indices[nIndexes + 2].v, &indices[nIndexes + 2].t, &indices[nIndexes+2].n);

            /* convert 1 based indeces to 0 based */
            indices[nIndexes].v--;
            indices[nIndexes + 1].v--;
            indices[nIndexes + 2].v--;

            indices[nIndexes].t--;
            indices[nIndexes + 1].t--;
            indices[nIndexes + 2].t--;

            indices[nIndexes].n--;
            indices[nIndexes + 1].n--;
            indices[nIndexes + 2].n--;
            nIndexes+=3;
        }
    }
    /* close input file handle */
    fclose(pFileInput);
    pFileInput = NULL;

    printf("File reading finished: Positions %d, Textures %d, indexes %d\n", nPositions, nTexCoords, nIndexes);
    
    header.nIndices = nIndexes;
    pOutputIndexs = (int *)malloc(sizeof(int)*header.nIndices);

    /* allocate memory for vertices and indices */
    int (*map)[nTexCoords] = (int(*)[nTexCoords])malloc(sizeof(int)*nPositions*nTexCoords);
    memset(map, -1, sizeof(int)*nPositions*nTexCoords); // -1 means does not exist
    pOutputVertices = (struct Vertex *)malloc(sizeof(struct Vertex)*header.nIndices);
    pOutputIndexs = (int *)malloc(sizeof(int)*header.nIndices);

    for (uint32_t idx = 0U; idx < header.nIndices; ++idx)
    {
        struct Index *tempIndex = &indices[idx];
        // printf("Index %d: %d/%d/%d\n", idx, tempIndex->v, tempIndex->t, tempIndex->n);
        printf("index: %d -> map[%d][%d] = %d\n", idx, tempIndex->v, tempIndex->t, map[tempIndex->v][tempIndex->t]);
        if(-1 == map[tempIndex->v][tempIndex->t])
        {
            printf("Creating new vertex for %d/%d at index %d\n\n", tempIndex->v, tempIndex->t, nOutputVertices);
            /*
              This combination of position and tex-coords is not referenced earlier,
              so create a new vertex by combining position and texture
             */
            pOutputVertices[nOutputVertices].x = positions[tempIndex->v].x;
            pOutputVertices[nOutputVertices].y = positions[tempIndex->v].y;
            pOutputVertices[nOutputVertices].z = positions[tempIndex->v].z;
            pOutputVertices[nOutputVertices].u = texCoords[tempIndex->t].u;
            pOutputVertices[nOutputVertices].v = texCoords[tempIndex->t].v;
            map[tempIndex->v][tempIndex->t] = nOutputVertices;
            nOutputVertices++;
        }
        else
        {
            /*
              This combination of position and tex-coords is referenced earlier and 
              map contains its location in final pOutputVertices array.
              Use this value as index
             */
            printf("vertex exits for %d/%d at index %d\n\n", tempIndex->v, tempIndex->t, map[tempIndex->v][tempIndex->t]);
        }

    }
    printf("Number of unique vertices: %d, total indices: %d\n", nOutputVertices, header.nIndices);
    header.nVertices = nOutputVertices;

    /* write data to file */
    fwrite(&header, sizeof(struct Header), 1UL, pFileOutput);
    fwrite(pOutputVertices, sizeof(struct Vertex), header.nVertices, pFileOutput);
    fwrite(pOutputIndexs, sizeof(int), header.nIndices, pFileOutput);
    

    /* release memory for indeces adn verticess */
    free(pOutputVertices);
    free(pOutputIndexs);
    free(map);

    /* close output file handle */
    fclose(pFileOutput);
    pFileOutput = NULL;
    return EXIT_SUCCESS;
}

