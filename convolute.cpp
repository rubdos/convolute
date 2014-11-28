#include <iostream>
#include <functional>
#include <cmath>
#include "gnuplot-iostream.h"

double integrate(double lower,
        double upper,
        double resolution,
        std::function<double (double t)> &f)
{
    double sum=0;
    double t = lower;
    while (t < upper)
    {
        sum +=resolution*f(t);
        t+=resolution;
    }
    return sum;
}

double convolute(
        double mininf,
        double plusinf,
        double resolution,
        double point,
        std::function<double (double t)> &f,
        std::function<double (double t)> &g
        )
{
    // returns int -inf -> +inf of f(u)g(t-u) du
    std::function<double (double t)> integrand = [&f,&g,&point](double u){
        return f(u)*g(point-u);
    };
    return integrate(mininf, plusinf, resolution, integrand);
}

int main()
{
    Gnuplot gp;
    std::vector<std::pair<double, double>> convolution;
    std::function<double (double t)> f = [](double t){
        if(t > 0)
            return exp(-t);
        else
            return 0.0;
    };
    std::function<double (double t)> g = [](double t){
        if(t > -.5 and t < .5)
            return 1.0;
        else
            return 0.0;
    };
    f=g;
    double min = -10;
    double plus = 10;
    double mininf = -1000;
    double plusinf = 1000;
    double integralresolution    = 0.005;
    double convolutionresolution = 0.05;
    gp << "set xrange [" << min << ":" << plus << "]\n";

    double t = min;
    std::cout << "Aantal convolutieproducten: " 
        << (plus - min)/convolutionresolution
        << std::endl
        << "Aantal integratiestappen: "
        << (plusinf - mininf)/integralresolution
        << std::endl;
    std::cout << "t\tc" << std::endl;
    double convolutionp;

    std::vector<std::pair<double, double>> fplot;
    std::vector<std::pair<double, double>> gplot;

    std::function<
        std::vector<
            std::pair<double, double>> (
                std::function<double (double)> f,
                double min,
                double max,
                double resolution
            )
        > 
        plot = [](
                std::function<double (double)> f,
                double min,
                double max,
                double resolution
                )
    {
        std::vector<std::pair<double, double>> r;
        double x = min;
        while(x<max)
        {
            r.push_back(std::make_pair(x, f(x)));
            x+=resolution;
        }
        return r;
    };

    fplot = plot(f, min, plus, convolutionresolution);
    
    while(t<plus)
    {
        gplot = plot([&](double x){return g(t-x);}, 
                min, 
                plus, 
                convolutionresolution
                );
        convolutionp = convolute(
            mininf,
            plusinf,
            integralresolution,
            t,
            f,
            g
        );
        convolution.push_back(std::make_pair(t, convolutionp));
        //std::cout << t << "\t" << convolution << std::endl;
        t+=convolutionresolution;

        // Plot convolution
        gp << "plot "
            << gp.file1d(convolution) << "with lines,";
        gp << gp.file1d(fplot) << "with lines,";
        gp << gp.file1d(gplot) << "with lines,";
        gp << "\n"; // Draw
    }
}
