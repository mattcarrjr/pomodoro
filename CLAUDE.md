I want to create a pomodoro timed apllication that will work in Linux (X11 and
Wayland) and MacOS.  This is primarily and application that I will use in house
to help manage time through the day based on the pomodoro method.

## Basic Functionality
- the app will run for a user specified amount of time (default 25 minutes)
before making a time sound.
- once the timer sounds the user will be asked to turn it off and start a break
  of a user specified amount of time (default five minutes).
- This cycle will run 4 times before taking a much longer break (default 60
minutes, also user specified)
- The app will have a very retro-computing interface, reminiscent of UNIX
applications of the 80's and 90's.
- the sounds will be made by me and eventually placed in a folder in thei
direcotry, but in the mean time we can use placeholder system sounds.
- I would like this program to be written in a very fast low level language like
  C

Lets start from there , and we will continue filling out this document
throughout the process.
