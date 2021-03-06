#include "FileProgression.h"

#include <cctype>
#include <codecvt>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <Windows.h>

#include "IDF.h"
#include "Normalizer.h"
#include "TF.h"
#include "Utility.h"

using namespace std;

const string METADATA_NAME = "metadata"; // metadata folder name
const string SUBFOLDER_NAME = "FolderList.txt"; // text file save name of active folder
const string INDEX_NAME = "Index.txt"; // text file save path to every file in dataset

StringArray str_arr; // StringBuilder for operating with dynamic string array

string* folder_list = nullptr; // Array of folder name
int folder_count = 0;

string** file_list = nullptr; // Array of file name 
int* file_count = nullptr; // Array of number of file

FolderData* folder_data; // Store everything in metadata, for the search function

string prev_dataset; // Store name of previous dataset loaded to this program

void prepareFile(const string& FolderPath) // Make text file of path for further operation
{
	makeFolderWrapper(METADATA_NAME);
	getFolderWrapper(FolderPath, SUBFOLDER_NAME); // get folder name of folder path and save

	ifstream input(SUBFOLDER_NAME, ios::in);

	string line;
	while (getline(input, line))
	{
		string path_to_folder = FolderPath + "\\" + line;
		string path_to_file = string(METADATA_NAME) + "\\" + line + ".txt";
		getFileWrapper(path_to_folder, path_to_file);
		makeFolderWrapper(string(METADATA_NAME) + "\\" + line);
	}

	input.close();
}

// Checking if metadata created
bool isFirstTime() {
	ifstream fin(SUBFOLDER_NAME, ios::in);
	if (!fin.is_open()) {
		fin.close();
		return true;
	}
	fin.close();
	return false;
} 

// create TF list from file of text
TF_list createTF(const string& FilePath) {
	TF_list L{};
	tfListInit(L);
	wifstream fin(FilePath, ios::in);
	fin.imbue(locale(locale::empty(), new codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>)); // setting up for reading UTF16-LE
	wstring s;

	while (fin >> s) {
		string converted_string = VEconvert(s);
		if (converted_string.length() == 0)
			continue;
		addString(str_arr, converted_string);
	}

	sortMultiThread(str_arr); // sorting every read word
	tfListInput(L, str_arr.array, str_arr.size); // then add to tf list
	fin.close();
	str_arr.size = 0; // reset but not free
	return L;
}

// Update folder list file with new folder
void updateFolder(const string& FolderName)
{
	initString(str_arr);
	
	loadTextToArray(str_arr, SUBFOLDER_NAME);
	bool is_exist = false;
	for(int i = 0; i < str_arr.size - 1; i++)
		if (FolderName == str_arr.array[i])
		{
			is_exist = true;
			break;
		}
	if (!is_exist)
	{
		ofstream ofs(SUBFOLDER_NAME, ios::out);

		for (int i = 0; i < str_arr.size - 1; i++)
			ofs << str_arr.array[i] << "\n";
		ofs << FolderName << "\n";
		ofs << str_arr.array[str_arr.size - 1];

		ofs.close();
	}

	deleteArray(str_arr);
}

// create IDF list from file of tf
IDF_list createIDF(const string& FolderPath)
{
	const string directory = FolderPath + ".txt";
	string file_name;
	ifstream path(directory, ios::in);

	str_arr.size = 0;

	IDF_list idfL{};
	idfListInit(idfL);

	int n_files = 0;// number of files
	int n_words = 0; //number of words

	while (getline(path, file_name))
	{
		if (file_name.length() == 0)
			continue;
		n_files++;

		string tf_path = FolderPath + "\\" + file_name + ".tf";

		TF_list tfL{};
		loadTFList(tf_path, tfL);

		for (int i = 0; i < tfL.size; i++)
		{
			string temp_str = tfL.arrNorm[i].word;
			if (temp_str.length() == 0)
				continue;
			n_words++;

			addString(str_arr, temp_str);
		}
		freeTFList(tfL);
	}

	sortMultiThread(str_arr); // sort every word from tf
	idfListInput(idfL, n_files, str_arr.array, n_words); // then add to idf list
	path.close();
	str_arr.size = 0;

	return idfL;
}

// Update metadata with provided path
void updateMetadata(string Path)
{
	int k = Path.length() - 1;
	while (k >= 0 && Path[k] != '\\') k--;

	string folder_name = Path.substr(k + 1); // Get only folder name from path
	copyFolderWrapper(Path, prev_dataset + "\\" + folder_name); // copy from source to dataset folder
	updateFolder(folder_name); // update
	
	// And then do regular thing as createMetadata;
	Path = prev_dataset + "\\" + folder_name;
	string path_to_file = string(METADATA_NAME) + "\\" + folder_name + ".txt";
	getFileWrapper(Path, path_to_file);
	makeFolderWrapper(string(METADATA_NAME) + "\\" + folder_name);

	ifstream ifs(path_to_file, ios::in);
	initString(str_arr);

	string filename;
	int fileCount = 0;
	while (getline(ifs, filename))
	{
		fileCount++;
		string tf_path = METADATA_NAME + "\\" + folder_name + "\\" + filename + ".tf";
		string file_path = Path + "\\" + filename;
		TF_list L = createTF(file_path);
		saveTFList(tf_path, L);
		freeTFList(L);
	}

	string folder_path = METADATA_NAME + "\\" + folder_name;
	IDF_list idf = createIDF(folder_path);
	string idf_path = folder_path + string(".idf");
	saveIDFList(idf_path, idf);
	freeIDFList(idf);

	ifs.close();

	deleteArray(str_arr);
}

// Initially create metadata folder from dataset folder
void createMetadata(const string& FolderDataset)
{
	initString(str_arr);
	prepareFile(FolderDataset);

	ifstream subFol(SUBFOLDER_NAME, ios::in);
	string folderName;

	while (getline(subFol, folderName))  // Reading folder name from file
	{
		cout << "Processing with folder " << folderName << "...\n";
		int n_files = 0;
		string list_file = METADATA_NAME + "\\" + folderName + ".txt";
		ifstream fr(list_file, ios::in);
		string file_name;

		while (getline(fr, file_name))  // Reading file name from file
		{
			n_files++;
			string tf_path = METADATA_NAME + "\\" + folderName + "\\" + file_name + ".tf";
			string file_path = FolderDataset + "\\" + folderName + "\\" + file_name;
			TF_list L = createTF(file_path); // create tf list from text file
			saveTFList(tf_path, L); // then save it to metadata
			freeTFList(L); // free used memory
		}
		fr.close();

		string path = METADATA_NAME + "\\" + folderName;
		IDF_list idf = createIDF(path); // create idf list from folder of tf
		string idf_path = path + string(".idf");
		saveIDFList(idf_path, idf); // then save it to metadata
		freeIDFList(idf); // free used memory
	}
	subFol.close();

	deleteArray(str_arr);

	// Write name of dataset to end of file
	fstream afs(SUBFOLDER_NAME, ios::app);
	afs << FolderDataset << "\n";
	afs.close();
}

// Loading database into RAM for searching
void loadToRAM()
{
	initString(str_arr);
	loadTextToArray(str_arr, SUBFOLDER_NAME);
	// Initially allocate memory for every array
	prev_dataset = str_arr.array[str_arr.size - 1];

	folder_count = str_arr.size - 1;
	folder_list = new string[folder_count];
	file_list = new string*[folder_count];
	file_count = new int[folder_count];

	folder_data = new FolderData[folder_count];

	for(int i = 0; i < folder_count; i++)
		folder_list[i] = str_arr.array[i];

	str_arr.size = 0;

	// Then loading TFs and IDFs
	for(int i = 0; i < folder_count; i++)
	{
		string file_path = string(METADATA_NAME) + "\\" + folder_list[i];
		loadTextToArray(str_arr, file_path + ".txt");
		
		loadIDFList(file_path + ".idf", folder_data[i].idfL);
		folder_data[i].tfLArr = new TF_list[str_arr.size];

		file_count[i] = str_arr.size;
		file_list[i] = new string[str_arr.size];

		for(int j = 0; j < str_arr.size; j++)
		{
			file_list[i][j] = str_arr.array[j];
			loadTFList(file_path + "\\" + str_arr.array[j] + ".tf", folder_data[i].tfLArr[j]);
		}
	}

	// Write path to every file to index.txt
	ofstream ofs(INDEX_NAME, ios::out);

	for (int i = 0; i < folder_count; i++)
		for (int j = 0; j < file_count[i]; j++)
			ofs << prev_dataset << "\\" << folder_list[i] << "\\" << file_list[i][j] << "\n";
	
	ofs.close();

	deleteArray(str_arr);
}

// Free RAM used for array
void freeRAM()
{
	delete[] folder_list;
	delete[] file_count;
	for (int i = 0; i < folder_count; i++)
	{
		delete[] file_list[i];
		freeIDFList(folder_data[i].idfL);

	}
	delete[] file_list;
}

// Query single word with database
ResponseData queryRequest(const string& Word) 
{
	int pos_idf;
	int pos_tf;

	ResponseData rd{};
	initResponse(rd);

	for(int i = 0; i < folder_count; i++) 
	{
		// search word in list of idf
		if((pos_idf = bSearchIDF(folder_data[i].idfL, Word)) != -1) // If found
		{
			const double idf_value = getIDFValue(folder_data[i].idfL, pos_idf);
			for(int j = 0; j < file_count[i]; j++)
			{
				// search word in list of tf
				if((pos_tf = bSearchTF(folder_data[i].tfLArr[j], Word)) != -1) // If found
				{
					const double tf_value = getTFValue(folder_data[i].tfLArr[j], pos_tf);
					// Then calculate the value of tf*idf save to response
					addResponse(rd, FileData{i, j, idf_value * tf_value, 1});
				}
			}
		}
	}
	return rd;
}

// find which response have largest size, return position
int findMaxSize(ResponseData* RespArr, const int ArrSize) {
	ResponseData res = RespArr[0];
	int idx = 0;
	for (int i = 1; i < ArrSize; i++) {
		if (res.size < RespArr[i].size) {
			res = RespArr[i];
			idx = i;
		}
	}
	return idx;
}

// Query sentence from input
void searchSentence(const string& Sentence) {
	// input sentence into string stream for easy separating
	istringstream iss(Sentence);
	string temp;
	StringArray string_arr{};
	initString(string_arr);
	while (iss >> temp) { // Add each word without punctuation to string array
		addString(string_arr,normalPunctuation(temp));
	}
	const int size = string_arr.size;
	ResponseData* res_response = new ResponseData[size];
	for (int i = 0; i < size; i++) {
		res_response[i] = queryRequest(string_arr.array[i]);
	} // Get all response data from database;

	// Then intersect max sized response with others to determine how many word been found
	const int max_index = findMaxSize(res_response, size);
	ResponseData rd = res_response[max_index];
	for (int i = 0; i < size; i++) {
		if (i == max_index) {
			continue;
		}
		intersectResponse(rd, res_response[i]);
	}
	sortResponse(rd); 
	const int res_size = rd.size;
	FileData* res_files = rd.file;
	if (res_size == 0) {
		cout << "No items match your search.\n";
	}
	else {
		int left = 0, right = min(res_size, 10);
		while (true) {
			evalCommand("cls");
			cout << "Your search query: " << Sentence << "\n";
			cout << "Displaying result of " << left << " - " << min(right, res_size) - 1 << " in " << res_size <<" files.!\n";
			for (int i = left; i < min(right, res_size); i++) {
				const FileData this_file = res_files[i];
				cout << i - left << ") " << fixed << setprecision(4) << this_file.value << " " << setw(30)
					<< file_list[this_file.posFolder][this_file.posFile] << setw(30) << " in folder "
					<< folder_list[this_file.posFolder] << "\n";
			}

			cout << "[Input number 0 - 9 to open file with notepad, n for next page, p for previous page and b to back to menu]\n";
			char op = 0;
			do
			{
				cout << "Your option?: ";
				cin >> op;
			}
			while (!isdigit(op) && op != 'n' && op != 'p' && op != 'b');

			if (op == 'b') break;

			if(op == 'n')
			{
				left += 10;
				right += 10;
				if (left > res_size)
				{
					left -= 10;
					right -= 10;
					cout << "This is the last page!\n";
					evalCommand("pause");
				}
				continue;
			}

			if(op == 'p')
			{
				left -= 10;
				right -= 10;
				if (left < 0)
				{
					left += 10;
					right += 10;
					cout << "This is the first page!\n";
					evalCommand("pause");
				}
				continue;
			}
			op -= 48;
			if (op + left >= res_size) continue;
			openFileWithNotepadWrapper(prev_dataset + "\\" + folder_list[res_files[op + left].posFolder] + "\\" + file_list[res_files[op + left].posFolder][res_files[op + left].posFile]);
		}
	}	
	delete[]res_response;
	deleteArray(string_arr);
}

