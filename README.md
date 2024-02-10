# You Spin Me Round Robin
Round Robin is a scheduling algorithm, this is an imitation of the scheduling algorithm with variable quantum lengths.

## Building

```shell
make
```

## Running

cmd for running
```shell
./rr processes.txt quantum_length
quantum_length is an integer that the user sets
```

results TODO
```shell
./rr processes.txt 3
```
Should return 
Average waiting time: 7.00
Average response time: 2.75
## Cleaning up

```shell
make clean
```
