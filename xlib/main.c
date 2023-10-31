#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct Point
{
    float x;
    float y;
    float z;
};

struct Vertex
{
    float x;
    float y;
    float z;
    float u;
    float v;
};

struct Triangle
{
    int indexA;
    int indexB;
    int indexC;
};

struct header
{
    int nVertex;
    int nIndexes;
};


int main()
{
    char buffer[128] = {0};
    int nVertex = 0;
    int nTriangles = 0;
    struct Point verts[100] = {0};
    struct Triangle triangles[100] = {0};
    struct Vertex *pVertices = NULL;
    struct header header = {0};



    printf("Reading vertex\n");
    FILE* cube = fopen("./cube.obj", "r");
    if(NULL == cube)
    {
        printf("failed to open file\n");
        return EXIT_FAILURE;
    }
    while (fgets(buffer, sizeof(buffer), cube)) 
    {
        if('v' == buffer[0])
        {
            if(' ' == buffer[1])
            {
                /* read vertex data */
                sscanf(buffer, "v %f %f %f", &verts[nVertex].x, &verts[nVertex].y, &verts[nVertex].z);
                printf("Vertex %d: [%f %f %f]\n", nVertex, verts[nVertex].x, verts[nVertex].y, verts[nVertex].z);
                nVertex++;
            }
            else if ('t' == buffer[1])
            {
                /* read texture co-ordinates */
            }
            else if('n' == buffer[1])
            {
                /* read normal data */
            }
        }
        else if ('f' == buffer[0])
        {
            int v1, v2, v3;
            int t1, t2, t3;
            int n1, n2, n3;
            sscanf(buffer, "f %d/%d/%d %d/%d/%d %d/%d/%d", &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3);
            triangles[nTriangles].indexA = v1;
            triangles[nTriangles].indexB = v2;
            triangles[nTriangles].indexC = v3;
            printf("Triangle %d: %d %d %d\n", nTriangles, triangles[nTriangles].indexA, triangles[nTriangles].indexB, triangles[nTriangles].indexC);
            nTriangles++;
        }
    }
    /* close file */
    fclose(cube);

    pVertices = (struct Vertex *)malloc(sizeof(struct Vertex)*nTriangles*3);
    for (uint32_t idx = 0U; idx < nTriangles; ++idx)
    {
        pVertices[idx * 3].x = verts[triangles[idx].indexA].x;
        pVertices[idx * 3].y = verts[triangles[idx].indexA].y;
        pVertices[idx * 3].z = verts[triangles[idx].indexA].z;

        pVertices[idx * 3 + 1].x = verts[triangles[idx].indexB].x;
        pVertices[idx * 3 + 1].y = verts[triangles[idx].indexB].y;
        pVertices[idx * 3 + 1].z = verts[triangles[idx].indexB].z;

        pVertices[idx * 3 + 2].x = verts[triangles[idx].indexC].x;
        pVertices[idx * 3 + 2].y = verts[triangles[idx].indexC].y;
        pVertices[idx * 3 + 2].z = verts[triangles[idx].indexC].z;
    }
    cube = fopen("model.hex", "wb");
    if(NULL == cube)
    {
        printf("failed to open db file");
        return EXIT_FAILURE;
    }
    fwrite(pVertices, sizeof(struct Vertex), nTriangles*3, cube);
    fclose(cube);
    printf("sizeof(float): %lu, %lu, triangles %d\n", sizeof(float), sizeof(struct Vertex)*nTriangles*3, nTriangles);
    return (0);
}

