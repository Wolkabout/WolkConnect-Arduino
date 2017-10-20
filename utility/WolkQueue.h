#ifndef QUEUE_H
#define QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief WOLK_ERR_T Boolean used for error handling in Queue module
 */
typedef unsigned char QUEUE_ERR_T;
/**
 * @brief WOLK_ERR_T Boolean used in Queuel module
 */
typedef unsigned char QUEUE_BOOL_T;

#define Q_TRUE 1
#define Q_FALSE 0


#define REFERENCE_SIZE 32
#define COMMAND_SIZE 32
#define VALUE_SIZE 128
#define QUEUE_SIZE 5

typedef struct _wolk_queue wolk_queue;
typedef struct _wolk_queue_elem wolk_queue_elem;


struct _wolk_queue_elem {

    char reference[REFERENCE_SIZE];
    char command[COMMAND_SIZE];
    char value[VALUE_SIZE];
};

struct _wolk_queue
{
    int head;
    int tail;
    wolk_queue_elem elem[QUEUE_SIZE];
};

QUEUE_ERR_T wolk_queue_init (wolk_queue *ctx);
QUEUE_ERR_T wolk_queue_clear (wolk_queue *ctx);
QUEUE_ERR_T wolk_queue_push (wolk_queue *ctx, const char *reference, const char *command, const char *value);
QUEUE_ERR_T wolk_queue_pop (wolk_queue *ctx, char *reference, char *command, char *value);




#ifdef __cplusplus
}
#endif

#endif

