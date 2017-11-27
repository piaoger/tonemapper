/*
    src/gui.cpp -- Graphical user interface

    Copyright (c) 2016 Tizian Zeltner

    Tone Mapper is provided under the MIT License.
    See the LICENSE.txt file for the conditions of the license.
*/
#include <global.h>
#include <cli.h>
#include <image.h>

// operators

// #include <operators/clamping.h>
// #include <operators/drago.h>
// #include <operators/exponential.h>
// #include <operators/exponentiation.h>
// #include <operators/ferwerda.h>
// #include <operators/filmic1.h>
// #include <operators/filmic2.h>
// #include <operators/insomniac.h>
// #include <operators/uncharted.h>
#include <operators/aces.h>
// #include <operators/linear.h>
// #include <operators/logarithmic.h>
// #include <operators/maxdivision.h>
// #include <operators/meanvalue.h>
// #include <operators/reinhard.h>
// #include <operators/reinhard_devlin.h>
// #include <operators/reinhard_extended.h>
// #include <operators/schlick.h>
// #include <operators/srgb.h>
// #include <operators/tumblin_rushmeier.h>
// #include <operators/ward.h>


ToneMapperCli::ToneMapperCli()
:m_image(nullptr)
{
    m_tonemapOperators = std::vector<TonemapOperator *>();
    m_tonemapOperators.push_back(new ACESOperator());
}

ToneMapperCli::~ToneMapperCli()
{
    if (m_image) {
        delete m_image;
        m_image = nullptr;
    }
}

void ToneMapperCli::setImage(const std::string &filename)
{
    if (m_image) {
        delete m_image;
        m_image = nullptr;
    }

    m_image = new Image(filename);

    if (m_image->getWidth() <= 0 || m_image->getHeight() <= 0) {
        delete m_image;
        m_image = nullptr;

        return;
    }

    for (auto tm : m_tonemapOperators) {
        tm->setParameters(m_image);
    }
}

void ToneMapperCli::setTonemapMode(int index)
{
}

void ToneMapperCli::setExposureMode(int index)
{

}

void ToneMapperCli::run(const std::string& input, const std::string& output,  const std::string& oper)
{

    setImage(input);

    m_exposure = m_image->getAutoKeyValue() / m_image->getLogAverageLuminance();

    std::size_t found = output.find_last_of(".");
    std::string filename = output;
    std::string ext = filename.substr(found+1);

    if (ext == "png") {
        std::cout<<"save as png"<<std::endl;
        m_image->saveAsPNG(filename, m_tonemapOperators[0], m_exposure, &m_progress);

    } else if (ext == "jpg") {
        std::cout<<"save as jpg"<<std::endl;
        m_image->saveAsJPEG(filename, m_tonemapOperators[0], m_exposure, &m_progress);

    } else {
        std::cout<<"unknown type ..."<<std::endl;
    }
}
