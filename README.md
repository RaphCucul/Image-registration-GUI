# Image registration GUI

The program is based on a console application written in C++ aimed on the registration of videoframes, using OpenCV libraries, which are used for image processing. The source code of the console app with additional informations (in Czech) can be found here: https://www.vutbr.cz/studenti/zav-prace/detail/110555

The program is designed to register videoframes with specific content - retinal fundus. The example of such videoframe is shown below.
![Retinal fundus frame example](https://github.com/RaphCucul/Image-registration-GUI/tree/master/example_images/videoFrameExample.png)
The program is using Frangi filter during preprocessing images. The code for Frangi filter can be found here: https://github.com/ntnu-bioopt/libfrangi.git

The program is being created using OpenCV 3.2.0.
