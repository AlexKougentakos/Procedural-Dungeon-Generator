# Procedural-Dungeon-Generation
In this project I am going to create an alogorithm that procedurally generates dungeon rooms in 2D. I will be using the first year Programming-02 framework to do it, as I believe it will be the easiest to get started on and implement as I don't any experience with Unreal Engine outside of blueprints and I want to write the project in C++.


![](https://external-content.duckduckgo.com/iu/?u=https%3A%2F%2Fcdn.tutsplus.com%2Fgamedev%2Fuploads%2F2013%2F07%2Fprocedural-content-room-connection.png&f=1&nofb=1&ipt=e41b428fb89c34b2194b5a4dafbef4fcf3296eeca89879e6bd62495b20190954&ipo=images)

The image above closely resembles what it would look like, but with a few more colours and maybe some sprites if there is time.

Itterations:
1. Basic Camera & Room Spawning working.
  -Camera needs a little work to be smoother
  -Room spawning and separation are working
  -Separation algorithm is to be reworked as it causes some issues, but it's fine for testing.
2. Delauny Triangulation algorithm working
  -Minimum Spanning Tree algorithm working (could use maps instead of member variables)
  -Delauny algorithm could be improved (super triangle could be more optimal)
