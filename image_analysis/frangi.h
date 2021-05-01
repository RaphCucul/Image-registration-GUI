#include <opencv2/opencv.hpp>

/**
 * @file frangi.h
 * Functions calculating frangi parameters.
 * For more info about the origin of these functions, please visit the repository of this program.
 */

/**
 * @struct frangi2d_opts_t
 * @brief options for the filter
 */
typedef struct{
	//vessel scales
    int sigma_start;
    int sigma_end;
	int sigma_step;

	//BetaOne: suppression of blob-like structures.
	//BetaTwo: background suppression. (See Frangi1998...)
    double BetaOne;
    double BetaTwo;

	bool BlackWhite; //enhance black structures if true, otherwise enhance white structures
} frangi2d_opts_t;

#define DEFAULT_SIGMA_START 1
#define DEFAULT_SIGMA_END 10
#define DEFAULT_SIGMA_STEP 1
#define DEFAULT_BETA_ONE 8.0 //1
#define DEFAULT_BETA_TWO 8.0 //10
#define DEFAULT_BLACKWHITE true


/////////////////
//Frangi filter//
/////////////////

/**
 * @brief Applies full Frangi filter to src. Vesselness is saved in J, scale is saved to scale, vessel angle is saved to
 * directions.
 * @param[in] src - input image
 * @param[out] J - frame with output values
 * @param[] scale - frame with scale information
 * @param[] directions - frame with calculated directories
 * @param[] opts - defined frangi parameters
 */
void frangi2d(const cv::Mat &src, cv::Mat &J, cv::Mat &scale, cv::Mat &directions, frangi2d_opts_t opts);



////////////////////
//Helper functions//
////////////////////

/**
 * @brief Runs 2D Hessian filter with parameter sigma on src, save to Dxx, Dxy and Dyy.
 * @param src
 * @param Dxx
 * @param Dxy
 * @param Dyy
 * @param sigma
 */
void frangi2d_hessian(const cv::Mat &src, cv::Mat &Dxx, cv::Mat &Dxy, cv::Mat &Dyy, float sigma);

/**
 * @brief Sets opts variable to default options (sigma_start = 1, sigma_end = 10, sigma_step = 1, BetaOne = 8.0, BetaTwo 8.0)
 * @param opts
 */
void frangi2d_createopts(frangi2d_opts_t *opts);

/**
 * @brief Estimates eigenvalues from Dxx, Dxy, Dyy. Save results to lambda1, lambda2, Ix, Iy.
 * @param Dxx
 * @param Dxy
 * @param Dyy
 * @param lambda1
 * @param lambda2
 * @param Ix
 * @param Iy
 */
void frangi2_eig2image(const cv::Mat &Dxx, const cv::Mat &Dxy, const cv::Mat &Dyy,
                       cv::Mat &lambda1, cv::Mat &lambda2, cv::Mat &Ix, cv::Mat &Iy);

