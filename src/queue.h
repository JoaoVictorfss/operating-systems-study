#define SUCCESS 2
#define QUEUE_IS_FULL 1
#define NULL_POINTER_EXCEPTION -2
#define MAX_SIZE 10

struct queue{
  int front, position, size;
  int data[MAX_SIZE];
};

typedef struct queue Queue;

void queue_create(Queue *q);
int queue_push(Queue *q, int val);
int queue_get(Queue *q);
int queue_full(Queue *q);
int queue_empty(Queue *q);
int queue_size(Queue *q);
int queue_pop(Queue *q);
