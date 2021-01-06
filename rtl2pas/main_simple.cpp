#include <iostream>
#include <vector>
#include <fstream>

using namespace std;

#define PASCAL_STR_LEN_MAX 255

string NOP = "$90";

bool isExecutable(FILE *f) {

    fgetc(f);
    if ('E' == fgetc(f) &&
        'L' == fgetc(f) &&
        'F' == fgetc(f)) {
        return true;
    }

    rewind(f);
    return  0xcf == fgetc(f) &&
            0xfa == fgetc(f) &&
            0xed == fgetc(f) &&
            0xfe == fgetc(f);
}

int main() {

    FILE *f = fopen(
            "/Users/a.mamaev/Work/CourseProject/A-Bauman-BTPC-macOS/rtl64",
            //"/Users/a.mamaev/Work/CourseProject/A-Bauman-BTPC-macOS/rtl64linux",
            "rb");

    if (isExecutable(f)) {
        rewind(f);

        vector<string> stringList;
        stringList.clear();

        stringList.push_back("  OutputCodeDataSize:=0;");

        string s = "  OutputCodeString(";
        int singleByte;

        int bytesInString = 0;
        int bytes = 0;
        while (!feof(f)) {

            if (PASCAL_STR_LEN_MAX == bytesInString) {
                s += ");";
                stringList.push_back(s);

                bytesInString = 0;
                s = "  OutputCodeString(";
            }

            singleByte = fgetc(f);
            s += "#" + to_string(singleByte);

            bytesInString++;
            bytes++;
        }

        while (bytesInString < PASCAL_STR_LEN_MAX) {

            s += "#" + NOP;
            bytesInString++;
        }
        s += ");";

        stringList.push_back(s);
        stringList.push_back("  OutputCodeDataSize:=" + to_string(bytes) + ";");


        ofstream stub;
        stub.open ("emitStubCode.txt");
        if (stub.is_open()) {

            for (int i = 0; i < bytes / PASCAL_STR_LEN_MAX + 1 + 2; i++) {

                cout << stringList[i] << endl;
                stub << stringList[i] << "\n";
            }

            stub.close();

        } else {
            cout << "cannot open output file" << endl;
        }

    } else {
        cout << "file is not of Mach-O format" << endl;
    }

    return 0;
}