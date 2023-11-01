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
    int texA;
    int texB;
    int texC;
};
struct Texture{
    float u;
    float v;
};

struct Header
{
    int nVertex;
    int nIndexes;
};


int main(int argc , char * argv[])
{
    char buffer[128] = {0};
    int nVertex = 0;
    int nTriangles = 0;
    int nTexels = 0;
    struct Point verts[512] = {0};
    struct Texture texels[512] = {0};
    struct Triangle triangles[512] = {0};
    struct Vertex *pVertices = NULL;
    struct Header header = {0};

    printf("Reading vertex\n");
    FILE* cube = fopen("cube.obj", "r");
    if(NULL == cube)
    {
        printf("failed to open obj file\n");
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
                sscanf(buffer, "vt %f %f", &texels[nTexels].u, &texels[nTexels].v);
                nTexels++;
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
            triangles[nTriangles].texA = t1;
            triangles[nTriangles].texB = t2;
            triangles[nTriangles].texC = t3;
            printf("Triangle %d: %d %d %d\n", nTriangles, triangles[nTriangles].indexA, triangles[nTriangles].indexB, triangles[nTriangles].indexC);
            nTriangles++;
        }
    }
    /* close file */
    fclose(cube);

    pVertices = (struct Vertex *)malloc(sizeof(struct Vertex)*nTriangles*3);
    for (uint32_t idx = 0U; idx < nTriangles; ++idx)
    {
        pVertices[idx * 3].x = verts[triangles[idx].indexA - 1].x;
        pVertices[idx * 3].y = verts[triangles[idx].indexA - 1 ].y;
        pVertices[idx * 3].z = verts[triangles[idx].indexA - 1].z;
        pVertices[idx * 3].u = texels[triangles[idx].texA - 1].u;
        pVertices[idx * 3].v = texels[triangles[idx].texA - 1].v;

        pVertices[idx * 3 + 1].x = verts[triangles[idx].indexB - 1].x;
        pVertices[idx * 3 + 1].y = verts[triangles[idx].indexB - 1].y;
        pVertices[idx * 3 + 1].z = verts[triangles[idx].indexB - 1].z;
        pVertices[idx * 3 + 1].u = texels[triangles[idx].texB - 1].u;
        pVertices[idx * 3 + 1].v = texels[triangles[idx].texB - 1].v;
        pVertices[idx * 3 + 2].x = verts[triangles[idx].indexC - 1].x;
        pVertices[idx * 3 + 2].y = verts[triangles[idx].indexC - 1].y;
        pVertices[idx * 3 + 2].z = verts[triangles[idx].indexC - 1].z;
        pVertices[idx * 3 + 2].u = texels[triangles[idx].texC - 1].u;
        pVertices[idx * 3 + 2].v = texels[triangles[idx].texC - 1].v;

    }
    cube = fopen("model.hex", "wb");
    if(NULL == cube)
    {
        printf("failed to open db file");
        return EXIT_FAILURE;
    }
    header.nVertex = nTriangles*3;
    fwrite(&header, sizeof(struct Header),1, cube);
    fwrite(pVertices, sizeof(struct Vertex), nTriangles*3, cube);
    fclose(cube);
    printf("sizeof(float): %lu, %lu, triangles %d\n", sizeof(float), sizeof(struct Vertex)*nTriangles*3, nTriangles);
    return (0);
}

