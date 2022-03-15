// C++ version of timer
// Originally written in Python 3

// By Ernold "C" McPuvlist, Jan 2022

/* Conventions used for comments: */
// Double slash are to comment the code
/* Slash/asterisk are debug stubs */

#include <fstream>
#include <iostream>
#include <ctime>
#include <string>
#include <iomanip>
#include <vector>

// #define DEBUG // Turn on to include debug code

using std::cout;
using std::cerr;
using std::cin;
using std::getline;
using std::endl;
using std::string;
using std::left;
using std::right;
using std::setw;
using std::setfill;
using std::ios;
using std::fstream;
using std::ostream;
using std::ofstream;
using std::streampos;
using std::exception;
using std::bad_alloc;
using std::vector;
using std::iter_swap;

// Static constants
static const int EVENT_NAME_LENGTH = 26;
static const size_t TIMESIZE = sizeof(time_t);
static const size_t BOOLSIZE = sizeof(bool);
static const time_t NULLTIME = (time_t)NULL;

// Re-usable messages
static const char OUT_OF_RANGE[] = "Out of range";
static const char NOT_VALID_NO[] = "Not a valid number";
static const char INVALID_CHOICE[] = "Not a valid choice";
static const char INVALID_DATE[] = "Not a valid date/time";
static const char OUT_OF_MEMORY[] = "Error allocating memory for new event";

// ANSI colour sequences
static const char ANSI_ESC[2] = {0x1b, '['};
// static const char ANSI_NOBOLD[2] = {'0', ';'};
static const char ANSI_BOLD[2] = {'1', ';'};
static const char ANSI_WHITE[3] = {'3', '7', 'm'};
static const char ANSI_GREEN[3] = {'3', '2', 'm'};
static const char ANSI_YELLOW[3] = {'3', '3', 'm'};

// Class definitions

class TimeDelta {
    // Class to hold the component parts of a timedelta
public:
    int weeks, days, hours, mins, secs;

    void make(const double tdiff) {
        // Break a timedelta down into its elements
        const int SECS_IN_MIN  =     60;
        const int SECS_IN_HOUR =   3600;
        const int SECS_IN_DAY  =  86400;
        const int SECS_IN_WEEK = 604800;

        int remaining = (int)tdiff;

        weeks = remaining / SECS_IN_WEEK;
        remaining = remaining % SECS_IN_WEEK;

        days = remaining / SECS_IN_DAY;
        remaining = remaining % SECS_IN_DAY;

        hours = remaining / SECS_IN_HOUR;
        remaining = remaining % SECS_IN_HOUR;

        mins = remaining / SECS_IN_MIN;
        secs = remaining % SECS_IN_MIN;
    }
};

// TimedEvent class contains a single timed event
class TimedEvent {

public:
    // Class members:
    string name;        // Event name
    time_t start_time;  // Start time
    time_t end_time;    // End time
    bool all_day;       // true if all day event (i.e. date only, time zeroed out)

    // Size of a TimedEvent for serialisation purposes
    static const int TIMEDEVENT_SIZE = EVENT_NAME_LENGTH + 2 * TIMESIZE + BOOLSIZE;

    // Construct as empty
    TimedEvent() {
        clear();
    }

    void clear() {
        // Clear all contents
        name = "";
        name.resize(EVENT_NAME_LENGTH);
        start_time = end_time = NULLTIME;
        all_day = false;
    }

    void get_inputs() {
    // Edit a single record. Used in Add and Edit menu options
        string input_buffer;
        struct tm tmp_tm;
        bool keep_trying = true;
        const char dateformat[] = "%Y-%m-%d";
        const char datetimeformat[] = "%Y-%m-%d %H:%M";

        // 1. Get name
        cout << "Enter event name (maximum " << EVENT_NAME_LENGTH << " characters):" << endl;
        if (name[0] != '\0') // if already has a name then show it
            cout << name << endl;
        getline(cin, input_buffer);

        if (input_buffer.length()) { // Keep previous name if nothing entered
            name = input_buffer;

            if (name.length() > EVENT_NAME_LENGTH) {
                name.resize(EVENT_NAME_LENGTH);
                cout << "Name is too long and has been truncated to:" << endl;
                cout << name << endl;
            }
            else
                // has to be normalised to EVENT_NAME_LENGTH anyway
                name.resize(EVENT_NAME_LENGTH);
        }

        // 2. Get all_day
        keep_trying = true;
        while(keep_trying) {
            cout << "All day? ";
            getline(cin, input_buffer);
            if (toupper(input_buffer[0]) == 'Y') {
                all_day = true;
                keep_trying = false;
            }
            else if (toupper(input_buffer[0]) == 'N') {
                all_day = false;
                keep_trying = false;
                }
            else
                cout << "That's wrong" << endl;
        }

        // 3. Get start time
        keep_trying = true;
        while(keep_trying) {
            if(all_day) {
                cout << "Enter event start date in ISO format (yyyy-mm-dd):" << endl;
                getline(cin, input_buffer);
                if(strptime(input_buffer.c_str(), dateformat, &tmp_tm) == NULL)
                    cout << INVALID_DATE << endl;
                else {
                    tmp_tm.tm_hour = 0;
                    tmp_tm.tm_min = 0;
                    tmp_tm.tm_sec = 0;
                    tmp_tm.tm_isdst = -1;
                    start_time = mktime(&tmp_tm);
                    keep_trying = false;
                }
            }
            else { // not all day
                cout << "Enter event start date/time in ISO format (yyyy-mm-dd hh:mm):" << endl;
                getline(cin, input_buffer);
                if(strptime(input_buffer.c_str(), datetimeformat, &tmp_tm) == NULL)
                    cout << INVALID_DATE << endl;
                else {
                    tmp_tm.tm_sec = 0;
                    tmp_tm.tm_isdst = -1;
                    start_time = mktime(&tmp_tm);
                    keep_trying = false;
                }
            }
        }

        // 4. Get end time
        keep_trying = true;
        while(keep_trying) {
            if(all_day) {
                cout << "Enter event end date in ISO format (yyyy-mm-dd):" << endl;
                getline(cin, input_buffer);
                if (input_buffer == "") { // No end time
                    end_time = NULLTIME;
                    keep_trying = false;
                }
                else if(strptime(input_buffer.c_str(), dateformat, &tmp_tm) == NULL)
                    cout << INVALID_DATE << endl;
                else {
                    tmp_tm.tm_hour = 0;
                    tmp_tm.tm_min = 0;
                    tmp_tm.tm_sec = 0;
                    tmp_tm.tm_isdst = -1;
                    end_time = mktime(&tmp_tm);
                    keep_trying = false;
                }
            }
            else { // not all day
                cout << "Enter event end date/time in ISO format (yyyy-mm-dd hh:mm):" << endl;
                getline(cin, input_buffer);
                if (input_buffer == "") { // No end time
                    end_time = NULLTIME;
                    keep_trying = false;
                }
                else if(strptime(input_buffer.c_str(), datetimeformat, &tmp_tm) == NULL)
                    cout << INVALID_DATE << endl;
                else {
                    tmp_tm.tm_sec = 0;
                    tmp_tm.tm_isdst = -1;
                    end_time = mktime(&tmp_tm);
                    keep_trying = false;
                }
            }
        }
    }
};

// Overload the << operator for pushing a TimedEvent object
// to output stream (i.e. display a TimedEvent record)
ostream& operator<<(ostream& stream, const TimedEvent& te)
{
    const time_t cur_time = time(NULL); // current time
    bool is_future;
    double time_interval;
    struct tm *temp_tm;
    char time_display_buf[EVENT_NAME_LENGTH]; // String buffer for time display
    TimeDelta td;

    static string date_format = "%d/%m/%Y";
    static string time_format = "%H:%M";
    static int date_pad_width = 10;
    static int time_pad_width = 5;

    // Work out the context of the time interval.
    // If there is no end time and event is in the past ->
    //     Time interval will be the current time minus the start time
    // If there is no end time and event is in the future ->
    //     Time interval will be the future start time minus the current time
    // If there is an end time ->
    //     Time interval will be the end time minus the start time

    if (te.end_time == (time_t)NULL) { // If there is no end time
        time_interval = difftime(cur_time, te.start_time);
        if (time_interval > 0)  // Event is in the past
            is_future = false;
        else {  // Event is in the future, flip interval from negative to positive
            time_interval = -time_interval;
            is_future = true;
        }
    }
    else {
        time_interval = difftime(te.end_time, te.start_time);
        is_future = false;
    }

    // We should now have:
    // 1. time_interval, guaranteed to be a positive value
    // 2. is_future flag correctly set

    // Start outputting
    // Event name and start date
    temp_tm = localtime(&te.start_time);
    strftime(time_display_buf, EVENT_NAME_LENGTH, date_format.c_str(), temp_tm);

    if (is_future)
        stream << ANSI_ESC << ANSI_GREEN;
    else
        stream << ANSI_ESC << ANSI_YELLOW;

    // Name is forced to a C string here using c_str(). For unknown reasons
    // using the C++ string direct causes setw() to be ignored for strings
    // that have been edited using the get_inputs method.
    stream << left << setfill(' ') << setw(EVENT_NAME_LENGTH) << te.name.c_str() << " ";
    stream << time_display_buf << " ";

    // Only display time if not an all-day event, otherwise pad with blank space
    if (te.all_day)
        stream << setw(time_pad_width + 1) << "";
    else {
        strftime(time_display_buf, EVENT_NAME_LENGTH, time_format.c_str(), temp_tm);
        stream << time_display_buf << " ";
    }

    // Display end date if it is not null, pad otherwise
    if(te.end_time == (time_t)NULL)
        stream << setw(date_pad_width + time_pad_width + 1) << "";
    else {
        temp_tm = localtime(&te.end_time);
        strftime(time_display_buf, EVENT_NAME_LENGTH, date_format.c_str(), temp_tm);
        stream << time_display_buf << " ";

        // Only display time if not an all-day event, otherwise pad with blank space
        if (te.all_day)
            stream << setw(time_pad_width) << "";
        else {
            strftime(time_display_buf, EVENT_NAME_LENGTH, time_format.c_str(), temp_tm);
            stream << time_display_buf;
        }
    }

    // Display the time interval
    td.make(time_interval);
    stream << " " << setw(4) << right << td.weeks << " " <<
        setw(2) << td.days;
    if (!te.all_day)
        stream << " " << setw(2) << td.hours << ":" << setw(2) << setfill('0') << td.mins;

    // Reset fill to ' '
    stream << setfill(' ') << "";

    return stream;
}

// TimedEventArray class, based on a vector (dynamic array)
class TimedEventArray : public vector<TimedEvent> {
    private:

    const char DISK_FILE[10] = "timer.dat";
    TimedEvent tmp_event; // temporary holding event object for marshalling prior to
                             // pushing to the array

public:

    ~TimedEventArray() {
    // Erase all members
        clear();
    }

    void new_event() {
        tmp_event.clear();
        tmp_event.get_inputs();
        try {
            push_back(tmp_event);
        }
        catch (bad_alloc&) {
            cerr << OUT_OF_MEMORY<< endl;
        }
    }

    // Comparison functions used by the sort_events method
    static int compare_by_name(TimedEvent &te1, TimedEvent &te2) {
        return te1.name.compare(te2.name);
    }

    static int compare_by_date(TimedEvent &te1, TimedEvent &te2) {
        return difftime(te1.start_time, te2.start_time);
    }

    void sort_events(int(*compare_func)(TimedEvent&, TimedEvent&)) {
    // Sort the event array.
    // compare_func = function to use for field comparison

    bool done;

    for(int i = 0; i < (int)size(); i++) {
        done = true;
            for(iterator itj = begin(); itj < end()-i-1; itj++) {
                if (compare_func(*itj, *(itj+1)) > 0) {
                    // swap
                    iter_swap(itj, itj+1);
                    done = false;
                }
            }
            if (done)
                break;
        }
    }

    void disk_save() {
        // Save (serialise) the event array to file stream
        ofstream f;

        f.open (DISK_FILE, ios::out | ios::binary);
        for(iterator it = begin(); it != end(); it++) {
            f.write (it->name.c_str(), EVENT_NAME_LENGTH);
            f.write ((char *)(&it->start_time), TIMESIZE);
            f.write ((char *)(&it->end_time), TIMESIZE);
            f.write ((char *)(&it->all_day), BOOLSIZE);
        }

        if (f.fail())
            cerr << "Cannot save data to disk" << endl;

        f.close();
    }

    void disk_load() {
        fstream f;
        char cbuffer [EVENT_NAME_LENGTH + 1]; /* Add a byte for terminating null if needed */
        const char file_err[26] = "Failed to open data file";
        streampos file_length;

        f.open (DISK_FILE, ios::in | ios::binary);
            if (f.fail()) {
                cerr << file_err << endl;
                return;
            }

        // Calculate the number of event records in the file
        f.seekg(0, ios::end);
        file_length = f.tellg();
        f.seekg(0, ios::beg);
        // Reserve space in the event array for incoming records
        reserve(file_length / TimedEvent::TIMEDEVENT_SIZE);

        f.read (cbuffer, EVENT_NAME_LENGTH);
        // Add a terminating null character in case string has max length
        cbuffer[EVENT_NAME_LENGTH] = '\0';

        while (!f.eof()) {
            tmp_event.name = cbuffer;
            tmp_event.name.resize(EVENT_NAME_LENGTH);
            f.read((char *)&tmp_event.start_time, TIMESIZE);
            f.read((char *)&tmp_event.end_time, TIMESIZE);
            f.read((char *)&tmp_event.all_day, BOOLSIZE);

            // Add the loaded event to the array
            try {
                push_back(tmp_event);
            }
            catch (bad_alloc&) {
                cerr << OUT_OF_MEMORY << endl;
                break;
            }

            // Next record, will raise eof when finished
            f.read (cbuffer, EVENT_NAME_LENGTH);
        }
        f.close();
    }

    void printout() {
        // Show all the events as a numbered list
        const time_t cur_time = time(NULL); // current time
        char timedisp[40]; // Output buffer for strftime
        int line_no = 1;

        // All header spacing takes EVENT_NAME_LENGTH into account, so 
        // that it can stretch accordingly

        cout << endl << setfill(' ') << ANSI_ESC << ANSI_BOLD << ANSI_WHITE << setw(5) << "" << 
            setw(EVENT_NAME_LENGTH+1) << left << "Name" << setw(17) << "Start" << 
            setw(16) << "End" << ANSI_ESC << ANSI_YELLOW << " Elapsed" << 
            ANSI_ESC << ANSI_WHITE << "/" << ANSI_ESC << ANSI_GREEN << "To go" << endl;
        cout << ANSI_ESC << ANSI_WHITE << setw(EVENT_NAME_LENGTH + 42) << "" << "W  D   H:M" << endl;
        cout << setfill('=') << setw(EVENT_NAME_LENGTH + 53) << "" << endl;

        // Use default space fill from now on
        cout << setfill(' ') << "";

        for (iterator it = begin(); it != end(); it++)
            cout << ANSI_ESC << ANSI_BOLD << ANSI_WHITE << right << setw(3) << line_no++ << ". " << *it << endl;

        strftime(timedisp, 40, "%a %d %b %Y %H:%M:%S %Z", localtime(&cur_time));
        cout << ANSI_ESC << ANSI_WHITE << endl << timedisp << endl;
    }
};

// Data container stored on the heap
static TimedEventArray event_array;

// Main
int main(int argc, char*argv[]) {

    string choice;
    int event_selected;
    char choice_code;
    bool running = true;
    bool is_dirty = false; // whether edits have happened or not

    // Welcome message
    cout << ANSI_ESC << ANSI_BOLD << ANSI_WHITE << "Event Timer 1.1" << endl;
    cout << "Ernold C Puvlist, 2022" << endl;

    // Load data from disk
    event_array.disk_load();

    // User menu loop

    while (running) {
        event_array.printout();

        cout << "(A) Add, (En) Edit, (Dn) Delete, (S) Sort, (X) Exit: ";
        getline(cin, choice);
        choice_code = toupper(choice[0]);

        switch (choice_code) {
            case 'A':
                event_array.new_event();
                is_dirty = true;
                break;
            case 'E':
                // Edit selected record.
                // Extract numeric part of user input
                try {
                    event_selected = stoi(choice.substr(1));
                    if (event_selected < 0 || event_selected > (int)event_array.size()) {
                        cerr << OUT_OF_RANGE << endl;
                        break;
                    }
                    else {
                        event_array.at(event_selected - 1).get_inputs();
                        is_dirty = true;
                    }
                }
                catch (exception &e) {
                    cerr << NOT_VALID_NO << endl;
                }
                break;
            case 'D':
                // Delete selected record
                // Extract numeric part of user input
                try {
                    event_selected = stoi(choice.substr(1));
                    if (event_selected < 0 || event_selected > (int)event_array.size()) {
                        cerr << OUT_OF_RANGE << endl;
                        break;
                    }
                    cout << "Delete event no. " << event_selected << " (" << event_array.at(event_selected - 1).name << ")? ";
                    getline(cin, choice);
                    if (toupper(choice[0]) == 'Y') {
                        event_array.erase(event_array.begin() + event_selected - 1);
                        is_dirty = true;
                    }
                    else
                        cout << "Not deleted" << endl;
                }
                catch (exception &e) {
                    cerr << NOT_VALID_NO<< endl;
                }
                break;
            case 'S': // Sort the events
                cout << "Sort on Name (N) or Start date (D)? ";
                getline(cin, choice);
                switch (toupper(choice[0])) {
                    case 'N':
                        event_array.sort_events(&TimedEventArray::compare_by_name);
                        break;
                    case 'D':
                        event_array.sort_events(&TimedEventArray::compare_by_date);
                        break;
                    default:
                        cout << INVALID_CHOICE << endl;
                }
                break;
            case 'X':
                if (is_dirty) {
                    cout << "Data has been changed. Save the data? ";
                    getline(cin, choice);
                    if (toupper(choice[0]) == 'Y')
                        event_array.disk_save();
                }

                cout << endl << "Thank you for using Event Timer." << endl;
                cout << "Now fuck off" << endl;
                running = false;
                break;
            default:
                cout << endl << INVALID_CHOICE << endl;
        }
    }
}
