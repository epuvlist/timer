# timer
Simple utility to show the time since or until events

The output shows:
- Event name (max 26 characters)
- Start date (optionally including time)
- End date (optionally including time)
- Calculated time interval between start and end times. If end time is not
  given, then the interval is calculated since or until the current time.
  
  Menu options:
  
  -A - Add a new event
  
  Enn - Edit event no. nn
  
  Dnn - Delete event no. nn
  
  X - Exit, optionally giving you the chance to save the data to disk
  
  Coding notes
  ============
  Written in C++, compile using g++ or similar.
  
  Events are stored using a C++ vector object. The maximum number of
  events is limited only by machine memory.
  
  There is no option to scroll or page the list of events.
