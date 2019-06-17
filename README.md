# Videoframes registration

The program is based on a console application written in C++ aimed on the registration of videoframes, using OpenCV libraries, which are used for image processing. The source code of the console app with additional informations (in Czech) can be found here: https://www.vutbr.cz/studenti/zav-prace/detail/110555

The program is designed to register videoframes with specific content - retinal fundus. The example of such videoframe is shown below.
![TEST](/example_images/videoFrameExample.png)

The program is using Frangi filter during videoframe preprocessing. The code for Frangi filter can be found here: https://github.com/ntnu-bioopt/libfrangi.git

The program is being created using OpenCV 3.2.0. To run the program properly, it is necessary to modify phase correlation functions in the OpenCV source before compiling the libraries. Folder [OpenCV_phaseCorr](https://github.com/RaphCucul/Image-registration-GUI/tree/master/OpenCV_phaseCorr) contains the files with modified content. If newer version of OpenCV is used, it is necessary to implement changes shown in those files.

## Sources of additional content
It was already mentioned in the text above, that this program uses third party Frangi filter C++ implementation.

The program contains icons. Some of them can be found here:
* https://icons8.com/iconizer/en/search/1/collection:Elegant_Themes_Icons
* https://icons8.com/iconizer/en/search/1/collection:Fugue_Icons
* https://icons8.com/iconizer/en/search/1/collection:Free_Applications_Icons

Some of them were created in online tool Vectr - 
https://vectr.com The rest was created in Gimp.
