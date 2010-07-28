#include <QDebug>

#include "DummyProcessor.h"
#include "Pin.h"
#include "OpenCVImage.h"
#include <opencv/cv.h>

using namespace plv;

#define INPUT_PIN_NAME "input"
#define OUTPUT_PIN_NAME "output"

DummyProcessor::DummyProcessor() :
        m_someInt(1337),
        m_someDouble(1.23456),
        m_someBool(true),
        m_someString("hello")
{
    m_inputPin = new InputPin<OpenCVImage>( INPUT_PIN_NAME, this, IInputPin::REQUIRED );
    addInputPin( m_inputPin );

    m_inputPinOptional = new InputPin<OpenCVImage>( "input2", this, IInputPin::OPTIONAL );
    addInputPin( m_inputPinOptional );

    m_outputPin = new OutputPin<OpenCVImage>( OUTPUT_PIN_NAME, this );
    addOutputPin( m_outputPin );
}

DummyProcessor::~DummyProcessor()
{
    delete m_inputPin;
    delete m_outputPin;
}

void DummyProcessor::init() throw (PipelineException)
{
}

bool DummyProcessor::isReadyForProcessing() const
{
    if( m_inputPin->isConnected() )
    {
        return m_inputPin->getConnection()->hasData();
    }
    return false;
}

//bool DummyProcessor::isBootstrapped() const
//{
//    return true;
//}

void DummyProcessor::process()
{
    assert(m_inputPin != 0);
    assert(m_outputPin != 0);

    RefPtr<OpenCVImage> img = m_inputPin->get();
    RefPtr<OpenCVImage> img2 = OpenCVImageFactory::instance()->get(
            img->getWidth(), img->getHeight(), img->getDepth(), img->getNumChannels() );

    // open for reading
    const IplImage* iplImg1 = img->getImage();

    // open image for writing
    IplImage* iplImg2 = img2->getImageForWriting();

    // do a flip of the image
    cvFlip( iplImg1, iplImg2, (int)m_someBool);

    // publish the new image
    m_outputPin->put( img2.getPtr() );

    this->setSomeInt(this->getSomeInt()+1);
}
