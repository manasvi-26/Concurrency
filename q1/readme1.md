
## Question 1
### Concurrent Merge-Sort
Comparing different ways of running merge-sort, one which does everything in one process and the other which creates separate processes for the array segments(For size < 5 , it just does selection sort). The bonus was also done by implementing two ways of multi threaded sorting

##### Concurrent Multiprocess Mergesort
In this , the left and right half of the array is given to two separate processes. A new left and right process is created using fork() system call. Both children are waited for using waitpid until they return back after sorting. The two parts are then merged using the regular merge function of a merge sort.

##### Concurrent Recursive Multi threaded Mergesort
In this , the left and right halves of the array are taken up by two separate threads. So the threads are recursively created for each child by pthread_create. They are then waited on by pthread_join and then merged.

##### Concurrent Parted Multi threaded Mergesort
In this , the array is sorted by using 8 threads and then giving them 1/8th of the array each to sort. These parts are then merged after joining all threads. This way was also done as recursively creating threads turned out to be too slow.

---

The running times were compared for both on sizes ranging from **n = 1** to **n = 50000** (each is an average of 15 runs)

##### For n = 1:
- Normal Merge sort took on average **0.000028** seconds
- Concurrent Multiprocess Merge sort took on average **0.000030** seconds
- Concurrent Recursive Multi threaded Mergesort took on average **0.000280** seconds
- Concurrent Parted Multi threaded Mergesort took on average **0.001000** seconds

##### For n = 10:
- Normal Merge sort took on avergae **0.000032** seconds
- Concurrent Multiprocess Merge sort took on average **0.000439** seconds
-  Concurrent Recursive Multi threaded Mergesort took on average **0.001500** seconds
- Concurrent Parted Multi threaded Mergesort took on average **0.000950** seconds

##### For n = 100:
- Normal Merge sort took on average **0.000081** seconds
- Concurrent Multiprocess Merge sort took on average **0.000356** seconds
-  Concurrent Recursive Multi threaded Mergesort took on average **0.016500** seconds
- Concurrent Parted Multi threaded Mergesort took on average **0.000840** seconds



##### For n = 1000:
- Normal Merge sort took on average **0.000589** seconds
- Concurrent Multiprocess Merge sort took on average **0.000354** seconds
- Concurrent Recursive Multi threaded Mergesort took on average **0.035000** seconds
- Concurrent Parted Multi threaded Mergesort took on average **0.000320** seconds



##### For n = 10000:
- Normal Merge sort took on average **0.003825** seconds
- Concurrent Multiprocess Merge sort took on average **0.000358** seconds
-  Concurrent Recursive Multi threaded Mergesort took on average **0.500000** seconds
- Concurrent Parted Multi threaded Mergesort took on average **0.001860** seconds



##### For n = 50000:
- Normal Merge sort took on average **0.014772** seconds
- Concurrent Multiprocess Merge sort took on average **0.000553** seconds
- Concurrent Recursive Multi threaded Mergesort took on average **2.020000** seconds
- Concurrent Parted Multi threaded Mergesort took on average **0.009600** seconds


---
##### Multiprocess
It is observed that after **n = 1000** , the concurrent merge sort started overtaking the normal merge sort.
 This is because below 1000 the **time for forking and creating processes was limiting factor for the concurrent merge sort**. But after that the **speed provided by concurrency(lot of processes making progress together) from the concurrent mergesort** made it more effecient then normal. 
 The speed gain by now multiple process sorting parts of the array is much more than the speed decresed by creating new processes. 

This was not tested for beyond **n = 3000** as the number of forks was beyond the limit of the current laptop the code was being tested on.

##### Multi threaded
The unusually high times for 1 sized array in both multi threaded sorts is due to the large over head of creating a thread. 

The recursive multi threaded sort increases greatly(2 seconds for 50,000) due to the fact that after a certain point , the large number of threads created stop working in parallel(due to limited number of cpu cores). So the process has a lot of threads , and is therefore a very heavy process using up all the cores which becomes very slow(atleast on the laptop tested)
The parted multithreaded sort is a little bit faster as only 8 threads are created.
It gets faster till 1000 sized array as concurrent threads are working together on the array. But after that , since each thread is running normal mergesort the time again increases. 
(Please note the times measured of multi threaded part are very machine specific)
