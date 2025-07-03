Simulator Blocks
================

The simulator internally solves the system based on the following primitives.

Non-Linear Equation
-------------------
A Non-Linear block has at least one output and any number of inputs, it is expected to be cheap to calculate like the multiply block or the add block.

this block ``apply`` is called in the inner-most non-linear solver steps, any number of times until convergence.

Examples: Multiply and Add blocks.

Non-Linear Stateful Equation
----------------------------
this is similar to the non-linear block and gets applied in the inner non-linear solver steps, 
but it also takes time as a parameter and has an update function that is called after every major step with the block inputs, 
and it also has a function that gets called after every major step if a registered zero crossing was triggered,
and it can also set specific time points at which the ``update`` method must be called, which causes the solver to start a new major step from this point.

this block ``apply`` is called in the inner-most non-linear solver steps, any number of times until convergence.

Examples: Derivative block.

Differential Equation
---------------------
A differential equation has at least one output and any number of inputs, it also takes time as input, but its output is the **slope** of the output at this time step,
which is used by whatever differential solver used (currently dopri5) to calculate the value of its outputs at every time step.

this block ``apply`` is called exactly once during each minor time step of the differential solver.

Examples: Integration block.

Source Equation
---------------
A source equation has only outputs, and it takes time as input, and can define time points at which it will get notified with ``EventTrigger``.

it gets called during the inner-most non-linear solver steps, any number of times, so it must be cheap, 
it can use the event triggers to set up the data for linear interpolation during the inner-most evaluation if needed.

Examples: all sources, Linear, step, sine, etc...

Observer
--------
An Observer has only inputs and no outputs, and it also takes the time as input, and is called after every major step, 
it is used to capture and display the data either on a scope or to IO, it also gets called at the start of the simulation with the start and end time of the simulation,
as well as after the simulation ends with the end time in case the simulator stops in the middle of the simulation.

this block ``apply`` is called exactly once after every major time step, and doesn't need to be cheap.

Examples: Scope block.

Flow Equation
-------------
this block can have any number of inputs and at least 1 in-out node, which is treated by the solver as a *flow node*, 
in the inner non-linear solver steps it outputs the flow **into** the block at each node.

flow nodes are either forced or estimated, for estimated nodes the solver will keep estimating their value until the sum of flow into the node is zero, 
forced nodes have no restrictions.

this block ``apply`` is called every inner-most non-linear solver steps to calculate the flow into the block at each connected node.

Potential Equation
------------------
this block must have exactly 2 in-out nodes, and any number of inputs, and exactly 1 output. in-out nodes are treated as *flow node*, 
it is called in the inner-most non-linear solver solver steps, and it is passed the **estimated** current at this time step, 
and it must output the potential difference between its two in-out nodes, the output node always represent the *estimated current* in case other blocks want to monitor this value.

it is quite the opposite of flow equations, and it can be used to create voltage sources or pumps.

this block ``apply`` is called every inner-most non-linear solver steps to calculate the potential difference between the two connected in-out nodes.
