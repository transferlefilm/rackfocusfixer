#ifndef __EASE_IN_OUT_H
#define __EASE_IN_OUT_H

// code inspired from
// http://gizma.com/easing
// by Robert Penner
#include <stdint.h>

/*!
 * \brief   The EaseInOut class allows to transform an input percentage or
 *          progression value into an eased-in|out version of the progress
 *          Usage example:
 *              float filteredPercentage = EaseInOut::easeInOut(percentage, Quadratic);
 */
class EaseInOut
{
    enum Method
    {
        Linear, Squareroot, Tanh, Sinusoidal, Quadratic, // you'll most likely want to use one of these
        Cubic, Quartic, Quintic, Exponential, Circular// these are there for the sake of completeness
    };

public:
    /*!
     * \brief easeIn generates Ease-In values from an input percentage value
     * \param percentage input value in the interval [0, 1]
     * \param method ease-in method
     * \return returns a filtered percentage value according to the chosen ease-in method
     */
    static float easeIn(const float percentage, const int method=Quadratic)
    {
        const float t(percentage);
        switch(method)
        {
        default:
        case Linear: return t;
        case Squareroot: return 1 - sqrt(1-t);
        case Tanh: return 1 - tanh(t*4);
        case Sinusoidal: return  -cosf(t * (M_PI*0.5)) + 1;
        case Quadratic: return t*t;
        case Cubic: return t*t*t;
        case Quartic: return t*t*t*t;
        case Quintic: return t*t*t*t*t;
        case Exponential: return powf( 2, 10 * (t - 1) );
        case Circular: return 1 - sqrt(1 - t*t);
        }
    }

    /*!
     * \brief easeOut generates Ease-Out values from an input percentage value
     * \param percentage input value in the interval [0, 1]
     * \param method ease-out method
     * \return returns a filtered percentage value according to the chosen ease-out method
     */
    static float easeOut(const float percentage, const int method=Quadratic)
    {
        const float t(percentage);
        switch(method)
        {
        default:
        case Linear: return t;
        case Squareroot: return sqrt(t);
        case Tanh: return tanh(t*4);
        case Sinusoidal: return  sinf(t * M_PI*0.5);
        case Quadratic: return -t*(t-2);
        case Cubic: return (t-1)*(t-1)*(t-1) + 1;
        case Quartic: return -(t-1)*(t-1)*(t-1)*(t-1) + 1;
        case Quintic: return (t-1)*(t-1)*(t-1)*(t-1)*(t-1) + 1;
        case Exponential: return -powf( 2, -10 * t ) + 1;
        case Circular: return sqrt(1 - (t-1)*(t-1));
        }
    }

    /*!
     * \brief easeInOut generates Ease-In-Out values from an input percentage value
     * \param percentage input value in the interval [0, 1]
     * \param method ease-in-out method
     * \return returns a filtered percentage value according to the chosen ease-in-out method
     */
    static float easeInOut(const float percentage, const int method=Quadratic)
    {
        const float t(percentage);
        switch(method)
        {
        default:
        case Linear: return t;
        case Squareroot: return (t < 0.5) ? -0.5 * (sqrt((1-2*t)) - 1) : 0.5*(sqrt(2*t-1) + 1);
        case Tanh: return 0.5 * (tanh((t-0.5)*8) + 1);
        case Sinusoidal: return -0.5 * (cosf(M_PI*t) - 1);
        case Quadratic: return (t < 0.5) ? 2*t*t : -2*t*(t-2) - 1;
        case Cubic: return (t < 0.5) ? 4*t*t*t : 4*(t-1)*(t-1)*(t-1) + 1;
        case Quartic: return (t < 0.5) ? 8*t*t*t*t : -8*(t-1)*(t-1)*(t-1)*(t-1) + 1;
        case Quintic: return (t < 0.5) ? 16*t*t*t*t*t : 16*(t-1)*(t-1)*(t-1)*(t-1)*(t-1) + 1;
        case Exponential: return (t < 0.5) ? 0.5 * powf( 2, 10 * (2*t - 1) ) : 0.5 * ( -powf( 2, -10 * (2*t - 1)) + 2 );
        case Circular: return (t < 0.5) ? -0.5 * (sqrt(1 - 4*t*t) - 1) : 0.5 * (sqrt(1 - 4*(t-1)*(t-1)) + 1);
        }
    }
};

#endif // __EASE_IN_OUT_H
