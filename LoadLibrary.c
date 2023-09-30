/*
    x----------------------------------------x
    |  - Author: Th3Spl                      |
    |  - Purpose: LoadLibrary Injection      |
    |  - Lang: C                             |
    x----------------------------------------x
*/

//Imports
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

//This function will help us find the process id
DWORD GetProcessId(const char* process_name)
{
    //Getting a snapshot of the processes
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    //Checking if the snapshot has been created successfully
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return 0;


    //Creating the structure that we need
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(pe32);

    //Checking every process until we find the one we're interested in
    if (Process32First(hSnapshot, &pe32)) {
        do
        {
            //Comparing the current szExeFile name to the process name we're looking for
            if (strcmp(pe32.szExeFile, process_name) == 0)
            {
                CloseHandle(hSnapshot);         //Closing the HANDLE
                return pe32.th32ProcessID;      //Returning the Process id+
            }
        }while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);     //Closing the HANDLE
    return 0;                   //Returning 0 if it fails
}

//Main function
int main()
{
    //Setting the console text color to --> Red
    HANDLE hConsole = GetConsoleWindow();
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED);

    //Setting the console title
    SetConsoleTitle(L"DLL Injector: LoadLibrary | Made by: Th3Spl");

    //Variables
    char dll_path[MAX_PATH];
    char process_name[MAX_PATH];
    DWORD process_id = 0;

    //Credits
    printf_s("\n x----------------------------------------x\n");
    printf_s(" |  - Author: Th3Spl                      |\n");
    printf_s(" |  - Injection Method: LoadLibrary       |\n");
    printf_s(" x----------------------------------------x\n\n");

    //Getting the .dll name or the path
    printf_s(" [+] Enter the .dll path: ");
    scanf_s("%s", &dll_path);

    //Getting the process name for the injection
    printf_s(" [+] Enter the process name: ");
    scanf_s("%s", &process_name);

    //Starting the injection
    printf_s("\n [+] Starting the injection: \n");

    //Getting the process id
    process_id = GetProcessId(process_name);
    if (process_id == 0)
    {
        printf_s(" [-] Unable to find the process!\n");
        system("pause>nul");
        return 1;
    }

    //Printing the process id
    printf_s(" [+] Process id: %d\n", process_id);

    //Opening the target process
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);
    if (hProcess == NULL)
    {
        printf_s(" [-] Couldn't open the process!\n");
        system("pause>nul");
        return 1;
    }

    //Printing the status
    printf_s(" [+] Process opened successfully!\n");

    //We need to allocate the memory where we'll put our .dll
    LPVOID memory = VirtualAllocEx(hProcess, NULL, strlen(dll_path) + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (memory == NULL)
    {
        CloseHandle(hProcess);
        printf_s(" [-] Memory allocation failed!\n");
        system("pause>nul");
        return 1;
    }

    //Printing the status
    printf_s(" [+] Memory allocated successfully!\n");

    //Writing the .dll path into the allocated memory
    BOOL write_memory = WriteProcessMemory(hProcess, memory, dll_path, strlen(dll_path) + 1, NULL);
    if (write_memory == FALSE)
    {
        CloseHandle(hProcess);
        VirtualFreeEx(hProcess, memory, 0, MEM_RELEASE);
        printf_s(" [-] Couldn't write the process memory!\n");
        system("pause>nul");
        return 1;
    }

    //Printing the status
    printf_s(" [+] Memory Written successfully!\n");

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, memory, 0, NULL);
    if (hThread == NULL)
    {
        printf_s(" [-] Couldn't Create The Remote Thread!\n");
        VirtualFreeEx(hProcess, memory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        system("pause>nul");
        return 1;
    }

    //Printing the status
    printf_s(" [+] Remote Thread Created Successfully!\n");

    //Wait for the thread
    WaitForSingleObject(hThread, INFINITE);

    //Cleaning up the memory
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, memory, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    //End
    printf_s(" [+] DLL LOADED SUCCESSFULLY!\n");
    scanf("%s", dll_path);
}