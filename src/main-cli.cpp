/*
    src/main.cpp -- Tone Mapper main function

    Copyright (c) 2016 Tizian Zeltner

    Tone Mapper is provided under the MIT License.
    See the LICENSE.txt file for the conditions of the license.
*/

#include <global.h>
#include <stdlib.h>
#include <cli.h>

#define DEBUG

#ifdef WIN32
	#include <windows.h>
#endif


#include <iostream>
#include <vector>
#include <string>

#define cimg_display 0
#include "CImg.h"
using namespace std;
using namespace cimg_library;


// Color Transfer implementations
// https://github.com/ProGamerGov/Neural-Tools
// others:
//   https://github.com/ZZPot/Xiao-transfer
///  https://github.com/ZZPot/Multi-color-transfer
//   https://github.com/MehdiNS/ColorTransfert

// Hu Yupan
// Implementation and improvement of paper: Color Transfer between Images,
// Erik Reinhard et al, 2001 —— A simple statistical analysis to impose one image’s color characteristics on another.
// https://github.com/HYPJUDY/color-transfer-between-images

CImg<float> RGB2Lab(CImg<unsigned int> RGBimg) {
    if (RGBimg._spectrum != 3) { // Number of colour channels
        cerr << "RGB2Lab(): Instance is not a RGB image." << endl;
    }
    CImg<float> Labimg(RGBimg._width, RGBimg._height, 1, 3, 0); // w,h,depth,spectrum,initVal
    float R, G, B, L, M, S, l, alpha, beta;
    cimg_forXY(RGBimg, x, y) {
        R = cimg::max(1.0 * RGBimg(x, y, 0) / 255, 1.0 / 255); // must be 1.0f, or 1/255 will be zero!!
        G = cimg::max(1.0 * RGBimg(x, y, 1) / 255, 1.0 / 255);
        B = cimg::max(1.0 * RGBimg(x, y, 2) / 255, 1.0 / 255);

        // RGB -> LMS
        L = 0.3811f*R + 0.5783f*G + 0.0402f*B;
        M = 0.1967f*R + 0.7244f*G + 0.0782f*B;
        S = 0.0241f*R + 0.1288f*G + 0.8444f*B;
        // Convert the data to logarithmic space
        L = log10(L); // log(x), x > 0
        M = log10(M);
        S = log10(S);
        // LMS -> Lab
        l = 1.0 / sqrt(3)*L + 1.0 / sqrt(3)*M + 1.0 / sqrt(3)*S;
        alpha = 1.0 / sqrt(6)*L + 1.0 / sqrt(6)*M - 2 / sqrt(6)*S;
        beta = 1.0 / sqrt(2)*L - 1.0 / sqrt(2)*M + 0 * S;

        Labimg(x, y, 0) = l;
        Labimg(x, y, 1) = alpha;
        Labimg(x, y, 2) = beta;
    }
    return Labimg;
}

CImg<float> transferOfStatistics(CImg<float> Lab_source, CImg<float> Lab_target) {
    CImg<float> Lab_result(Lab_source._width, Lab_source._height, 1, 3, 0);
    CImg<float> mean_source(Lab_source._spectrum, 1, 1, 1, 0);
    CImg<float> std_source(Lab_source._spectrum, 1, 1, 1, 0);
    CImg<float> mean_target(Lab_target._spectrum, 1, 1, 1, 0);
    CImg<float> std_target(Lab_target._spectrum, 1, 1, 1, 0);
    /* Compute the means and standard deviations
    /* for each axis(L,a,b three axes in total) separately in Lab space */
    // source
    unsigned long n_source = Lab_source._width * Lab_source._height;
    cimg_forXYC(Lab_source, x, y, c)
        mean_source(c) += Lab_source(x, y, c);
    cimg_forX(mean_source, c)
        mean_source(c) /= n_source;
    cimg_forXYC(Lab_source, x, y, c)
        std_source(c) += pow(Lab_source(x, y, c) - mean_source(c), 2.0);
    cimg_forX(mean_source, c)
        std_source(c) = sqrt(std_source(c) / n_source);

    // target
    unsigned long n_target = Lab_target._width * Lab_target._height;
    cimg_forXYC(Lab_target, x, y, c)
        mean_target(c) += Lab_target(x, y, c);
    cimg_forX(mean_target, c)
        mean_target(c) /= n_target;
    cimg_forXYC(Lab_target, x, y, c)
        std_target(c) += pow(Lab_target(x, y, c) - mean_target(c), 2.0);
    cimg_forX(mean_target, c)
        std_target(c) = sqrt(std_target(c) / n_target);

    /* Transfer by adjust the mean and standard deviations along each of the three axes */
    cimg_forXYC(Lab_source, x, y, c) {
        Lab_result(x, y, c) = Lab_source(x, y, c) - mean_source(c);
        /* PureColorGuidedStyle method: Input a pure color target image and tune the
        *  scale value to get a result image more conveniently and controllable. */
        float scale_val = 0.5;
        Lab_result(x, y, c) = Lab_result(x, y, c) * cimg::max(scale_val,
            (std_target(c) / std_source(c)));
        Lab_result(x, y, c) = Lab_result(x, y, c) + mean_target(c);
    }
    return Lab_result;
}


CImg<unsigned int> Lab2RGB(CImg<float> Labimg) {
    if (Labimg._spectrum != 3) {
        cerr << "Lab2RGB(): Instance is not a Lab image." << endl;
    }
    CImg<unsigned int> RGBimg(Labimg._width, Labimg._height, 1, 3, 0);
    float R, G, B, L, M, S, l, alpha, beta;
    cimg_forXY(Labimg, x, y) {
        l = Labimg(x, y, 0);
        alpha = Labimg(x, y, 1);
        beta = Labimg(x, y, 2);

        // Lab -> LMS
        L = sqrt(3.0) / 3.0 * l + sqrt(6) / 6.0 * alpha + sqrt(2) / 2.0 * beta;
        M = sqrt(3.0) / 3.0 * l + sqrt(6) / 6.0 * alpha - sqrt(2) / 2.0 * beta;
        S = sqrt(3.0) / 3.0 * l - sqrt(6) / 3.0 * alpha - 0 * beta;
        // Raising the pixel values to the power ten to go back to linear space
        L = pow(10.0, L);
        M = pow(10.0, M);
        S = pow(10.0, S);
        // LMS -> RGB
        R = 4.4679*L - 3.5873*M + 0.1193*S;
        G = -1.2186*L + 2.3809*M - 0.1624*S;
        B = 0.0497*L - 0.2439*M + 1.2045*S;

        RGBimg(x, y, 0) = cimg::max(cimg::min(R * 255, 255), 0); // range 0-255!!
        RGBimg(x, y, 1) = cimg::max(cimg::min(G * 255, 255), 0);
        RGBimg(x, y, 2) = cimg::max(cimg::min(B * 255, 255), 0);
    }
    return RGBimg;
}

CImg<unsigned int> colorTransfer(CImg<unsigned int> source, CImg<unsigned int> target) {
    CImg<float> Lab_result = transferOfStatistics(RGB2Lab(source), RGB2Lab(target));
    CImg<unsigned> RGBimg = Lab2RGB(Lab_result);
    return RGBimg;
}

//  骆宏迪 skylhd
// https://github.com/skylhd/ColorTransfer
class ColorTransfer {
  public:
    // Run the approach of the paper
    static CImg<float> Run(CImg<unsigned char> source, CImg<unsigned char> target) {
        CImg<float> target_lab = target.get_RGBtoLab();
        CImg<float> source_lab = source.get_RGBtoLab();
        CImg<float> ans_lab(source_lab);
        CImg<float> new_l_channel = ColorTransfer::CombineChannel(target_lab.get_channel(0), source_lab.get_channel(0));
        CImg<float> new_a_channel = ColorTransfer::CombineChannel(target_lab.get_channel(1), source_lab.get_channel(1));
        CImg<float> new_b_channel = ColorTransfer::CombineChannel(target_lab.get_channel(2), source_lab.get_channel(2));
        cimg_forXY(ans_lab, x, y) {
            ans_lab(x, y, 0) = new_l_channel(x, y);
            ans_lab(x, y, 1) = new_a_channel(x, y);
            ans_lab(x, y, 2) = new_b_channel(x, y);
        }
        CImg<float> ans = ans_lab.get_LabtoRGB();
        return ans;
    }

    /*
    Combine the value of two channels according to the paper
    */
    static CImg<float> CombineChannel(CImg<float> target, CImg<float> source) {

        double t_mean = target.mean(), s_mean = source.mean();
        double t_var = target.variance(), s_var = source.variance();
        CImg<float> ans(source);
        cimg_forXY(ans, x, y) {
            ans(x, y) = t_var / s_var * (source(x, y) - s_mean) + t_mean;
        }
        return ans;
    }

    //A sample function to show the histogram of a picture
    template <typename T>
    static void ShowHistogram(CImg<T> img, int v, int min, int max, char * filepath = "") {
        CImg<float> his = img.get_histogram(v, min, max);
        if (filepath != "")
        his.display_graph().save_bmp(filepath);
    }

    // show the histogram
    // useless
    static CImg<float> GetHis(CImg<float> img, int c, int min, int max) {
        int v = (max - min + 1)*c;
        CImg<float> ans(v, 1, 1, 1, 0);
        cimg_forXY(img, x, y) {
            float t = img(x, y);
            int val = (int)roundf((img(x, y) - min) * c);
            ans(val) += 1.0;
        }
        return ans;
    }

    //NewCC means NewCombineChannels.
    //A new approach to the paper, modify by me.
    //I use the hisogram equalization to modify the job.
    static CImg<float> NewCC(CImg<float> target, CImg<float> source, int min, int max, int c) {
        CImg<float> ans(source);
        int v = (max - min + 1)*c;
        CImg<float> t_his = ColorTransfer::GetHis(target, c, min, max);
        CImg<float> s_his = ColorTransfer::GetHis(source, c, min, max);
        CImg<float> t_cdf(v, 1, 1, 1), s_cdf(v, 1, 1, 1);
        float t_sum = 0.0, s_sum = 0.0;
        float t_size = target._width * target._height;
        float s_size = source._width * source._height;
        cimg_forX(t_his, x) {
            t_sum += t_his(x);
            s_sum += s_his(x);
            t_cdf(x) = t_sum;
            s_cdf(x) = s_sum;
        }
        cimg_forX(s_his, x) {
            t_cdf(x) /= t_sum;
            s_cdf(x) /= s_sum;
        }
        map<int, int> T;
        for (int i = 0, j = 0; i < v; ) {
            if (s_cdf(i) <= t_cdf(j)) {
                T[i] = j;
                i++;
            } else {
                j++;
            }
        }
        cimg_forXY(ans, x, y) {
            float temp = (float)(T[(int)((source(x, y) - min)*c)]) / c + min;
            ans(x, y) = temp;
        }
        return ans;
    }

    //Run the new approach
    static CImg<float> Run_New(CImg<unsigned char> target, CImg<unsigned char> source, int c) {
        CImg<float> target_lab = target.get_RGBtoLab();
        CImg<float> source_lab = source.get_RGBtoLab();
        CImg<float> ans_lab(source_lab);
        CImg<float> new_l_channel;
        new_l_channel = ColorTransfer::CombineChannel(target_lab.get_channel(0), source_lab.get_channel(0));
        CImg<float> new_a_channel = ColorTransfer::NewCC(target_lab.get_channel(1), source_lab.get_channel(1), -128, 127, c);
        CImg<float> new_b_channel = ColorTransfer::NewCC(target_lab.get_channel(2), source_lab.get_channel(2), -128, 127, c);
        cimg_forXY(ans_lab, x, y) {
            ans_lab(x, y, 0) = new_l_channel(x, y);
            ans_lab(x, y, 1) = new_a_channel(x, y);
            ans_lab(x, y, 2) = new_b_channel(x, y);
        }
        CImg<float> ans = ans_lab.get_LabtoRGB();
        ans.sharpen(50.0f);
        return ans;
    }
};

int xmain() {
    CImg<unsigned int> source_img, target_img;
    vector<const char*> num = { "0", "1", "2", "3", "4", "5", "6", "7",
        "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18",
        "19", "20", "21", "22", "23", "24", "25", "26", "27", "28" , "29"
        , "30" , "31" , "32", "33" };
    for (int i = 0; i <= 14; ++i) {
        // load source image
        char sourcePath[80];
        strcpy(sourcePath, "images/");
        strcat(sourcePath, num[i]);
        strcat(sourcePath, "source.bmp");
        source_img.load_jpeg(sourcePath);

        // load target image
        char targetPath[80];
        strcpy(targetPath, "images/");
        strcat(targetPath, num[i]);
        strcat(targetPath, "_target.bmp");
        target_img.load_jpeg(targetPath);

        char resultPath[80];
        strcpy(resultPath, "images/");
        strcat(resultPath, num[i]);
        strcat(resultPath, "_result.bmp");

        bool usehyp = true;

        if(usehyp) {
            //perform image transfer between two images and write to file
            CImg<unsigned int> result_img = colorTransfer(source_img, target_img);
            result_img.save(resultPath);
        } else {
            CImg<float> source_img2, target_img2;
            source_img2.load_jpeg(sourcePath);
            target_img2.load_jpeg(targetPath);
            CImg<unsigned int> result_img2 = ColorTransfer::Run(source_img2,target_img2);
            result_img2.save(resultPath);
        }
    }

    return 0;
}



void usage() {
    printf("help\n");
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        usage();
        return EXIT_FAILURE;
    }

    std::string oper = argv[1];
    std::cout<<"oper: "<<oper<<std::endl;

    if(oper == "aces") {
        std::string input = argv[2];
        std::string output = argv[3];

        std::cout<<"input:"<<input<<std::endl;
        std::cout<<"output:"<<output<<std::endl;

        ToneMapperCli* tonemapper = new ToneMapperCli();
        tonemapper->run(input, output, oper);
        delete tonemapper;

    } else if (oper == "colorxfer") {
        std::string input = argv[2];
        std::string ref = argv[3];
        std::string output = argv[3];

        std::cout<<"input:"<<input<<std::endl;
        std::cout<<"ref:"<<ref<<std::endl;
        std::cout<<"output:"<<output<<std::endl;

        xmain();
    }



    return 0;
}