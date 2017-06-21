
#define MAX_QUEUE 100

struct requset
{
  int sector;
  int current_nr_sectors;
  char *buffer;
  int write;
};
struct request_queue
{
  struct requset data[MAX_QUEUE];
  int head = tail = num = 0;
};
struct gendisk
{
  int major;
  int first_minor;
  int last_minor;
  char disk_name[32];
  struct request_queue *queue;
};
extern int request_queue_push(&request_queue);
extern requset request_queue_pop(&request_queue);
extern int request_queue_empty(&request_queue);
