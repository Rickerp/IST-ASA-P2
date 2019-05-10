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

typedef struct node {
    void* data;
    struct node* next;
} node;

#define MIN(X, Y) (X < Y ? X : Y)

node* new_node(void* data, node* next) {
    node* n = (node*)malloc(sizeof(node));
    n->data = data;
    n->next = next;
    return n;
}

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
        int v_c;
        scanf("%d", &v_c);
        G->V[i_edge + 2] = new_vertex(v_c, 0);
        G->E[i_edge] = new_edge(&G->V[0], &G->V[i_edge + 2], v_c, v_c);
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

int isBackEdge(edge* e, vertex* v) { return e->d == v; }

void find_edges_av(edge** v_edges, graph* G, vertex* v) {
    int n_edges = 0;

    for (int i = 0; i < G->nE; i++) {
        v_edges[i] = NULL;
        if ((G->E[i].o == v && G->E[i].f != G->E[i].c) ||
            (G->E[i].d == v && G->E[i].f > 0)) {
            v_edges[n_edges++] = &G->E[i];
        }
    }
    return;
}

int edges_min_h(vertex* v, edge** es) {
    int min_h = -1;
    for (int i = 0; es[i] != NULL; i++) {
        if (isBackEdge(es[i], v)) {
            if (min_h == -1 || es[i]->o->h < min_h) min_h = es[i]->o->h;
        } else {
            if (min_h == -1 || es[i]->d->h < min_h) min_h = es[i]->d->h;
        }
    }
    return min_h;
}

void push(edge* e, vertex* v) {
    if (isBackEdge(e, v)) {
        int fSent = MIN(e->f, v->e);
        v->e -= fSent;
        e->o->e += fSent;
        e->f -= fSent;
    } else {
        int fSent = MIN(e->c - e->f, v->e);
        v->e -= fSent;
        e->d->e += fSent;
        e->f += fSent;
    }
}

void ins_q(vertex** Q, int* len, int* ptrN, vertex* vi, vertex* no) {
    int nQ = *ptrN;
    if (*len <= *ptrN + 1) {
        *len *= *len;
        Q = (vertex**)realloc(Q, sizeof(vertex*) * (*len));
    }
    if (vi != no) Q[nQ++] = vi;
    *ptrN = nQ;
}

void push_relabel(graph* G) {
    int q_len = G->nE * G->nV;
    edge** v_edges = (edge**)malloc(sizeof(edge*) * G->nE);
    vertex** Q = (vertex**)malloc(sizeof(vertex*) * q_len);
    for (int i = 0; i < G->nSup; i++) Q[i] = &G->V[i + 2];

    int q_index = 0, nQ = G->nSup;
    while (Q[q_index] != NULL) {
        while (Q[q_index]->e > 0) {
            find_edges_av(v_edges, G, Q[q_index]);
            Q[q_index]->h = edges_min_h(Q[q_index], v_edges) + 1;

            int edge_index = 0;
            while (v_edges[edge_index] != NULL) {
                if (isBackEdge(v_edges[edge_index], Q[q_index])) {
                    if (Q[q_index]->h > v_edges[edge_index]->o->h) {
                        push(v_edges[edge_index], Q[q_index]);
                        ins_q(Q, &q_len, &nQ, v_edges[edge_index]->o, &G->V[0]);
                    }
                } else {
                    if (Q[q_index]->h > v_edges[edge_index]->d->h) {
                        push(v_edges[edge_index], Q[q_index]);
                        ins_q(Q, &q_len, &nQ, v_edges[edge_index]->d, &G->V[1]);
                    }
                }
                edge_index++;
            }
        }
        q_index++;
    }
    free(v_edges);
    free(Q);
}

void print_node(graph* G, node* n) {
    for (node* i = n; i != NULL; i = i->next)
        printf("%ld ", ((vertex*)i->data) - G->V);
}

void print_hash(graph* G, node** h, int max) {
    for (int i = 0; i < max; i++) {
        printf("\n%d: ", i);
        print_node(G, h[i]);
    }
}

void print_edge(vertex* startPtr, edge* e) {
    printf("{ %ld:(%d,%d) -> %ld:(%d,%d), %d/%d} \n", e->o - startPtr, e->o->h,
           e->o->e, e->d - startPtr, e->d->h, e->d->e, e->c, e->f);
}

void print_graph(graph* G) {
    printf("\nGRAPH (V, E)(nSuppliers) = (%d, %d)(%d):\n", G->nV, G->nE,
           G->nSup);
    for (int i = 0; i < G->nE; i++) print_edge(G->V, &G->E[i]);
}

void parse_output(graph* G) {
    // FIRST LINE OUTPUT : MAXIMUM FLOW
    printf("%d\n", -G->V[0].e);

    // DEALING WITH LAST LINES OUTPUT : EDGES TO BE RAISED
    int max_h = 2 * G->nV;
    node** h_list = (node**)malloc(sizeof(node*) * max_h);

    for (int i = 0; i < G->nV; i++) {
        if (i + 1 >= max_h) {
            max_h *= 2;
            h_list = (node**)realloc(h_list, sizeof(node*) * max_h);
        }
        h_list[G->V[i].h] = new_node(&G->V[i], h_list[G->V[i].h]);
    }

    node *t_side = NULL, *s_side = NULL;

    int i_gap = -1;
    while (h_list[++i_gap] != NULL)
        for (node* i_node = h_list[i_gap]; i_node != NULL;
             i_node = i_node->next)
            t_side = new_node(i_node->data, t_side);

    // Beginning of the gap
    while (h_list[++i_gap] == NULL)
        ;
    // End of the gap
    i_gap -= 1;
    while (h_list[++i_gap] != NULL)
        for (node* i_node = h_list[i_gap]; i_node != NULL;
             i_node = i_node->next)
            s_side = new_node(i_node->data, s_side);

    node* cut = NULL;
    for (int i = 0; i < G->nE; i++) {
        int flag_so = 0;

        for (node* i_node = s_side; i_node != NULL; i_node = i_node->next) {
            if (G->E[i].o == ((vertex*)i_node->data)) {
                flag_so = 1;
                break;
            }
        }
        if (flag_so)  // origin of edge is in the s-side
            for (node* i_node = t_side; i_node != NULL; i_node = i_node->next) {
                if (G->E[i].d == i_node->data) {
                    cut = new_node(&G->E[i], cut);
                    break;
                }
            }
        else  // origin of edge is in the t-side
            for (node* i_node = s_side; i_node != NULL; i_node = i_node->next) {
                if (G->E[i].d == i_node->data) {
                    cut = new_node(&G->E[i], cut);
                    break;
                }
            }
    }

    // Having the cut edges, we just need to figure out which are whom

    for (node* i_node = cut; i_node != NULL; i_node = i_node->next)
        if ((((edge*)i_node->data)->o - G->V) >= (G->nSup + 2))
            printf("%ld", ((edge*)i_node->data)->o - G->V);
    printf("\n");
}

void free_graph(graph* G) {
    free(G->V);
    free(G->E);
    free(G);
}

int main() {
    graph* G = (graph*)malloc(sizeof(graph));
    parse_input(G);
    push_relabel(G);
    // print_graph(G);
    parse_output(G);
    free_graph(G);
}

/**
 * printf(
            "fSent = min(eC-eF, vE) = min(%d, %d) = %d ::: (0ld, New):\n\tvE = "
            "(%d, %d)\n\teF = (%d, %d)\n",
            e->c - e->f, v->e, fSent, e->o->e, e->o->e + fSent, e->f,
            e->f - fSent);
 */