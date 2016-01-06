#ifndef ATLAS_INCLUDE_ATLAS_SOLVERS_HPP
#define ATLAS_INCLUDE_ATLAS_SOLVERS_HPP

#pragma once

#include "Float.hpp"
#include "Math.hpp"
#include "Constants.hpp"
#include "Atlas.hpp"

#include <vector>

namespace atlas
{
    template <typename GenType = float>
    int solveQuadric(std::vector<GenType>& coeffs, 
        std::vector<GenType>& roots)
    {
        // Quadratic: x^2 + px + q = 0
        GenType p, q, D;
        p = coeffs[1] / (2 * coeffs[2]);
        q = coeffs[0] / coeffs[2];

        D = p * p - q;

        if (isZero<GenType>(D))
        {
            roots[0] = -p;
            return 1;
        }
        else if (D > GenType(0.0))
        {
            GenType sqrtD = sqrt(D);
            roots[0] = sqrtD - p;
            roots[1] = -sqrtD - p;
            return 2;
        }
        else
        {
            return 0;
        }
    }

    template <typename GenType = float>
    int solveCubic(std::vector<GenType>& coeffs, std::vector<GenType>& roots)
    {
        USING_GLM_NS;

        int num;
        GenType sub;
        GenType A, B, C;
        GenType sqA, p, q;
        GenType cbP, D;

        // Cubic: x^3 + Ax^2 + Bx + C = 0
        A = coeffs[2] / coeffs[3];
        B = coeffs[1] / coeffs[3];
        C = coeffs[0] / coeffs[3];

        // Substitute x = y - A/3 to eliminate quadric term:
        // x^3 + px + q = 0
        sqA = A * A;
        p = GenType(1.0) / GenType(3.0) * (-GenType(1.0) /
            GenType(3.0) * sqA + B);
        q = GenType(1.0) / GenType(3.0) * (GenType(2.0) /
            GenType(27) * A * sqA -
            GenType(1.0) / GenType(3.0) * A * B + C);

        // Use Cardano's formula.
        cbP = p * p * p;
        D = q * q + cbP;

        if (isZero<GenType>(D))
        {
            if (isZero<GenType>(q))
            {
                // Multiplicity 3.
                roots[0] = 0;
                num = 1;
            }
            else
            {
                // Multiplicity 2 and 1.
                GenType u = cbrt(-q);
                roots[0] = 2 * u;
                roots[1] = -u;
                num = 2;
            }
        }
        else if (D < 0)
        {
            // Multiplicity 1 all.
            GenType phi = GenType(1.0) / GenType(3.0) *
                acos(-q / sqrt(-cbP));
            GenType t = 2 * sqrt(-p);

            roots[0] = t * cos(phi);
            roots[1] = -t * cos(phi + pi<GenType>() /
                GenType(3));
            roots[2] = -t * cos(phi - pi<GenType>() /
                GenType(3));
            num = 3;
        }
        else
        {
            // One real solution.
            GenType sqrtD = sqrt(D);
            GenType u = cbrt(sqrtD - q);
            GenType v = -cbrt(sqrtD + q);

            roots[0] = u + v;
            num = 1;
        }

        // Resubstitute.
        sub = GenType(1.0) / GenType(3) * A;

        for (int i = 0; i < num; ++i)
        {
            roots[i] -= sub;
        }

        return num;
    }

    template <typename GenType = float>
    int solveQuartic(std::vector<GenType>& coeffs,
        std::vector<GenType>& roots)
    {
        std::vector<GenType> c(4);
        GenType z, u, v, sub;
        GenType A, B, C, D;
        GenType sqA, p, q, r;
        int num;

        // Quartic: x^4 + Ax^3 + Bx^2 + Cx + D = 0.
        A = coeffs[3] / coeffs[4];
        B = coeffs[2] / coeffs[4];
        C = coeffs[1] / coeffs[4];
        D = coeffs[0] / coeffs[4];

        // Substitute x = y - A / 4 to eliminate cubic term:
        // x^4 + px^2 + qx + r = 0.

        sqA = A * A;
        p = GenType(3.0) / GenType(8) * sqA + B;
        q = GenType(1.0) / GenType(8) * sqA * A - GenType(1.0) /
            GenType(2) * A * B + C;
        r = -GenType(3.0) / GenType(256) * sqA * sqA + GenType(1.0) /
            GenType(16) * sqA * B - GenType(1.0) / GenType(4) * A * C + D;

        if (isZero<GenType>(r))
        {
            // No absolute term: y(y^3 + py + q) = 0.
            c[0] = p;
            c[1] = 1;
            c[2] = 0;
            c[3] = 1;

            num = solveCubic<GenType>(c, roots);
            roots[num++] = 0;
        }
        else
        {
            // Solve the resolvent cubic...
            c[0] = GenType(1.0) / GenType(2) * r * p - GenType(1.0) /
                GenType(8) * q * q;
            c[1] = -r;
            c[2] = -GenType(1.0) / GenType(2) * p;
            c[3] = 1;

            solveCubic<GenType>(c, roots);

            // And take the one real solution
            z = roots[0];

            // To build two quadratics.
            u = z * z - r;
            v = 2 * z - p;

            if (isZero<GenType>(u))
            {
                u = 0;
            }
            else if (u > 0)
            {
                u = sqrt(u);
            }
            else
            {
                return 0;
            }

            if (isZero<GenType>(v))
            {
                v = 0;
            }
            else if (v > 0)
            {
                v = sqrt(v);
            }
            else
            {
                return 0;
            }

            c[0] = z - u;
            c[1] = (q < 0) ? -v : v;
            c[2] = 1;

            num = solveQuadric<GenType>(c, roots);

            c[0] = z + u;
            c[1] = (q < 0) ? v : -v;
            c[2] = 1;


            std::vector<GenType> s(2);
            int tmp = num;
            num += solveQuadric<GenType>(c, s);
            roots[tmp] = s[0];
            roots[tmp + 1] = s[1];
        }

        // Resubstitute.
        sub = GenType(1.0) / GenType(4) * A;

        for (int i = 0; i < num; ++i)
        {
            roots[i] -= sub;
        }

        return num;

    }
}

#endif