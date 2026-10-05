# Assignment 1 - Joshua Ganschow

## Build and run instructions

### R1
To build, run, and observe output of part R1 (filename `r1_test.c`), follow the below instructions

1. run `gcc queue.c r1_test.c -pthread -o r1`
2. run `./r1 128 32 4 4` to run the test program with 128 jobs divided among 4 producers placed into the queue with a bound of 32 jobs and have 4 consumers consume the jobs
3. Observe output being 
```
Expected job sum: 8128
Actual job sum: 8128
Did all jobs run exactly once? Yes
```
There are two shared atomic variables keeping track of the sum of seen job IDs and keeping track of which jobs it has seen. The output shows that the sum is exactly as expected as well as each job was run exactly one time. 

### R2
To build, run, and observe output of part R2 (filename `r2_main.c`), follow the steps below.

1. run `gcc queue.c pool.c r2_main.c -pthread -o r2`
2. run `./r2 4 32` to run the worker pool simulation program with 4 workers and 32 jobs queued.
3. Observe output to see how quickly the pool completed all 32 jobs and see statistics about the processing.

## Timing method

I used the suggested monotonic `clock_gettime` function from `time.h`. I followed [this article](https://stackoverflow.com/questions/53708076/what-is-the-proper-way-to-use-clock-gettime) on stack overflow for the timing calculation.

## Assumptions and known limitations

I assumed that the main thread who enqueues the jobs does not need direct access to the completed work. This is a limitation but could easily be worked around by designing the job that is enqueued a little differently (outputting results to a file, for example).

## AI use statement

I have used AI only for the allowed purposes; specifically I used it for re-familiarizing myself with the `pthread` library and for general syntax queries (i.e. "How to instantiate a 2D array with malloc", "Can you cast any type in C to a `void*`?")

I used ChatGPT; here is the link to my conversation so you can see exactly the questions that I asked: [link](https://chatgpt.com/share/6aa309ea-d9e8-83ea-a250-cc8931c6c8b9)