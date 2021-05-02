#include "DirectoryMonitor.h"

std::vector<fileChangeData> fileChangeBuf;

void WatchDirectory(LPTSTR lpDir, std::mutex& dirLock)
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
  
    

    auto fileHashes = CreateFileHashes(lpDir);


    for (auto iter = fileHashes.begin(); iter != fileHashes.end(); ++iter) {
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
        if (!FindNextChangeNotification(dirChangeHandle)) {
            printf("\n File Next File change Failed.");
        }

        dirLock.lock();
        std::cout << "locking dirLock" << std::endl;
        auto dirChanges = getDirectoryChanges(lpDir, fileHashes);

        fileChangeBuf.reserve(fileChangeBuf.size() + dirChanges.size() + 10);
        fileChangeBuf.insert(fileChangeBuf.end(), dirChanges.begin(), dirChanges.end());

        std::cout << "unlocking dirLock" << std::endl;
        dirLock.unlock();
    }
}
// Creates a map of all filePaths to their hash.
std::map<std::wstring, uint64_t> CreateFileHashes(const std::wstring dirPath) {
	std::map<std::wstring, uint64_t> pathToHash;

	std::vector<std::wstring> filepaths = getFilepaths(dirPath);

	//Inserts all the hashes using getFileHash
	for (std::wstring path : filepaths) {
		pathToHash.insert(pathToHash.end(), { getRelativeToDocuments(path), getFileHash(path) });
	}
	return pathToHash;
}

std::vector<fileChangeData> getDirectoryChanges(LPTSTR lpDir, std::map<std::wstring, uint64_t>& prevDir)
{
	//This is probably innefficient most likely I should get filepaths only and hash only as needed.
	auto newDir = CreateFileHashes(lpDir);
	//Storing the file changes
	std::vector<fileChangeData> fileChanges;
	//Every file in the directory we want to check if is in the old directory, if it we check if the hashes are the same, if they aren't it the file change was an edit.  If the file is in the new directory but not in the old directory the file change is an Addition.
	for (auto outerIter = newDir.begin(); outerIter != newDir.end(); ++outerIter) {
		auto filePath = outerIter->first;

		//If prevDir.count == 1 then the file is in both the prevDir and the newDir
		if (prevDir.count(filePath)) {
			auto filesDiffer = prevDir[filePath] != newDir[filePath];

			//IF the files differ the change was an edit.
			if (filesDiffer) {
				fileChanges.insert(fileChanges.end(), { filePath,newDir[filePath], fileChangeType::Edit });
			}
			else {
				continue;
			}
		}
		//If there was no matching file in the new directory the change was an additon.
		else {
			fileChanges.insert(fileChanges.end(), { filePath, newDir[filePath], fileChangeType::Addition });
		}
	}
	//we do one last check for deletions by checking the files that show up in the old directory but not in the new directory.
	for (auto outerIter = prevDir.begin(); outerIter != prevDir.end(); ++outerIter) {
		auto filePath = outerIter->first;
		if (!newDir.count(filePath)) {
			fileChanges.insert(fileChanges.end(), { filePath, NULL, fileChangeType::Deletion });
		}
	}
	prevDir = newDir;
	return fileChanges;
}

std::vector<fileChangeData> getDirectoryChanges(std::map<std::wstring, uint64_t>& hostDir, std::map<std::wstring, uint64_t>& foreignDir)
{
	//This is probably innefficient most likely I should get filepaths only and hash only as needed.
	//Storing the file changes
	std::vector<fileChangeData> fileChanges;
	//Every file in the directory we want to check if is in the old directory, if it we check if the hashes are the same, if they aren't it the file change was an edit.  If the file is in the new directory but not in the old directory the file change is an Addition.
	for (auto outerIter = foreignDir.begin(); outerIter != foreignDir.end(); ++outerIter) {
		auto filePath = outerIter->first;

		//If prevDir.count == 1 then the file is in both the prevDir and the newDir
		if (hostDir.count(filePath)) {
			auto filesDiffer = hostDir[filePath] != foreignDir[filePath];

			//IF the files differ the change was an edit.
			if (filesDiffer) {
				fileChanges.insert(fileChanges.end(), { filePath,foreignDir[filePath], fileChangeType::Edit });
			}
			else {
				continue;
			}
		}
		//If there was no matching file in the new directory the change was an additon.
		else {
			fileChanges.insert(fileChanges.end(), { filePath, foreignDir[filePath], fileChangeType::Addition });
		}
	}
	//we do one last check for deletions by checking the files that show up in the old directory but not in the new directory.
	

	return fileChanges;
}

void printChanges(std::vector<fileChangeData> fileChanges) {
	for (auto fileChange : fileChanges) {
		std::wcout << fileChange.filePath;
		switch (fileChange.change) {
		case fileChangeType::Addition:
			std::wcout << L" File Added" << std::endl;
			break;
		case fileChangeType::Deletion:
			std::wcout << L" File Deleted" << std::endl;
			break;
		case fileChangeType::Edit:
			std::wcout << L" File Edit" << std::endl;
			break;
		}
	}
}