#include <graphviz/gvc.h>

int main(int argc, char** argv) {
    Agraph_t* g;
    GVC_t* gvc;
    Agedge_t *edge1, *edge2, *edge3;
    Agnode_t *node1, *node2, *node3;

    char* args[4]; 
    int i;
  
  
    args[0] = "dot";
    args[1] = "-Tpng";
    args[2] = "-o";
    args[3] = "example.png";

    gvc = gvContext(); /* library function */

    gvParseArgs(gvc, 4, args);
    g = agopen("g", Agdirected, 0);

    agsafeset(g, "rankdir", "LR", "");
    agattr(g, AGNODE, "shape", "rectangle");

    node1 = agnode(g, "Feed", 1);
    node2 = agnode(g, "Unit 1", 1);
    node3 = agnode(g, "Unit 2", 1);

    edge1 = agedge(g, node1, node2, 0, 1);

    edge2 = agedge(g, node2, node3, 0, 1);
    agsafeset(edge2, "color", "blue", "");
    agsafeset(edge2, "tailport", "n", "");
    agsafeset(edge2, "headport", "w", "");
    
    edge3 = agedge(g, node2, node3, 0, 1);
    agsafeset(edge3, "color", "red", "");
    agsafeset(edge3, "tailport", "s", "");
    agsafeset(edge3, "headport", "w", "");

    gvLayoutJobs(gvc, g);
    /* Write the graph according to -T and -o options */
    gvRenderJobs(gvc, g);

    gvFreeLayout(gvc, g); /* library function */
    agclose (g); /* library function */
    gvFreeContext(gvc);

    return 0;
}
