
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <errno.h>
#include <stdarg.h>
/* MACROS */
#ifdef ADCL_DISPLAY
  #define DISPLAY(A) ADCL_display A
#else
  #define DISPLAY(A)
#endif

#define ADCL_DISPLAY_POINTS 0
#define ADCL_DISPLAY_MESSAGE 1
#define ADCL_DISPLAY_CHANGE_FUNCTION 2
#define ADCL_DISPLAY_WINNER_DECIDED 3
#define ADCL_DISPLAY_COMM_FINAL 4

int ADCL_display_init();
int ADCL_display(int type,...);
int ADCL_display_finalize();
