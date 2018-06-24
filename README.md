
This is a WIP, and I'm learning Git/GitHub, so this might dis-appear at any moment, while I get comfortable with the whole thing. The code in reference is not by me.

# General Purpose GOAP

## Introduction
GOAP, or Goal Oriented Action Planning is a powerful tool to create game AI. For all the details I will refer to [Jeff Orkin's collection of articles](http://web.media.mit.edu/~jorkin/goap.html). But in short: GOAP will let computer controlled characters (NPCs) make action plans that can achieve desired goals. It will do so in a highly maintainable, easily extendible, highly modular fashion. Naive implementation of AI code will invariably blow up for any non trivial problem. GOAP on the other hand, is robust and is unlikely to buckle under large complexity. This software implements GOAP in the C programming language. It does so in a generic fashion, which makes it suitable for many projects.

## Basics of GOAP
Creating a plan for AI controlled entities comes down to the following steps:

1. Describe the repertoire of actions available.
2. Describe the current state of the world.
3. Describe the desired state of the world (goal).

To describe the actions, we specify:

* The preconditions for the action.
* The postconditions (effects) of the action.
* The cost of the action.

To describe world state, we define a set of world state atoms. Each atom has a tag, and a boolean value.

The planner will then be able to formulate a plan of actions that takes the world to the desired state, provided such a path exists. The plan formulated is guaranteed to be the lowest cost plan.

## Example Scenario

Let us consider a planner for an AI soldier. Our soldier can perform the following actions: scout, approach, aim, shoot, load, detonatebomb, flee.

* **scout** requires: *armedwithgun*. Effect: *enemyvisible*.
* **approach** requires: *enemyvisible*. Effect: *nearenemy*.
* **aim** requires: *enemyvisible* and *weaponloaded*. Effect: *enemylinedup*.
* **shoot** requires: *enemylinedup*. Effect: *!enemyalive*.
* **load** requires: *armedwithgun*. Effect: *weaponloaded*.
* **detonatebomb** requires: *armedwithbomb* and *nearenemy*. Effect: *!alive* and *!enemyalive*.
* **flee** requires: *enemyvisible*. Effect: *!nearenemy*.

Next, we will tell the planner that currently:
Enemy is not visible, we are armed with gun, our weapon is not loaded, enemy is not lined up, enemy is alive, we are armed with bomb, we are not near enemy, we are alive.

Then we tell our planner what our desired world looks like. We only care about one thing: our enemy is not alive.

With this, the planner can formulate a plan of actions to make this happen. For the moment, let's assume all our actions have the default cost 1 associated with it. This is what the planner will return to us:

                           ARMEDWITHGUN,enemyvisible,nearenemy,weaponloaded,enemylinedup,ENEMYALIVE,ARMEDWITHBOMB,ALIVE,
    0: scout               ARMEDWITHGUN,ENEMYVISIBLE,nearenemy,weaponloaded,enemylinedup,ENEMYALIVE,ARMEDWITHBOMB,ALIVE,
    1: approach            ARMEDWITHGUN,ENEMYVISIBLE,NEARENEMY,weaponloaded,enemylinedup,ENEMYALIVE,ARMEDWITHBOMB,ALIVE,
    2: detonatebomb        ARMEDWITHGUN,ENEMYVISIBLE,NEARENEMY,weaponloaded,enemylinedup,enemyalive,ARMEDWITHBOMB,alive,

Note: this notation uses lowercase if the condition is false, and uppercase if the condition is true.
The first line shows the current world state, and the last line the desired world state, with all intermediate states in between. The plan to execute is: (scout, approach, detonatebomb).

If we follow this plan, we have the unfortunate side effect that not only our enemy dies, but we die as well. This is easily solved by making detonatingbomb a higher cost action. But a more elegant approach would be to change our desired world state, and tell the planner that not only do we want our enemy dead, we want to ourselves alive. The planner will now create:

                           ARMEDWITHGUN,enemyvisible,nearenemy,weaponloaded,enemylinedup,ENEMYALIVE,ARMEDWITHBOMB,ALIVE,
    0: scout               ARMEDWITHGUN,ENEMYVISIBLE,nearenemy,weaponloaded,enemylinedup,ENEMYALIVE,ARMEDWITHBOMB,ALIVE,
    1: load                ARMEDWITHGUN,ENEMYVISIBLE,nearenemy,WEAPONLOADED,enemylinedup,ENEMYALIVE,ARMEDWITHBOMB,ALIVE,
    2: aim                 ARMEDWITHGUN,ENEMYVISIBLE,nearenemy,WEAPONLOADED,ENEMYLINEDUP,ENEMYALIVE,ARMEDWITHBOMB,ALIVE,
    3: shoot               ARMEDWITHGUN,ENEMYVISIBLE,nearenemy,WEAPONLOADED,ENEMYLINEDUP,enemyalive,ARMEDWITHBOMB,ALIVE,


## Example Code

The entire scenario described above is implemented succinctly in these few lines of code:


#include "goappp.hpp"


int __cdecl main ( ) {

    planner<std::uint16_t> ap;

    ap.action_ante ( "scout", "armedwithgun", true );
    ap.action_post ( "scout", "enemyvisible", true );

    ap.action_ante ( "approach", "enemyvisible", true );
    ap.action_post ( "approach", "nearenemy", true );

    ap.action_ante ( "aim", "enemyvisible", true );
    ap.action_ante ( "aim", "weaponloaded", true );
    ap.action_post ( "aim", "enemylinedup", true );

    ap.action_ante ( "shoot", "enemylinedup", true );
    ap.action_post ( "shoot", "enemyalive", false );

    ap.action_ante ( "load", "enemyvisible", true );
    ap.action_ante ( "load", "armedwithgun", true );
    ap.action_post ( "load", "weaponloaded", true );

    ap.action_ante ( "detonatebomb", "armedwithbomb", true );
    ap.action_ante ( "detonatebomb", "nearenemy", true );
    ap.action_post ( "detonatebomb", "alive", false );
    ap.action_post ( "detonatebomb", "enemyalive", false );

    ap.action_ante ( "flee", "enemyvisible", true );
    ap.action_post ( "flee", "nearenemy", false );

    ap.action_cost ( "detonatebomb", cost_type { 5 } );

    ap.current_world ( "enemyvisible", false );
    ap.current_world ( "armedwithgun", true );
    ap.current_world ( "weaponloaded", false );
    ap.current_world ( "enemylinedup", false );
    ap.current_world ( "enemyalive", true );
    ap.current_world ( "armedwithbomb", true );
    ap.current_world ( "nearenemy", false );
    ap.current_world ( "alive", true );

    ap.target_attributes ( "enemyalive", false );
    ap.target_attributes ( "alive", true );

    ap.print_actions ( );
    ap.print_current_world ( ); std::cout << nl;
    ap.print_target_attributes ( ); std::cout << nl;

    ap.plan ( );

    return 0;
}

## Bugs

Always.
