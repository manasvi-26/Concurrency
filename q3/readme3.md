

## Question 3
##  Musical Mayhem
This question simulates a Music club event where k musicians and singers(threads) arriving at random times , try to acquire a stage(acoustic or electric depending on instrument) to perform and a tshirt for their performances(resources). This is done by utilizing semaphores , threads , and conditional waits and signaling


### Overview

A musician or singer on arrival looks for an empty stage to perform. He/she can perform for a random amount of time between t1 and t2 seconds (which are taken as input). A singer also has the opportunity to join a singer on the stage and extend the total performance time by 2 seconds. Each musician/singer will get impatient after waiting for more than t seconds for a stage. After performing the musician/singer goes to c coordinators to collect a tshirt. The coordinators take 2 seconds to give them the tshirt.


A musician can take an electric stage , acoustic stage or both depending on the instrument they play.(Piano - both , Guitar - both , Bassist - electric , Violinist - Acoustic). A singer can take both.

semaphores used : acoustic_musician, electric_musician, total_singer,coordinator

acoustic_musician keeps track of number acoustic stages left for musician to perform
same goes for electric

total_singer keeps track of total number of stages available for a singer.


when a musician can perform on both kinds of stages a new thread is created and both time wait simultaneously. a global variable is used to inform which stage has been accessed first.


singer thread does a timed wait on the total singer semaphore.
It runs through the available stages - can either be emepty or have a musician
conditional wait is used when a singer is performing with a muscian.

If a singer or a musician finishes his performance he waits for a tshirt.
The semaphore C is waited on. Since the value is c , only c threads can enter at a time and others are blocked until they get their turn 
