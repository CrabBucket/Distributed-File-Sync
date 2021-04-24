#include "DirectoryMonitor.h"


void WatchDirectory(LPTSTR lpDir)
{
    
    //Status on the directory watch waiting, ie if a directory has been changed the wait status will free
    DWORD dirWaitStatus;
    //The handles to the windows Directory Change Notification
    HANDLE dirChangeHandle;
    //Name of the drive the directory is located on.
    TCHAR dirDrive[4];


    //Micrsoft stdlib function that the lpDir (pointer to the string with the directory path) 
    //https://titanwolf.org/Network/Articles/Article?AID=7d033004-eb4a-4d38-b335-0ed45c948f2e#gsc.tab=0 explains the function better than what I could find from microsoft.
    _tsplitpath_s(lpDir, dirDrive, 4, NULL, 0, NULL, 0, NULL, 0);

    //No idea why this is here or what it does not documented at all on Microsofts website. Could be adding a null terminator? No idea.
    dirDrive[2] = (TCHAR)'\\';
    dirDrive[3] = (TCHAR)'\0';

    // Watch the directory for file creation and deletion editing and directory changes. 
    //Microsoft has 6 different ways to look for file or directory updates, we watch them all besides security changes.
    dirChangeHandle = FindFirstChangeNotification(
        lpDir,                         // directory to watch 
        TRUE,                         // watch subtree 
        FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_LAST_WRITE); // watch file name changes 


    // Make a final validation check on our handles.
  
    

    auto filehashes = CreateFileHashes(lpDir);


    for (auto iter = filehashes.begin(); iter != filehashes.end(); ++iter) {
        std::wcout << iter->first << std::endl;
    }
    // Change notification is set. Now wait on both notification 
    // handles and refresh accordingly. 

    while (TRUE)
    {
        // Wait for notification.
        if ((dirChangeHandle == NULL))
        {
            printf("\n ERROR: Unexpected NULL from FindFirstChangeNotification.\n");
            ExitProcess(GetLastError());
        }
        printf("\nWaiting for notification...\n");
        //function sets the wait status
        dirWaitStatus = WaitForSingleObject(dirChangeHandle, INFINITE);

        if (dirWaitStatus != WAIT_TIMEOUT) {

            HandleDirectoryChange(lpDir);
            if (FindNextChangeNotification(dirChangeHandle) == FALSE)
            {
                printf("\n ERROR: FindNextChangeNotification function failed.\n");
                ExitProcess(GetLastError());
            }
           
        }

    }
}
std::map<std::wstring, uint64_t> CreateFileHashes(const std::wstring dirPath) {
    std::map<std::wstring, uint64_t> pathToHash;
   
    std::vector<std::wstring> filepaths = getFilepaths(dirPath);

    for (std::wstring path : filepaths) {
        pathToHash.insert({ path, getFileHash(path)});
    }
    return pathToHash;

}


void HandleDirectoryChange(LPTSTR lpDir, std::map<std::wstring, uint64_t> prevDir)
{
    std::vector<fileChangeData> fileChanges;
    for (auto outerIter = prevDir.begin(); outerIter != prevDir.end(); ++outerIter) {
        auto filePath = outerIter->first;
        auto fileFound = false;
        for (auto interIter = prevDir.begin(); interIter != prevDir.end(); ++interIter) {

        }
        if (!fileFound) {
            fileChanges.insert(fileChanges.end(), { filePath,getFileHash(filePath), fileChangeType::Deletion });
        }

    }

    _tprintf(TEXT("Directory (%s) changed.\n"), lpDir);
}