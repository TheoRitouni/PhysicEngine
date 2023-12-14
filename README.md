# Physic Engine, BroadPhase
Mathematic project, C++, SDL, OpenGL
<br><br>

## Summary
___
<br>

This study project is a Physic Engine that can produce Collision.   
It was realized by Théo Ritouni student at Isart Digital.  
This plugin has been developed on Visual Studio in C++. 

<br>

## How to us it 
___
<br>
Go to folder "Bin" and execute .exe

You have differents scene in this project.
<br>

**Scene** 

    - Scene Simple Physic
    - Scene Complex Physic "with 25 object"
    - Scene Small Physic


Press "F1" to reset scene.

Press "F2" to previous scene.

Press "F2" to next scene.

**Debug Mode**

Press "F4" to debug mode.

    The debug mode show you all AABB with differents state display by color.
    He displaying the duration of BroadPhase too. 
    You see the normal and point of collision in polygon.

**Lock FPS**

Press "F5" to lock fps.

Lock fps have three mode (30fps, 60fps and unlimited)

<br>

## Features
___
<br>

- Response to Collision in folder "PhysicEngine.cpp"
- Narrow Phase in folder "Polygon.cpp"
- GJK algorithm 
- Change color of Polygon when a collision happened
- Change color of AABB when collide with other AABB
- Draw AABB with debug mode
- Implementation of AABB class 
- BroadPhase, Sweep And Prune

<br>

## To Do
___
<br>

- Optimization of BroadPhase
- More options and precision in algorithms
- New Broad Phase 
- Select different BroadPhase
- Rework the code 