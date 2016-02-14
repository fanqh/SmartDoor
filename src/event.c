#include "event.h"
#include "string.h"


static Hal_EventTypedef Lock_Events[LOCK_EVENT_RING_SIZE] = {{EVENT_NONE,0}};
static int get_event_index = 0;
static int put_event_index = 0;

static inline int next_event_index(int i) 
{

  if (i == LOCK_EVENT_RING_SIZE - 1)
    return 0;

  return ++i;
}

Hal_EventTypedef GetEvent(void)
{

  Hal_EventTypedef e; 
  if (get_event_index == put_event_index) 
	{
    e.event = EVENT_NONE;
		memset(&e.data,0,sizeof(EventDataTypeDef));
    return e;
  }

  e = Lock_Events[get_event_index];
  Lock_Events[get_event_index].event = EVENT_NONE;
	memset(&Lock_Events[get_event_index].data, 0, sizeof(EventDataTypeDef));

  get_event_index = next_event_index(get_event_index);

  return e;
}

void PutEvent(Hal_EventTypedef e)
{
	
  if (next_event_index(put_event_index) == get_event_index)
    return;
  Lock_Events[put_event_index] = e;
  put_event_index = next_event_index(put_event_index);
}

void ClearAllEvent(void)
{
	memset(Lock_Events, LOCK_EVENT_RING_SIZE, 0);
}

