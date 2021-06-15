﻿#include <string>
#include <iostream>
#include <fstream>
#include <codecvt>
using namespace std;


wchar_t inverseMap[] = { L'a',L'á',L'à',L'ả',L'ã',L'ạ',L'â',L'ấ',L'ầ',L'ẩ',L'ẫ',L'ậ',L'ă',L'ắ',L'ằ',L'ẳ',L'ẵ',L'ặ', \
  L'e',L'ê',L'ế',L'ề',L'ể',L'ễ', L'ệ', L'é', L'è', L'ẻ', L'ẽ', L'ẹ', \
  L'i',L'í',L'ì',L'ỉ',L'ĩ', L'ị', \
  L'o',L'ó',L'ò',L'ỏ',L'õ', L'ọ', L'ô', L'ố', L'ồ', L'ổ', L'ỗ', L'ộ', L'ớ', L'ờ', L'ở', L'ỡ', L'ợ', L'ơ', \
  L'u',L'ú',L'ù',L'ủ',L'ũ', L'ụ', L'ư', L'ứ', L'ừ', L'ử', L'ự', L'ữ', \
  L'b',L'c',L'đ',L'd',L'f',L'g',L'h',L'j',L'k',L'l',L'm',L'n',L'p',L'q',L'r',L's',L't',L'v',L'w',L'x',L'y',L'z' };

string telexMap[] = { "a","as","af","ar","ax","aj","aa","aas","aaf","aar","aax","aaj","aw","aws","awf","awr","awx","awj", \
  "e","ee","ees","eef","eer","eex","eej","es","ef","er","ex","ej", \
  "i","is","if","ir","ix","ij", \
  "o","os","of","or","ox","oj","oo","oos","oof","oor","oox","ooj","ows","owf","owr","owx","owj","ow", \
  "u","us","uf","ur","ux","uj","uw","uws","uwf","uwr","uwj","uwx", \
  "b","c","dd","d","f","g","h","j","k","l","m","n","p","q","r","s","t","v","w","x","y","z" };

wchar_t inverseCapitalMap[] = { L'A',L'Á',L'À',L'Ả',L'Ã',L'Ạ',L'Â',L'Ấ',L'Ầ',L'Ẩ',L'Ẫ',L'Ậ',L'Ă',L'Ắ',L'Ằ',L'Ẳ',L'Ẵ',L'Ặ', \
  L'E',L'Ê',L'Ế',L'Ề',L'Ể',L'Ễ', L'Ệ', L'É', L'È', L'Ẻ', L'Ẽ', L'Ẹ', \
  L'I',L'Í',L'Ì',L'Ỉ',L'Ĩ', L'Ị', \
  L'O',L'Ó',L'Ò',L'Ỏ',L'Õ', L'Ọ', L'Ô', L'Ố', L'Ồ', L'Ổ', L'Ỗ', L'Ộ', L'Ớ', L'Ờ', L'Ở', L'Ỡ', L'Ợ', L'Ơ', \
  L'U',L'Ú',L'Ù',L'Ủ',L'Ũ', L'Ụ', L'Ư', L'Ứ', L'Ừ', L'Ử', L'Ự', L'Ữ', \
  L'B',L'C',L'Đ',L'D',L'F',L'G',L'H',L'J',L'K',L'L',L'M',L'N',L'P',L'Q',L'R',L'S',L'T',L'V',L'W',L'X',L'Y',L'Z' };

char normalMap[] = { 'a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a', \
  'e','e','e','e','e','e','e','e','e','e','e','e', \
  'i','i','i','i','i','i', \
  'o','o','o','o','o','o','o','o','o','o','o','o','o','o','o','o','o','o', \
  'u','u','u','u','u','u','u','u','u','u','u','u', \
  'b','c','d','d','f','g','h','j','k','l','m','n','p','q','r','s','t','v','w','x','y','z' };



string VEconvert(wstring source) {
  string result = "";
  for(unsigned int i = 0; i < source.length(); i++) {
    int pos = 0;
	wchar_t t = source[i];
	for (; pos < 88 && t != inverseMap[pos] && t != inverseCapitalMap[pos]; pos++);
	if (pos == 88) {
		result += t;
		continue;
	}
    result += normalMap[pos];
  }
  return result;
}

string VEconvert_telex(wstring source) {
	string result = "";
	for (unsigned int i = 0; i < source.length(); i++) {
		int pos = 0;
		wchar_t t = source[i];
		for (; pos < 88 && t != inverseMap[pos] && t != inverseCapitalMap[pos]; pos++);
		if (pos == 88) {
			result += t;
			continue;
		}
		result += telexMap[pos];
	}
	return result;
}

#pragma warning(disable:4996)

// int main()
// {
	// wifstream fin("input.txt");
	// fin.imbue(locale(locale::empty(), new codecvt_utf8<wchar_t>));
	// wstring s;
	// getline(fin, s);

	// cout << VEconvert(s) << endl;

	// fin.close();

    // return 0;
// }
