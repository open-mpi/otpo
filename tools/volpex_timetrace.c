#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <mpi.h>

static double irecv_time=0.0;
static double isend_time=0.0;
static double recv_time=0.0;
static double send_time=0.0;
static double wait_time=0.0;
static double allreduce_time=0.0;
static double reduce_time=0.0;
static double alltoall_time=0.0;
static double alltoallv_time=0.0;

#define USE_PAPI_REAL_USEC 1
#define DISPLAY_MSG_LEN_INFO 1
#ifdef USE_PAPI_REAL_USEC
#include <limits.h>
#include "papi.h"

#define TIMETYPE long_long
#define TIME     timer_usec
#define TIMEDIFF timer_diff
#define TOP_COUNT 10
#define FINAL_TOP_COUNT 20
/****************Linked List start************************/
static int tot_message_length_counter=1;
static int message_length_arr[TOP_COUNT*64]={0};
static int counter_arr[TOP_COUNT*64]={0};
static int mpi_alltoall_count=0;
static int mpi_allreduce_count=0;
static int mpi_alltoallv_count=0;

struct node
{
  struct node *prev;
  int message_length;
  int counter;
  struct node *next;
}*h,*temp,*temp1;

/* To create an empty node */
static void volpex_create_node(int message_length,int counter)
{
  temp =(struct node *)malloc(1*sizeof(struct node));
  temp->prev = NULL;
  temp->next = NULL;
  temp->message_length = message_length;
  temp->counter = counter;
}

/*  To insert at beginning */
static void volpex_insert_beg(int message_length,int counter)
{
  temp1=NULL;
  if (h == NULL)
    {
      volpex_create_node(message_length,counter);
      h = temp;
      temp1 = h;
    }
  else
    {
      volpex_create_node(message_length,counter);
      temp->next = h;
      h->prev = temp;
      h = temp;
    }
}

/*insert at the end*/
static void volpex_insert_end(int message_length,int counter)
{
  tot_message_length_counter++;
  if (h == NULL)
    {
      volpex_create_node(message_length,counter);
      h = temp;
      temp1 = h;
    }
  else
    {
      volpex_create_node(message_length,counter);
      temp1->next = temp;
      temp->prev = temp1;
      temp1 = temp;
    }
}

/* Traverse from beginning */
static void volpex_print_linkedlist()
{
  struct node* temp2 = h;
  int rank,i=0,j=0;
  PMPI_Comm_rank ( MPI_COMM_WORLD, &rank );
  if (temp2 == NULL)
    {
      printf("%d: List empty to display \n", rank);
      return;
    }

  while (temp2->next != NULL)
    {
      message_length_arr[j]=temp2->message_length;
      counter_arr[j]=temp2->counter;
      temp2 = temp2->next;
      j++;
    }
  message_length_arr[j]=temp2->message_length;
  counter_arr[j]=temp2->counter;
  j++;
}


/* To update a node value in the list */
static void volpex_update_counter(int message_length,int counter)
{
  struct node* temp2 = h;
  if (temp2 == NULL)
    {
      printf("\nList is empty./n");
      return;
    }
  while (temp2 != NULL)
    {
      if (temp2->message_length== message_length)
        {
          temp2->counter= (temp2->counter)+counter;
          return;
        }
      else
        temp2 = temp2->next;
    }
  printf("\n Error : %d not found in list to update", message_length);
}


/* To search for an element in the list */
static void volpex_search_inc(int message_length,int counter)
{
  struct node* temp2 = h;
  //printf("data to be added: %d and counter = %d \n",message_length,counter);

  //added
  if (temp2 == NULL)
    {
      volpex_insert_beg(message_length,counter);
      return;
    }
  while (temp2 != NULL)
    {
      if (temp2->message_length == message_length)
        {
          volpex_update_counter(message_length,counter);
          return;
        }
      else{
        temp2 = temp2->next;
      }
    }
  volpex_insert_end(message_length,counter);
  return;
}




/* To sort the linked list according to message length */
static void volpex_sort_message_length()
{
  float x;
  int y;
  struct node* temp2 = h;
  struct node* temp4 = h;
  if (temp2 == NULL)
    {
      //printf("\n List empty to sort");
      return;
    }

  for (temp2 = h; temp2 != NULL; temp2 = temp2->next)
    {
      for (temp4 = temp2->next; temp4 != NULL; temp4 = temp4->next)
        {
          if (temp2->message_length > temp4->message_length)
            {
              x = temp2->message_length;
              temp2->message_length = temp4->message_length;
              temp4->message_length = x;

              y = temp2->counter;
              temp2->counter = temp4->counter;
              temp4->counter = y;

            }
        }
    }
  //printf("Sorting according to increasing order of message length");
  volpex_print_linkedlist();
}


/* To sort the linked list according to number of occurances */
static void volpex_sort_occurances()
{
  int x;
  int y,i=0;
  struct node* temp2 = h;
  struct node* temp4 = h;
  if (temp2 == NULL)
    {
      // printf("\n List empty to sort");
      return;
    }

  for (temp2 = h; temp2 != NULL; temp2 = temp2->next)
    {
      for (temp4 = temp2->next; temp4 != NULL; temp4 = temp4->next)
        {
          if (temp2->counter < temp4->counter)
            {
              x = temp2->counter;
              temp2->counter = temp4->counter;
              temp4->counter = x;
	      
              y = temp2->message_length;
              temp2->message_length = temp4->message_length;
              temp4->message_length = y;
            }
        }
    }
  //printf("Sorting according to decreasing order of occurances\n");
  volpex_print_linkedlist();
}

static void volpex_freeList()
{
  struct node* tmp;

  while (h != NULL)
    {
      tmp = h;
      h = h->next;
      free(tmp);
    }
}
/*****************Linked List end******************/

/* Note that timer_msec returns milliseconds */
static long_long timer_usec( void )
{
    static int is_init=0;

    if ( !is_init ) {
	int code;
	is_init = 1;
	if ( (code = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT ) {
	    printf("Error initializing PAPI\n");
	}
    }
    return PAPI_get_real_usec();
}
static double timer_diff ( long_long t2, long_long t1 )
{
    double retval = 0;

    if ( t2 >= t1 ) {
	retval = ((double) (t2-t1))/1000.0;
    }
    else {
	retval = ((double) ((__LONG_LONG_MAX__ - t1)+t2))/1000.0; 
	printf("Time spent in Reduce:%lf, t1 = %ld, t2 = %ld\n",retval,t1,t2);
    }
    return retval;
}
#else



#define TIMETYPE double
#define TIME     MPI_Wtime
#define TIMEDIFF(t2,t1) (t2-t1)

#endif

int MPI_Reduce(const void *sendbuf, void *recvbuf, int count,
	       MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm)
{
  int ret,size;
    TIMETYPE t1, t2;
    //added  for message length linked list
    int typelen;
    PMPI_Comm_size ( comm, &size );
    PMPI_Type_size ( datatype, &typelen);
    //printf("MPI_REDUCE: message length is %d and size is %d\n",count*typelen,size);
    volpex_search_inc(count*typelen,1);
    t1 = TIME();
    ret = PMPI_Reduce ( sendbuf, recvbuf, count, datatype, op, root, comm);
    t2 = TIME();
    reduce_time += TIMEDIFF(t2, t1);
    return ret;
}


int MPI_Allreduce(const void *sendbuf, void *recvbuf, int count,
		   MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
    int ret;
    TIMETYPE t1, t2;
#ifdef DISPLAY_MSG_LEN_INFO
    int typelen;
    int rank,size;
    PMPI_Type_size ( datatype, &typelen);
    PMPI_Comm_rank ( comm, &rank );
    PMPI_Comm_size ( comm, &size );
    if ( rank == 0 ) {
      //printf("MPI_ALLREDUCE:[%d] Allreduce msglength = %d\n", rank, count *typelen);
	//add to linked list
    }
    volpex_search_inc(count*typelen,1);
    mpi_allreduce_count++;
    #endif

    t1 = TIME();
    ret = PMPI_Allreduce ( sendbuf, recvbuf, count, datatype, op, comm);
    t2 = TIME();
    allreduce_time += TIMEDIFF(t2, t1);

    return ret;
}
int MPI_Alltoall(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
		 void *recvbuf, int recvcount,
		 MPI_Datatype recvtype, MPI_Comm comm)
{
    int ret;
    TIMETYPE t1, t2;
#ifdef DISPLAY_MSG_LEN_INFO
    int stypelen, rtypelen;
    int rank,size;
    PMPI_Type_size ( sendtype, &stypelen);
    PMPI_Type_size ( recvtype, &rtypelen);
    PMPI_Comm_size ( comm, &size );
    //add to linked list
    //printf("MPI_ALLTOALL_SEND:message length is %d and size is %d\n",sendcount*stypelen, size);
    //printf("MPI_ALLTOALL_RECVmessage length is %d and size is %d\n",recvcount*rtypelen,size);
    volpex_search_inc(sendcount*stypelen,size);
    mpi_alltoall_count++;
    PMPI_Comm_rank ( comm, &rank );
    //printf("[%d]: Alltoall sendlength=%d recvlength=%d\n", rank, 
    //	   sendcount*stypelen, recvcount*rtypelen);
#endif
    

    t1 = TIME();
    ret = PMPI_Alltoall ( sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm);
    t2 = TIME();
    alltoall_time += TIMEDIFF(t2, t1);

    return ret;
}
int MPI_Alltoallv(const void *sendbuf, const int *sendcounts, const int *sdispls,
		  MPI_Datatype sendtype,  void *recvbuf, const int *recvcounts,
		  const int *rdispls, MPI_Datatype recvtype, MPI_Comm comm)
{
    int ret; 
    TIMETYPE t1, t2;

#ifdef DISPLAY_MSG_LEN_INFO
    int stypelen;
    int rank, size, i;
    int minlen, maxlen;
    long  avg, sum=0.0;
    PMPI_Type_size ( sendtype, &stypelen);
    PMPI_Comm_rank ( comm, &rank );
    PMPI_Comm_size ( comm, &size );
    minlen = sendcounts[0]*stypelen;
    maxlen = sendcounts[0]*stypelen;
    sum = sendcounts[0]*stypelen;
    for (i=1; i<size; i++ ) {
	sum += sendcounts[i]*stypelen;
	if ( sendcounts[i]*stypelen > maxlen ) 
	    maxlen = sendcounts[i]*stypelen;
	if ( sendcounts[i]*stypelen < minlen ) 
	    minlen = sendcounts[i]*stypelen;
    }
    avg = sum / size;

    //printf("MPI_ALLTOALLV[%d]: message length=%d count=%d\n",rank, sendcounts[0]*stypelen, size);
    //add to linked list
    volpex_search_inc( sendcounts[0]*stypelen,size);
    mpi_alltoallv_count++;
#endif


    t1 = TIME();
    ret = PMPI_Alltoallv ( sendbuf, sendcounts, sdispls, sendtype, 
			   recvbuf, recvcounts, rdispls, recvtype, comm);
    t2 = TIME();
    alltoallv_time += TIMEDIFF(t2, t1);

    return ret;
}

int MPI_Irecv( void *buf, int count, MPI_Datatype datatype, int source,
              int tag, MPI_Comm comm,MPI_Request *request)
{
    int ret;
    TIMETYPE t1, t2;
    t1 = TIME();

    ret = PMPI_Irecv ( buf, count, datatype, source, tag, comm, request);

    t2 = TIME();
    irecv_time += TIMEDIFF(t2, t1);

    return ret;
}

int MPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest,
             int tag, MPI_Comm comm, MPI_Request *request)
{
    int ret;
    TIMETYPE t1, t2;
    ////added for message length in linked list
#ifdef DISPLAY_MSG_LEN_INFO
    int stypelen, rtypelen;
    int rank,size;
    PMPI_Type_size ( datatype, &stypelen);
    PMPI_Comm_rank ( comm, &rank );
    PMPI_Comm_size ( comm, &size );
    //printf("MPI_ISEND: message length is %d and size is 1\n",count*stypelen);
    //add to linked list
    volpex_search_inc( count*stypelen,1);
#endif
    ////
    t1 = TIME();
    ret = PMPI_Isend ( buf, count, datatype, dest, tag, comm, request);
    t2 = TIME();
    isend_time += TIMEDIFF(t2, t1);

    return ret;
}

int MPI_Recv( void *buf, int count, MPI_Datatype datatype, int source,
	      int tag, MPI_Comm comm,MPI_Status *status)
{
    int ret;
    TIMETYPE t1, t2;
    ////added for message length in linked list
#ifdef DISPLAY_MSG_LEN_INFO
    int stypelen, rtypelen;
    int rank,size;
    //PMPI_Type_size ( sendtype, &stypelen);
    PMPI_Type_size ( datatype, &rtypelen);
    PMPI_Comm_rank ( comm, &rank );
    PMPI_Comm_size ( comm, &size );
    //add to linked list
    //printf("\nMPI_RECV: message length is %d and count is %d and size is %d\n",count*rtypelen,count,size);
    //volpex_search_inc( count*rtypelen,1);
#endif
    ////
    t1 = TIME();
    ret = PMPI_Recv ( buf, count, datatype, source, tag, comm, status);
    t2 = TIME();
    recv_time += TIMEDIFF(t2, t1);

    return ret;
}

int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest,
	     int tag, MPI_Comm comm)
{
    int ret;
    TIMETYPE t1, t2;
    ////added for message length in linked list
#ifdef DISPLAY_MSG_LEN_INFO
    int stypelen, rtypelen;
    int rank,size;
    PMPI_Type_size ( datatype, &stypelen);
    //PMPI_Type_size ( recvtype, &rtypelen);
    PMPI_Comm_rank ( comm, &rank );
    PMPI_Comm_size ( comm, &size );
    //add to linked list
    //printf("MPI_SEND: message length is %d and size is %d\n",count*stypelen, 1);
    volpex_search_inc(count*stypelen,1);
#endif
    ////

    t1 = TIME();
    ret = PMPI_Send ( buf, count, datatype, dest, tag, comm);
    t2 = TIME();
    send_time += TIMEDIFF(t2, t1);

    return ret;
}

int MPI_Wait(MPI_Request *request, MPI_Status *status)
{
    int ret;
    TIMETYPE t1, t2;
    t1 = TIME();
    ret = PMPI_Wait ( request, status );
    t2 = TIME();
    wait_time += TIMEDIFF(t2, t1);

    return ret;
}

int MPI_Finalize (void)
{
  int total_message_length=0,i;//added for array
  double time_local[9], time_global[9];
  int rank, size;
  int *message_length_arr1 = NULL;
  int *counter_arr1 = NULL;
#ifdef DISPLAY_PROC_LOCATION
  char l_hostname[64];
#endif
  
  time_local[0] = reduce_time;
  time_local[1] = allreduce_time;
  time_local[2] = alltoall_time;
  time_local[3] = alltoallv_time;
  time_local[4] = irecv_time;
  time_local[5] = isend_time;
  time_local[6] = recv_time;
  time_local[7] = send_time;
  time_local[8] = wait_time;
  
  PMPI_Reduce ( time_local, time_global, 9, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD );
  PMPI_Comm_rank ( MPI_COMM_WORLD, &rank );
  PMPI_Comm_size ( MPI_COMM_WORLD, &size );
  PMPI_Barrier(MPI_COMM_WORLD);
  if ( rank == 0 ) {
    printf("Time spent in Reduce:%lf\n", time_global[0]);
    printf("Time spent in Allreduce:%lf\n", time_global[1]);
    printf("Time spent in Alltoall:%lf\n", time_global[2]);
    printf("Time spent in Alltoallv:%lf\n", time_global[3]);
    printf("Time spent in Irecv:%lf\n", time_global[4]);
    printf("Time spent in Isend:%lf\n", time_global[5]);
    printf("Time spent in Recv:%lf\n", time_global[6]);
    printf("Time spent in Send:%lf\n", time_global[7]);
    printf("Time spent in Wait:%lf\n", time_global[8]);
  }
  volpex_sort_occurances();
  message_length_arr1 = (int *)malloc( size * TOP_COUNT * sizeof(int));
  if(NULL == message_length_arr1){
    printf("malloc for message length failed.\n ");
    exit(-1);

  }
  counter_arr1 = (int *)malloc( size * TOP_COUNT * sizeof(int));
  if(NULL == counter_arr1){
    printf("malloc for message length failed.\n ");
    exit(-1);

  }

//Rank 0 gathers all the array elements
  PMPI_Gather(&message_length_arr,TOP_COUNT,MPI_INT,message_length_arr1,TOP_COUNT,MPI_INT,0,MPI_COMM_WORLD);
  PMPI_Gather(&counter_arr,TOP_COUNT,MPI_INT,counter_arr1,TOP_COUNT,MPI_INT,0,MPI_COMM_WORLD);
  volpex_freeList();
  if ( rank == 0 ) {
    for (i=0; i<TOP_COUNT*size; i++) {
      message_length_arr[i]=0;
      counter_arr[i]=0;
      volpex_search_inc(message_length_arr1[i],counter_arr1[i]);
    }
    volpex_sort_occurances();
    for (i=0;i<size*TOP_COUNT;i++){  
      if((0 == message_length_arr[i]) && (0 == counter_arr[i]))
	break;
      printf("%d:TOP messages used are: length %d, counter %d\n",rank,message_length_arr[i], counter_arr[i]);
    }
    printf("MPI_Alltoall %d\nMPI_Allreduce %d\nMPI_Alltoallv %d", mpi_alltoall_count,mpi_allreduce_count,mpi_alltoallv_count);
    printf("\n\n");
  }
  free(message_length_arr1);
  free(counter_arr1);
  volpex_freeList();
  
#ifdef DISPLAY_PROC_LOCATION
  /* Display location information */
  gethostname ( l_hostname, 64 );
  if ( rank == 0 ) {
    int i;
    
    printf(" Displaying process location information.\n");
    printf("Rank 0 is on host %s\n", l_hostname );
    
    for ( i=1; i < size; i++ ) {
      MPI_Recv ( l_hostname, 64, MPI_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
      printf("Rank %d is on host %s\n", i, l_hostname );
    }
  }
  else {
    MPI_Send ( l_hostname, 64, MPI_CHAR, 0, 0, MPI_COMM_WORLD );
  }
#endif
    return PMPI_Finalize();
}


