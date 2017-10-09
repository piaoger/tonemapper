/*
    src/gui.h -- Graphical user interface

    Copyright (c) 2016 Tizian Zeltner

    Tone Mapper is provided under the MIT License.
    See the LICENSE.txt file for the conditions of the license.
*/

#pragma once

#include <global.h>

#include <thread>



class Image;
class TonemapOperator;

class ToneMapperCli {

    public:

        ToneMapperCli();
        ~ToneMapperCli();

        void setImage(const std::string &filename);
        void setTonemapMode(int index);
        void setExposureMode(int index);

        void run(const std::string& input, const std::string& output,  const std::string& oper) ;

    private:
        Image* m_image;
        std::vector<TonemapOperator *> m_tonemapOperators;
        float m_exposure = 1.f;
        float m_progress = 0.f;
};
