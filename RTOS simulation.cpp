#include <iostream>
#include <pthread.h>
#include <unistd.h>  
#include <cstring>
using namespace std;

struct Task {
    int pid;
    int arrival;                     //arrival time
    int burst;                            //process time
    int deadline;                   //using soft deadline
    int completion;             //burst + start time
    pthread_t thread;
};

struct threadarg {
    Task* task;
    int   pipe_fd;  //file descriptor for writing
    int   start_time;
};

void* run_task(void* argfun) {  //finds if deadline was missed or not
    threadarg* a = (threadarg*)(argfun);  //contains start time etc
    Task* t2 = a->task;

    int finishtime = a->start_time + t2->burst;  //finish time

    cout <<endl <<  "Task " << t2->pid << " done at " << finishtime << " deadline " << t2->deadline << " ";
    if(finishtime > t2->deadline)  //if deadline is missed
    {
      cout << " MISSED" << endl;
    }
    cout << endl;
    // Write a fixed message to pipe
    string buff = "task: "  +to_string(t2->pid) + " done. \n";  //had to use cstring to do this
    write(a->pipe_fd, buff.c_str(), buff.size());
    return NULL;
    }

int main()
{
    int n;
    cout << "Enter number of tasks: ";   //n tasks
    cin  >> n;

    Task* tasks = new Task[n];  //task array
    threadarg* args  = new threadarg[n];  //argument array

    for(int i = 0; i < n; i++)
    {  //entering ID, arrival, burst and deadline
        cout << endl <<  "Task " << i+1 << " ID: ";
        cin  >> tasks[i].pid;
        cout << endl << "arrival time: ";
        cin  >> tasks[i].arrival;
        cout << endl <<  "burst time: ";
        cin  >> tasks[i].burst;
        cout << endl << "deadline: ";
        cin  >> tasks[i].deadline;
    }

    // bubble sorting to find out smallest value of arrival time
    int i , j ;
    Task tempo;  //temporary variable for swapping
    for(int i = 0 ; i < n ; i++)
    {
      for(int j = 0 ; j < n - i -1 ; j++)
      {
        if(tasks[j].arrival > tasks[j+1].arrival)
        {
          //swap
          tempo = tasks[j];
          tasks[j] = tasks[j+1];
          tasks[j+1] = tempo;
        }
      }
    }
    
 

    int pipe_fd[2];          //file descriptors
    pipe(pipe_fd);              // initializing
    int read_fd  = pipe_fd[0];      //read
    int write_fd = pipe_fd[1];        //write

    cout <<endl << " FCFS RTOS     " << endl;
    int current_time = 0;  //starting

    for(int i = 0; i < n; i++) 
    {
        Task &temp = tasks[i];  //temp variable
        if (current_time < temp.arrival)
        {
            current_time = temp.arrival;   //starting time
        }

        cout << "time " << current_time << ": starting task " << temp.pid <<endl;

        args[i].task = &temp;
        args[i].pipe_fd = write_fd;
        args[i].start_time = current_time;

        if (pthread_create(&temp.thread, NULL, run_task, &args[i]) != 0)   //error case
        {
            perror("pthread_create");
            return 1;
        }
        

        cout << "time "<< current_time << ": task "<< temp.pid <<" started"<< endl;

        pthread_join(temp.thread, NULL);  //joining with main

        temp.completion = current_time + temp.burst;  //end time 
        cout << "time " << temp.completion << ": task " << temp.pid << " finished" << endl; 
        if ( temp.completion > temp.deadline)  //task exiting message
        {
          cout << "Deadline was missed" << endl << endl;
        }
        current_time = temp.completion;
        
    }

    close(write_fd);
    cout << "pipe message" << endl;
    char buffer[256];
    int r = read(pipe_fd[0],&buffer, sizeof(buffer)-1);
    if( r > 0)
    {
      buffer[r] = '\0';
      cout << buffer << endl;
    }
      

    close(read_fd);
    pthread_exit(NULL);
    delete[] tasks;
    delete[] args;
    return 0;
}
