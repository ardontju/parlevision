#include <QDebug>

#include "AddSub.h"
#include "Pin.h"
#include "OpenCVImage.h"
#include <opencv/cv.h>

using namespace plv;

#define INPUT_PIN_NAME1 "input 1"
#define INPUT_PIN_NAME2 "input 2"
#define OUTPUT_PIN_NAME "output"

AddSub::AddSub() :
        m_method(1),
        m_normalize ( false )
{
    m_inputPin1 = createInputPin<OpenCVImage>( INPUT_PIN_NAME1, this );
    m_inputPin2 = createInputPin<OpenCVImage>( INPUT_PIN_NAME2, this );
    m_outputPin = createOutputPin<OpenCVImage>( OUTPUT_PIN_NAME, this );
}

AddSub::~AddSub()
{
}

void AddSub::init() throw (PipelineException)
{
}

bool AddSub::isReadyForProcessing() const
{
    return (m_inputPin1->hasData() && m_inputPin2->hasData());
}

void AddSub::process()
{
    assert(m_inputPin1 != 0);
    assert(m_inputPin2 != 0);
    assert(m_outputPin != 0);
    assert(m_method >= 0);
    assert(m_method <= METHOD_MAX);

    RefPtr<OpenCVImage> img1 = m_inputPin1->get();
    RefPtr<OpenCVImage> img2 = m_inputPin2->get();
    // open input images for reading
    const IplImage* iplImgIn1 = img1->getImage();
    const IplImage* iplImgIn2 = img2->getImage();
    CvSize size1=cvGetSize(iplImgIn1);
    CvSize size2=cvGetSize(iplImgIn2);
    //check format of images?
    if(   img1->getDepth() != img2->getDepth()
       ||
          img1->getNumChannels() != img2->getNumChannels()
        ||
           size1.height != size2.height
        ||
           size1.width != size2.width
       )
    {
        //TODO: we could use some modifications when the images do not match -- e.g., copy one of the mismatching images into a duplicate that DOES match (stretch? shrink? add depth?)
        throw std::runtime_error("The two images need to be the same in depth, size and nr of channels");
    }

    //need this because I need to scale down input for adding, otherwise I get too many white areas
    RefPtr<OpenCVImage> imgTempIn1 = OpenCVImageFactory::instance()->get(
            img1->getWidth(), img1->getHeight(), img1->getDepth(), img1->getNumChannels() );
    RefPtr<OpenCVImage> imgTempIn2 = OpenCVImageFactory::instance()->get(
            img2->getWidth(), img2->getHeight(), img2->getDepth(), img2->getNumChannels() );

    // open temp images for writing
    IplImage* iplImgTempIn1 = imgTempIn1->getImageForWriting();
    IplImage* iplImgTempIn2 = imgTempIn2->getImageForWriting();

    //get a new output image of same depth and size as input image
    RefPtr<OpenCVImage> imgOut = OpenCVImageFactory::instance()->get(
            img1->getWidth(), img1->getHeight(), img1->getDepth(), img1->getNumChannels() );

    // open output image for writing
    IplImage* iplImgOut = imgOut->getImageForWriting();

    // do the add/sub/diff operation
    switch(m_method)
    {
        case METHOD_NOTHING:
            //NOOP we do nothing
            break;
        case METHOD_ADD:
            //scale down to prevent saturation during the add operation
            cvConvertScale(iplImgIn1,iplImgTempIn1, 0.5, 0);
            cvConvertScale(iplImgIn2,iplImgTempIn2, 0.5, 0);
            cvAdd(iplImgTempIn1,iplImgTempIn2,iplImgOut, NULL);
            //scale back up again
            if (!m_normalize)cvConvertScale(iplImgOut,iplImgOut, 2, 0);
            break;
        case METHOD_SUB:
            //subtract 2nd source image from 1st. Store in iplImgOut.
            cvSub(iplImgIn1,iplImgIn2,iplImgOut);
            break;

    }


    // publish the new image
    m_outputPin->put( imgOut.getPtr() );
}

void AddSub::setMethod(int i)
{
    if (i > METHOD_MAX || i < 0) i=m_method;
    m_method = i;
    emit(methodChanged(m_method));
}