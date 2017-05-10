//------------------------------------------------------------------------
//  Author: Paul Roberts 2016
//------------------------------------------------------------------------

#ifndef LUNAR_CONSTANTS_H
#define LUNAR_CONSTANTS_H

//Screen dimensions.
const int	 kLunarScreenWidth			 = 500;		//In pixels.
const int	 kLunarScreenHeight			 = 500;

const float  kThrust					 = 0.03f;

const float  kGravity					 = 0.001f;
const float  kLandingSpeed				 = 0.08f;//40.0f;

const int	 kNumberOfAILanders			 = 100;
const int	 kHalfNumberOfAILanders		 = 50;
const int	 kNumberOfChromosomeElements = 100;

const int	 kCrossoverRate				 = 5000; //This is in a range of ints up to 10000 making it 50%
const int	 kMutationRate				 = 500;	 //This is in a range of ints up to 10000 making it 5%

//const float	 kLandingBonus				 = 1000.0f;
//const double kDistWeight				 = 0.33;
//const double kRotWeight					 = 400.0;
//const double kAirTimeWeight				 = 4.0;

const float	 kLandingBonus				 = 100000.0f;
const double kDistWeight				 = 20;
const double kRotWeight					 = 40;
const double kAirTimeWeight				 = 1;

const double kThrustDuration			 = 0.1;

#endif //LUNAR_CONSTANTS_H