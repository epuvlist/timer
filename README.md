# timer
Simple utility to show the time since or until events

The output shows:
- Event name (max 26 characters)
- Start date (optionally including time)
- End date (optionally including time)
- Calculated time interval between start and end times. If end time is not
  given, then the interval between then and the current time is calculated.
  
## Menu options:
  
- A - Add a new event  
- Enn - Edit event no. nn
- Dnn - Delete event no. nn
- X - Exit, giving you the chance to save the data if it has been changed
  
## Coding notes

Written in C++ on Raspberry Pi OS 11 (Bullseye) and compiled using g++ 10.2.1.
  
Events are stored using a C++ vector object. The maximum number of
events is limited only by OS-allocated memory.
  
There is no option to scroll or page the list of events.

## Data file timer.dat
The data is stored in a file named timer.dat, which is assumed to be in the
current working directory. This is a binary file and cannot be edited other than
by the timer program. If timer.dat does not exist then it can be created from
scratch by adding new events using the timer program.

Note that because Unix timestamps can be 32-bit or 64-bit depending on the
architecture of the machine they are hosted on, the timer.dat file will
contain 32-bit timestamps if timer is compiled on a 32-bit system, or 64-bit
timestamps under a 64-bit compilation. The two versions of timer.dat are not
compatible with each other.
