//NB-IoT NPSS sequence detector
//Verification of the synchronization signals quality of detection 
// in the NB-IoT interface in difficult environmental conditions
// 
//transformation from Engineer Diploma MATLAB ---> C++
//just for fun and learn :>

//15.01.2024 work in progress...


#define _USE_MATH_DEFINES

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <cmath>
#include <fftw3.h>
#include <iomanip>
#include <fstream>
#include <algorithm>


struct IQData {
    int16_t I; //Real
    int16_t Q; //Imag
};


static void NPSS_reference_signal(fftw_complex NPSS_reference_sequence[1508]);

int main()
{
    const char* filename = "12.dat";  // Input file

    //Input - binary int 16 file, 30s of 4G IoT Signal, title is SNR
    
    FILE* fid = fopen(filename, "rb");
    if (!fid) 
    {
        std::cerr << "Can't open file." << std::endl;
        return 1;
    }

    const size_t chunksize = 6144000; //short file elements for test
    const size_t startpos = 1228800000;
    fseek(fid, startpos, SEEK_SET);

    int16_t* data = new int16_t[chunksize];
    fread(data, sizeof(int16_t), chunksize, fid);

    double* I = new double[chunksize / 2];
    double* Q = new double[chunksize / 2];

    for (size_t i = 0, j = 0; i < chunksize; i += 2, ++j) 
    {
        I[j] = static_cast<double>(data[i]);
        Q[j] = static_cast<double>(data[i + 1]);
    }

    fftw_complex* received_signal = reinterpret_cast<fftw_complex*>(fftw_malloc(sizeof(fftw_complex) * (chunksize / 2)));

    for (size_t i = 0; i < chunksize / 2; ++i) {
        received_signal[i][0] = I[i]/std::pow(2,14);
        received_signal[i][1] = Q[i]/ std::pow(2, 14);
    }

    fclose(fid);

    

    for (int i = 0; i < 100; i++)
    {
        std::cout << std::setprecision(5);
        std::cout << received_signal[i][0] << " + " << received_signal[i][1] << std::endl;
    }

    
    delete[] I;
    delete[] Q;
        
}
    
    
    




static void ifft(const int fft_size, fftw_complex* in, fftw_complex* out)
{
    fftw_plan plan = fftw_plan_dft_1d(fft_size, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);
    fftw_execute(plan);
    fftw_destroy_plan(plan);
}

static void NPSS_reference_signal(fftw_complex NPSS_reference_sequence[1508])
{
    const int NN_size = 11;
    int NN[NN_size];

    fftw_complex ref[NN_size];
    //Zadoff-Chu sequence LTE - 3GPP TS 36.211 Rel 15
    for (int i = 0; i < NN_size; ++i) {
        ref[i][0] = cos(-1 * (5 * M_PI * NN[i] * (NN[i] + 1)) / 11);
        ref[i][1] = sin(-1 * (5 * M_PI * NN[i] * (NN[i] + 1)) / 11);

    }

    fftw_complex S[11] = { {1, 1}, {1, 1}, {1, 1}, {1, 1}, {-1, -1}, {-1, -1}, {1, 1}, {1, 1}, {1, 1}, {-1, -1}, {1, 1} };

    fftw_complex M[11][11];
    for (int i = 0; i < 11; i++)
    {
        for (int j = 0; j < 11; j++)
        {
            M[i][j][0] = S[j][0] * ref[i][0];
            M[i][j][1] = S[j][1] * ref[i][1];

        }
    }

    const int ofdm_size = 1408;
    const int fft_size = 128;
    fftw_complex ofdm[ofdm_size];
    fftw_complex out[fft_size];
    fftw_complex symbols[fft_size] = { 0,0 };

    for (int i = 0; i < 11; i++)
    {
        for (int j = 1; j < 12; j++)
        {
            symbols[j][0] = M[j - 1][i][0];
            symbols[j][1] = M[j - 1][i][1];
        }

        ifft(fft_size, symbols, out);

        for (int k = 0; k < fft_size; k++)
        {
            ofdm[(i * fft_size) + k][0] = out[k][0] / fft_size;
            ofdm[(i * fft_size) + k][1] = out[k][1] / fft_size;
        }

    }

    //include cycle prefix

    struct Reference
{
    int Start;
    int End;
    int Init;
    Reference(int start, int end, int init)
    {
        Start = Start;
        End = end;
        Init = init;
    }

}

List<Reference> refs = new Reference
{
    new Reference(0, 9, 119),
    new Reference(9, 137, 0),
    new Reference(137, 146, 247),
    new Reference(146, 274, 128),
    new Reference(275, 283, 375),
    new Reference(284, 411, 256),
    new Reference(411, 420, 503),
    new Reference(420, 548, 384),
    new Reference(548, 558, 630),
    new Reference(558, 686, 512),
    new Reference(686, 695, 759),
    new Reference(695, 823, 640),
    new Reference(823, 832, 887),
    new Reference(832, 960, 768),
    new Reference(960, 969, 1015),
    new Reference(969, 1097, 896),
    new Reference(1097, 1106, 1143),
    new Reference(1106, 1234, 1024),
    new Reference(1234, 1243, 1271),
    new Reference(1243, 1371, 1152),
    new Reference(1371, 1380, 1399),
    new Reference(1380, 1508, 1280)
    };
}

foreach (var reference in refs)
{
    for (int i = refs.Start, j = refs.Init; i < refs.End; i++, j++)
    {
        NPSS_reference_sequence[i][0] = ofdm[j][0];
        NPSS_reference_sequence[i][1] = ofdm[j][1];
    }
}
return NPSS_reference_sequence;
}
