/* functrace.cpp */
#include "pin.H"
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h> // For getpid()

// Global output file stream.
// We use a KNOB to allow the user to specify the output file name on the command line.
KNOB<std::string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "functrace.out", "specify output file name");

// File stream object
static std::ofstream TraceFile;

// This function is called before every function in the instrumented application.
// It logs the process ID, image name, and function name.
VOID log_function_call(const char *img_name, const char *func_name)
{
    // Get the process ID
    PIN_LockClient();
    pid_t pid = PIN_GetPid();
    PIN_UnlockClient();

    // Write to the trace file
    TraceFile << "[PID: " << pid << "] Image: " << img_name << " -> Function: " << func_name << std::endl;
}

// Pin calls this function for every image loaded into the process's address space.
// An image is either an executable or a shared library.
VOID ImageLoad(IMG img, VOID *v)
{
    // We iterate through all the routines (functions) in the image.
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            // For each routine, we insert a call to our analysis function `log_function_call`.
            // RTN_InsertCall places a call at the beginning of the routine.
            // IARG_ADDRINT: Passes the address of an argument.
            // IARG_PTR: Passes a pointer-sized value. We use it for the image and routine names.
            // IARG_END: Marks the end of arguments.
            RTN_Open(rtn);

            RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)log_function_call,
                           IARG_PTR, IMG_Name(img).c_str(),
                           IARG_PTR, RTN_Name(rtn).c_str(),
                           IARG_END);

            RTN_Close(rtn);
        }
    }
}

// Pin calls this function when the application is about to fork a new process.
// Returning TRUE tells Pin to follow and instrument the child process.
BOOL FollowChild(CHILD_PROCESS childProcess, VOID *v)
{
    TraceFile << "[PID: " << PIN_GetPid() << "] Forking a new process..." << std::endl;
    TraceFile.flush();
    return TRUE; // Follow the child
}

// This function is called when the application exits.
// It's a good place to do cleanup, like closing the log file.
VOID Fini(INT32 code, VOID *v)
{
    TraceFile << "[PID: " << PIN_GetPid() << "] Application finished with code " << code << std::endl;
    TraceFile.close();
}

// Pintool entry point
int main(int argc, char *argv[])
{
    // Initialize PIN symbols. This is required for routine-level instrumentation.
    PIN_InitSymbols();

    // Initialize PIN. This must be the first function called.
    if (PIN_Init(argc, argv))
    {
        std::cerr << "PIN_Init failed" << std::endl;
        return 1;
    }

    // Open the output file.
    TraceFile.open(KnobOutputFile.Value().c_str());
    if (!TraceFile.is_open())
    {
        std::cerr << "Could not open output file: " << KnobOutputFile.Value() << std::endl;
        return 1;
    }

    // Register the function to be called for every loaded image.
    IMG_AddInstrumentFunction(ImageLoad, 0);
    
    // Register the function to handle child processes. This is key for tracing forks.
    PIN_AddFollowChildProcessFunction(FollowChild, 0);

    // Register the Fini function to be called when the application exits.
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
