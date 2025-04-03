/*
	This is a quickly written tool to find if there are DAG files with more than 1 object count
	since it's faster to write and compile that code than ask on discord/forums and wait for answer
	that is, if anyone even knows
	Usage: just drop all the DAG files to temp folder next to compiled exe and run, it should read data
	and write txt file with names of dag files where there's more than one object (I there's even such file
	///edit: apparently there isn't but keeping this shitty code, maybe someone finds it useful for smth)
	Better yet, run it through cmd to be sure it really works
*/

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

void PrintFilename(FILE* file, const char* name)
{
	uint32_t tmp = 0;

	fseek(file, ftell(file)+16, SEEK_SET);
	fread(&tmp, 4, 1, file);

	FILE* tmpF;
	const char* tmpName = "out.txt";

	if (tmp > 1)
	{
		fopen_s(&tmpF, tmpName, "a+");
		std::ofstream out(tmpF);
		out << name << "\n";
	}
}

int main()
{
	std::string pathIn = "temp";
	uint32_t fileCount = 0;
	uint32_t i;
	std::vector<std::string> fileList, filenames;

	for (const auto& entry : std::filesystem::directory_iterator(pathIn))
	{
		fileList.emplace_back(entry.path().string());
		filenames.emplace_back(entry.path().stem().string());
		++fileCount;
	}

	for (i = 0; i < fileCount; i++)
	{
		FILE* file;
		const char* tmp = fileList[i].c_str();

		fopen_s(&file, tmp, "rb");
		PrintFilename(file, tmp);
		_fcloseall();
		
		// This bugger is here so you're sure it actually does smth
		std::cout << "=";
	}

	return 0;
};
