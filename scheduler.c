#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_BURST 5
#define MAX_BURST_TIME 30
#define MAX_PROCESS 10
#define MAX_READY_QUEUE 100
#define MAX_WAITING_QUEUE 100
#define MAX_ARRIVAL_TIME 10
#define MAX_PRIORITY 10
#define RR_UNIT 2
#define MAX_RECORD 1000000

typedef struct process
{
    int bursts[MAX_BURST];
    int burst_tot;
    int burst_idx;
    int cur_burst;
    int arrival_time;
    int priority;
} Process;

typedef struct priority_rr
{
    int process[100];
    int cur_excute;
    int cur_remaintime;
    int process_tot;
} PriorityRR;

Process processes[MAX_PROCESS];
Process original[MAX_PROCESS];
int process_tot;

int waiting_queue[MAX_PROCESS],
    ready_queue[MAX_PROCESS],
    terminated_process[MAX_PROCESS];

int waiting_time[MAX_PROCESS];
int process_log[MAX_RECORD];

PriorityRR PRR[MAX_PRIORITY];

int t;

void record_to_log(int process)
{
    process_log[t] = process + 1;
}

void add_to_ready_queue(int process_idx);

int no_zero_rand(int range)
{
    return (rand() % range) + 1;
}

void FCFS();
void SJF();
void SJF_preemptive();
void Priority();
void Priority_preemptive();
void Priority_RR();
void RR();
void Lottery();
void analysis(char *s);

void init(char *filepath)
{
    FILE *fp = NULL;
    int user_process_tot;
    int cur_burstes;
    srand((unsigned int)time(NULL));
    process_tot = no_zero_rand(MAX_PROCESS);
    for (int i = 0; i < process_tot; ++i)
    {
        cur_burstes = no_zero_rand(MAX_BURST);
        for (int j = 0; j < cur_burstes; ++j)
        {
            processes[i].bursts[j] = (j % 2 == 0 ? 1 : -1) * no_zero_rand(MAX_BURST_TIME);
        }
        processes[i].burst_tot = cur_burstes;
        processes[i].burst_idx = 0;
        processes[i].arrival_time = rand() % MAX_ARRIVAL_TIME;
        processes[i].priority = rand() % MAX_PRIORITY;
    }
    fp = fopen(filepath, "r");
    fscanf(fp, "%d\n", &user_process_tot);
    if (user_process_tot != -1)
    {
        process_tot = user_process_tot;
        for (int i = 0; i < process_tot; ++i)
        {
            fscanf(fp, "%d %d %d\n", &processes[i].burst_tot, &processes[i].arrival_time, &processes[i].priority);
            for (int j = 0; j < processes[i].burst_tot; ++j)
            {
                fscanf(fp, "%d ", &processes[i].bursts[j]);
            }
            processes[i].burst_idx = 0;
        }
    }
    fclose(fp);

    for (int i = 0; i < MAX_PROCESS; ++i)
    {
        original[i] = processes[i];
    }
}

int main(int argc, char *argv[])
{
    if (argc == 1)
        init("input.txt");
    else
        init(argv[1]);

    FCFS();
    analysis("FCFS");
    SJF();
    analysis("SJF");
    SJF_preemptive();
    analysis("SJF_preemptive");
    Priority();
    analysis("Priority");
    Priority_preemptive();
    analysis("Priority_preemptive");
    RR();
    analysis("RR");
    Priority_RR();
    analysis("PRR");
    Lottery();
    analysis("Lottery");
    return 0;
}

void increase_waiting_time()
{
    for (int i = 0; i < process_tot; ++i)
    {
        if (ready_queue[i] == 0)
            continue;
        ++waiting_time[i];
    }
}

void refresh_ready_queue(int time, int cur_process_idx)
{
    for (int i = 0; i < process_tot; ++i)
    {
        if (i == cur_process_idx)
            continue;
        if (ready_queue[i] == 1)
            continue;
        if (waiting_queue[i] == 1)
            continue;
        if (terminated_process[i] > 0)
            continue;
        if (processes[i].arrival_time > time)
            continue;
        ready_queue[i] = 1;
        PRR[processes[i].priority].process[PRR[processes[i].priority].process_tot++] = i;
        // 새로운 프로세스 PRR에 추가.
    }
}

int select_process_from_ready_queue()
{
    int min = MAX_ARRIVAL_TIME + 1;
    int min_idx = -1;
    for (int i = 0; i < process_tot; ++i)
    {
        if (ready_queue[i] == 0)
            continue;
        if (terminated_process[i] > 0)
            continue;
        if (min > processes[i].arrival_time)
        {
            min = processes[i].arrival_time;
            min_idx = i;
        }
    }
    if (min_idx != -1)
        ready_queue[min_idx] = 0;
    return min_idx;
}

int work_job_queue(int cur_process_idx)
{
    record_to_log(cur_process_idx);
    if (--processes[cur_process_idx].bursts[processes[cur_process_idx].burst_idx] == 0)
    {
        return cur_process_idx;
    }
    return -1;
}

void work_waiting_queue()
{
    for (int i = 0; i < process_tot; ++i)
    {
        if (waiting_queue[i] == 0)
            continue;
        if (++processes[i].bursts[processes[i].burst_idx] < 0)
            continue;
        waiting_queue[i] = 0;
        add_to_ready_queue(i);
    }
}

int check_process_terminated(int process_idx)
{
    if (++processes[process_idx].burst_idx == processes[process_idx].burst_tot)
    {
        terminated_process[process_idx] = t;
        return 1;
    }
    return 0;
}

int check_all_process_terminated()
{
    for (int i = 0; i < process_tot; ++i)
    {
        if (terminated_process[i] == 0)
            return 0;
    }
    return 1;
}

void add_to_waiting_queue(int process_idx)
{
    if (check_process_terminated(process_idx))
        return;
    waiting_queue[process_idx] = 1;
}

void add_to_ready_queue(int process_idx)
{
    if (check_process_terminated(process_idx))
        return;
    ready_queue[process_idx] = 1;
}

void init_cpu()
{
    t = 0;
    for (int i = 0; i < MAX_PROCESS; ++i)
    {
        processes[i] = original[i];
        ready_queue[i] = 0;
        waiting_queue[i] = 0;
        terminated_process[i] = 0;
        waiting_time[i] = 0;
    }
    for (int i = 0; i < MAX_RECORD; ++i)
        process_log[i] = 0;
    for (int i = 0; i < MAX_PRIORITY; ++i)
    {
        PRR[i].process_tot = 0;
        PRR[i].cur_excute = 0;
        PRR[i].cur_remaintime = RR_UNIT;
    }

    printf("\tProcess\t\tArrivals\t\tPriority\t\tBursts\n");
    for (int i = 0; i < process_tot; ++i)
    {
        printf("\tProcess %d\t", i);
        printf("arrival time : %4d\t Priority : %4d\t Burst : ", processes[i].arrival_time, processes[i].priority);
        for (int j = 0; j < processes[i].burst_tot; ++j)
        {
            printf("%d\t", processes[i].bursts[j]);
        }
        printf("\n");
    }
}

void FCFS()
{
    int cur_process_idx;
    int ended_process_idx;
    init_cpu();
    while (1)
    {
        refresh_ready_queue(t, -1);
        cur_process_idx = select_process_from_ready_queue();
        if (cur_process_idx == -1) // 현재 실행할 프로세스가 없음.
        {
            if (check_all_process_terminated() == 1)
                return;
            ++t;
            increase_waiting_time();
            work_waiting_queue();
            continue;
        }
        while (1)
        {
            ended_process_idx = work_job_queue(cur_process_idx);
            work_waiting_queue();
            ++t;
            increase_waiting_time();
            refresh_ready_queue(t, cur_process_idx);
            if (ended_process_idx > -1)
            {
                add_to_waiting_queue(ended_process_idx);
                break;
            }
        }
    }
}

int select_process_from_ready_queue_SJF()
{
    int min = MAX_BURST_TIME + 1;
    int min_idx = -1;
    for (int i = 0; i < process_tot; ++i)
    {
        if (ready_queue[i] == 0)
            continue;
        if (terminated_process[i] > 0)
            continue;
        if (min > processes[i].bursts[processes[i].burst_idx])
        {
            min = processes[i].bursts[processes[i].burst_idx];
            min_idx = i;
        }
    }
    if (min_idx != -1)
        ready_queue[min_idx] = 0;
    return min_idx;
}

void SJF()
{
    int cur_process_idx;
    int ended_process_idx;
    init_cpu();
    while (1)
    {
        refresh_ready_queue(t, -1);
        cur_process_idx = select_process_from_ready_queue_SJF();
        if (cur_process_idx == -1) // 현재 실행할 프로세스가 없음.
        {
            if (check_all_process_terminated() == 1)
                return;
            ++t;
            increase_waiting_time();
            work_waiting_queue();
            continue;
        }
        while (1)
        {
            ended_process_idx = work_job_queue(cur_process_idx);
            work_waiting_queue();
            ++t;
            increase_waiting_time();
            refresh_ready_queue(t, cur_process_idx);
            if (ended_process_idx > -1)
            {
                add_to_waiting_queue(ended_process_idx);
                break;
            }
        }
    }
}

void SJF_preemptive()
{
    int cur_process_idx;
    int ended_process_idx;
    init_cpu();
    while (1)
    {
        refresh_ready_queue(t, -1);
        cur_process_idx = select_process_from_ready_queue_SJF();
        if (cur_process_idx == -1) // 현재 실행할 프로세스가 없음.
        {
            if (check_all_process_terminated() == 1)
                return;
            ++t;
            increase_waiting_time();
            work_waiting_queue();
            continue;
        }
        ended_process_idx = work_job_queue(cur_process_idx);
        work_waiting_queue();
        ++t;
        increase_waiting_time();
        if (ended_process_idx > -1)
            add_to_waiting_queue(ended_process_idx);
        else
            ready_queue[cur_process_idx] = 1;
    }
}

int select_process_from_ready_queue_Priority()
{
    int min = MAX_PRIORITY + 1;
    int min_idx = -1;
    for (int i = 0; i < process_tot; ++i)
    {
        if (ready_queue[i] == 0)
            continue;
        if (terminated_process[i] > 0)
            continue;
        if (min > processes[i].priority)
        {
            min = processes[i].priority;
            min_idx = i;
        }
    }
    if (min_idx != -1)
        ready_queue[min_idx] = 0;
    return min_idx;
}

void Priority()
{
    int cur_process_idx;
    int ended_process_idx;
    init_cpu();
    while (1)
    {
        refresh_ready_queue(t, -1);
        cur_process_idx = select_process_from_ready_queue_Priority();
        if (cur_process_idx == -1) // 현재 실행할 프로세스가 없음.
        {
            if (check_all_process_terminated() == 1)
                return;
            ++t;
            increase_waiting_time();
            work_waiting_queue();
            continue;
        }
        while (1)
        {
            ended_process_idx = work_job_queue(cur_process_idx);
            work_waiting_queue();
            ++t;
            increase_waiting_time();
            refresh_ready_queue(t, cur_process_idx);
            if (ended_process_idx > -1)
            {
                add_to_waiting_queue(ended_process_idx);
                break;
            }
        }
    }
}

void Priority_preemptive()
{
    int cur_process_idx;
    int ended_process_idx;
    init_cpu();
    while (1)
    {
        refresh_ready_queue(t, -1);
        cur_process_idx = select_process_from_ready_queue_Priority();
        if (cur_process_idx == -1) // 현재 실행할 프로세스가 없음.
        {
            if (check_all_process_terminated() == 1)
                return;
            ++t;
            increase_waiting_time();
            work_waiting_queue();
            continue;
        }
        ended_process_idx = work_job_queue(cur_process_idx);
        work_waiting_queue();
        ++t;
        increase_waiting_time();
        if (ended_process_idx > -1)
            add_to_waiting_queue(ended_process_idx);
        else
            ready_queue[cur_process_idx] = 1;
    }
}

int select_process_from_ready_queue_RR(int former_idx)
{
    int next_idx = -1;
    int cur_idx;
    for (int i = 1; i <= process_tot; ++i)
    {
        cur_idx = (former_idx + i) % process_tot;
        if (ready_queue[cur_idx] == 0)
            continue;
        if (terminated_process[cur_idx] > 0)
            continue;
        next_idx = cur_idx;
        break;
    }
    if (next_idx != -1)
        ready_queue[next_idx] = 0;
    return next_idx;
}

int select_process_from_ready_queue_Priority_RR()
{

    int next_idx = -1;
    int cur_idx;
    int cur_execute;
    int max_priority = MAX_PRIORITY + 1;
    // 가장 높은 Priority 찾기
    for (int i = 0; i <= process_tot; ++i)
    {
        if (ready_queue[i] == 0)
            continue;
        if (terminated_process[i] > 0)
            continue;
        if (processes[i].priority < max_priority && PRR[processes[i].priority].process_tot > 0)
        {
            max_priority = processes[i].priority;
        }
    }
    if (MAX_PRIORITY + 1 == max_priority)
        return -1;
    for (int i = 0; i <= PRR[max_priority].process_tot; ++i)
    {
        cur_idx = (PRR[max_priority].cur_excute + i) % PRR[max_priority].process_tot; // 다음 RR로 넘어가기.
        cur_execute = cur_idx; // idx 저장
        cur_idx = PRR[max_priority].process[cur_idx]; // 실제 프로세스의 idx 적용.
        if (i > 0)
            PRR[max_priority].cur_remaintime = RR_UNIT; // 기존 프로세스가 아닌 프로세스는 시간 유닛을 초기화해줌.
        if (ready_queue[cur_idx] == 0)
            continue;
        if (terminated_process[cur_idx] > 0)
            continue;
        if (PRR[max_priority].cur_remaintime == 0)
        { // 0이면 다음으로 넘어가야함.
            continue;
        }
        next_idx = cur_idx;
        PRR[max_priority].cur_excute = cur_execute; //다음에 실행할 거 저장.
        break;
    }
    ready_queue[next_idx] = 0;
    PRR[max_priority].cur_remaintime--; // 시간을 깎아줌.
    return next_idx;
}

void RR()
{
    int cur_process_idx;
    int ended_process_idx;
    int time_unit = 0;
    int former_idx = -1;
    init_cpu();
    while (1)
    {
        refresh_ready_queue(t, -1);
        cur_process_idx = select_process_from_ready_queue_RR(former_idx);
        former_idx = cur_process_idx;
        if (cur_process_idx == -1) // 현재 실행할 프로세스가 없음.
        {
            if (check_all_process_terminated() == 1)
                return;
            ++t;
            increase_waiting_time();
            work_waiting_queue();
            continue;
        }
        time_unit = 0;
        while (time_unit < RR_UNIT)
        {
            ++time_unit;
            ended_process_idx = work_job_queue(cur_process_idx);
            work_waiting_queue();
            ++t;
            increase_waiting_time();
            refresh_ready_queue(t, cur_process_idx);
            if (ended_process_idx > -1)
            {
                add_to_waiting_queue(ended_process_idx);
                break;
            }
        }
    }
}

void Priority_RR()
{
    int cur_process_idx;
    int ended_process_idx;
    int time_unit = 0;
    int former_idx = -1;
    int former_priority = -1;
    init_cpu();
    while (1)
    {
        refresh_ready_queue(t, -1);
        cur_process_idx = select_process_from_ready_queue_Priority_RR();
        if (cur_process_idx == -1) // 현재 실행할 프로세스가 없음.
        {
            if (check_all_process_terminated() == 1)
                return;
            ++t;
            increase_waiting_time();
            work_waiting_queue();
            continue;
        }

        ended_process_idx = work_job_queue(cur_process_idx);
        work_waiting_queue();
        ++t;
        increase_waiting_time();
        if (ended_process_idx > -1)
            add_to_waiting_queue(ended_process_idx);
        else  
            ready_queue[cur_process_idx] = 1;
    }
}

int select_process_from_ready_queue_Lottery()
{
    int lottery_tot = 0;
    int cur_lottery;
    int lottery_idx = -1;

    for (int i = 0; i < process_tot; ++i)
    {
        if (ready_queue[i] == 0)
            continue;
        if (terminated_process[i] > 0)
            continue;
        lottery_tot += processes[i].priority;
    }
    if(lottery_tot == 0) return -1;
    cur_lottery = rand() % lottery_tot;
    lottery_tot = 0;
    for (int i = 0; i < process_tot; ++i)
    {
        if (ready_queue[i] == 0)
            continue;
        if (terminated_process[i] > 0)
            continue;
        lottery_tot += processes[i].priority;
        ready_queue[i] = 0;
        if(lottery_tot >= cur_lottery) return i;
    }

    return -1;

}

void Lottery()
{
    int cur_process_idx;
    int ended_process_idx;
    init_cpu();
    while (1)
    {
        refresh_ready_queue(t, -1);
        cur_process_idx = select_process_from_ready_queue_Lottery();
        if (cur_process_idx == -1) // 현재 실행할 프로세스가 없음.
        {
            if (check_all_process_terminated() == 1)
                return;
            ++t;
            increase_waiting_time();
            work_waiting_queue();
            continue;
        }
        ended_process_idx = work_job_queue(cur_process_idx);
        work_waiting_queue();
        ++t;
        increase_waiting_time();
        if (ended_process_idx > -1)
            add_to_waiting_queue(ended_process_idx);
        else
            ready_queue[cur_process_idx] = 1;
    }
}

void analysis(char *s)
{
    int total_waiting_time = 0;
    int total_turnaround_time = 0;
    int next_process;
    int cur_process;
    printf("\n\t\t=========%s=========\t\t\n", s);
    printf("\tProcess\t\tTurnAround\t\tWaiting\n");
    for (int i = 0; i < process_tot; ++i)
    {
        printf("\tProcess %d\t", i);
        total_waiting_time += waiting_time[i];
        printf("waiting time : %4d\t", waiting_time[i]);
        total_turnaround_time += terminated_process[i] - processes[i].arrival_time;
        printf("turnaround time : %4d\n", terminated_process[i] - processes[i].arrival_time);
    }
    printf("Average Waiting Time : %.2f\n", ((float)total_waiting_time / process_tot));
    printf("Average Turnaround Time : %.2f\n\n", ((float)total_turnaround_time / process_tot));
    printf("\t-------------------------------------------------\n");
    for (int i = 0; i < t; ++i)
    {
        cur_process = process_log[i] - 1;
        next_process = process_log[i + 1] - 1;
        if (cur_process < 0)
            printf("\t|\t\t\tNOP\t\t\t|\n");
        else
            printf("\t|\t\t\t%3d\t\t\t|\n", cur_process);
        if (cur_process == next_process && cur_process >= 0)
            printf("\t|\t\t\t\t\t\t|\n");
        else
            printf("%4d\t-------------------------------------------------\n", i + 1);
    }
}