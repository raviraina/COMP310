# COMP310 Assignment 2

## Authors

Ravi Raina, 260828720

Sarvasv Arora, 260979687

## Comments

For the assignment, we used our Assignment 1 shell code based off of the original base code provided.
Code additions and improvements include:

- PCB and Ready Queue support.
- Support for loading and cleaning up scripts from the shell memory.
- Four new schedulers: First Come First Served (FCFS), Shortest Job First (SJF), Round Robin (RR), and SJF with job aging (AGING).
- Scripts executed using `run` and `exec  ` command are now first loaded into shell memory, then executed using the specified scheduler.
- Variables initialized in a certain program pertain to that environment without interfering with variables from other programs. When the program terminates, variables from that program are also deleted.
- Support for recursive `run` and `exec` commands.
