# GameB

 Watch the series on YouTube [here.](https://www.youtube.com/playlist?list=PLlaINRtydtNWuRfd4Ra3KeD6L9FP_tDE7)
 
![YouTube_Screenshot](YoutubeScreenshot.png "YouTube_Screenshot")

![YouTube_Screenshot2](YoutubeScreenshot2.png "YouTube_Screenshot2")

![YouTube_Screenshot2](YoutubeScreenshot3.png "YouTube_Screenshot3")

 We are going to make a retro-style video game from scratch, using the C programming language.
 
 This source code repository is companion material to go along with my video series on YouTube.

 Updated Wednesdays and Saturdays.

 If you've ever wanted to get into programming, even if you have no coding experience, well... you could start here.

 Watch the series on YouTube [here.](https://www.youtube.com/playlist?list=PLlaINRtydtNWuRfd4Ra3KeD6L9FP_tDE7)
 
 Follow me on Twitter @JosephRyanRies

# Build Instructions
--------------------
This project contains a post-build step. The post-build step runs the batch file CopyAssets.bat.
The CopyAssets.bat batch file contains an environment variable that you need to create on your machine.
The environment variable is called GAMEBDIR and it should point to the fully-qualified path to the project on your machine.
You can use the setx utility to create this environment variable. For example:

`C:\Windows\system32>setx GAMEBDIR C:\Users\ryanr\source\repos\GameB`

` `

`SUCCESS: Specified value was saved.`

You probably have to restart Visual Studio after creating this environment variable for the first time.


 
# License
----------
The source code in this project is licensed under the MIT license.
The media assets such as artwork, custom fonts, music and sound effects are licensed under a separate license.
A copy of that license can be found in the 'Assets' directory.
stb_vorbis by Sean Barrett is public domain and a copy of its license can be found in the stb_vorbis.c file.
miniz by Rich Geldreich <richgel99@gmail.com> is public domain (or possibly MIT licensed) and a copy of its license can be found in the miniz.c file.