/* FuncTracer.cpp */
#include "pin.H"
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h> // For getpid()

// This function is called before every function in the instrumented application.
// It logs the process ID, image name, and function name.
VOID log_function_call(const char *img_name, const char *func_name)
{
    // Build the output string
    std::stringstream ss;
    // Get the process ID
    PIN_LockClient();
    pid_t pid = PIN_GetPid();
    PIN_UnlockClient();
    ss << "[PID:" << pid << "] [Image:" << img_name << "] [Called:" << func_name << "]\n" ;
    LOG(ss.str());
}

// Pin calls this function for every image loaded into the process's address space.
// An image is either an executable or a shared library.
VOID ImageLoad(IMG img, VOID *v)
{
    // We iterate through all the sections of the image.
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        LOG("[Image:" + IMG_Name(img) + "] [Section:" + SEC_Name(sec) + "]\n");
        // We iterate through all the routines (functions) in the image.
        if (SEC_Type(sec)!=SEC_EXEC) continue; // Only instrument executable sections
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            std::stringstream ss;
            RTN_Open(rtn);
            // We log the image name and function name so we can see which function is being instrumented.
            ss << "[Image:" << IMG_Name(img) << "] [Function:" << RTN_Name(rtn) << "]\n" ;
            LOG(ss.str());
            // For each routine, we insert a call to our analysis function `log_function_call`.
            // RTN_InsertCall places a call at the beginning of the routine.
            // IARG_ADDRINT: Passes the address of an argument.
            // IARG_PTR: Passes a pointer-sized value. We use it for the image and routine names.
            // IARG_END: Marks the end of arguments.

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
    //LOG( "New PID: " + decstr(PIN_GetPid()) + "\n");
    //TraceFile << "[PID: " << PIN_GetPid() << "] Forking a new process..." << std::endl;
    //TraceFile.flush();
    return TRUE; // Follow the child
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
    // Register the function to be called for every loaded image.
    IMG_AddInstrumentFunction(ImageLoad, 0);
    
    // TODO: check if childs are automatically followed or not
    PIN_AddFollowChildProcessFunction(FollowChild, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
