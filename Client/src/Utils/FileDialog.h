#pragma once
#include <Windows.h>
#include <string>
#include <shobjidl.h> 

namespace file {
	class FileDialog {
	public:
		FileDialog();
		bool openFile();
		
		std::string getSelectedFile() { return sSelectedFile; };
		std::string getFilePath() { return sFilePath; };
	private:

		std::string sSelectedFile;
		std::string sFilePath;
	};
}