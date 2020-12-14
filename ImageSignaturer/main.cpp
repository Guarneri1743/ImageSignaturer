#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <iostream>
#include <vector>
#include "ImageSignaturer.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    int mode;
    cout << "sign: 0 / decryption: 1:" << endl;
    cin >> mode;

    if (mode == 0)
    {
        string filename;
        string key;
        cout << "enter a target image path(24-bits only): " << endl;
        cin >> filename;
        cout << "enter a key image path(24-bits only): " << endl;
        cin >> key;
        ImageSignaturer::sign(filename, key, "en.bmp");
    }
    else if(mode == 1)
    {
        string encrypted_filename;
        cout << "enter en encrypted file path: " << endl;
        cin >> encrypted_filename;
        ImageSignaturer::decrypt(encrypted_filename, "de.bmp");
    }

    return 0;
}


