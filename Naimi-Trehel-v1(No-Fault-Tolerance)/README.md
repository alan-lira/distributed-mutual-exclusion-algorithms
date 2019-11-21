# distributed-mutual-exclusion-algorithms/Naimi-Trehel-v1(No-Fault-Tolerance)
Implementation based on "A LOG (N) DISTRIBUTED MUTUAL EXCLUSION ALGORITHM BASED ON THE PATH REVERSAL" paper by Mohamed NAIMI, Michel TREHEL & André ARNOLD.

M. Naimi, M. Trehel, A. Arnold, A Log(N) distributed mutual exclusion algorithm based on path reversal, J. Parallel Distributed Comput. 34 (1) (April
1996).

TRY IT OUT:

---------- For Linux Users: ----------

   DEPENDENCIES:

      First step: sudo apt update

      "make": sudo apt install make

      "Open MPI": sudo apt install libopenmpi-dev

   TUTORIAL:

      1) Clone (or Download ZIP) this repository;

      2) Unzip it;

      3) Open this directory 'Naimi-Trehel-v1(No-Fault-Tolerance)' into Terminal;

      4) Run the following instructions to COMPILE:
         4.1) cp makefile.txt Makefile
         4.2) make

      5) Run the following instructions to EXECUTE:
         5.1) mpiexec -np X naimi-trehel

         (Where X equals to number of MPI processes)

            5.1.1) Include -p argument to print node events on terminal.
            5.1.2) Include -l argument to log node events into file.

-------------------------------------