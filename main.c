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
    struct node *next, *prev;
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

int isfStation(graph* G, int fStation) {
    return fStation > pairStation(G, fStation) && isStation(G, fStation);
}

int vIndex(vertex* v0, vertex* vi) { return vi - v0; }

void parse_input(graph* G) {
    int s, f;
    int i_edge, v_c;
    int eo, ed, ec, flow, s_f = 0;
    int nSuppliers, nStations, nEdges, i;

    if (scanf("%d %d %d", &nSuppliers, &nStations, &nEdges) != 3) exit(1);

    G->nV = nSuppliers + nStations * 2 + 2;
    G->nE = nEdges + nSuppliers + nStations;
    G->V = (vertex*)malloc(sizeof(vertex) * G->nV);
    G->E = (edge*)malloc(sizeof(edge) * G->nE);
    G->nSup = nSuppliers;

    G->V[0] = new_vertex(0, 0);
    G->V[1] = new_vertex(0, G->nV);
    for (i_edge = 0; i_edge < nSuppliers; i_edge++) {
        if (scanf("%d", &v_c) != 1) exit(1);
        G->V[i_edge + 2] = new_vertex(0, 0);
        G->E[i_edge] = new_edge(&G->V[i_edge + 2], &G->V[0], 0, v_c);
    }

    for (i = 0; i < nStations; i++) {
        s = i_sStation(G, i);
        f = i_fStation(G, i);
        G->V[s] = G->V[f] = new_vertex(0, 0);
        if (scanf("%d", &v_c) != 1) exit(1);
        G->E[i_edge++] = new_edge(&G->V[s], &G->V[f], 0, v_c);
    }

    for (i = 0; i < nEdges; i++) {
        flow = 0;
        if (scanf("%d %d %d", &eo, &ed, &ec) != 3) exit(1);

        if (ed == 1) {
            flow = ec;
            G->V[eo].e = flow;
            s_f -= flow;
        }

        if (isStation(G, ed)) {
            G->E[i_edge++] =
                new_edge(&G->V[pairStation(G, ed)], &G->V[eo], flow, ec);
        } else {
            G->E[i_edge++] = new_edge(&G->V[ed], &G->V[eo], flow, ec);
        }
    }
    G->V[1].e = s_f;
}

int isBackEdge(edge* e, vertex* v) { return e->d == v; }

void find_edges_av(edge** v_edges, graph* G, vertex* v) {
    int n_edges = 0;

    int i;
    for (i = 0; i < G->nE; i++) {
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
    int i;
    for (i = 0; es[i] != NULL; i++) {
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
    int i, nQ = 0, q_index = 0, edge_index;
    edge** v_edges = (edge**)malloc(sizeof(edge*) * G->nE);
    vertex** Q = (vertex**)malloc(sizeof(vertex*) * q_len);
    for (i = 0; i < q_len; i++) Q[i] = NULL;

    for (i = 0; i < G->nE; i++)
        if (G->E[i].o == &G->V[1]) Q[nQ++] = G->E[i].d;

    while (Q[q_index] != NULL) {
        while (Q[q_index]->e > 0) {
            find_edges_av(v_edges, G, Q[q_index]);
            Q[q_index]->h = edges_min_h(Q[q_index], v_edges) + 1;

            edge_index = 0;
            while (v_edges[edge_index] != NULL) {
                if (isBackEdge(v_edges[edge_index], Q[q_index])) {
                    if (Q[q_index]->h > v_edges[edge_index]->o->h) {
                        push(v_edges[edge_index], Q[q_index]);
                        ins_q(Q, &q_len, &nQ, v_edges[edge_index]->o, &G->V[1]);
                    }
                } else {
                    if (Q[q_index]->h > v_edges[edge_index]->d->h) {
                        push(v_edges[edge_index], Q[q_index]);
                        ins_q(Q, &q_len, &nQ, v_edges[edge_index]->d, &G->V[0]);
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

void free_nodes(node* head) {
    if (head == NULL) return;
    free_nodes(head->next);
    free(head);
}

int conc_ints(int x, int y, int max) {
    int p = 10;
    while (max > p) p *= 10;
    return x * p + y;
}

void parse_output(graph* G) {
    int i;
    int** iV_edge;
    int max_h = 2 * G->nV;
    int first = 1, i_gap = -1, flag_so = 0;
    node *i_node, *p_node, *min_node, *s_node;
    node *t_side = NULL, *s_side = NULL, *cut = NULL;
    node** h_list = (node**)malloc(sizeof(node*) * max_h);

    /* FIRST LINE OUTPUT : MAXIMUM FLOW */
    printf("%d\n", G->V[0].e);

    /* DEALING WITH LAST LINES OUTPUT : EDGES TO BE RAISED */
    for (i = 0; i < max_h; i++) h_list[i] = NULL;

    for (i = 0; i < G->nV; i++) {
        if (i + 1 >= max_h) {
            max_h *= 2;
            h_list = (node**)realloc(h_list, sizeof(node*) * max_h);
        }
        h_list[G->V[i].h] = new_node(&G->V[i], h_list[G->V[i].h]);
    }

    while (h_list[++i_gap] != NULL)
        for (i_node = h_list[i_gap]; i_node != NULL; i_node = i_node->next)
            t_side = new_node(i_node->data, t_side);

    /* Beginning of the gap */
    while (h_list[++i_gap] == NULL)
        ;
    /* End of the gap */

    i_gap -= 1;
    while (h_list[++i_gap] != NULL)
        for (i_node = h_list[i_gap]; i_node != NULL; i_node = i_node->next)
            s_side = new_node(i_node->data, s_side);

    /* Clean unused hashtable: */
    for (i = 0; i < max_h; i++) free_nodes(h_list[i]);
    free(h_list);

    for (i = 0; i < G->nE; i++) {
        flag_so = 0;
        if (G->E[i].c == G->E[i].f) {
            for (i_node = s_side; i_node != NULL; i_node = i_node->next) {
                if (G->E[i].o == ((vertex*)i_node->data)) {
                    flag_so = 1;
                    break;
                }
            }
            if (flag_so) /* origin of edge is in the s-side */
                for (i_node = t_side; i_node != NULL; i_node = i_node->next) {
                    if (G->E[i].d == i_node->data) {
                        iV_edge = (int**)malloc(sizeof(int*));
                        *iV_edge = (int*)malloc(sizeof(int) * 2);
                        (*iV_edge)[0] = vIndex(G->V, G->E[i].o);
                        (*iV_edge)[1] = vIndex(G->V, G->E[i].d);
                        cut = new_node(iV_edge, cut);
                        break;
                    }
                }
            else /* origin of edge is in the t-side */
                for (i_node = s_side; i_node != NULL; i_node = i_node->next) {
                    if (G->E[i].d == i_node->data) {
                        iV_edge = (int**)malloc(sizeof(int*));
                        *iV_edge = (int*)malloc(sizeof(int) * 2);
                        (*iV_edge)[0] = vIndex(G->V, G->E[i].o);
                        (*iV_edge)[1] = vIndex(G->V, G->E[i].d);
                        cut = new_node(iV_edge, cut);
                        break;
                    }
                }
        }
    }
    free_nodes(s_side);
    free_nodes(t_side);

    /*
    | Having the cut edges, we just need to figure out which are whom
    */

    /* Linked list to double linked list */
    for (i_node = cut, p_node = NULL; i_node != NULL; i_node = i_node->next) {
        i_node->prev = p_node;
        p_node = i_node;
    }

    /* Sorting List of Edges */
    for (s_node = cut; s_node->next != NULL;) {
        min_node = s_node;
        for (i_node = s_node; i_node != NULL; i_node = i_node->next) {
            int i_o = (*(int**)i_node->data)[0];
            int i_d = (*(int**)i_node->data)[1];
            int m_o = (*(int**)min_node->data)[0];
            int m_d = (*(int**)min_node->data)[1];
            i_o = isfStation(G, i_o) ? pairStation(G, i_o) : i_o;
            i_d = isfStation(G, i_d) ? pairStation(G, i_d) : i_d;
            m_o = isfStation(G, m_o) ? pairStation(G, m_o) : m_o;
            m_d = isfStation(G, m_d) ? pairStation(G, m_d) : m_d;
            if (conc_ints((*(int**)i_node->data)[1], i_o, G->nV) <
                conc_ints((*(int**)min_node->data)[1], m_o, G->nV)) {
                min_node = i_node;
            }
        }
        if (min_node != s_node) {
            min_node->prev->next = min_node->next;
            if (min_node->next != NULL) {
                min_node->next->prev = min_node->prev;
            }

            min_node->next = s_node;
            min_node->prev = s_node->prev;

            if (s_node != cut)
                s_node->prev->next = min_node;
            else
                cut = min_node;
            s_node->prev = min_node;
        } else
            s_node = s_node->next;
        if ((min_node != NULL && min_node->prev == min_node) ||
            (i_node != NULL && i_node->prev == i_node) ||
            (s_node != NULL && s_node->prev == s_node)) {
            printf(
                "\ns: (%d, %d)\ns->p: (%d, %d)\nm->p: (%d, %d)\nm: (%d, "
                "%d)\nm->n: "
                "(%d, %d)\n\n",
                (*(int**)s_node->data)[0], (*(int**)s_node->data)[1],
                (*(int**)s_node->prev->data)[0],
                (*(int**)s_node->prev->data)[1],
                (*(int**)min_node->prev->data)[0],
                (*(int**)min_node->prev->data)[1], (*(int**)min_node->data)[0],
                (*(int**)min_node->data)[1], (*(int**)min_node->next->data)[0],
                (*(int**)min_node->next->data)[1]);
        }
    }
    /* List of cut edges sorted */

    /*
    | Print second line of output
    */
    for (i_node = s_node; i_node != NULL; i_node = i_node->prev) {
        if (isStation(G, (*(int**)i_node->data)[0]) &&
            pairStation(G, (*(int**)i_node->data)[1]) ==
                (*(int**)i_node->data)[0]) {
            if (!first)
                printf(" ");
            else
                first = 0;
            printf("%d", (*(int**)i_node->data)[0]);
        }
    }
    printf("\n");
    /*
    | Printed second line of output
    */

    /*
    | Print the last lines
    */
    for (i_node = cut; i_node != NULL; i_node = i_node->next) {
        if ((*(int**)i_node->data)[1] != 0 &&
            !(isStation(G, (*(int**)i_node->data)[0]) &&
              pairStation(G, (*(int**)i_node->data)[1]) ==
                  (*(int**)i_node->data)[0])) {
            printf("%d %d\n", (*(int**)i_node->data)[1],
                   isfStation(G, (*(int**)i_node->data)[0])
                       ? pairStation(G, (*(int**)i_node->data)[0])
                       : (*(int**)i_node->data)[0]);
        }
    }
    /*
    | Printed the last lines
    */

    /* FREE(cut) */
    for (i_node = cut; i_node != NULL; i_node = i_node->next) {
        free(*((int**)i_node->data));
        free(i_node->data);
    }
    free_nodes(cut);
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
    parse_output(G);
    free_graph(G);
    return 0;
}

/*
printf(
            "fSent = min(eC-eF, vE) = min(%d, %d) = %d ::: (0ld, New):\n\tvE
 = "
            "(%d, %d)\n\teF = (%d, %d)\n",
            e->c - e->f, v->e, fSent, e->o->e, e->o->e + fSent, e->f,
            e->f - fSent);

PRINT CUT EDGES NODE:
    for (node* i_node = cut; i_node != NULL; i_node = i_node->next) {
        printf("(%d->%d) -> ", (*(int**)i_node->data)[0],
               (*(int**)i_node->data)[1]);
    }

PRINT CUT EDGES WITH CONC COORDS:
for (node* i_node = cut; i_node != NULL; i_node = i_node->next) {
        printf("(%d->%d = %d) -> ", (*(int**)i_node->data)[0],
               (*(int**)i_node->data)[1],
               conc_ints((*(int**)i_node->data)[1],
                         isfStation(G, (*(int**)i_node->data)[0])
                             ? pairStation(G, (*(int**)i_node->data)[0])
                             : (*(int**)i_node->data)[0],
                         G->nV));
    }

PRINT SIDES OF CUT:
    printf("\nprint \'s_side\': ");
    print_vertex_node(G, s_side);
    printf("print \'t_side\': ");
    print_vertex_node(G, t_side);
    printf("\n");


void print_edge(vertex* startPtr, edge* e) {
    printf("{ %ld:(%d,%d) -> %ld:(%d,%d), %d/%d } \n", e->o - startPtr, e->o->h,
           e->o->e, e->d - startPtr, e->d->h, e->d->e, e->c, e->f);
}

void print_graph(graph* G) {
    printf("\nGRAPH (V, E)(nSuppliers) = (%d, %d)(%d):\n", G->nV, G->nE,
           G->nSup);
           int i;
    for( i = 0; i < G->nE; i++) print_edge(G->V, &G->E[i]);
}


void print_vertex_node(graph* G, node* n) {
    for (node* i = n; i != NULL; i = i->next)
        printf("%d -> ", vIndex(G->V, (vertex*)i->data));
    printf("\n");
}

void print_edge_node(graph* G, node* n) {
    printf("\nprint_edge_node:\n");
    for (node* i = n; i != NULL; i = i->next) print_edge(G->V, i->data);
}

void print_hash(graph* G, node** h, int max) {
    int i;
    for( i = 0; i < max; i++) {
        printf("%d: ", i);
        print_vertex_node(G, h[i]);
    }
}


*/