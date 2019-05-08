#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct vertex {
    int e;
    int h;
} vertex;

typedef struct edge {
    vertex* o;
    vertex* d;
    int f;
    int c;
} edge;

typedef struct graph {
    int nV;
    int nE;
    vertex* V;
    edge* E;
    int nSup;
} graph;

edge new_edge(vertex* o, vertex* d, int f, int c) {
    edge e;
    e.o = o;
    e.d = d;
    e.f = f;
    e.c = c;
    return e;
}

vertex new_vertex(int e, int h) {
    vertex v;
    v.e = e;
    v.h = h;
    return v;
}

int i_sStation(graph* G, int i) { return G->nSup + 2 + i; }

int i_fStation(graph* G, int i) { return G->nV - 1 - i; }

int pairStation(graph* G, int station) {
    return G->nV - 1 - station + i_sStation(G, 0);
}

int isStation(graph* G, int i) { return i >= i_sStation(G, 0); }

void parse_input(graph* G) {
    int nSuppliers, nStations, nEdges;
    scanf("%d %d %d", &nSuppliers, &nStations, &nEdges);

    G->nV = nSuppliers + nStations * 2 + 2;
    G->nE = nEdges + nSuppliers + nStations;
    G->V = (vertex*)malloc(sizeof(vertex) * G->nV);
    G->E = (edge*)malloc(sizeof(edge) * G->nE);
    G->nSup = nSuppliers;

    int s_f = 0, i_edge;
    G->V[0] = new_vertex(0, G->nV);
    G->V[1] = new_vertex(0, 0);
    for (i_edge = 0; i_edge < nSuppliers; i_edge++) {
        G->V[i_edge + 2] = new_vertex(0, 0);
        int v_c;
        scanf("%d", &v_c);
        G->E[i_edge] = new_edge(&G->V[0], &G->V[i_edge + 2], 0, v_c);
        s_f += v_c;
    }
    G->V[0].e = -s_f;

    for (int i = 0; i < nStations; i++) {
        int s = i_sStation(G, i), f = i_fStation(G, i);
        G->V[s] = G->V[f] = new_vertex(0, 0);
        int v_c;
        scanf("%d", &v_c);
        G->E[i_edge++] = new_edge(&G->V[s], &G->V[f], 0, v_c);
    }

    for (int i = 0; i < nEdges; i++) {
        int eo, ed, ec;
        scanf("%d %d %d", &eo, &ed, &ec);
        if (isStation(G, eo)) {
            G->E[i_edge++] =
                new_edge(&G->V[pairStation(G, eo)], &G->V[ed], 0, ec);
        } else {
            G->E[i_edge++] = new_edge(&G->V[eo], &G->V[ed], 0, ec);
        }
    }
}

void print_graph(graph* G) {
    printf("\nn-sup: %d\ni-start: %d\nf-start: %d\npair-i: %d\n\n", G->nSup,
           i_sStation(G, 0), i_fStation(G, 0),
           pairStation(G, i_sStation(G, 0)));
    for (int i = 0; i < G->nV; i++) {
        printf("%d ", i);
    }
}

int main() {
    graph* G = (graph*)malloc(sizeof(graph));
    parse_input(G);
    print_graph(G);
}