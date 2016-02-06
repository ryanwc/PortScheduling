int: nr; % number of reclaimers
         % number of rails = nr for stages A,B,C,D,E
         % number of rails = (nr + 1) div 2 for stage F
set of int: RECLAIMER = 1..nr;
bool: stageF; % is this stage F data

int: ns; % number of stockpiles
set of int: STOCKPILE = 1..ns;
array[STOCKPILE] of int: size; % size in 10000 tonnes
array[STOCKPILE] of SHIP: ship; % which ship carries stockpile

% assert each ship has cargo (check for the data)
constraint forall(sh in SHIP)
                 (assert(exists(s in STOCKPILE)(ship[s] = sh),
                  "Ship " ++ show(sh) ++ " has no cargo!"));
 
int: maxtime; % time considered
set of int: TIME = 0..maxtime;

int: nsh; % number of ships
set of int: SHIP = 1..nsh;
array[SHIP] of TIME: arrival; % when ship arrives in port

int: len; % length of pad
set of int: POSITION = 0..len;

% stack time for a stockpile is stack_time * size[i]
int: stack_time;  % time to stack one unit

% stack time for a stockpile is reclaim_time * size[i]
int: reclaim_time;  % time to reclaim one unit
int: reclaim_speed;  % time a reclaimer moves one unit of distance

%%% CRITICAL DECISIONS for Stage A %%%

% where is the westend of the stockpile
array[STOCKPILE] of var POSITION: westend;
% where is the eastend of the stockpile (based on size of the stockpile and westend)
array[STOCKPILE] of var POSITION: eastend;
% when does stacking start for a stockpile
array[STOCKPILE] of var TIME: stack;
% when does stacking end for a stockpile
array[STOCKPILE] of var TIME: endstack;
% amount of time each stockpile sits idle
%array[STOCKPILE] of var TIME: dwelltime;
%var 0..ns*maxtime: totalDwelltime = sum(i in STOCKPILE)(dwelltime[i]);
% when does reclaiming start for a stockpile
array[STOCKPILE] of var TIME: reclaim;
% when does reclaiming end for a stockpile
array[STOCKPILE] of var TIME: finished;
% how long does a stockpile exist for
array[STOCKPILE] of var TIME: timeExisted;
% which reclaimer does the reclaiming for a stockpile (useless in Stage A)
array[STOCKPILE] of var RECLAIMER: which;
% for what times is a stockpile being reclaimed
array[STOCKPILE] of var set of TIME: reclaimSpan;

%%% CONSTRAINTS %%%

% Stage A: 1) Make sure none of the stockpiles overlap in space time.

% end of stacking for a stockpile is the start of its stacking plus the unit stacking speed * stockpile size
constraint forall(i in STOCKPILE)
                 (endstack[i] = stack[i] + stack_time * size[i]);

% end of reclaiming for a stockpile is the start of its reclaiming plus the unit reclaiming speed * stockpile size
constraint forall(i in STOCKPILE)
                 (finished[i] = reclaim[i] + reclaim_time * size[i]);

% stockpile cannot be reclaimed until it is done being stocked
constraint forall(i in STOCKPILE)
                 (endstack[i] <= reclaim[i]);

% dwell time for a stockpile is the start of its reclaiming minus the end of its stacking
%constraint forall(i in STOCKPILE)
 %                (dwelltime[i] = reclaim[i] - endstack[i]);

% the amount of time a stockpile exists for is its finished time minus its start time.
constraint forall(i in STOCKPILE)
                 (timeExisted[i] = finished[i] - stack[i]);

% eastend of each stockpile is the stockpile's westend plus size
constraint forall(i in STOCKPILE)
                 (eastend[i] = westend[i] + size[i]);

% ensure stockpiles that overlap in time (excluding start and end of their existence) do not overlap in space
%include "diffn.mzn";
%constraint diffn(westend, stack, eastend, timeExisted);

constraint forall(i1, i2 in STOCKPILE where i1 < i2)
                 ( eastend[i1] <= westend[i2] \/
                   eastend[i2] <= westend[i1] \/
                   finished[i1] <= stack[i2] \/
                   finished[i2] <= stack[i1] );
 
% Stage B: 1) Assign stockpiles to reclaimers (give "which" meaningful values)
%          2) Ensure no reclaimer is reclaiming two stockpiles at the same time

% the times at which a stockpile is being recalimed is the start of its reclaiming to the end of its reclaiming.
constraint forall(i in STOCKPILE)
                 (reclaimSpan[i] = reclaim[i]..finished[i]);

% stockpiles with the same reclaimer can only overlap in time for being reclaimed at the end of a reclaim period
constraint forall(p1, p2 in STOCKPILE where p1 < p2)
                 ( (which[p1] = which[p2]) -> (reclaimSpan[p1] intersect reclaimSpan[p2] = {}) );

% Stage C: Use shipping data.  
%          1) No stockpile can be reclaimed before the ship it is assigned to has arrived
%          2) No two stockpiles can be reclaimed onto the same ship at the same time

% No stockpile can be reclaimed before the ship it is assigned to has arrived
constraint forall(i in STOCKPILE)
                 (reclaim[i] >= arrival[ship[i]]);

% No two stockpiles can be reclaimed onto the same ship at the same time
constraint forall(i1, i2 in STOCKPILE where i1 < i2)
                 ( (ship[i1] = ship[i2]) -> 
                   ((finished[i1] < reclaim[i2]) \/ (finished[i2] < reclaim[i1])) );

% Stage D: Reclaimers are not infinitely fast, cannot move instantly to another place.
%          Reclaimers reclaim from the westend of a stockpile.  So:
%          1) If a reclaimer finishes reclaiming a stockpile with westend at position x,
%             and next starts reclaiming a stockpile with westend at positiony,
%             there is at least |y-x|*reclaim_speed time between those events.

constraint forall(i1, i2 in STOCKPILE where i1 < i2)
                 ( (which[i1] = which[i2]) ->
                   ( (reclaim[i2] - finished[i1] >= abs(westend[i2] - westend[i1])*reclaim_speed) \/
                     (reclaim[i1] - finished[i2] >= abs(westend[i1] - westend[i2])*reclaim_speed) ) ); 

% Stage E: Up until now, we were only satisfying constraints.
%          But, the objective is to minimize the amount of time a ship is at berth.
%          A ship is at berth from when it arrives to when the last stockpile for that
%          to when the last stockpile for that ship is finished reclaiming.
%          1) Minimize the time each ship is at berth.

% berth for a ship is the finish time of its last stockpile minus its arrival time

array[SHIP] of set of int: cargo = [ { j | j in STOCKPILE where ship[j] = i } | i in SHIP ];

array[SHIP,STOCKPILE] of 0..ns: cargo2D = 
     array2d(SHIP,STOCKPILE,
             [ if i in cargo[j] then i else 0 endif
              | j in SHIP, i in STOCKPILE ]);
array[SHIP] of var TIME: berth;

constraint forall(i in SHIP)
                 (berth[i] = max(j in STOCKPILE where cargo2D[i,j] != 0)(finished[cargo2D[i,j]]) - arrival[i]);

var 0..nsh*maxtime: totalBerth;

constraint totalBerth = sum(i in SHIP)(berth[i]);

% Stage F: Previously, we considered each reclaimer on its own rail.
%          But, usually 2 reclaimers per rail.
%          Add constraints to:
%          1) Ensure that reclaimers 2i-1 and 2i which are on rail i are such that
%             reclaimer 2i-1 is always west of 2i
%             (or at least reclaimer 2i is never west of 2i-1)


%constraint forall(i in RECLAIMER)
 %                ( (i mod 2 = 0) -> forall(j1, j2 in STOCKPILE where j1 < j2)
  %         ((((reclaimSpan[j1] intersect reclaimSpan[j2] != {}) /\ (which[j1] = i) /\ (which[j2] = (i-1))) ->
   %                           (westend[j1] > westend[j2])) /\
    %        (((reclaimSpan[j1] intersect reclaimSpan[j2] != {}) /\ (which[j2] = i) /\ (which[j1] = (i-1))) ->
     %                         (westend[j2] > westend[j1]))));

constraint forall(i in RECLAIMER)
                 ( ((i mod 2 = 0) -> forall(j in STOCKPILE)
                                           ( (which[j] = i) -> (westend[j] >= len div 2) ) )
                    /\
                   (((i mod 2 = 1) /\ i != nr) -> forall(j in STOCKPILE)
                                                        ( (which[j] = i) -> (westend[j] <= len div 2) ) ) );


%%% Solve and Search %%%

solve :: int_search(finished, dom_w_deg, indomain_min, complete)
         minimize totalBerth;



%output for submission
output 
["westend  = ", show(westend), ";\n"] ++
["eastend  = ", show(eastend), ";\n"] ++
["stack    = ", show(stack), ";\n"] ++
["endstack = ", show(endstack), ";\n"] ++
["reclaim  = ", show(reclaim), ";\n"] ++
["finish   = ", show(finished) , ";\n"] ++
["which    = ", show(which), ";\n"]
;

% output for solution visualization
% warning: this is an experimental feature.  Feedback and suggestions are encouraged.
% 
%include "portschedule_viz.mzn";
%output [portschedule_viz(westend,eastend,stack,endstack,reclaim,finished,which)];