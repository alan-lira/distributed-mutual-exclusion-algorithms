# distributed-mutual-exclusion-algorithms/Naimi-Trehel-v2(Fault-Tolerance)
Implementation based on "HOW TO DETECT A FAILURE AND REGENERATE THE TOKEN IN THE LOG(N) DISTRIBUTED ALGORITHM FOR MUTUAL EXCLUSION" paper by Mohamed NAIMI & Michel TREHEL.

Mohamed, N., & Michel, T. (1988). How to detect a failure and regenerate the token in the Log(n) distributed algorithm for mutual exclusion. Lecture Notes in Computer Science, 155–166.

TRY IT OUT:

---------- For Linux Users: ----------

   DEPENDENCIES:

      First step: sudo apt update

      "make": sudo apt install make

      "Open MPI": sudo apt install libopenmpi-dev

   TUTORIAL:

      1) Clone (or Download ZIP) this repository;

      2) Unzip it;

      3) Open this directory 'Naimi-Trehel-v2(Fault-Tolerance)' into Terminal;

      4) Run the following instructions to COMPILE:
         4.1) cp makefile.txt Makefile
         4.2) make

      5) Run the following instructions to EXECUTE:
         5.1) mpiexec -np X naimi-trehel (Where X equals to number of MPI processes)

-------------------------------------