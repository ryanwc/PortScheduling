# PortScheduling Overview

At ports, bulk mineral cargo reclaimers stockpile resources from outside sources and then "reclaim" the resources onto arriving transportation.  So, scheduling cargo reclaimers is a complex and important task for many ports.

PortScheduling is a program that optimizes cargo reclaimer activity at seaports subject to nearly real-world constraints (for example, varying schedules of arriving ships with various cargo requirements).

This program was written as a final project for the class [Modeling Discrete Optimization](https://www.coursera.org/course/modelingoptimization) offered by The University of Melbourne on Coursera.

# Technical Requirements to Run PortScheduling

To run this software, you need MiniZinc 2.0.x and the GECODE 4.4.x solver. Both of these are included in the bundled version of the MiniZinc IDE 0.9.9, which you can find at the [MiniZinc website](http://www.minizinc.org).

# PortScheduling Problem Motivation and Purpose

At seaports, bulk mineral cargo reclaimers stockpile resources from trains or other outside sources and then "reclaim" the resources onto arriving ships.  Stockpiles are stored on pads, which are very long and have fixed width.  As trains bring in materials, stockpiles are stacked onto the pads.  So, stockpiling resources on the pad is a two dimensional packing problem because we must reserve space on the pad for a stockpile from the moment we start stacking to the moment it is fully reclaimed (i.e., two stockpiles cannot coexist in space time).

The purpose of the PortScheduling software is to produce schedules for reclaimer activity that satisfy an optimization objective (for example, "minimize total time each ship spends waiting in port").

# How PortScheduling Works

This section explains the variables that PortScheduling considers and the constraints PortScheduling faces while optimizing reclaimer activity.

### Variables PortScheduling Considers

The PortScheduling program will optimize reclaimer activity according to the following variables given at the start of the program:

1. Number of stockpiles
2. Size of each stockpile
3. Length of the pad
4. Number of reclaimers
5. Reclaimer movement speed
6. Reclaimer stockpiling speed
7. Reclaimer reclaiming speed
8. Number of ships arriving in port
9. Arrival times of each ship
10. The ID of the ship that "owns" each stockpile
11. A time limit for completing all reclaiming

Note that this software considers the simple example of a single long pad that holds all stockpiles.

### Constraints on Reclaimer Activity

Given the variables described above, PortScheduling will optimize reclaimer activity subject to the following constraints:

1. Reclaimer movement:
	- Each reclaimer must run on only one of the one or more parallel rails beside the pad.
	- There can be one or two reclaimers on each rail.
	- Reclaimers on the same rail cannot pass each other (but CAN be in the same position).
	- Reclaimers on separate rails are free to move past each other.
	- The time a reclaimer takes to move one unit of distance along the pad is determined by the given reclaimer movement speed.
	- A reclaimer cannot be in two places at once.
2. Stacking stockpiles:
	- A stockpile's "position" is its "lowest" (or "westernmost") edge.
	- A stockpile's size and position determines the range of space on the pad that it occupies by "lowest+size=highest", with the stockpile's range of space equaling lowest-to-highest.  For example, a stockpile with size "3" and position "1" will occupy three units of space on the pad starting at 1 and ending at 4.  You can imagine the pad runs west to east, so "lowest" is actually the west end of the stockpile and "highest" is actually the east end of the stockpile.
	- Once a stockpile starts being stacked, it immediately claims all of the space on the pad that it will eventually occupy.
	- Stockpiles cannot overlap in space and time simultaneously (but they CAN overlap in either space OR time).  For example, if there is a size 3 stockpile at position 1 from time 10-20, there CANNOT be a stockpile of size 2 at position 3 from time 15-25, but there CAN be a stockpile of size 2 at position 3 from time 20-30 (overlaps in space but not time) or a stockpile of size 2 at position 4 from time 15-25 (overlaps in time but not space).  Put another way, if one stockpile occupies pad positions 1-4 from time 10-20, stockpiles that occupy pad positions 0-1 and 4-6 from times 0-10 and 20-30, respectively, do not overlap with it in time OR space.
	- The time required to stack a stockpile is determined by the size of the stockpile and the given stacking speed.
	- Stockpiling is not done by reclaimers and so not subject to their constraints.  So, if there is currently a space on the pad large enough to accomodate the stockpile, stacking can just "begin" for that stockpile regardless of how many untasked reclaimers there are and where those reclaimers are on the pad.  This is a simplification that could be expanded.
3. Reclaiming stockpiles:
	- A stockpile must be completely stacked before its reclaiming can begin.
	- A stockpile must be reclaimed by one and only one specific reclaimer.
	- A stockpile can be reclaimed starting at or after the time the ship that "owns" it arrives.
	- A stockpile must be reclaimed onto the ship that "owns" it.
	- No two stockpiles can be reclaimed onto the same ship at the same time.
	- The reclaimer must be at the "lowest" (or "westernmost") end of the stockpile while reclaiming it.
	- Once a reclaimer starts reclaiming a stockpile, it must continue reclaiming that stockpile until the stockpile is completely reclaimed.
	- The time required to reclaim a stockpile is determined by the size of the stockpile and the given reclaimer reclaiming speed.

NOTE: To simplify development, the project was completed in stages, with each stage building upon the last in complexity.  So, in the portschedule.mzn file (the file that contains the logic for the model), you will see comments indicating which "stage" a constraint belongs to.

# Optimization Objective

Given the above variables and constraints, PortScheduling's objective is to minimize the total time ships spend at berth. A ship is at berth from its arrival time to the end of the reclaiming of the last stockpile it "owns".

The optimization objective can easily be changed by modifying a few lines of code.

# How to Use PortScheduling

PortScheduling is written in the MiniZinc language, which is a medium-level constraint programming language.  A MiniZinc program execution consists of a model, whose file ends in ".mzn", paired with data, whose file ends in ".dzn".  The model can be run with any data as long as the data conforms to the definitions the model expects. Please see the [MiniZinc website](http://www.minizinc.org) if you would like to learn more about it.

**Running the Program As-Is**

There are many ways to run a MiniZinc program.  Here is one common way to run this program as-is:

1. Download all of the files in this repo to the same directory on your computer.
2. Open the file named "portschedule.mzn".
3. Click "show project explorer" in the top right of the MiniZincIDE.
4. Right click on "Data (right click to run)".
5. Select all of the data files that you would like to import into the project explorer and choose "open".
6. Right click on one of the data files in the project explorer and choose "Run the model with this data".

**Extending the Program to Run with Your Own Data**

You can also create your own scenarios to pair with the given model.  To do so, you must create a ".dzn" file and populate it with data of the following form:

```
nr = [integer];
stageF = [boolean];
ns = [integer];
nsh = [integer];
size = [array of integers];
ship = [array of integers];
stack_time = [integer];
reclaim_time = [integer];
reclaim_speed = [integer];
maxtime = [integer];
arrival = [array of integers];
len = [integer];
```

where:

1. "nr" is the number of reclaimers.
2. "stageF" is a boolean indicating whether the constraints related to number of reclaimers on the same rail and reclaimers being able to pass each other should be turned on.
3. "ns" is the number of stockpiles.
4. "nsh" is the number of ships arriving in port.
5. "size" is an array with the size of each stockpile where the ith entry in the array corresponds to the number of units (size) the ith stockpile.
6. "ship" is an array with the ship that "owns" each stockpile where the ith entry in the array is the number of the ship that owns the ith stockpile.
7. "stack_time" is the time it takes for one unit of a stockpile to load onto the pad.
8. "reclaim_time" is the time it takes a reclaimer to load one unit of a stockpile onto a ship.
9. "reclaim_speed" is the time it takes a reclaimer to move one unit in either direction along the pad.
10. "maxtime" is the time limit for reclaiming all stockpiles.
11. "arrivial" is an array with arrivial times for each ship where the ith entry in the array is the arrival time of the ith ship.
12. "len" is the length of the pad.

**Modifying the Model**

If you wish to modify the model (for example, to remove or add constraints), you should modify the file called "portschedule.mzn".  WARNING: This may make the model function incorrectly.

# Output

PortScheduling makes the following critical decisions subject to the data given at runtime and the constraints programmed into the model (as described above):

1. The time at which each stockpile starts being stacked onto the pad.
2. The position of each stockpile's lowest (or "westernmost") edge on the pad. 
3. The time at which each stockpile starts being reclaimed onto the ship that "owns" it.
4. The specific reclaimer used to reclaim each stockpile.

Given N stacks, the output of the program, which documents the above decisions, takes the following form:

```
westend = [WestEnd1, WestEnd2, ..., WestEndN]; 
eastend = [EastEnd1, EastEnd2, ..., EastEndN]; 
stack = [StackStartTime1, StackStartTime2, ..., StackStartTimeN];
endstack = [StackEndTime1, StackEndTime2, ..., StackEndTimeN];
reclaim = [RecStartTime1, RecStartTime2, ..., RecStartTimeN]; 
finish = [RecEndTime1, RecEndTime2, ..., RecEndTimeN]; 
which = [RecID1, RecID2, ..., RecIDN];
```

where

1. "westend" is an array of integers with the ith entry indicating the position of the ith stockpile's lowest (or "westernmost") edge on the pad.
2. "eastend" is an array of integers with the ith entry indicating the position of the ith stockpile's highest (or "easternmost") edge on the pad.
3. "stack" is an array of integers with the ith entry indicating the time at which the ith stockpile starts being stacked onto the pad.
4. "stack" is an array of integers with the ith entry indicating the time at which the ith stockpile is finished being stacked onto the pad.
5. "reclaim" is an array of integers with the ith entry indicating the time at which the ith stockpile starts being reclaimed onto the ships that "owns" it.
6. "finish" is an array of integers with the ith entry indicating the time at which the ith stockpile is finished being reclaimed onto the ships that "owns" it.
7. "which" is an array of integers with the ith entry indicating the ID of the reclaimer that reclaimed the ith stockpile.

Note that the pad can be imagined to run west to east, so on a pad of length 10, "0" is the westernmost position and "10" is the easternmost position.

The format and substance of the output can easily be modified to show different information about the optimal schedule (for example, to show the value related to the optimization objective, which currently is "total time of ships spend in berth").

# License

This software is licensed under the [GNU General Public License v3.0](https://www.gnu.org/licenses/gpl.html).  Accordingly, you are free to run, study, share, and modify this software only if you give these same freedoms to users of *your* implementation of this software.

# Credit

This README is based on 1) information from the problem definition PDF provided by The University of Melbourne for the class [Modeling Discrete Optimization](https://www.coursera.org/course/modelingoptimization) offered on Coursera and 2) my experience constructing the program specified in that PDF.

# Last Note: Experimental Visualization

At the bottom of the portschedule.mzn file, you may notice some commented out code that says "output for solution visualization".  This is experimental code that will visualize the reclaimer activity.

I did not use this feature when developing the software, so I can't offer many tips for using this feature or interpreting the visualization.  However, I included the file dependencies in this repo in case you want to play with it.

To visualize the results, simply uncomment the last two lines of the portschedule.mzn file and run the program normally.  Note that this feature is experimental, with the development team providing this warning: "Be aware that sometimes the visualization can show a crossing which does not need to be there since it simply connects the reclaim jobs in order."  In reality, the automatic grader used for the course would throw an error if reclaimers on the same rail crossed.

All credit for the visualization feature goes to the University of Melbourne team.