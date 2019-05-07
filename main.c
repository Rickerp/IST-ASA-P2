#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct vertice {
    int id;
} vertice;

void parse_input() {
    int nFrnc, nEst, nArst;
    scanf("%d %d %d", &nFrnc, &nEst, &nArst);

    vertice** frnc = (vertice**)malloc(sizeof(vertice*) * nFrnc);
    vertice** ids = (vertice**)malloc(sizeof(vertice*) * (nFrnc * nEst + 1));
}

int main() { parse_input(); }