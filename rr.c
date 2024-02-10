#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef uint32_t u32;
typedef int32_t i32;

struct process
{
  u32 pid;
  u32 arrival_time;
  u32 burst_time;

  TAILQ_ENTRY(process) pointers;

  /* Additional fields here */
  u32 remaining_burst_time;
  u32 added;
  u32 started;
  u32 end_time;
  u32 start_time;
  /* End of "Additional fields here" */
};

TAILQ_HEAD(process_list, process);

u32 next_int(const char **data, const char *data_end)
{
  u32 current = 0;
  bool started = false;
  while (*data != data_end)
  {
    char c = **data;

    if (c < 0x30 || c > 0x39)
    {
      if (started)
      {
        return current;
      }
    }
    else
    {
      if (!started)
      {
        current = (c - 0x30);
        started = true;
      }
      else
      {
        current *= 10;
        current += (c - 0x30);
      }
    }

    ++(*data);
  }

  printf("Reached end of file while looking for another integer\n");
  exit(EINVAL);
}

u32 next_int_from_c_str(const char *data)
{
  char c;
  u32 i = 0;
  u32 current = 0;
  bool started = false;
  while ((c = data[i++]))
  {
    if (c < 0x30 || c > 0x39)
    {
      exit(EINVAL);
    }
    if (!started)
    {
      current = (c - 0x30);
      started = true;
    }
    else
    {
      current *= 10;
      current += (c - 0x30);
    }
  }
  return current;
}

void init_processes(const char *path,
                    struct process **process_data,
                    u32 *process_size)
{
  int fd = open(path, O_RDONLY);
  if (fd == -1)
  {
    int err = errno;
    perror("open");
    exit(err);
  }

  struct stat st;
  if (fstat(fd, &st) == -1)
  {
    int err = errno;
    perror("stat");
    exit(err);
  }

  u32 size = st.st_size;
  const char *data_start = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (data_start == MAP_FAILED)
  {
    int err = errno;
    perror("mmap");
    exit(err);
  }

  const char *data_end = data_start + size;
  const char *data = data_start;

  *process_size = next_int(&data, data_end);

  *process_data = calloc(sizeof(struct process), *process_size);
  if (*process_data == NULL)
  {
    int err = errno;
    perror("calloc");
    exit(err);
  }

  for (u32 i = 0; i < *process_size; ++i)
  {
    (*process_data)[i].pid = next_int(&data, data_end);
    (*process_data)[i].arrival_time = next_int(&data, data_end);
    (*process_data)[i].burst_time = next_int(&data, data_end);
  }

  munmap((void *)data, size);
  close(fd);
}

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    return EINVAL;
  }
  struct process *data;
  u32 size;
  init_processes(argv[1], &data, &size);

  u32 quantum_length = next_int_from_c_str(argv[2]);

  struct process_list list;
  TAILQ_INIT(&list);

  u32 total_waiting_time = 0;
  u32 total_response_time = 0;

  /* Your code here */
  
  //if it is -1 it hasnt been added to queue
    for(int i = 0; i < size; i++){
    data[i].added = -1;
    data[i].started = -1;
    data[i].remaining_burst_time = data[i].burst_time;
    data[i].start_time = -1;
    }
  // insert first process
 TAILQ_INSERT_TAIL(&list, &data[0], pointers);
  data[0].added = 0;
//calculate total burst time
  int total_burst_time =0;
  for(int i = 0; i < size;i++)
  {
    total_burst_time = data[i].burst_time + total_burst_time;
  }
  //while loop that carries out processes
  int time = 0;
  struct process *curr_proc;
  while (time != total_burst_time){
      curr_proc = TAILQ_FIRST(&list);
      //check if burst time is less or equal to than quantum length, if it is we increment time accordingly and 
      //break out of the loop, this means the process 
       // if this is first time starting process, update start  and update started parameter
      //move to the back and update the remaining burst time
      if(curr_proc->remaining_burst_time <= quantum_length)
      {
        if(curr_proc->start_time == -1){
         //printf("Initializing start time for process %u at time %u\n", curr_proc->pid, time);
          curr_proc->start_time = time;
          curr_proc->started = 0;
          } 
        for(int i = 0; i < curr_proc->remaining_burst_time; i++){ //run current process
        time++;
        //printf("time %u: \n", time);
      //add processes to queue based on current clock
        for(int i = 0; i < size + 1; i++){
          if(data[i].arrival_time <= time && data[i].added == -1){
          TAILQ_INSERT_TAIL(&list, &data[i], pointers);
          //printf("added current time: %u , pid added: %u\n", time, data[i].pid);
          data[i].added = 0;
        }
      }
      }
      curr_proc->end_time = time;
      //printf("current pid ended %u, current time %u\n", curr_proc->pid, time);
      TAILQ_REMOVE(&list, curr_proc, pointers); // we can remove the process because it is finished
      continue;
      }
       // if this is first time starting process, update start  and update started parameter
      //move to the back and update the remaining burst time
        if(curr_proc->start_time == -1){   
     //printf("Initializing start time for process %u at time %u\n", curr_proc->pid, time);
      curr_proc->start_time = time;
      curr_proc->started = 0;
      } 
      for(int i = 0; i < quantum_length; i++){ //run current process not finished scenario
        time++;
        //printf("time %u: \n", time);
      //add processes to queue based on current clock
        for(int i = 0; i < size + 1; i++){
          if(data[i].arrival_time <= time && data[i].added == -1){
          TAILQ_INSERT_TAIL(&list, &data[i], pointers);
          //printf("added current time: %u , pid added: %u\n", time, data[i].pid);
          data[i].added = 0;
        }
      }
      }
     
       TAILQ_REMOVE(&list, curr_proc, pointers);
       TAILQ_INSERT_TAIL(&list, curr_proc, pointers);
       curr_proc->remaining_burst_time = curr_proc->remaining_burst_time - quantum_length;
  }

  for(int i = 0; i < size; i++){ //add up wait times
    total_waiting_time = total_waiting_time +  (data[i].end_time-data[i].arrival_time - data[i].burst_time);
    total_response_time = total_response_time +  (data[i].start_time - data[i].arrival_time);
   // printf("pid: %u, endtime: %u, arrival time: %u, starttime: %u, \n", data[i].pid,data[i].end_time,data[i].arrival_time,data[i].start_time);
  }
  /* End of "Your code here" */

  printf("Average waiting time: %.2f\n", (float)total_waiting_time / (float)size);
  printf("Average response time: %.2f\n", (float)total_response_time / (float)size);

  free(data);
  return 0;
  }
