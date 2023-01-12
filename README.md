# Procedural Dungeon Generator

In this project, I made a procedural dungeon generator and showcase
how it could be used.

![](https://i.imgur.com/E94ZbZy.gif)



## Features

- Generating a random dungeon at high speeds
- **Parameters** that allow you to tweak the dungeon
- Timer that changes dungeons automatically **for showcase**


## How To use

 To use the software all you need to do is download the project from
 the [releases](https://github.com/AlexKougentakos/Procedural-Dungeon-Generator/releases/tag/1.0).

- Use the arrow keys to move around
- Space to pause the timer
- Use the scroll wheel to zoom In/Out

 
# How It Works

## Room Spawning

![](https://i.imgur.com/WF4KAQU.gif)

There are many different ways to spawn rooms. Here I spawn a lot of rooms in a small space and have them separate from each other.
By doing this I can keep the rooms closely packed, which offers better looking, and more game-ready, dungeons.

After that, we delete rooms until we reach the minimum amount of rooms set by the user.

## Connecting Rooms

In order to connect rooms first we need to connect all of the rooms together with their closest neighbouring rooms.

To achieve this I used the [Bowyer-Watson](https://en.wikipedia.org/wiki/Bowyer%E2%80%93Watson_algorithm) algorithm to create a [Delaunay Triangulation](https://en.wikipedia.org/wiki/Delaunay_triangulation).

![](https://i.imgur.com/tBBGW7D.gif)

Connecting every room to every neighbour, however, would cause some very weird-looking dungeons that have too many loops.
That is why we get the [Minimum Spanning Tree](https://upload.wikimedia.org/wikipedia/commons/thumb/d/d2/Minimum_spanning_tree.svg/450px-Minimum_spanning_tree.svg.png) (MST) that connects every room to a very close neighbour.
I did that using [Kruskal's Algorithm](https://en.wikipedia.org/wiki/Kruskal%27s_algorithm). After that, we loop over all the edges that the MST 
algorithm deleted and give each one about a 20% chance to be introduced back to the graph.
Using this combination of algorithms we guarantee that every room has been reached and some loops occur in the dungeon so as to not be stale.

![](https://upload.wikimedia.org/wikipedia/commons/thumb/d/d2/Minimum_spanning_tree.svg/450px-Minimum_spanning_tree.svg.png)

# Summary

To summarise, we spawn rooms and then we separate them. They create a triangulation between them and find the minimum spanning tree that connects all of the rooms together. Add back a few edges to form loops between the rooms and create the hallways.

This is a fairly simple take on procedurally generated dungeons. This approach doesn't take into account the paths created and the intersections between them. Adding everything to a hidden grid-based system and using the A* algorithm to connect rooms and merge corridors together would be the next step in improving this algorithm.
