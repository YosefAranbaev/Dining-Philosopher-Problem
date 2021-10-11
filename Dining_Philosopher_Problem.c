#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
pthread_mutex_t lock;

typedef struct stick_
{
  int value;
  int is_lifted;
} stick;
typedef struct philosopher_
{
  stick left;
  stick right;
  int counter;
} philosopher;

philosopher *ph;
pthread_t *tid;
int *sticks;
int n;
//-------------------------------------------
int randomsInt(int lower, int upper)
{
  return (rand() % (upper - lower + 1)) + lower;
}
//-------------------------------------------
void *raise_stick(void *f)
{
  int *num = (int *)f;
  int index_ = *num;
  while (1)
  {
    int count = 0;
    //if we picking the second stick or we cat to pick both of sticks
    if (((ph[index_].left.value < ph[index_].right.value) && (ph[index_].counter == 1) && (ph[index_].left.is_lifted == 1 && (ph[index_].right.is_lifted == 0)) ||
         ((ph[index_].right.value < ph[index_].left.value) && (ph[index_].counter == 1) && (ph[index_].right.is_lifted == 1) && ((ph[index_].left.is_lifted == 0))) ||
         ((ph[index_].right.is_lifted == 0) && (ph[index_].left.is_lifted == 0))))
    {
      if (ph[index_].right.is_lifted == 0) //if the left is not lifted
      {
        pthread_mutex_lock(&lock);
        printf("philosopher number %d trying to pick the right stick\n", index_);
        ph[index_].right.is_lifted = 1;
        ++ph[index_].counter;
        if (index_ > 0)
        {
          ph[index_ - 1].left.is_lifted = 1;
          ++ph[index_ - 1].counter;
        }
        else
        {
          ph[n - 1].left.is_lifted = 1;
          ++ph[n - 1].counter;
        }
        printf("philosopher number %d picked the right stick\n", index_);
        pthread_mutex_unlock(&lock);
      }
      if (ph[index_].left.is_lifted == 0) //if the right is not lifted
      {
        pthread_mutex_lock(&lock);
        printf("philosopher number %d trying to pick the left stick\n", index_);
        ph[index_].left.is_lifted = 1;
        ++ph[index_].counter;
        if (index_ != n - 1)
        {
          ph[index_ + 1].right.is_lifted = 1;
          ++ph[index_ + 1].counter;
        }
        else
        {
          ph[0].right.is_lifted = 1;
          ++ph[0].counter;
        }
        printf("philosopher number %d picked the left stick\n", index_);
        pthread_mutex_unlock(&lock);
      }
      int random_thinking_time = randomsInt(10, 20);
      int random_eating_time = randomsInt(1, 10);
      printf("philosopher number %d eating for %d seconds\n", index_, random_eating_time);
      sleep(random_eating_time);
      printf("philosopher number %d finished eating for %d seconds\n", index_, random_eating_time);
      pthread_mutex_lock(&lock);
      ph[index_].left.is_lifted = 0;
      if (index_ > 0)
      {
        ph[index_ - 1].left.is_lifted = 0;
        --ph[index_ - 1].counter;
      }
      else
      {
        ph[n - 1].left.is_lifted = 0;
        --ph[n - 1].counter;
      }

      ph[index_].right.is_lifted = 0;
      if (index_ != n - 1)
      {
        ph[index_ + 1].right.is_lifted = 0;
        --ph[index_ + 1].counter;
      }
      else
      {
        ph[0].right.is_lifted = 0;
        --ph[0].counter;
      }
      pthread_mutex_unlock(&lock);
      ph[index_].counter = 0;
      printf("philosopher number %d starting to think for %d seconds\n", index_, random_thinking_time);
      sleep(random_thinking_time);
      printf("philosopher number %d finished to think for %d seconds\n", index_, random_thinking_time);
    }
    // if the left is the easier and is not lifted and the right lifted
    if ((ph[index_].left.value < ph[index_].right.value) &&
        ((ph[index_].right.is_lifted != 0) && (ph[index_].left.is_lifted == 0)))
    {
      pthread_mutex_lock(&lock);
      printf("philosopher number %d trying to pick the left stick\n", index_);
      sticks[index_] = 1;
      ph[index_].left.is_lifted = 1;
      ph[index_ + 1].right.is_lifted = 1;
      ++ph[index_].counter;
      ++ph[index_ + 1].counter;
      printf("philosopher number %d picked the right stick\n", index_);
      pthread_mutex_unlock(&lock);
    }
    // if the right is the easier and is not lifted and the left lifted
    if ((ph[index_].left.value > ph[index_].right.value) &&
        ((ph[index_].right.is_lifted == 0) && (ph[index_].left.is_lifted != 0)))
    {
      pthread_mutex_lock(&lock);
      sticks[index_ - 1] = 1;
      printf("philosopher number %d trying to pick the right stick\n", index_);
      ph[index_].right.is_lifted = 1;
      ph[index_ - 1].left.is_lifted = 1;
      ++ph[index_].counter;
      ++ph[index_ - 1].counter;
      printf("philosopher number %d picked the right stick\n", index_);
      pthread_mutex_unlock(&lock);
    }
  }
  return NULL;
}

int main(int argc, const char *argv[])
{
  if (argc < 2)
  {
    printf("Too few arguments, you didnt enter a number\n");
    return 0;
  }
  char str[20];
  void *ret;
  strcpy(str, argv[1]);
  n = atoi(str);
  ph = (philosopher *)malloc(sizeof(philosopher) * n);
  tid = (pthread_t *)malloc(sizeof(pthread_t) * n);
  int *arr = (int *)malloc(sizeof(int) * n);
  sticks = (int *)malloc(sizeof(int) * n);
  ph[0].left.value = 1;
  for (int i = 0; i < n; i++) // initializion
  {
    ph[i].right.is_lifted = 0;
    ph[i].left.is_lifted = 0;
    ph[i].counter = 0;
  }
  for (int i = 1; i < n; i++) // The values of the sticks
  {
    ph[i].right.value = ph[i - 1].left.value;
    ph[i].left.value = ph[i].right.value + 1;
  }
  ph[0].right.value = ph[n - 1].left.value;
  if (pthread_mutex_init(&lock, NULL) != 0)
  {
    printf("\n mutex init failed\n");
    return 1;
  }
  for (int i = 0; i < n; i++)
  {
    arr[i] = i;
    sticks[i] = 0;
    if (pthread_create(&tid[i], NULL, raise_stick, &arr[i]))
      return 1;
  }
  for (int i = 0; i < n; i++)
  {
    if (pthread_join(tid[i], &ret))
    {
      fprintf(stderr, "Could not join raise_stick Thread\n");
      return 1;
    }
  }
  for (int i = 0; i < n; i++)
  {
    printf("the index:%d,left_:%d, right_:%d\n", i, ph[i].left.is_lifted, ph[i].right.is_lifted);
  }
  free(ph);
  free(tid);
  free(arr);
  free(sticks);
  return 0;
}
