#ifndef _ACTION_H_
#define _ACTION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

enum action_result
{
    ACTION_NO_CHANGE,
    ACTION_HEATER_ON,
    ACTION_HEATER_OFF,
    
    ACTION_TOP
};

void action_init(void);
enum action_result action_do_something(void *param);
void action_status(void);
const char* action_get_text_result(enum action_result action);

#ifdef __cplusplus
}
#endif

#endif /* _ACTION_H_ */