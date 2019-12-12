#ifndef GLOBAL_H
#define GLOBAL_H

/* Possible modes of the system */
typedef enum {SINGLE, 
              MANUAL, 
              AUTO} Modality;
/* Possible direction of the motor */
typedef enum {RIGHT = 1, 
              LEFT = -1
            } Direction;

#define CLOVE_NUMBER 16               /* Number of segments in which the 180 degrees are divided */
#define INITIAL_MOTOR_POSITION 0      /* Initial angula position of the motor */
#define BASE_WAIT 675                 /* Base value of motor period */
#define VELOCITY_MULTIPLIER 75        /* Value to remove from BASE_WAIT. (It has to be multiplie to velocity) */

#endif
