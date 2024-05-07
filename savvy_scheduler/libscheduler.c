/**
 * savvy_scheduler
 * CS 241 - Spring 2022
 */
#include "libpriqueue/libpriqueue.h"
#include "libscheduler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "print_functions.h"

/**
 * The struct to hold the information about a given job
 */
typedef struct _job_info {
    int id;
    double arrival_time;
    double burst_time;
    double remain_time;
    double start_time;
    double last_start_time;
    int priority;
    /* TODO: Add any other information and bookkeeping you need into this
     * struct. */
} job_info;


double waiting_time = 0;
double turnaround_time = 0;
double response_time = 0;
int num_jobs = 0;


void scheduler_start_up(scheme_t s) {
    switch (s) {
    case FCFS:
        comparision_func = comparer_fcfs;
        break;
    case PRI:
        comparision_func = comparer_pri;
        break;
    case PPRI:
        comparision_func = comparer_ppri;
        break;
    case PSRTF:
        comparision_func = comparer_psrtf;
        break;
    case RR:
        comparision_func = comparer_rr;
        break;
    case SJF:
        comparision_func = comparer_sjf;
        break;
    default:
        printf("Did not recognize scheme\n");
        exit(1);
    }
    priqueue_init(&pqueue, comparision_func);
    pqueue_scheme = s;
    // Put any additional set up code you may need here
}

static int break_tie(const void *a, const void *b) {
    return comparer_fcfs(a, b);
}

int comparer_fcfs(const void *a, const void *b) {
    // TODO: Implement me!
    job_info* joba = ((job*)a)->metadata;
    job_info* jobb = ((job*)b)->metadata;
    if(joba->arrival_time == jobb->arrival_time) {
        return 0;
    } else if (joba->arrival_time > jobb->arrival_time) {
        return 1;
    } else {
        return -1;
    }
}

int comparer_ppri(const void *a, const void *b) {
    // Complete as is
    return comparer_pri(a, b);
}

int comparer_pri(const void *a, const void *b) {
    // TODO: Implement me!
    job_info* joba = ((job*)a)->metadata;
    job_info* jobb = ((job*)b)->metadata;
    if(joba->priority == jobb->priority) {
        return break_tie(a, b);
    } else if (joba->priority > jobb->priority) {
        return 1;
    } else {
        return -1;
    }
}

int comparer_psrtf(const void *a, const void *b) {
    // TODO: Implement me!
    job_info* joba = ((job*)a)->metadata;
    job_info* jobb = ((job*)b)->metadata;
    if(joba->remain_time == jobb->remain_time) {
        return break_tie(a, b);
    } else if (joba->remain_time > jobb->remain_time) {
        return 1;
    } else {
        return -1;
    }
    return 0;
}

int comparer_rr(const void *a, const void *b) {
    // TODO: Implement me!
    return 1;
}

int comparer_sjf(const void *a, const void *b) {
    // TODO: Implement me!
    job_info* joba = ((job*)a)->metadata;
    job_info* jobb = ((job*)b)->metadata;
    if(joba->burst_time == jobb->burst_time) {
        return break_tie(a, b);
    } else if (joba->burst_time > jobb->burst_time) {
        return 1;
    } else {
        return -1;
    }
}

// Do not allocate stack space or initialize ctx. These will be overwritten by
// gtgo
void scheduler_new_job(job *newjob, int job_number, double time,
                       scheduler_info *sched_data) {
    // TODO: Implement me!
    job_info* newjob_info = malloc(sizeof(job_info));
    newjob_info->id = job_number;
    newjob_info->arrival_time = time;
    newjob_info->burst_time = sched_data->running_time;
    newjob_info->priority = sched_data->priority;
    newjob_info->remain_time = sched_data->running_time;
    newjob_info->start_time = -1;
    newjob_info->last_start_time = -1;
    newjob->metadata = (void*) newjob_info;
    priqueue_offer(&pqueue, (void*) newjob);
    num_jobs++;
}

job *scheduler_quantum_expired(job *job_evicted, double time) {
    // TODO: Implement me!
    if(job_evicted == NULL) {
        
    } else if(pqueue_scheme == PPRI || pqueue_scheme == PSRTF || pqueue_scheme == RR) {
        job_info* evic_info = job_evicted->metadata;
        evic_info->remain_time -= time - evic_info->last_start_time;
        if(evic_info->remain_time > 0) {
            priqueue_offer(&pqueue, job_evicted);
        }
    } else {
        job_info* evic_info = job_evicted->metadata;
        if(time - evic_info->start_time < evic_info->burst_time) {
            return job_evicted;
        }
    }



    job* next_job = priqueue_poll(&pqueue);
    if(next_job == NULL) {
        return NULL;
    }

    job_info* next_info = next_job->metadata;
    if(next_info->start_time == -1){
        next_info->start_time = time;
    }
    next_info->last_start_time = time;
    return next_job;

}

void scheduler_job_finished(job *job_done, double time) {
    // TODO: Implement me!
    job_info* info = job_done->metadata;
    waiting_time += (time - info->arrival_time) - info->burst_time;
    turnaround_time += time - info->arrival_time; 
    response_time += info->start_time - info->arrival_time;

    if(job_done->metadata != NULL) {
        free(job_done->metadata);
        job_done->metadata=NULL;
    }
}

static void print_stats() {
    fprintf(stderr, "turnaround     %f\n", scheduler_average_turnaround_time());
    fprintf(stderr, "total_waiting  %f\n", scheduler_average_waiting_time());
    fprintf(stderr, "total_response %f\n", scheduler_average_response_time());
}

double scheduler_average_waiting_time() {
    // TODO: Implement me!
    return waiting_time/num_jobs;
}

double scheduler_average_turnaround_time() {
    // TODO: Implement me!
    return turnaround_time/num_jobs;
}

double scheduler_average_response_time() {
    // TODO: Implement me!
    return response_time/num_jobs;
}

void scheduler_show_queue() {
    // OPTIONAL: Implement this if you need it!
}

void scheduler_clean_up() {
    priqueue_destroy(&pqueue);
    print_stats();
}
