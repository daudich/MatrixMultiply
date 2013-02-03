# Cannon's Algorithm
***

## What is Cannon's Algorithm?

Cannon's Algorithm was proposed in 1969 for a MIMD(Multiple Instruction Multiple Data).
Since it was hard to find the original paper online, used another paper which describes the original
algorithm more clearly.

* Citation:

  * *Original* : Cannon, L. E. (1969). A Cellular Computer To Implement The Kalman Filter Algorithm
    (No. 603-Tl-0769). Montana State Univ Bozeman Engineering Research Labs.

  * *My Reference* : Lee, H. J., Robertson, J. P., & Fortes, J. A. (1997, July). Generalized
    Cannon's algorithm for parallel matrix multiplication. In Proceedings of the 11th international
    conference on Supercomputing (pp. 44-51). ACM.

## What did I do?

I implemented the algorithm on a cluster and tried to improve the performance by also adding a
multi-core support for the algorithm which the present algorithm does not take into consideration.
Furthermore, to ensure that I complete the project, I limited the input to only two
N x N matrices, and N % Processors = 0 for distribution.

The project was implemented on SHARCNET using the Pilot library.

### What is SHARCNET?
*From its [website](https://www.sharcnet.ca/my/about)*

SHARCNET is a consortium of Canadian academic institutions who share a network of high performance computers.

### What is Pilot?
*From its [website:](http://carmel.socs.uoguelph.ca/pilot)*

A new method of programming HPC clusters that leverages standard MPI while being easier for novice
users to understand and utilize. MPI’s large and daunting API presents multiprogramming hazards
that even trained programmers find difficult to cope with (such as dead-lock.) In such instances
detecting the condition, let alone diagnosing the cause is overly challenging for beginning
scientific programmers.

Pilot is designed for scientific programmers that are experts in their own field but often possess
only elementary C and FORTRAN knowledge. In Pilot, formal elements from CSP, valuable for creating
a sound theoretical basis, are kept “under the hood” of an “stdio.h - inspired” interface to avoid
intimidating users with the need to learn a process algebra. In short, the goal of this work is to
make HPC programming more accessible and hazard-free for scientific users.

## Lessons learnt and outcome.

This was an interesting undertaking. The algorithm has been around for quite sometime and there
have been several implementations. Implementing it in Pilot was challenging overall. Since, it
was suggested to me that I can test my implementation against one already out there and forego
the pthread implementation, I have done so. However, towards the end several attempts(unsuccessful)
were made to implement a pthread version.

The primary challenge was to decide how to distribute the matrices, the original algorithm does
not have a suggestion, but assumes that there are N x N number of processors and does all the shift
and compute calculation with that in mind. I decided to use the block distributed. To ensure that
I focus more on the algorithm than all the possible test cases, I decided to only accept two
dimensionally identical matrices and only accept number of processors which can evenly divide the
matrix. This was a good design decision, since I did manage to complete the project.

In hind sight, it was a worthwhile project, since I got a chance to implement a well known algorithm
and also understand my own deficiencies and where I can improve, if I were to redo this project.
For example, the original algorithm relies on shift-and-multiply. This is hard to do since every
processor would have to be in a matrix where everyone is sharing information with their neighbour.
Looking at other implementations I have come to realise that this is somewhat possible with some
low level complicated MPI calls.

Another problem, I encountered was that I had to maintain a reference matrix which instead of simply
building a smaller matrix on the slave node, I decided to have the full fledged matrix on every node,
this causes problems since computing the next shift became O(n^2) this brought down the efficiency
considerable. I sort of expected this, hence, I was going to try and have two matrices as references,
and have the other matrix recompute the next shift on a separate thread/core whilst the former matrix
would be used as references on a different core/thread. I am unhappy I made this decision. However,
it was the only way I could think of to make it consistent and sound as possible.

A few other blunders/short sightedness includes choosing to do everything under the precision of
int, this caused headaches later when I realised I could not have numbers beyond a certain point
otherwise the results become negative. I was at crossroads since changing everything to long long
int had unintended consequences of having different answers. Another challenge had to do with the
fact that there was no way to avoid sending messages back to the master node. This caused most
hold ups as the matrix size increases there is more data being passed back to the master and
hence more latency.

Using Pilot was good, very intuitive and it helped me write more clear and concise  code. I
should have/could have included MPI code directly in the Pilot program. Despite the fact that
the MPI version code was faster, I feel much proud in the fact that my program is capable on
running on over 200 cores and still produce the same output. Mathematically speaking there were
a lot of challenges in terms of coming up with algorithms to compute the values in the Reference
matrix in less than O(n^2), something I can improve next time around.

After submitting my proposal, I took a careful look at the feedback I received and I like the idea
of comparing a Pilot version with the MPI version and I have enclosed the corresponding graphs
later in this section and with that respect I succeeded. As a note, the MPI version had to
manipulated slightly for timing purposes. I also managed to have a serial and a Pilot version,
hence success. The pthreads issue, as discussed earlier would have turned out slower than the
Pilot and the serial version so it was abandoned completely.

## How to run?

* To run the cannon.c program simply run the Makefile, and then:

  For 1 processor(serial):
      sqsub -q mpi -r 2 -n 1 -o log1 ./cannon matrices

  For N processors(parallel):
      sqsub -q mpi -r 2 -n N -o logN ./cannon matrices

*NOTE: The matrices A and B are stored in the “matrices” folder. The matrices have to be both
N x N, i.e. same size. Furthermore, the processors assigned have to be such that: N^2 mod numProcs = 0,
they have to be perfectly divisible.*

*To run the mmult_cannon.c program(MPI) version that was not coded by me, simply run the Makefile and then:

    	sqsub -q mpi -n N -o logN ./mmult_cannon.c

*NOTE: There were a few side notes on how to run the program.*

  * Input               : Read files (mdata1.inp) for first input matrix 
                        and (mdata2.inp) for second input matrix 

  * Output              : Result of matrix matrix multiplication on Processor 0. 

  * Necessary Condition : Number of Processes should be less than 
                        or equal to 8. Matrices A and B should be 
                        equally striped. that is Row size and 
                        Column size should be properly divisible 
                        by Number of processes used.


Moreover, both programs only accept 2 matrices.
Finally, both Makefiles have a clean dependency to clean the directory after you are done.

*The mmult program was NOT written by me. You can find it [here](http://www.cs.umanitoba.ca/~comp4510/examples.html)*