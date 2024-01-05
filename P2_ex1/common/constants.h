#ifndef CONSTANTS_H
#define CONSTANTS_H

#define MAX_RESERVATION_SIZE (256)
#define STATE_ACCESS_DELAY_US (500000)  // 500ms
#define MAX_JOB_FILE_NAME_SIZE (256)
#define MAX_SESSION_COUNT (8)

#define S (20) // NUM_SESSIONS

/* tfs_open flags */
enum {
    EMS_O_CREAT = 0b001,
    EMS_O_TRUNC = 0b010,
    EMS_O_APPEND = 0b100,
};

/* operation codes (for client-server requests) */
enum {
    EMS_OP_CODE_SETUP = 1,
    EMS_OP_CODE_QUIT = 2,
    EMS_OP_CODE_CREATE = 3,
    EMS_OP_CODE_RESERVE = 4,
    EMS_OP_CODE_SHOW = 5,
    EMS_OP_CODE_LIST_EVENTS = 6,
};

#endif /* CONSTANTS_H */