<h1 align="left">Stress test performance comparison between variable size memory (de)allocation with w/o lock </h1>
<h2 align="center"> 
 
  ## Goal
 Using **compressed data** and **quantization** techniques to lessen the solving time in 0/1 knapsack problem with **acceptable deviation rate** from optimal solution.  
 
  ## Introduction
  
**Dynamic programming (DP)** is one of efficient way to solve 0/1 knapsack problem (**maximization problem with constraint**). However, when the selected items and contraint become larger, it **increases computation time** and **takes more memory to store the result of subproblem**. In real case, the weights (cost) distribution of items is **not uniform**. Normally, the distribution is skewed toward the lower half of the possible range of the weights. I utilize this characteristics and make some items with small cost to merge into one new item with bigger cost, resulting in smaller number of items being considered.

  
  
  ## Description
First, I create a binomial tree to each bucket of weights (e.g., bucket1: [w:1 - w:9], bucket2: [w:10 - w:19],...). The reason of choosing **binomial heap** to record items is because it only takes **O(1) time** for **deleting**, **inserting**, and **merging operation**. Second, traverse the heaps to search if there are possible sub-heaps that could be merged into the bucket with higher range of cost. After compression, I also quantized the cost in the same bucket to trim some variability in weights, accelerating the process in solving problem.
 <p align="center">
 <img src="https://github.com/ychuang1234/lock-free-dynamic-memroy--de-allocation-with-variable-size/blob/b24fa924e98c27b3ac28f8997d653ac91f57cf57/procedure.JPG" width="75%" height="75%">
 </p>
 
## Overall result
When the weights distribution is skewed more toward lower part of the possible range of weights, the compression rate become higher and the reduction rate of the elapsed time become higher. And the obtimality of the result is still retained around 95% of the optimal solution.

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
| Thread 3       | 0.001, 0.004, 0.004 | 0.002, 0.036, 0.187 
| Thread 4       | 0.003, 0.002, 0.004 | 0.004, 0.026, 0.195 
## Experiment settings and detailed results

This is the output from .ipynb file with various experiment settings.
<p align="center">
 <img src="https://github.com/ychuang1234/lock-free-dynamic-memroy--de-allocation-with-variable-size/blob/b24fa924e98c27b3ac28f8997d653ac91f57cf57/result1.JPG " height="80%">
 </p>
 
<p align="center">
 <img src="https://github.com/ychuang1234/lock-free-dynamic-memroy--de-allocation-with-variable-size/blob/b24fa924e98c27b3ac28f8997d653ac91f57cf57/result2.JPG " height="80%">
 </p>
  
  <p align="center">
 <img src="https://github.com/ychuang1234/lock-free-dynamic-memroy--de-allocation-with-variable-size/blob/b24fa924e98c27b3ac28f8997d653ac91f57cf57/result3.JPG " height="80%">
 </p>

