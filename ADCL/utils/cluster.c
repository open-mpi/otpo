/* Standard C header files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* Local header files */
#include "cluster.h" /* The C clustering library */
#include "data.h"      /* Includes data handling and file reading/writing */
                       /* The routines in the C Clustering Library are called */
                       /* from data.c. */
extern int outlier_fraction;

int nmaxOutlier;
double tol;

/*============================================================================*/
/* Data declaration                                                           */
/*============================================================================*/

int _rows = 0;
int _columns = 0;
double* _geneweight = NULL;
double* _arrayweight = NULL;
double* _geneorder = NULL;  /* Saves gene order in the data file */
double* _arrayorder = NULL; /* Saves array order in the data file */
int* _geneindex = NULL;   /* Set by clustering methods for file output */
int* _arrayindex = NULL;  /* Set by clustering methods for file output */
char* _uniqID = NULL;     /* Stores UNIQID identifier in the data file */
char** _geneuniqID = NULL;
char** _genename = NULL;
char** _arrayname = NULL;
double** _data = NULL;
int** _mask = NULL;


int get_index(int node){
   /* returns array index to nodecounts, etc. */
   int idx;
   if (node < 0) /* cluster */
      idx = -node-1;
   else
      idx =  node;
   return idx;
}

int get_points(int node, int idx, int* nodecounts){
   /* returns number of points/genes of node */
   int npoints;
   if (node < 0) /* cluster */
      npoints = nodecounts[idx];
   else
      npoints = 1;
   return npoints;
}

void filter_out(const int node, const int idx, int* filtered, int* outliers, int* cnt){
   /* - marks nodes for filtering
      - filters out points / genes and increments cnt */
   if (node < 0){  /* node */
      filtered[idx] = -1;
      //printf("Node %d marked to be filtered out.\n", -node1);
   }
   else {
      (*cnt)++;
      outliers[node] = 1;
      //printf("Point %d filtered out.\n", node1);
   }
}

int HierarchicalClusterAnalysis(char metric, int transpose, char method, 
   double* avg, int* nfilt)
{ int i;
  double* nodeorder;
  int* nodecounts;
  char** nodeID;

  const int nNodes = (transpose ? _columns : _rows) - 1;
  const double* order = (transpose==0) ? _geneorder : _arrayorder;
  double* weight = (transpose==0) ? _arrayweight : _geneweight;
  const char* keyword = (transpose==0) ? "GENE" : "ARRY";

  /* Perform hierarchical clustering. */
  Node* tree = treecluster(_rows, _columns, _data, _mask, weight, transpose,
                           metric, method, NULL);
  if (!tree) return 0;

  if (metric=='e' || metric=='b')
  /* Scale all distances such that they are between 0 and 1 */
  { double scale = 0.0;
    for (i = 0; i < nNodes; i++)
      if (tree[i].distance > scale) scale = tree[i].distance;
    if (scale) for (i = 0; i < nNodes; i++) tree[i].distance /= scale;
  }

  /* Now we join nodes */
  nodeorder = malloc(nNodes*sizeof(double));
  nodecounts = malloc(nNodes*sizeof(int));
  nodeID = malloc(nNodes*sizeof(char*));

  for (i = 0; i < nNodes; i++)
  { int min1 = tree[i].left;
    int min2 = tree[i].right;
    /* min1 and min2 are the elements that are to be joined */
    double order1;
    double order2;
    int counts1;
    int counts2;
    char* ID1;
    char* ID2;
    nodeID[i] = MakeID ("NODE",i+1);
    if (min1 < 0)
    { int index1 = -min1-1;
      order1 = nodeorder[index1];
      counts1 = nodecounts[index1];
      ID1 = nodeID[index1];
      tree[i].distance = max(tree[i].distance, tree[index1].distance);
    }
    else
    { order1 = order[min1];
      counts1 = 1;
      ID1 = MakeID (keyword, min1);
    }
    if (min2 < 0)
    { int index2 = -min2-1;
      order2 = nodeorder[index2];
      counts2 = nodecounts[index2];
      ID2 = nodeID[index2];
      tree[i].distance = max(tree[i].distance, tree[index2].distance);
    }
    else
    { order2 = order[min2];
      counts2 = 1;
      ID2 = MakeID (keyword, min2);
    }

    ////printf ("%s\t%s\t%s\t%d\t%d\t%d", nodeID[i], ID1, ID2, counts1+counts2, counts1, counts2);
    //printf ("%s\t%s\t%d\t%s\t%d\t%d\t", nodeID[i], ID1, counts1, ID2, counts2, counts1+counts2);
    //printf ("%f\n", 1.0-tree[i].distance);

    if (min1>=0) free(ID1);
    if (min2>=0) free(ID2);

    nodecounts[i] = counts1 + counts2;
    nodeorder[i] = (counts1*order1 + counts2*order2) / (counts1 + counts2);
  }
  
  { 
    int j, k, *filtered, nfiltered, *outliers, npoints1, npoints2, npoints, 
       nremoved,  node, idx, idx1, idx2, node1, node2;

    nfiltered = 0;

    //printf("nmaxOutlier:%d\n", nmaxOutlier); 


    filtered = malloc((nNodes)*sizeof(int));
    for (k=0; k < nNodes; k++) filtered[k] = 0;
    outliers = malloc((nNodes+1)*sizeof(int));
    for (k=0; k <= nNodes; k++) outliers[k] = 0;

    /* k is index to arrays tree, nodecounts and filtered */
    /* genes are numbered from 0 to nNodes-1, nodeIds are +-1 to +-nNodes */
    for (k = nNodes-1; k >= 0; k--){
       /* are clusters similar */
       if (tree[k].distance <= tol) {
         //printf("Finished sucessfully. %d nodes filtered out\n", nfiltered);
         break; 
       }

       /* is cluster is already filtered out? */
       if (filtered[k]) continue;

       /* check which cluster is smaller */
/*       idx1 = tree[k].left - signum(tree[k].left);
       npoints1 = nodecounts[signum(idx1)*idx1];
       if (tree[k].left > 0) npoints1 = 1;
       idx2 = tree[k].right - signum(tree[k].right);
       npoints2 = nodecounts[signum(idx2)*idx2]; */
       //if (tree[k].right > 0) npoints1 = 1;

       node1 = tree[k].left;
       node2 = tree[k].right;
       idx1     = get_index (node1);
       npoints1 = get_points(node1, idx1, nodecounts);
       idx2     = get_index (node2);
       npoints2 = get_points(node2, idx2, nodecounts);

       if (npoints1 < npoints2) {  npoints = npoints1; idx = idx1; node = node1;  }
       else                     {  npoints = npoints2; idx = idx2; node = node2;  }

       /* too many outliers? */
       if (npoints + nfiltered > nmaxOutlier){
          /*printf("Warning: maximal number of outliers (%d) exceeded.\n", nmaxOutlier);
          printf("Stopped at node %d with %d points and distance %f.\n", -node, npoints, tree[k].distance);
          printf("%d outliers have been removed.\n", nfiltered); */
          break;
       }

       filter_out(node, idx, filtered, outliers, &nfiltered);
       if (node >= 0) continue;

       /* filter out node and sons*/
       nremoved = 0;
       while  (nremoved < npoints){
          /* handle mark nodes */
          for (j = 0; j <= k; j++){
             if (filtered[j] != -1) continue;
             filtered[j] = 1;

             int node1, node2, idx1, idx2;
             node1 = tree[j].left;  
             idx1  = get_index(node1);
             node2 = tree[j].right; 
             idx2  = get_index(node2);

             filter_out(node1, idx1, filtered, outliers, &nremoved);
             filter_out(node2, idx2, filtered, outliers, &nremoved);
          }
       }
       nfiltered += npoints;
    }

    double sum = 0.0;
    for (k = 0; k <= nNodes; k++)
       if (! outliers[k])
         sum = sum + _data[k][0];
    *avg = sum/(nNodes+1-nfiltered);
    *nfilt = nfiltered;
    //printf("average value (discarding %d outliers): %f\n", *nfilt, *avg );

    free(filtered);
    free(outliers);
  }

  /* Now set up order based on the tree structure */
  TreeSort((transpose==0) ? 'g' : 'a', nNodes, order, nodeorder, nodecounts,
           tree);
  free(nodecounts);

  free(nodeorder);
  for (i = 0; i < nNodes; i++) free(nodeID[i]);
  free(nodeID);
  free(tree);

  return 1;
}

void init_cluster_vars(const int ndata, double *data){
   int row, column, n;

   nmaxOutlier = outlier_fraction * ndata / 100 ;
   tol = 0.01;

   _rows = ndata;
   _columns = 1;

   /* Allocate space for array weights */
   _arrayweight = malloc(_columns*sizeof(double));
   _arrayorder = malloc(_columns*sizeof(double));
   _arrayindex = malloc(_columns*sizeof(int));
   for (column = 0; column < _columns; column++){
      _arrayweight[column] = 1.;
      _arrayorder[column] = column;
   }

   /* Allocate space for gene quantities */
   _geneweight = malloc(_rows*sizeof(double));
   _geneorder = malloc(_rows*sizeof(double));
   _geneindex = malloc(_rows*sizeof(int));
   for (row = 0; row < _rows; row++){
      _geneweight[row] = 1.;
      _geneorder[row] = row;
   }

   n = strlen("YORF");
   _uniqID = malloc((n+1)*sizeof(char));
   strcpy(_uniqID, "YORF\n");

   _geneuniqID = malloc(_rows*sizeof(char*));
   _genename = malloc(_rows*sizeof(char*));
   /* init with measurment number 1,2,3,4, .... */
   for (row = 0; row < _rows; row++) {
      const int n = strlen("0000");
      _geneuniqID[row] = malloc((n+1)*sizeof(char));
      sprintf(_geneuniqID[row], "%d\n", row);
      _genename[row] = NULL;
   }

   n = strlen("time");
   _arrayname    = malloc(sizeof(char*));
   _arrayname[0] = malloc((n+1)*sizeof(char));
   strcpy(_arrayname[0],"time");

   _data = malloc(_rows*sizeof(double*));
   _mask = malloc(_rows*sizeof(int*));
   for (row = 0; row < _rows; row++) { 
      _data[row] = malloc(_columns*sizeof(double));
      _mask[row] = malloc(_columns*sizeof(int));
      _data[row][0] = data[row];
      _mask[row][0] = 1;
   }
   sort (_rows, _geneorder, _geneindex);
   sort (_columns, _arrayorder, _arrayindex);

}


void free_cluster_vars(){
   int row;

   /* Allocate space for array weights */
   free(_arrayweight);
   free(_arrayorder);
   free(_arrayindex);

   free(_geneweight);
   free(_geneorder);
   free(_geneindex);

   free(_uniqID);
   free(_genename);

   for (row = 0; row < _rows; row++) {
      free(_geneuniqID[row]);
   }
   free(_geneuniqID);

   free(_arrayname[0]);
   free(_arrayname);

   for (row = 0; row < _rows; row++) { 
      free(_data[row]);
      free(_mask[row]);
   }

   free(_data);
   free(_mask);
}
