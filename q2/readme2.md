
## Question 2
##  Back to College
This question uses the concept of concurrency and threads to simulate a covid environment in the college with **n** Companies producing vaccine batches , **m** Zones performing vaccine trials with those batches and **o** Students that take those vaccines.

### Overview

The code makes each company , each zone and each student a thread. So there are n+m+o threads created in the program. With the usage of threads and appropriate syncronization , the program is able to run a simulation where Companies are producing batches , Zones are accepting bathces and running vaccination phases by giving slots to students , who on taking the vaccine take a test to determine its success. And all this can happen concurrently , and hence is not sequential:-
 - Companies do not give batches one by one and lots of companies can give batches to empty zones at the same time.
 - Multiple zones can be accepting of these bacthes.
 - A zone can allow multiple students to come and take slots at once.

The pthreads POSIX library is used for creating and joining threads , and also creating and using locks. The pthread locks are used in various places in the code to protect shared resources among the threads. For eg. :-

- Two companies not trying to give a batch to the same zone.
- The zone is fully being assigned a batch before it starts its operations with it(which were being manipulated by the company thread while assigning).
- Two or more zones trying to assign to the same student.
- Manipulating the number of students variable by any thread.

All of these resources are protected. The code uses **m** locks(company_zone_lock) , one for each zone so it is locked during the company deliviring it a batch. The code uses **o** locks(zone_student_lovk), one for each student which locks it during a zone being assigned to a student. And then it uses another lock -  student_lock which prevents race conditions on the change of variable students(number of students remaining). The simulation ends , when number of students remaining is 0 i.e all students were either recovered or sent back.



### Assumptions / Points

- A company takes 2 to 5 seconds to prepare its batches which have 10 to 20 vaccines each.
- The probability of success of its vaccines are linked to the company during input itself
- A company , for the distribution of its batches , will pick randomly from zones 1 to m give a batch. If the zone is already having a batch then the company keeps looking
- A company only starts producing when all the batches that it gave out are consumed.
- A zone after recieveing a batch , starts a vaccine phase by assigning k random slots after setup of 1 to 2 seconds
- This k is minimum of 8 , number of students left , and the number of vaccines left in the zone.
- A zone is said to finish a phase when it has assigned all its slots atleast once(no matter failure or success in antibodies , the vaccine still gets used)
- A zone cannot start start assigning slots before its current phase is finished
- A student , takes 1 to 2 seconds after arriving at the gate to start looking for slots. 
- A student has 4 states - WAITING , SITTING ,  VACCINATING and DONE
- A zone assigns student a slot only if his/her state is WAITING , and then changes it to SITTING.
- Once the zone is done seating all , it then changes all students sitting to VACCINATING
- A student may take 1 to 3 seconds after sitting on a slot to get his vaccination done
- A student takes 1 to 3 seconds to get his antibodies test done
- He has 3 attempts. If he fails he is sent back to arrive and wait (state back to WAITING) and at the 3rd fail he is sent back home(state to DONE).
- If the test succeeds he is sent to college(state to DONE)
- The number of companies , zones and students should all be atleast 1.
- For the probability of success of a vaccine , the code takes an input till 2 decimal places exactly.
- The minimum number of companies, zones and students all are 1.
- All companies and zones halt operations once the number of students become zero
- The maximum threads limit of the program is 1024 for each(company , zone and thread)

### Code Explanation

- ##### Threads/Variables/Locks

    There are **n Company threads** , **m Zone threads** and **o Student threads**. There is also an **array of m locks** and an **array of o locks** for the **m company_zone locks** and **o zone_student locks**. 

    *Company struct* -> (Batches produced , Batches left , Probability of success , Number of vaccines per batch)
    *Zone struct* -> (Company that gave batch , Number of current vaccines , Probability of success of each vaccine , Free slots available)
    *Student struct* -> (Zone assigned , state(WAITING , VACCINATING , DONE) , Number of tries left)


//company_init() and distribute function()

Here the first if condition lets the zone keep waiting until it is assigned a batch(no longer -1 then so goes through)
Here there is another trylock of this zones own lock. This is to prevent any conflicts when the company is delivering this zone a batch and then the zone thread starts running. The trylock over here can be unlocked only when the company is finished manupilating the zone_arr and done assigning. The zone waits 1 to 2 seconds before each vaccination phase and keeps going until it runs out of vaccines. After that the zone is said to have emptied the batch and there for the batches_left in the company assigned to it is decremented


// Vaccinate phase()
A random k slots is picked. Then the phase goes on in the while loop until all the slots are assigned to a student or number of students become 0. The trylock here is zone_student lock. It lockes the particular students(which is also picked at random) lock and starts manipulating the stud_arr. It makes all the students seated to the state SITTING. The while loop(phase) is broken early if the number of students at the gate become zero . 
After the while loop , the zone makes all students seated to the VACCINATING state


//student init()
The student busy waits at both the WAITING and SITTING state at the while loops. 
Here again it attempts to lock itself first to assure that the assigning by zone(changing in stud_arr) before advancing int this thread. 
The student sleeps for 1 to 3 seconds to get a vaccine as well as the test after that.



A random integer test is chosen from 1 to 100. If the number is less or equal to (Probability of success*100) then , its a success otherwise a fail. If a sucess , the state is changed to DONE and the number of students is decremented.
The decrement happens inside the zone protected by the student_lock to avoid race condition. If the test fails , the student is changed back to waiting and continued or changed to DONE if fails the 3rd time. If the student is back to waiting state then the gate variable is also incremented as he is back at the gate.
 After any of these the zone_student lock is also unlocked.


---
**The entire Simulation ends when the students become 0.(All while loops execute and exit and all the threads exit)**


    


    








