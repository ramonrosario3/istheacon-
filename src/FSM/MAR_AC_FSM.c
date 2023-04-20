#include <stdio.h>

// This code prints text that can be use to create Final State Machines using Graphviz Online

typedef struct {
    char name[20];
} state;

typedef struct {
    char name[20];
} event;

typedef struct{
    char initial_state[32];
    char transition_event[32];
    char final_state[32];
} transition;

state states[] = {
    {"Get Temperature"},
    {"Publish Temperature"},
    {"Read Temperature"},
    {"Waiting"},
    {"Display Temperature"}
};

event events[] = {
    {"update_queue"},
    {"published"},
    {"receive_temperature"},
    {"update_display"},
};

transition read_transitions[] = {
    {"Read Temperature", "update_queue", "Read Temperature"},
};

transition publish_transitions[] = {
    {"Get Temperature", "update_queue", "Publish Temperature"},
    {"Publish Temperature", "published", "Get Temperature"},
};

transition display_transitions[] = {
    {"Waiting", "receive_temperature", "Display Temperature"},
    {"Display Temperature", "update_display", "Waiting"},
};

void fsm_printer(transition t[], int size){
    printf("digraph fsm {\n");
    for(int i = 0; i<size; i++){
        printf("  \"%s\" -> \"%s\" [label=\"%s\"];\n", t[i].initial_state, t[i].final_state, t[i].transition_event);
    }
    printf("}\n\n");
}


int main(){
    fsm_printer(display_transitions, sizeof(display_transitions)/sizeof(display_transitions[0]));
    fsm_printer(publish_transitions, sizeof(publish_transitions)/sizeof(publish_transitions[0]));
    fsm_printer(read_transitions, sizeof(read_transitions)/sizeof(read_transitions[0]));
    
    return 0;
}