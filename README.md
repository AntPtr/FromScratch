Following Casey Muratori footsteps im going to program a game in C using some C++ features without external libraries or dependency, the game currently use win32 api for graphics and sound and software rasterizer.
The building is done with a simple bat file.
<br>
I'm going to list some of them time by time: 
<br>
-Software rasterizer for scaling/rotating and bilinear sample with sRGB, optimized with SIMD SSE2 instructions for better cpu performance
<br>
![Demo](https://i.imgur.com/JsuI1Mm.gif)
<br>
-Collisions detection
<br>
![Demo](https://i.imgur.com/9HlO3g4.gif)
