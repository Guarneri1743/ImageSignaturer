#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <fstream>
#include "BMP.hpp"

using namespace std;

namespace ImageSignaturer
{
    typedef struct
    {
        double real;
        double image;
    }complex;

    complex add(complex, complex);
    complex sub(complex, complex);
    complex mul(complex, complex);

#define M_PI 3.1415926
#define intsize sizeof(int)
#define complexsize sizeof(complex)

    int* a, * b;
    complex* A;
    complex* W;
    int fft_width, image_width, fft_height, image_height, N, M;


    int calculate_m(int len)
    {
        int i;
        int k;

        i = 0;
        k = 1;
        while (k < len)
        {
            k = k * 2;
            i++;
        }

        return i;
    }

    void fft(int fft_w, int fft_m)
    {
        int i;
        int lev, dist, p, t;
        complex B;

        W = new complex[fft_w / 2]; 

        for (lev = 1; lev <= fft_m; lev++)
        {
            dist = (int)pow(2, lev - 1);
            for (t = 0; t < dist; t++)
            {
                p = t * (int)pow(2, fft_m - lev);
                W[p].real = (double)cos(2 * M_PI * p / fft_w);
                W[p].image = (double)(-1 * sin(2 * M_PI * p / fft_w));
                for (i = t; i < fft_w; i = i + (int)pow(2, lev))
                {
                    B = add(A[i], mul(A[i + dist], W[p]));
                    A[i + dist] = sub(A[i], mul(A[i + dist], W[p]));
                    A[i].real = B.real;
                    A[i].image = B.image;
                }
            }
        }

        delete W;
    }

    void reverse(int len, int M)
    {
        int i, j;

        a = new int[M];
        b = new int[len];

        for (i = 0; i < M; i++)
        {
            a[i] = 0;
        }

        b[0] = 0;
        for (i = 1; i < len; i++)
        {
            j = 0;
            while (a[j] != 0)
            {
                a[j] = 0;
                j++;
            }

            a[j] = 1;
            b[i] = 0;
            for (j = 0; j < M; j++)
            {
                b[i] = b[i] + a[j] * (int)pow(2, M - 1 - j);
            }
        }
    }

    complex add(complex c1, complex c2)
    {
        complex c;
        c.real = c1.real + c2.real;
        c.image = c1.image + c2.image;
        return c;
    }

    complex sub(complex c1, complex c2)
    {
        complex c;
        c.real = c1.real - c2.real;
        c.image = c1.image - c2.image;
        return c;
    }

    complex mul(complex c1, complex c2)
    {
        complex c;
        c.real = c1.real * c2.real - c1.image * c2.image;
        c.image = c1.real * c2.image + c2.real * c1.image;
        return c;
    }

    void fdomain2image(complex* encrypted_data)
    {
        int i, j;

        for (i = 0; i < fft_height; i++)
        {
            for (j = 0; j < fft_width; j++)
            {
                encrypted_data[i * fft_width + j].image = -encrypted_data[i * fft_width + j].image;
            }
        }

        A = new complex[fft_width];
        reverse(fft_width, N);
        for (i = 0; i < fft_height; i++)
        {
            for (j = 0; j < fft_width; j++)
            {
                A[j].real = encrypted_data[i * fft_width + b[j]].real;
                A[j].image = encrypted_data[i * fft_width + b[j]].image;

            }

            fft(fft_width, N);
            for (j = 0; j < fft_width; j++)
            {
                encrypted_data[i * fft_width + j].real = A[j].real;
                encrypted_data[i * fft_width + j].image = A[j].image;

            }
        }
        delete[] A;
        delete[] a;
        delete[] b;

        A = new complex[fft_height];
        reverse(fft_height, M);
        for (i = 0; i < fft_width; i++)
        {
            for (j = 0; j < fft_height; j++)
            {
                A[j].real = encrypted_data[b[j] * fft_width + i].real;
                A[j].image = encrypted_data[b[j] * fft_width + i].image;
            }

            fft(fft_height, M);
            for (j = 0; j < fft_height; j++)
            {
                encrypted_data[j * fft_width + i].real = A[j].real;
                encrypted_data[j * fft_width + i].image = A[j].image;

            }
        }
        delete[] A;
        delete[] a;
        delete[] b;
    }

    void save_decrypted_image(string output_path, complex* input_data)
    {
        bitmap_image image(fft_width, fft_height);
        image.set_all_channels(0, 0, 0);
        image_drawer draw(image);

        for (unsigned int i = 0; i < fft_height; ++i)
        {
            for (unsigned int j = 0; j < fft_width; ++j)
            {
                double visualization = sqrt(pow(input_data[i * fft_width + j].real, 2) + pow(input_data[i * fft_width + j].image, 2)) / (fft_width * fft_height);
                unsigned char r = (unsigned char)std::max(std::min(visualization * 255.0, 255.0), 0.0);
                unsigned char g = (unsigned char)std::max(std::min(visualization * 255.0, 255.0), 0.0);
                unsigned char b = (unsigned char)std::max(std::min(visualization * 255.0, 255.0), 0.0);
                draw.pen_color(r, g, b);
                draw.plot_pixel(i, j);
            }
        }
        image.save_image(output_path);
        cout << "image decrypted: " << output_path << endl;
    }

    complex* image2fdomain(bitmap_image& image)
    {
        image_width = image.width();
        image_height = image.height();

        M = calculate_m(image_height);
        N = calculate_m(image_width);
        fft_width = (int)pow(2, N);
        fft_height = (int)pow(2, M);

        complex* fdomain_data = new complex[fft_width * fft_height];

        int row = 0;
        int col = 0;

        for (row = 0; row < image_height; row++)
        {
            for (col = 0; col < image_width; col++)
            {
                fdomain_data[row * fft_width + col].real = image.get_pixel(col, row).red * pow(-1, row + col);
                fdomain_data[row * fft_width + col].image = 0.0;
            }
        }

        for (row = 0; row < fft_height; row++)
        {
            for (col = image_width; col < fft_width; col++)
            {
                fdomain_data[row * fft_width + col].real = 0.0;
                fdomain_data[row * fft_width + col].image = 0.0;
            }
        }

        for (row = image_height; row < fft_height; row++)
        {
            for (col = 0; col < image_width; col++)
            {
                fdomain_data[row * fft_width + col].real = 0.0;
                fdomain_data[row * fft_width + col].image = 0.0;
            }
        }

        A = new complex[fft_width];

        int LPF;
        double* D;
        double* H;
        double D0;
        double n;

        /*cout << "choose a LPF: \n 1: ILPF \n 2: BLPF \n 3: GLPF \n" << endl;
        cin >> LPF;*/

        D = new double[fft_height * fft_width];
        H = new double[fft_height * fft_width];

        int i = 0;
        int j = 0;

        D0 = max(fft_width, fft_height); // keep all signals
        for (i = 0; i < fft_height; i++)
        {
            for (j = 0; j < fft_width; j++)
            {
                D[i * fft_width + j] = sqrt(pow((i - fft_height / 2), 2) + pow((j - fft_width / 2), 2));
                H[i * fft_width + j] = exp(-pow(D[i * fft_width + j], 2) / (2 * pow(D0, 2)));
            }
        }
        /*switch (LPF)
        {
        case 1:
            cout << "enter D0" << endl;
            cin >> D0;
            for (i = 0; i < fft_height; i++)
            {
                for (j = 0; j < fft_width; j++)
                {
                    D[i * fft_width + j] = sqrt(pow((i - fft_height / 2), 2) + pow((j - fft_width / 2), 2));
                    if (D[i * fft_width + j] <= D0)
                    {
                        H[i * fft_width + j] = 1;
                    }
                    else
                    {
                        H[i * fft_width + j] = 0;
                    }
                }
            }
            break;
        case 2:
            cout << "enter D0 and n" << endl;
            cin >> D0;
            cin >> n;
            for (i = 0; i < fft_height; i++)
            {
                for (j = 0; j < fft_width; j++)
                {
                    D[i * fft_width + j] = sqrt(pow((i - fft_height / 2), 2) + pow((j - fft_width / 2), 2));
                    H[i * fft_width + j] = 1 / (1 + pow(D0 / D[i * fft_width + j], 2 * n));
                }
            }
            break;
        case 3:
            cout << "enter D0" << endl;
            cin >> D0;
            for (i = 0; i < fft_height; i++)
            {
                for (j = 0; j < fft_width; j++)
                {
                    D[i * fft_width + j] = sqrt(pow((i - fft_height / 2), 2) + pow((j - fft_width / 2), 2));
                    H[i * fft_width + j] = exp(-pow(D[i * fft_width + j], 2) / (2 * pow(D0, 2)));
                }
            }
            break;
        }*/

        reverse(fft_width, N);

        for (i = 0; i < fft_height; i++)
        {
            for (j = 0; j < fft_width; j++)
            {
                A[j].real = fdomain_data[i * fft_width + b[j]].real;
                A[j].image = fdomain_data[i * fft_width + b[j]].image;
            }

            fft(fft_width, N);
            for (j = 0; j < fft_width; j++)
            {
                fdomain_data[i * fft_width + j].real = A[j].real;
                fdomain_data[i * fft_width + j].image = A[j].image;
            }
        }

        delete A;
        delete a;
        delete b;

        A = new complex[fft_height];
        reverse(fft_height, M);
        for (i = 0; i < fft_width; i++)
        {
            for (j = 0; j < fft_height; j++)
            {
                A[j].real = fdomain_data[b[j] * fft_width + i].real;
                A[j].image = fdomain_data[b[j] * fft_width + i].image;
            }

            fft(fft_height, M);
            for (j = 0; j < fft_height; j++)
            {
                fdomain_data[j * fft_width + i].real = A[j].real;
                fdomain_data[j * fft_width + i].image = A[j].image;
            }
        }

        delete A;

        for (i = 0; i < fft_height; i++)
        {
            for (j = 0; j < fft_width; j++)
            {
                fdomain_data[i * fft_width + j].real *= H[i * fft_width + j];
                fdomain_data[i * fft_width + j].image *= H[i * fft_width + j];
            }
        }

        return fdomain_data;
    }

    void save_signed_image(string output_path, complex* encrypted_image)
    {
        bitmap_image image(fft_width, fft_height);
        image.set_all_channels(0, 0, 0);
        image_drawer draw(image);
        for (unsigned int i = 0; i < fft_height; ++i)
        {
            for (unsigned int j = 0; j < fft_width; ++j)
            {
                double value = sqrt(pow(encrypted_image[i * fft_width + j].real, 2)) / (fft_height * fft_width);
                value *= pow(-1.0, i + j);
                unsigned char r = (unsigned char)std::max(std::min(value, 255.0), 0.0);
                unsigned char g = (unsigned char)std::max(std::min(value, 255.0), 0.0);
                unsigned char b = (unsigned char)std::max(std::min(value, 255.0), 0.0);
                draw.pen_color(r, g, b);
                draw.plot_pixel(j, i);
            }
        }
        image.save_image(output_path);
    }

    void decrypt(string filename, string output_path)
    {
        bitmap_image image(filename);
        save_decrypted_image(output_path, image2fdomain(image));
    }

    void sign(string filename, string key_image, string output_path)
    {
        bitmap_image plain_text(filename);
        bitmap_image key(key_image);
        complex* fdomain = image2fdomain(plain_text);

        // image based encryption
        for (int i = 0; i < fft_height; i++)
        {
            for (int j = 0; j < fft_width; j++)
            {
                int index = i * fft_width + j;
                fdomain[index].real += ((double)key.get_pixel(i, j).red / 255.0) * fft_width * fft_height;
                fdomain[index].image += ((double)key.get_pixel(i, j).red / 255.0) * fft_width * fft_height;
            }
        }

        fdomain2image(fdomain);
        save_signed_image(output_path, fdomain);
        cout << "image encrypted: " << output_path << endl;
    }
}