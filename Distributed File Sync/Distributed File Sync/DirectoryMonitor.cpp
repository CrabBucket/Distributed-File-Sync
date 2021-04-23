


//Watch directory takes a pointer to a non-constant string indicated the path to the directory that is being monitored
void WatchDirectory(LPTSTR lpDir)
{
    //Status on the directory watch waiting, ie if a directory has been changed the wait status will free
    DWORD dirWaitStatus;
    //The handles to the windows Directory Change Notification
    HANDLE dirChangeHandles[5];
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
    dirChangeHandles[0] = FindFirstChangeNotification(
        lpDir,                         // directory to watch 
        TRUE,                         // watch subtree 
        FILE_NOTIFY_CHANGE_FILE_NAME); // watch file name changes 
    dirChangeHandles[1] = FindFirstChangeNotification(
        dirDrive,                       // directory to watch 
        TRUE,                          // watch the subtree 
        FILE_NOTIFY_CHANGE_DIR_NAME);  // watch dir name changes 
    dirChangeHandles[2] = FindFirstChangeNotification(
        dirDrive,                       // directory to watch 
        TRUE,                          // watch the subtree 
        FILE_NOTIFY_CHANGE_ATTRIBUTES);  // watch file attritbute changes 
    dirChangeHandles[3] = FindFirstChangeNotification(
        dirDrive,                       // directory to watch 
        TRUE,                          // watch the subtree 
        FILE_NOTIFY_CHANGE_SIZE);  // watch file size changes
    dirChangeHandles[4] = FindFirstChangeNotification(
        dirDrive,                       // directory to watch 
        TRUE,                          // watch the subtree 
        FILE_NOTIFY_CHANGE_LAST_WRITE);  // watch last write changes


    // Make a final validation check on our handles.

    if ((dirChangeHandles[0] == NULL) || (dirChangeHandles[1] == NULL) || (dirChangeHandles[2] == NULL) || (dirChangeHandles[3] == NULL) || (dirChangeHandles[4] == NULL))
    {
        printf("\n ERROR: Unexpected NULL from FindFirstChangeNotification.\n");
        ExitProcess(GetLastError());
    }

    // Change notification is set. Now wait on both notification 
    // handles and refresh accordingly. 

    while (TRUE)
    {
        // Wait for notification.

        printf("\nWaiting for notification...\n");
        //function sets the wait status
        dirWaitStatus = WaitForMultipleObjects(5, dirChangeHandles,
            FALSE, INFINITE);

        if (dirWaitStatus != WAIT_TIMEOUT) {

            RefreshDirectory(lpDir);
            if (FindNextChangeNotification(dirChangeHandles[0]) == FALSE)
            {
                printf("\n ERROR: FindNextChangeNotification function failed.\n");
                ExitProcess(GetLastError());
            }
            break;
        }
        
    }
}

void HandleDirectoryChange(LPTSTR lpDir)
{
    //This is where we should figure out what changed and how to proceed as far as syncing goes.

    _tprintf(TEXT("Directory (%s) changed.\n"), lpDir);
}

