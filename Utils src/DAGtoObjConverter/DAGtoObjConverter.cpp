/*
	A crude tool to convert DAG files to Wawefront obj format
	Uses chunks of my old code made for different tool so it may be shitty but cba to refactor it as long as it works
	Usage: drop DAG files you want to convert to `in` folder next to exe and run (optionally via cmd) exe.
	If you run via cmd: DAGtoObjConverter.exe [optional]
	Optional argument can be anything, it signals to the program you want to adjust scale to fit to toee_map_render_template_wip.blend
*/

#include <iostream>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <string>

struct vertexPos {
	double x;
	double y;
	double z;
};

struct triangleVertexIndex {
	uint16_t index1;
	uint16_t index2;
	uint16_t index3;
};

float ReadFloat(FILE* filename)
{
	float tmp = 0;

	fread(&tmp, 4, 1, filename);

	return tmp;
}

uint16_t ReadUInt16(FILE* filename)
{
	uint16_t tmp = 0;

	fread(&tmp, 2, 1, filename);

	return tmp;
}

uint32_t ReadVertexCount(FILE* filename)
{
	uint32_t temp = 0;

	fseek(filename, 0x18, SEEK_SET);
	fread(&temp, 4, 1, filename);

	return temp;
}

uint32_t ReadTriangleCount(FILE* filename)
{
	uint32_t temp = 0;

	fseek(filename, 0x1C, SEEK_SET);
	fread(&temp, 4, 1, filename);

	return temp;
}

uint32_t ReadVertexDataPos(FILE* filename)
{
	uint32_t temp = 0;

	fseek(filename, 0x20, SEEK_SET);
	fread(&temp, 4, 1, filename);

	return temp;
}

uint32_t ReadTriangleDataPos(FILE* filename)
{
	uint32_t temp = 0;

	fseek(filename, 0x24, SEEK_SET);
	fread(&temp, 4, 1, filename);

	return temp;
}

void ReadVertexData(FILE* file, uint32_t count, std::vector<vertexPos>* vertices, bool adjust)
{
	double scaleAdjustment = 1.f;

	// 0.0225 is a value deduced from scaling down imported human male model
	if (adjust)
		scaleAdjustment = .0225;

	if (!count)
		return;

	for (uint32_t i = 0; i < count; i++)
	{
		vertexPos temp = { 0.f };
		temp.x = (double)ReadFloat(file) * scaleAdjustment;
		temp.y = (double)ReadFloat(file) * scaleAdjustment;
		temp.z = (double)ReadFloat(file) * scaleAdjustment;

		vertices->push_back(temp);
	}
}

void ReadTriangleData(FILE* file, uint32_t count, std::vector<triangleVertexIndex>* triangles)
{
	if (!count)
		return;

	for (uint32_t i = 0; i < count; i++)
	{
		triangleVertexIndex temp = { 0 };
		// obj vertex indexes start at 1 while DAG at 0 so need to adjust
		temp.index1 = ReadUInt16(file) + 1;
		temp.index2 = ReadUInt16(file) + 1;
		temp.index3 = ReadUInt16(file) + 1;

		triangles->push_back(temp);
	}
}

void WriteObjFile(FILE* file, std::string filename, std::vector<vertexPos>* vertices, std::vector<triangleVertexIndex>* triangles)
{
	std::cout << filename << ".obj\n";
	std::ofstream out(file);
	// Write header string
	std::cout << "=";
	out << "# Created by DAGtoObjConverter\n";
	std::cout << "=";
	out << "# https://github.com/Alyst3r/ToEE-Modding-Assets\n";
	std::cout << "=";
	// object name
	out << "o " << filename << "\n";
	std::cout << "=";
	// vertex data
	for (vertexPos temp : *vertices)
	{
		out << "v " << std::to_string(temp.x) << " " << std::to_string(temp.y) << " " << std::to_string(temp.z) << "\n";
		std::cout << "=";
	}
	// triangle data
	for (triangleVertexIndex temp : *triangles)
	{
		out << "f " << std::to_string(temp.index1) << " " << std::to_string(temp.index2) << " " << std::to_string(temp.index3) << "\n";
		std::cout << "=";
	}
	std::cout << "\n";
}

int main(int argc, char* argv[])
{
	bool adjustScale = false;
	std::string pathIn = "in";
	std::string pathOut = "out";
	std::vector<std::string> fileList, filenames;
	uint32_t fileCount = 0;
	uint32_t i = 0;
	uint32_t vCount = 0;
	uint32_t tCount = 0;
	uint32_t vDataPos = 0;
	uint32_t tDataPos = 0;

	if (argv[1])
		adjustScale = true;

	for (const auto& entry : std::filesystem::directory_iterator(pathIn))
	{
		fileList.emplace_back(entry.path().string());
		filenames.emplace_back(entry.path().stem().string());
		++fileCount;
	}

	for (i = 0; i < fileCount; ++i)
	{
		std::vector<vertexPos> vertices;
		std::vector<triangleVertexIndex> triangles;

		FILE* file;
		const char* tmp = fileList[i].c_str();
		fopen_s(&file, tmp, "rb");

		FILE* objFile;
		std::string fName = pathOut + "/" + filenames[i] + ".obj";
		const char* tmpName = fName.c_str();
		fopen_s(&objFile, tmpName, "w+");

		vCount = ReadVertexCount(file);
		tCount = ReadTriangleCount(file);
		vDataPos = ReadVertexDataPos(file);
		tDataPos = ReadTriangleDataPos(file);

		fseek(file, vDataPos, SEEK_SET);
		ReadVertexData(file, vCount, &vertices, adjustScale);
		fseek(file, tDataPos, SEEK_SET);
		ReadTriangleData(file, tCount, &triangles);
		WriteObjFile(objFile, filenames[i], &vertices, &triangles);
		_fcloseall();
	}

	std::cout << "Done";
	return 0;
}
