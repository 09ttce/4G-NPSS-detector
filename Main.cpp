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

    for (int i = 0, j = 119; i < 9; i++, j++) {
        NPSS_reference_sequence[i][0] = ofdm[j][0];
        NPSS_reference_sequence[i][1] = ofdm[j][1];
    }

    for (int i = 9, j = 0; i < 137; i++, j++) {
        NPSS_reference_sequence[i][0] = ofdm[j][0];
        NPSS_reference_sequence[i][1] = ofdm[j][1];
    }


    for (int i = 137, j = 247; i < 146; i++, j++) {
        NPSS_reference_sequence[i][0] = ofdm[j][0];
        NPSS_reference_sequence[i][1] = ofdm[j][1];
    }

    for (int i = 146, j = 128; i < 274; i++, j++) {
        NPSS_reference_sequence[i][0] = ofdm[j][0];
        NPSS_reference_sequence[i][1] = ofdm[j][1];
    }

    for (int i = 275, j = 375; i < 283; i++, j++) {
        NPSS_reference_sequence[i][0] = ofdm[j][0];
        NPSS_reference_sequence[i][1] = ofdm[j][1];
    }

    for (int i = 284, j = 256; i < 411; i++, j++) {
        NPSS_reference_sequence[i][0] = ofdm[j][0];
        NPSS_reference_sequence[i][1] = ofdm[j][1];
    }

    for (int i = 411, j = 503; i < 420; i++, j++) {
        NPSS_reference_sequence[i][0] = ofdm[j][0];
        NPSS_reference_sequence[i][1] = ofdm[j][1];
    }

    for (int i = 420, j = 384; i < 548; i++, j++) {
        NPSS_reference_sequence[i][0] = ofdm[j][0];
        NPSS_reference_sequence[i][1] = ofdm[j][1];
    }

    for (int i = 548, j = 630; i < 558; i++, j++) {
        NPSS_reference_sequence[i][0] = ofdm[j][0];
        NPSS_reference_sequence[i][1] = ofdm[j][1];
    }

    for (int i = 558, j = 512; i < 686; i++, j++) {
        NPSS_reference_sequence[i][0] = ofdm[j][0];
        NPSS_reference_sequence[i][1] = ofdm[j][1];
    }

    for (int i = 686, j = 759; i < 695; i++, j++) {
        NPSS_reference_sequence[i][0] = ofdm[j][0];
        NPSS_reference_sequence[i][1] = ofdm[j][1];
    }

    for (int i = 695, j = 640; i < 823; i++, j++) {
        NPSS_reference_sequence[i][0] = ofdm[j][0];
        NPSS_reference_sequence[i][1] = ofdm[j][1];
    }

    for (int i = 823, j = 887; i < 832; i++, j++) {
        NPSS_reference_sequence[i][0] = ofdm[j][0];
        NPSS_reference_sequence[i][1] = ofdm[j][1];
    }

    for (int i = 832, j = 768; i < 960; i++, j++) {
        NPSS_reference_sequence[i][0] = ofdm[j][0];
        NPSS_reference_sequence[i][1] = ofdm[j][1];
    }

    for (int i = 960, j = 1015; i < 969; i++, j++) {
        NPSS_reference_sequence[i][0] = ofdm[j][0];
        NPSS_reference_sequence[i][1] = ofdm[j][1];
    }

    for (int i = 969, j = 896; i < 1097; i++, j++) {
        NPSS_reference_sequence[i][0] = ofdm[j][0];
        NPSS_reference_sequence[i][1] = ofdm[j][1];
    }

    for (int i = 1097, j = 1143; i < 1106; i++, j++) {
        NPSS_reference_sequence[i][0] = ofdm[j][0];
        NPSS_reference_sequence[i][1] = ofdm[j][1];
    }

    for (int i = 1106, j = 1024; i < 1234; i++, j++) {
        NPSS_reference_sequence[i][0] = ofdm[j][0];
        NPSS_reference_sequence[i][1] = ofdm[j][1];
    }

    for (int i = 1234, j = 1271; i < 1243; i++, j++) {
        NPSS_reference_sequence[i][0] = ofdm[j][0];
        NPSS_reference_sequence[i][1] = ofdm[j][1];
    }

    for (int i = 1243, j = 1152; i < 1371; i++, j++) {
        NPSS_reference_sequence[i][0] = ofdm[j][0];
        NPSS_reference_sequence[i][1] = ofdm[j][1];
    }

    for (int i = 1371, j = 1399; i < 1380; i++, j++) {
        NPSS_reference_sequence[i][0] = ofdm[j][0];
        NPSS_reference_sequence[i][1] = ofdm[j][1];
    }

    for (int i = 1380, j = 1280; i < 1508; i++, j++) {
        NPSS_reference_sequence[i][0] = ofdm[j][0];
        NPSS_reference_sequence[i][1] = ofdm[j][1];
    }
}