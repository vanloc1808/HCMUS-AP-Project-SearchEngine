#include "TF.h"
#include <fstream>
using namespace std;
#pragma warning(disable:4996)

double getTFValue(TF_list List, int i)
{
	if(i >= List.size)
		return 0;
	return ((double)List.arrNorm[i].count / List.totalCount);
}

void TFListInit(TF_list &List)
{
	List.size = 0;
	List.totalCount = 0;
	List.capacity = 1000;
	List.arrNorm = new TF[1000];
}

void addTF(TF_list &List, TF data)
{
	if (List.size == List.capacity) {
		List.capacity += 1000;

		TF* temp1 = new TF[List.capacity];
		TF* temp2 = new TF[List.capacity];
		for (int i = 0; i < List.capacity - 1000; i++) temp1[i] = List.arrNorm[i];
		delete[] List.arrNorm;
		List.arrNorm = temp1;
	}

	List.totalCount += data.count;
	List.arrNorm[List.size++] = data;
}

void LoadTFList(string filename, TF_list& List)
{
	ifstream fr(filename, ios::in);

	//FreeTFList(List);
	
	fr >> List.capacity >> List.size >> List.totalCount;
	fr.ignore();

	string s = "";

	List.arrNorm = new TF[List.capacity];

	for (int i = 0; i < List.size; i++)
	{
		getline(fr, s);
		if (s.length() > 0 && s.back() == '\r') s.pop_back();
		List.arrNorm[i].word = s;
		getline(fr, s);
		if (s.length() > 0 && s.back() == '\r') s.pop_back();
		List.arrNorm[i].count = stoi(s);
	}

	fr.close();
}

void SaveTFList(string filename, TF_list List)
{
	ofstream fw(filename, ios::out);

	fw << List.size << "\n" << List.size << "\n" << List.totalCount << "\n";
	for (int i = 0; i < List.size; i++)
	{
		fw << List.arrNorm[i].word << "\n";
		fw << List.arrNorm[i].count << "\n";
	}

	fw.close();
}

void FreeTFList(TF_list &List)
{
	delete[] List.arrNorm;
	List.arrNorm = nullptr;
	List.totalCount = 0;
	List.capacity = 0;
	List.size = 0;
}

void TFList_Input(TF_list& List, string* data, int n) // data is sorted increasingly
{
	int count = 0;
	for (int i = 0; i < n - 1; i++)
	{
		count++;
		if (data[i] != data[i + 1]) {
			TF t{ data[i], count };
			addTF(List, t);
			count = 0;
		}
	}
	if (count > 0)
	{
		TF t{ data[n - 1], count };
		addTF(List, t);
		count = 0;
	}
}