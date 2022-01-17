<h1 align="left">Stress test performance comparison between variable size memory (de)allocation with w/o lock </h1>
<h2 align="center"> 
 
  ## Goal
 Compare performance (**elapsed time**, **scalability** in multi-threading) of consecutive operations in memory management with **lock (with mutex)** and **lock-free (atomic operation provided since c++11)** version. 
 
  ## Introduction
Memory management is one of critical part in OS kernel service. It allocates memory buffer to function and ensure the memory buffer allocated could not be allocated to another function until function releases the ownership of that memory buffer. When systems run in multi-threading mode, ideally, memory management should run under critical section, preventing two threads modify the information in memory management at the same time, resulting in **race condition**. Therefore, **synchronization techniques** are required in this scenario. 
  
  ## Description
In this project, I used two ways of synchronization to implement memory management service, which are **mutex** and **atomic operation**. The main difference between lock and lock-free synchronization techniques is wheather threads are racing to seize the lock or just load and store variables stores in atomic memory buffer without any seizing and waiting. However, the disavantage of atomic operation is that when atomic store (std::atomic_store(...))is not successful, the subsequent cost is to re-do the whole operation again.<br>
 I choose **singly linked list** to implement memory buffer in order to reduce the memory quantity being modified when processing memory allocation and deallocation, which benefits the easiness in implementing the lock-free memory management service. Nevertheless, in mutex version, it is expected to be implemented in much more complexed data structure, e.g., **balanced binary search tree** or **B-tree**. Because the main purpose is performance comparison in this project, I choose the simple data structure to implement two version of memory management.
 
 <p align="center">
 <img src="https://github.com/ychuang1234/lock-free-dynamic-memroy--de-allocation-with-variable-size/blob/b24fa924e98c27b3ac28f8997d653ac91f57cf57/procedure.JPG" width="100%" height="100%">
 </p> 
 
## Experiment settings and detailed results
In this project, I designed stress tests with different number of transaction and number of threads (number of p_thread in C or thread number assigned to OpenMP in C++). The results are shown below:

 * Allocation : Deallocation = 10:0
  
| Multi-threading settings   | Mutex (lock) Elapsed time (sec)<br> (Transaction num: 100, 1000, 10000)| Atomic (lock free) Elapsed time (sec)<br>  (Transaction num: 100, 1000, 10000)
| ----------- |:----------:| :--------------:  
| Thread 2       | 0.001, 0.001, 0.002 | 0.002, 0.007, 0.095  
| Thread 3       | 0.001, 0.001, 0.002 | 0.005, 0.036, 0.140 
| Thread 4       | 0.001, 0.001, 0.002 | 0.006, 0.043, 0.165  

 * Allocation : Deallocation = 7.5:2.5
  
| Multi-threading settings   | Mutex (lock) Elapsed time (sec)<br> (Transaction num: 100, 1000, 10000)| Atomic (lock free) Elapsed time (sec)<br>  (Transaction num: 100, 1000, 10000)
| ----------- |:----------:| :--------------:  
| Thread 2       | 0.002, 0.002, 0.003 | 0.002, 0.011, 0.147  
| Thread 3       | 0.001, 0.003, 0.003 | 0.003, 0.024, 0.120 
| Thread 4       | 0.002, 0.002, 0.002 | 0.007, 0.050, 0.175  
  
  * Allocation : Deallocation = 5:5
  
| Multi-threading settings   | Mutex (lock) Elapsed time (sec)<br> (Transaction num: 100, 1000, 10000)| Atomic (lock free) Elapsed time (sec)<br>  (Transaction num: 100, 1000, 10000)
| ----------- |:----------:| :--------------:  
| Thread 2       | 0.002, 0.002, 0.002 | 0.003, 0.013, 0.187  
| Thread 3       | 0.001, 0.004, 0.004 | 0.002, 0.036, 0.190 
| Thread 4       | 0.003, 0.002, 0.004 | 0.004, 0.026, 0.195 
 
 * Line chart of memory (de)allocation performance under setting **Allocation : Deallocation = 10:0**
<p align="center">
 <img src="https://github.com/ychuang1234/lock-free-dynamic-memroy--de-allocation-with-variable-size/blob/b24fa924e98c27b3ac28f8997d653ac91f57cf57/result1.JPG " height="80%">
 </p>
 
  * Line chart of memory (de)allocation performance under setting **Allocation : Deallocation = 7.5:2.5**
<p align="center">
 <img src="https://github.com/ychuang1234/lock-free-dynamic-memroy--de-allocation-with-variable-size/blob/b24fa924e98c27b3ac28f8997d653ac91f57cf57/result2.JPG " height="80%">
 </p>
 
 
   * Line chart of memory (de)allocation performance under setting **Allocation : Deallocation = 5:5**
  <p align="center">
 <img src="https://github.com/ychuang1234/lock-free-dynamic-memroy--de-allocation-with-variable-size/blob/df953519fdfde2816836c241c6de687e44494ae9/result3.JPG " height="80%">
 </p>

## Findings
* Memory management with mutex outperformed lock-free version
 
   * Possible reason 1: Collision in stress test is highly likely to happen.
   * Possible reason 2: Retrying costs substancially compared to waiting and blocking.
 * However, could stress test be considered as the true scenario when kernel processing system call? 
   * We may need to take the real severity of collision in memory management (especially time calling the memory management system call) into consideration.
   * When the collision is not that high, the benefit of using atomic operation may be appeared.
 
