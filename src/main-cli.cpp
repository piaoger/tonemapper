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

    std::string input = argv[1];
    std::string output = argv[2];
    std::string oper = argv[3];

    std::cout<<"input:"<<input<<std::endl;
    std::cout<<"output:"<<output<<std::endl;
    std::cout<<"oper: "<<oper<<std::endl;

    ToneMapperCli* tonemapper = new ToneMapperCli();
    tonemapper->run(input, output, oper);
    delete tonemapper;

    return 0;
}